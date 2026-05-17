"""
LayoutModule: port of CSpaceLayoutManager's energy function and optimizer.

The C++ side runs Powell / Conjugate Gradient over the same pairwise
energy. Here we use torch.optim.LBFGS, which is the closest equivalent in
PyTorch (also a quasi-Newton, also expects a closure). The energy itself
is vectorized using broadcasted pairwise tensors — this part is *not* a
literal transcription; it produces the same value but exploits torch
parallelism.

Per-pair link weight matches LoadSizesLinks:
    avg_act[i,j] = a_i + a_j
    weight[i,j]  = (a_i / avg_act * gain_i_to_j + a_j / avg_act * gain_j_to_i
                    + LINK_WEIGHT_EPS) * avg_act
"""

from __future__ import annotations

from typing import Optional, Tuple

import torch
from torch import nn

from ._constants import (
    ENERGY_DIST_EPS,
    ENERGY_REP_EPS,
    K_POS,
    K_REP,
    LINK_WEIGHT_EPS,
    OPT_DIST,
    SIZE_FLOOR,
    SIZE_SCALE,
)
from .space import SpaceTensors


def _pairwise_gain_matrix(space: SpaceTensors) -> torch.Tensor:
    """Build a dense (N, N) gain matrix from the directed edge list.

    `gain[i, j]` is the CNodeLink::GetGain() from i to j, or 0.0 if no edge.
    """
    n = space.n
    g = torch.zeros(n, n, dtype=space.edge_weight.dtype)
    if space.e == 0:
        return g
    src = space.edge_index[0]
    dst = space.edge_index[1]
    g[src, dst] = space.edge_gain
    return g


def pairwise_link_weights(
    activations: torch.Tensor, gain: torch.Tensor
) -> Tuple[torch.Tensor, torch.Tensor]:
    """Compute (avg_act, link_weight) over all (i, j) pairs.

    Matches LoadSizesLinks. Returns dense (N, N) tensors that the caller
    typically masks to the upper triangle.
    """
    n = activations.shape[0]
    a_i = activations.view(n, 1).expand(n, n)
    a_j = activations.view(1, n).expand(n, n)
    avg = a_i + a_j
    # Avoid div-by-zero for inactive pairs. We mask out i==j and pairs with
    # avg==0 downstream, so the value here only matters for finiteness.
    safe_avg = torch.where(avg > 0, avg, torch.ones_like(avg))
    weight = (
        a_i / safe_avg * gain + a_j / safe_avg * gain.t() + LINK_WEIGHT_EPS
    ) * avg
    weight = torch.where(avg > 0, weight, torch.zeros_like(weight))
    return avg, weight


class LayoutModule(nn.Module):
    """Force-directed layout energy as an nn.Module.

    positions is exposed as an nn.Parameter so torch.optim works directly.
    The energy mirrors CSpaceLayoutManager::operator() — attraction is
    K_POS * weight * (act_dist - OPT_DIST)^2 and repulsion is
    K_REP * avg_act / (x_ratio + y_ratio + ENERGY_REP_EPS).
    """

    def __init__(
        self,
        space: SpaceTensors,
        k_pos: float = K_POS,
        k_rep: float = K_REP,
        size_scale: float = SIZE_SCALE,
        size_floor: float = SIZE_FLOOR,
        opt_dist: float = OPT_DIST,
        dist_eps: float = ENERGY_DIST_EPS,
        rep_eps: float = ENERGY_REP_EPS,
    ) -> None:
        super().__init__()
        self.space = space
        self.k_pos = k_pos
        self.k_rep = k_rep
        self.size_scale = size_scale
        self.size_floor = size_floor
        self.opt_dist = opt_dist
        self.dist_eps = dist_eps
        self.rep_eps = rep_eps

        # Learnable parameter: 2D positions per node.
        self.positions = nn.Parameter(space.positions.clone())

        # Precomputed / fixed-per-call buffers. These are derived from the
        # current activation snapshot — the C++ recomputes them inside
        # LoadSizesLinks before each call. If you call layout multiple
        # times across activation changes, rebuild via refresh_from_space().
        self.register_buffer("_sizes", torch.empty(0))
        self.register_buffer("_avg_act", torch.empty(0))
        self.register_buffer("_link_weight", torch.empty(0))
        self.refresh_from_space()

    # ------------------------------------------------------------------ API

    def refresh_from_space(self) -> None:
        """Recompute the per-pair size/activation/weight matrices.

        Mirrors LoadSizesLinks. Call this whenever activations change
        between layout runs.
        """
        sp = self.space
        sizes = self.size_scale * sp.radii + self.size_floor
        gain = _pairwise_gain_matrix(sp)
        avg_act, link_weight = pairwise_link_weights(sp.activation(), gain)

        # Mask to upper triangle (i < j) — the C++ iterates each pair once
        # in its outer/inner loops.
        n = sp.n
        triu = torch.triu(torch.ones(n, n, dtype=torch.bool), diagonal=1)
        avg_act = torch.where(triu, avg_act, torch.zeros_like(avg_act))
        link_weight = torch.where(triu, link_weight, torch.zeros_like(link_weight))

        self._sizes = sizes
        self._avg_act = avg_act
        self._link_weight = link_weight

    def forward(self) -> torch.Tensor:
        """Return the scalar layout energy at the current positions."""
        pos = self.positions
        n = pos.shape[0]

        # Pairwise displacements (i, j) for i, j in [0, N).
        x = pos[:, 0].view(n, 1) - pos[:, 0].view(1, n)
        y = pos[:, 1].view(n, 1) - pos[:, 1].view(1, n)

        # Note: the C++ uses the average of the two node sizes squared
        # *as a single number* in both x and y denominators (the same ss
        # is reused as ssx_sq and ssy_sq) — see SpaceLayoutManager.cpp.
        # We mirror that here.
        s_i = self._sizes.view(n, 1)
        s_j = self._sizes.view(1, n)
        ss = (s_i + s_j) * 0.5
        ss_sq = ss * ss
        # Guard against ss_sq == 0 (shouldn't happen with SIZE_FLOOR>0,
        # but be safe under autograd).
        ss_sq = torch.where(ss_sq > 0, ss_sq, torch.ones_like(ss_sq))

        x_ratio = x * x / ss_sq
        y_ratio = y * y / ss_sq
        rsq = x_ratio + y_ratio

        act_dist = torch.sqrt(rsq + self.dist_eps)
        dist_error = act_dist - self.opt_dist

        # Attraction: factor = K_POS * link_weight ; energy += factor * dist_error^2
        attr = self.k_pos * self._link_weight * dist_error * dist_error

        # Repulsion: factor_rep = K_REP * avg_act ; energy += factor_rep / (rsq + 3.0)
        rep = self.k_rep * self._avg_act / (rsq + self.rep_eps)

        # Sum upper triangle — _link_weight and _avg_act are already
        # zeroed below the diagonal in refresh_from_space().
        return attr.sum() + rep.sum()

    def optimize(
        self,
        n_steps: int = 100,
        lr: float = 1.0,
        optimizer: str = "adam",
    ) -> float:
        """Run `n_steps` optimization steps; return the final energy.

        `optimizer` selects the algorithm:
          - "adam":  torch.optim.Adam (default; robust on this non-convex
                     pairwise energy, similar role to a damped gradient
                     descent in spirit).
          - "lbfgs": torch.optim.LBFGS with strong_wolfe line search. Closer
                     in spirit to the C++ Powell / Conjugate Gradient path,
                     but more sensitive to initial conditions on this
                     landscape — pick lr carefully and/or wrap in a loop.
        """
        if optimizer == "adam":
            opt = torch.optim.Adam([self.positions], lr=lr)
            e = self.forward()
            for _ in range(n_steps):
                opt.zero_grad()
                e = self.forward()
                e.backward()
                opt.step()
            return float(e.item())
        elif optimizer == "lbfgs":
            opt = torch.optim.LBFGS(
                [self.positions],
                lr=lr,
                max_iter=n_steps,
                line_search_fn="strong_wolfe",
            )

            def closure() -> torch.Tensor:
                opt.zero_grad()
                e = self.forward()
                e.backward()
                return e

            e_final = opt.step(closure)
            return float(e_final.item())
        else:
            raise ValueError(f"unknown optimizer: {optimizer!r}")

    def write_positions_to_space(self) -> None:
        """Copy current positions back into the SpaceTensors snapshot."""
        with torch.no_grad():
            self.space.positions = self.positions.detach().clone()

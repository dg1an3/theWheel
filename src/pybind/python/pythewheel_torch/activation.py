"""
ActivationModule: bit-for-bit port of CSpace::ActivateNode.

The C++ algorithm is a DFS through the link graph with a per-link
HasPropagated guard and depth-attenuated init_scale. Because the read of
each source/target activation comes from the stable pre-cycle primary +
secondary values (CNode mutates only m_newSecondaryActivation during the
cycle), the DFS is order-sensitive only in *which* links it visits — not
in the activation deltas computed per visit. We mirror the recursion
literally with Python; the tensor state lives in SpaceTensors.

Vectorization is intentionally not done here: this module's job is to be
a faithful oracle / differentiable transcription of the C++ logic. A
vectorized cousin would live alongside it.
"""

from __future__ import annotations

from typing import Optional

import torch
from torch import nn

from ._constants import (
    DEFAULT_NORMALIZE_SUM,
    PRIM_FRAC,
    PRIM_NORM_SCALE,
    PROPAGATE_ALPHA,
    PROPAGATE_SCALE,
    PROPAGATE_THRESHOLD_WEIGHT,
    SEC_NORM_SCALE,
    STABILIZER_ATTENUATION,
    TOTAL_ACTIVATION,
    TOTAL_ACTIVATION_FRACTION,
)
from .space import SpaceTensors


class ActivationModule(nn.Module):
    """One full CSpace::ActivateNode cycle as an nn.Module.

    Stateless w.r.t. learnable parameters in this sketch — primary/secondary
    activations live on the supplied SpaceTensors and are mutated in place.
    A natural extension is to expose a learnable scalar gain on PROPAGATE_SCALE
    or per-edge multipliers; both fit cleanly under torch.autograd.
    """

    def __init__(
        self,
        prim_frac: float = PRIM_FRAC,
        propagate_scale: float = PROPAGATE_SCALE,
        propagate_alpha: float = PROPAGATE_ALPHA,
        propagate_threshold_weight: float = PROPAGATE_THRESHOLD_WEIGHT,
        total_activation_cap: float = TOTAL_ACTIVATION * TOTAL_ACTIVATION_FRACTION,
        prim_norm_scale: float = PRIM_NORM_SCALE,
        sec_norm_scale: float = SEC_NORM_SCALE,
        stabilizer_attenuation: float = STABILIZER_ATTENUATION,
    ) -> None:
        super().__init__()
        self.prim_frac = prim_frac
        self.propagate_scale = propagate_scale
        self.propagate_alpha = propagate_alpha
        self.propagate_threshold_weight = propagate_threshold_weight
        self.total_activation_cap = total_activation_cap
        self.prim_norm_scale = prim_norm_scale
        self.sec_norm_scale = sec_norm_scale
        self.stabilizer_attenuation = stabilizer_attenuation

    # ------------------------------------------------------------------ API

    def forward(
        self,
        space: SpaceTensors,
        node_index: int,
        scale: float,
        normalize_sum: float = DEFAULT_NORMALIZE_SUM,
    ) -> SpaceTensors:
        """Apply ActivateNode + NormalizeNodes to `space` in place.

        Returns the same SpaceTensors for chaining.
        """
        self._set_activation_explicit(space, node_index, self._cap(space, node_index, scale))
        new_secondary = space.secondary_activation.clone()
        self._propagate(
            space=space,
            new_secondary=new_secondary,
            from_idx=node_index,
            delta_activation=0.0,
            init_scale=self.propagate_scale,
            alpha=self.propagate_alpha,
            propagated=torch.zeros(space.e, dtype=torch.bool),
        )
        space.secondary_activation = new_secondary
        self._normalize(space, normalize_sum)
        return space

    # --------------------------------------------------------- ActivateNode

    def _cap(self, space: SpaceTensors, node_index: int, scale: float) -> float:
        """Mirrors the activation-cap line in CSpace::ActivateNode."""
        old = float(space.activation()[node_index].item())
        return old + scale * (self.total_activation_cap - old)

    def _set_activation_explicit(
        self, space: SpaceTensors, node_index: int, new_activation: float
    ) -> None:
        """Mirrors CNode::SetActivation with pActivator == NULL."""
        old = float(space.activation()[node_index].item())
        delta = new_activation - old
        space.primary_activation[node_index] += self.prim_frac * delta
        space.secondary_activation[node_index] += (1.0 - self.prim_frac) * delta

    # ----------------------------------------------------- PropagateActivation

    def _propagate(
        self,
        space: SpaceTensors,
        new_secondary: torch.Tensor,
        from_idx: int,
        delta_activation: float,
        init_scale: float,
        alpha: float,
        propagated: torch.Tensor,
    ) -> None:
        """Literal DFS port of CNode::PropagateActivation.

        `new_secondary` is the in-progress accumulator (matches m_newSecondaryActivation).
        `propagated` is the per-edge HasPropagated flag, reset per ActivateNode cycle.
        """
        # Accumulate into the destination's new_secondary if positive.
        if delta_activation > 0.0:
            new_secondary[from_idx] = new_secondary[from_idx] + delta_activation

        # Recurse through each outgoing link of from_idx in C++ order.
        for edge in space.links_from(from_idx):
            self._propagate_link(
                space=space,
                new_secondary=new_secondary,
                edge_idx=int(edge.item()),
                source_idx=from_idx,
                init_scale=init_scale,
                alpha=alpha,
                propagated=propagated,
            )

    def _propagate_link(
        self,
        space: SpaceTensors,
        new_secondary: torch.Tensor,
        edge_idx: int,
        source_idx: int,
        init_scale: float,
        alpha: float,
        propagated: torch.Tensor,
    ) -> None:
        """Literal DFS port of CNodeLink::PropagateActivation."""
        if propagated[edge_idx]:
            return
        weight = float(space.edge_weight[edge_idx].item())
        if weight <= self.propagate_threshold_weight:
            return
        propagated[edge_idx] = True

        target_idx = int(space.edge_index[1, edge_idx].item())
        # Source / target reads use the pre-cycle activation (primary + secondary),
        # NOT the in-progress new_secondary. This is critical: the C++ never
        # reads m_newSecondaryActivation during propagation.
        source_act = float(space.activation()[source_idx].item())
        target_activation = source_act * weight

        is_stabilizer = bool(space.edge_is_stabilizer[edge_idx].item())
        if is_stabilizer:
            target_activation *= self.stabilizer_attenuation

        target_act = float(space.activation()[target_idx].item())
        delta = (target_activation - target_act) * init_scale * weight

        self._propagate(
            space=space,
            new_secondary=new_secondary,
            from_idx=target_idx,
            delta_activation=delta,
            init_scale=init_scale * alpha,
            alpha=alpha,
            propagated=propagated,
        )

    # --------------------------------------------------------- NormalizeNodes

    def _normalize(self, space: SpaceTensors, sum_target: float) -> None:
        """Mirrors CSpace::NormalizeNodes(sum)."""
        total = float(space.activation().sum().item())
        if total <= 0.0:
            return
        diff_frac = sum_target / total - 1.0
        if diff_frac > 0.0:
            diff_frac = 0.0
        space.primary_activation = space.primary_activation + (
            space.primary_activation * diff_frac * self.prim_norm_scale
        )
        space.secondary_activation = space.secondary_activation + (
            space.secondary_activation * diff_frac * self.sec_norm_scale
        )

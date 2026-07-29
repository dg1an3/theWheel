"""
SpaceTensors: dataclass holding the per-node and per-link state needed by
ActivationModule and LayoutModule.

All tensors are 1-D over nodes or 2-D (E, 2) over directed edges. Activations
use float32 to match the C++ REAL alias on this codebase.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Optional

import torch


@dataclass
class SpaceTensors:
    """Tensor view of a CSpace, suitable for PyTorch modules.

    Attributes mirror the C++ representation:
        primary_activation:    (N,) float, mirrors CNode::m_primaryActivation
        secondary_activation:  (N,) float, mirrors CNode::m_secondaryActivation
        positions:             (N, 2) float, world-space xy
        radii:                 (N,) float, CNode::GetRadius()
        edge_index:            (2, E) long, [source, target] per directed edge.
                               Order within each source matches the C++ link
                               order, which matters for the DFS PropagateActivation
                               since the per-link HasPropagated flag short-circuits.
        edge_weight:           (E,) float, CNodeLink::GetWeight()
        edge_gain:             (E,) float, CNodeLink::GetGain()
        edge_is_stabilizer:    (E,) bool
    """

    primary_activation: torch.Tensor
    secondary_activation: torch.Tensor
    positions: torch.Tensor
    radii: torch.Tensor
    edge_index: torch.Tensor
    edge_weight: torch.Tensor
    edge_gain: torch.Tensor
    edge_is_stabilizer: torch.Tensor

    # Cached per-source [start, end) slices into edge_index for fast lookup.
    _edge_offsets: Optional[torch.Tensor] = field(default=None, repr=False)

    @property
    def n(self) -> int:
        return int(self.primary_activation.shape[0])

    @property
    def e(self) -> int:
        return int(self.edge_index.shape[1])

    def activation(self) -> torch.Tensor:
        """Total activation per node (matches CNode::GetActivation())."""
        return self.primary_activation + self.secondary_activation

    def links_from(self, source: int) -> torch.Tensor:
        """Edge indices whose source is `source`, in their original C++ order.

        Returned indices are into edge_weight / edge_is_stabilizer / etc.
        """
        if self._edge_offsets is None:
            self._build_edge_offsets()
        start = int(self._edge_offsets[source].item())
        end = int(self._edge_offsets[source + 1].item())
        return torch.arange(start, end, dtype=torch.long)

    def _build_edge_offsets(self) -> None:
        # Compute a CSR-style offsets vector for the directed adjacency,
        # assuming edge_index is grouped by source. (build_from_pythewheel
        # produces edges in that order; if a caller hands in unsorted edges,
        # this will misbehave — the docstring says so.)
        n = self.n
        offsets = torch.zeros(n + 1, dtype=torch.long)
        if self.e > 0:
            src = self.edge_index[0]
            for i in range(self.e):
                s = int(src[i].item())
                offsets[s + 1] += 1
        self._edge_offsets = torch.cumsum(offsets, dim=0)

    # ------------------------------------------------------------------ I/O

    @staticmethod
    def from_pythewheel(cspace) -> "SpaceTensors":
        """Build a SpaceTensors snapshot from a pythewheel.Space.

        The C++ Space owns its CNode/CNodeLink objects; this just reads their
        current state into tensors. Pure copy — no live binding back.
        """
        n = cspace.get_node_count()
        prim = torch.zeros(n, dtype=torch.float32)
        sec = torch.zeros(n, dtype=torch.float32)
        pos = torch.zeros(n, 2, dtype=torch.float32)
        rad = torch.zeros(n, dtype=torch.float32)

        # Map CNode object identity to index so we can resolve link targets.
        node_to_idx = {}
        nodes = []
        for i in range(n):
            node = cspace.get_node_at(i)
            nodes.append(node)
            # Use id() since the C++ NodeLink.get_target returns the same
            # Python wrapper object across calls (return_value_policy::reference).
            node_to_idx[id(node)] = i
            prim[i] = node.get_primary_activation()
            sec[i] = node.get_secondary_activation()
            p = node.get_position()
            pos[i, 0] = p[0]
            pos[i, 1] = p[1]
            rad[i] = node.get_radius()

        src_list, dst_list, w_list, g_list, stab_list = [], [], [], [], []
        for i, node in enumerate(nodes):
            for k in range(node.get_link_count()):
                link = node.get_link_at(k)
                target = link.get_target()
                tid = id(target)
                if tid not in node_to_idx:
                    # Link target is outside the space's node array (the
                    # hidden root or an external node). Skip — the C++
                    # SortNodes pipeline normally keeps everything in the
                    # array, but be defensive.
                    continue
                src_list.append(i)
                dst_list.append(node_to_idx[tid])
                w_list.append(link.get_weight())
                g_list.append(link.get_gain())
                stab_list.append(bool(link.is_stabilizer()))

        if src_list:
            edge_index = torch.tensor([src_list, dst_list], dtype=torch.long)
            edge_weight = torch.tensor(w_list, dtype=torch.float32)
            edge_gain = torch.tensor(g_list, dtype=torch.float32)
            edge_stab = torch.tensor(stab_list, dtype=torch.bool)
        else:
            edge_index = torch.zeros(2, 0, dtype=torch.long)
            edge_weight = torch.zeros(0, dtype=torch.float32)
            edge_gain = torch.zeros(0, dtype=torch.float32)
            edge_stab = torch.zeros(0, dtype=torch.bool)

        return SpaceTensors(
            primary_activation=prim,
            secondary_activation=sec,
            positions=pos,
            radii=rad,
            edge_index=edge_index,
            edge_weight=edge_weight,
            edge_gain=edge_gain,
            edge_is_stabilizer=edge_stab,
        )

    def clone(self) -> "SpaceTensors":
        return SpaceTensors(
            primary_activation=self.primary_activation.clone(),
            secondary_activation=self.secondary_activation.clone(),
            positions=self.positions.clone(),
            radii=self.radii.clone(),
            edge_index=self.edge_index.clone(),
            edge_weight=self.edge_weight.clone(),
            edge_gain=self.edge_gain.clone(),
            edge_is_stabilizer=self.edge_is_stabilizer.clone(),
        )

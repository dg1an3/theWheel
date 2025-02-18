""" NodeLink.py - Represents a weighted connection between nodes in the network. """

from typing import Optional
from pyweel.node import Node


class NodeLink:
    """
    Represents a weighted connection between nodes in the network.
    """

    PROPAGATE_THRESHOLD_WEIGHT = 0.01

    def __init__(self, to_node: "Node", weight: float):
        self.target = to_node
        self.weight = weight
        self.gain = 1.0
        self.is_stabilizer = False
        self.has_propagated = True  # Initialize to prevent immediate propagation

    @property
    def gain_weight(self) -> float:
        """Gets the gain weight = weight * gain"""
        return 0.99 * self.gain * self.weight + 0.01 * self.weight

    def propagate_activation(
        self, from_node: "Node", init_scale: float, alpha: float
    ) -> None:
        """Propagates activation through the link if conditions are met."""
        if not self.has_propagated and self.weight > self.PROPAGATE_THRESHOLD_WEIGHT:
            # Set flag to prevent re-propagation
            self.has_propagated = True

            # Compute desired new activation = this activation * weight
            target_activation = from_node.activation * self.weight

            # Attenuate if the link is a stabilizer
            from_node_to_use:Optional["Node"] = from_node
            if self.is_stabilizer:
                target_activation *= 0.25
                from_node_to_use = None  # Prevent propagating node identity

            # Compute new actual activation
            delta_activation = (
                (target_activation - self.target.activation) * init_scale * self.weight
            )

            # Propagate to the target
            self.target.propagate_activation(
                from_node_to_use, delta_activation, init_scale * alpha, alpha
            )

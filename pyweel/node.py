""" """

from typing import List, Dict, Optional
import math
import numpy as np

from pyweel.node_link import NodeLink
from pyweel.space import Space


class Node:
    """
    Represents a node in the network with activation values and links to other nodes.
    """

    PRIM_FRAC = 0.5

    def __init__(self, space: "Space", name: str, description: str = ""):
        self.space = space
        self.parent: Optional["Node"] = None
        self.name = name
        self.description = description
        self.image_filename: str = ""
        self.node_class: str = ""

        # Activation values
        self.primary_activation = 0.005
        self.secondary_activation = 0.005
        self.new_secondary_activation = 0.005

        # Position
        self.position = np.zeros(3)
        self.position_reset = False
        self.reset_position_valid = False
        self.reset_position = np.zeros(3)

        # Activation tracking
        self.max_activator: Optional["Node"] = None
        self.max_delta_activation = 0.0

        # State flags
        self.is_sub_threshold = True
        self.post_super_count = 0

        # Collections
        self.children: List[Node] = []
        self.links: List[NodeLink] = []
        self.links_map: Dict["Node", NodeLink] = {}

    @property
    def activation(self) -> float:
        """Returns the node's total activation."""
        return self.primary_activation + self.secondary_activation

    def set_activation(
        self,
        new_activation: float,
        activator: Optional["Node"] = None,
        weight: float = 0.0,
    ) -> None:
        """Sets the node's activation with optional activator tracking."""
        delta_activation = new_activation - self.activation

        if activator is None:
            self.primary_activation += self.PRIM_FRAC * delta_activation
            self.secondary_activation += (1.0 - self.PRIM_FRAC) * delta_activation

            if self.space:
                self.space.total_primary_activation += self.PRIM_FRAC * delta_activation
                self.space.total_secondary_activation += (
                    1.0 - self.PRIM_FRAC
                ) * delta_activation
        else:
            self.secondary_activation += delta_activation

            if self.space:
                self.space.total_secondary_activation += delta_activation

    def get_radius(self) -> float:
        """Returns the radius based on current activation."""
        return self.get_radius_for_activation(self.activation)

    def get_radius_for_activation(self, activation: float) -> float:
        """Calculates radius for a given activation value."""
        return math.sqrt(activation)

    def link_to(
        self, to_node: "Node", weight: float, reciprocal_link: bool = True
    ) -> None:
        """Creates or updates a link to another node."""
        if to_node is None or to_node is self:
            return

        link = self.links_map.get(to_node)

        if link is None:
            # Create new link
            link = NodeLink(to_node, weight)
            self.links.append(link)
            self.links_map[to_node] = link

            # Create reciprocal link if requested
            if reciprocal_link:
                to_node.link_to(self, weight, False)
        else:
            # Update existing link
            link.weight = weight

            if reciprocal_link:
                other_weight = to_node.get_link_weight(self)
                to_node.link_to(self, (weight + other_weight) * 0.5, False)

    def unlink(self, to_node: "Node", unlink_reciprocal_link: bool):
        """Removes a link to another node."""
        # search through the links,
        for link in self.links:
            if link.target == to_node:
                # remove link from links
                self.links.remove(link)

        # TODO: does this delete the link object???
        # remove from the weight map
        del self.links_map[to_node]

        # if the reciprocal link should be removed,
        if unlink_reciprocal_link:
            # then unlink in the other direction
            to_node.unlink(self, False)

    def get_link_to(self, to_node: "Node") -> Optional["NodeLink"]:
        """Returns the weight of the link to the specified node."""
        return self.links_map.get(to_node)

    def get_link_weight(self, to_node: "Node") -> float:
        """Returns the weight of the link to the specified node."""
        link = self.links_map.get(to_node)
        return link.weight if link else 0.0

    def get_link_gain_weight(self, to_node: "Node") -> float:
        """returns the gained weight for the given link"""
        link = self.get_link_to(to_node)
        if link:
            return link.gain_weight
        return 0.0

    def propagate_activation(
        self,
        from_node: Optional["Node"],
        delta_activation: float,
        init_scale: float,
        alpha: float,
    ) -> None:
        """Updates node's activation and propagates through connected nodes."""
        if delta_activation > 0.0:
            self.new_secondary_activation += delta_activation

            if from_node is not None and delta_activation > self.max_delta_activation:
                self.max_activator = from_node
                self.max_delta_activation = delta_activation

        # Propagate through all links
        for link in self.links:
            link.propagate_activation(self, init_scale, alpha)

    def reset_for_propagation(self) -> None:
        """Resets propagation state for this node and its children."""
        for link in self.links:
            link.has_propagated = False

        self.new_secondary_activation = self.secondary_activation
        self.max_delta_activation = 0.0
        self.max_activator = None

        for child in self.children:
            child.reset_for_propagation()

    def update_from_new_activation(self) -> None:
        """Updates activation values after propagation."""
        if self.space:
            self.space.total_secondary_activation += (
                self.new_secondary_activation - self.secondary_activation
            )

        self.secondary_activation = self.new_secondary_activation

        for child in self.children:
            child.update_from_new_activation()

    def update_post_super(self) -> None:
        """ helper to update node post-super """
        # adjust the sub-threshold nodes
        if self.is_sub_threshold:
            # and flag as no longer sub threshold
            self.is_sub_threshold = False

            # set the post super count
            self.post_super_count = 5

        elif self.post_super_count > 0:
            # decrement post super count
            self.post_super_count -= 1

        if self.reset_position_valid:
            self.reset_position = self.position.copy()

    def position_new_super(self) -> None:
        """Positions newly super-threshold nodes."""
        if self.is_sub_threshold:
            self.position = np.zeros(3)
            self.position_reset = True

            if self.max_activator:
                self.position = self.max_activator.position.copy()
                self.reset_position_valid = True
                self.reset_position = self.max_activator.position.copy()

            # Set gain to full for all links
            for link in self.links:
                link.gain = 1.0
                target = link.target
                reverse_link = target.links_map.get(self)
                if reverse_link:
                    reverse_link.gain = 1.0

"""_summary_"""
import random
from typing import List, Optional

from pyweel.node import Node


class Space:
    """
    Represents a space containing nodes and managing their interactions.
    """

    # Constants
    PRIM_NORM_SCALE = 0.020 * 3.5
    SEC_NORM_SCALE = 0.060 * 3.5
    TOTAL_ACTIVATION = 1.0  # Placeholder value
    TOTAL_ACTIVATION_FRACTION = 0.40
    PROPAGATE_SCALE = 0.40
    PROPAGATE_ALPHA = 0.99
    MIN_INITIAL_WGT = 0.4
    MAX_INITIAL_WGT = 0.7
    DEFAULT_SPRING_CONST = 0.95

    def __init__(self):
        self.root_node:Optional["Node"] = None
        self.nodes_sorted = False
        self.layout_manager = None  # Would need separate implementation
        self.last_post_super = 0
        self.current_node = None

        # Activation tracking
        self.total_primary_activation = 0.0
        self.total_secondary_activation = 0.0

        # Layout properties
        self.spring_const = self.DEFAULT_SPRING_CONST

        # Node storage
        self.nodes: List[Node] = []

        # Class color mapping (could be implemented differently in Python)
        self.class_colors = {}

    def get_node_count(self) -> int:
        """Returns the number of nodes in the space."""
        return len(self.nodes)

    def get_node_at(self, index: int) -> Node:
        """Returns the node at the specified index."""
        return self.nodes[index]

    def add_node(self, new_node: Node, parent_node: Optional[Node] = None) -> None:
        """Adds a new node to the space with optional parent."""
        # Generate random initial weight
        act_weight = random.uniform(self.MIN_INITIAL_WGT, self.MAX_INITIAL_WGT)

        if parent_node:
            # Set up the link
            new_node.link_to(parent_node, act_weight)

            # Add new node to parent
            new_node.parent = parent_node

        # Add node to array
        self.add_node_to_array(new_node)

        # Set activation after adding to array (to update total activation)
        new_node.set_activation(act_weight * 0.1)

    def remove_node(self, marked_node: Node) -> None:
        """Removes a node from the space."""
        # Unlink from all current nodes
        for node in self.nodes:
            node.unlink(marked_node, False)

        # Remove node from parent
        marked_node.parent = None

        # De-activate
        marked_node.set_activation(0.00001)

        # Remove from array
        if marked_node in self.nodes:
            self.nodes.remove(marked_node)

    def activate_node(self, node: Node, scale: float) -> None:
        """Activates a particular node and propagates the activation."""
        assert self.root_node is not None

        # Compute new activation
        old_activation = node.activation
        new_activation = old_activation + scale * (
            self.TOTAL_ACTIVATION * self.TOTAL_ACTIVATION_FRACTION - old_activation
        )

        # Set node's new activation
        node.set_activation(new_activation)

        # Prepare for propagation
        self.root_node.reset_for_propagation()

        # Propagate activation
        node.propagate_activation(None, 0.0, self.PROPAGATE_SCALE, self.PROPAGATE_ALPHA)

        # Update activation for all nodes
        self.root_node.update_from_new_activation()

        # Normalize nodes
        self.normalize_nodes()

        # Flag nodes as unsorted
        self.nodes_sorted = False

    def normalize_nodes(self, sum_target: float = TOTAL_ACTIVATION) -> None:
        """Normalizes node activations to sum to a target value."""
        total_activation = self.get_total_activation(True)
        diff_frac = sum_target / total_activation - 1.0
        if diff_frac > 0.0:
            diff_frac = 0.0

        for node in self.nodes:
            # Scale primary activation
            self.total_primary_activation -= node.primary_activation
            node.primary_activation += (
                node.primary_activation * diff_frac * self.PRIM_NORM_SCALE
            )
            self.total_primary_activation += node.primary_activation

            # Scale secondary activation
            self.total_secondary_activation -= node.secondary_activation
            node.secondary_activation += (
                node.secondary_activation * diff_frac * self.SEC_NORM_SCALE
            )
            self.total_secondary_activation += node.secondary_activation

    def sort_nodes(self) -> None:
        """Sorts nodes by activation value."""
        if not self.nodes_sorted:
            self.nodes.sort(
                key=lambda x: (x.activation + 0.1 * x.post_super_count / 40.0),
                reverse=True,
            )
            self.nodes_sorted = True

    def add_node_to_array(self, node: Node) -> None:
        """Adds a node and its children to the node array."""
        # Set space pointer
        node.space = self
        self.total_primary_activation += node.primary_activation
        self.total_secondary_activation += node.secondary_activation

        # Add to array
        self.nodes.append(node)

        # Add children recursively
        for child in node.children:
            self.add_node_to_array(child)

        # Flag nodes as needing sort
        self.nodes_sorted = False

    def get_total_activation(self, recompute: bool = False) -> float:
        """Returns total activation of all nodes."""
        return self.get_total_primary_activation(
            recompute
        ) + self.get_total_secondary_activation(recompute)

    def get_total_primary_activation(self, recompute: bool = False) -> float:
        """Returns total primary activation of all nodes."""
        if recompute:
            self.total_primary_activation = sum(
                node.primary_activation for node in self.nodes
            )
        return self.total_primary_activation

    def get_total_secondary_activation(self, recompute: bool = False) -> float:
        """Returns total secondary activation of all nodes."""
        if recompute:
            self.total_secondary_activation = sum(
                node.secondary_activation for node in self.nodes
            )
        return self.total_secondary_activation

    def create_simple_space(self) -> bool:
        """Creates a simple test space with some nodes."""
        # Clear existing content
        self.nodes.clear()
        self.total_primary_activation = 0.0
        self.total_secondary_activation = 0.0

        # Create root node
        self.root_node = Node(self, "%%hiddenroot%%")

        # Create main node
        main_node = Node(self, "root")
        main_node.parent = self.root_node
        self.add_node(main_node)

        # Create some test nodes
        child1 = Node(self, "Child1")
        self.add_node(child1, main_node)
        self.add_node(Node(self, "Child2"), main_node)
        self.add_node(Node(self, "Child3"), main_node)

        # Initialize activations
        main_node.set_activation(0.5)
        main_node.reset_for_propagation()
        main_node.propagate_activation(None, 0.0, 0.2, 0.9)

        self.sort_nodes()
        self.normalize_nodes()

        return True

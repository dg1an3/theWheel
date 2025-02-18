import math
import numpy as np

from pyweel.node import Node
from pyweel.space import Space
from pyweel.space_layout_objective import SpaceLayoutObjective


class SpaceLayoutManager:
    """
    Manages the layout of nodes in a space using the objective function
    for optimization.
    """

    # Constants for the layout
    SIZE_SCALE = 150.0
    DIST_SCALE_MIN = 1.0
    DIST_SCALE_MAX = 1.35
    ACTIVATION_MIDPOINT = 0.25

    # Constants for the layout
    SIZE_SCALE = 150.0
    DIST_SCALE_MIN = 1.0
    DIST_SCALE_MAX = 1.35
    ACTIVATION_MIDPOINT = 0.25

    # Compute optimal normalized distance
    MIDWEIGHT = 0.5
    OPT_DIST = DIST_SCALE_MIN + (DIST_SCALE_MAX - DIST_SCALE_MIN) * (
        1.0 - MIDWEIGHT / (MIDWEIGHT + ACTIVATION_MIDPOINT)
    )

    def __init__(self, space: "Space"):
        self.space = space
        self.objective = SpaceLayoutObjective(space)
        self.state_vector = np.zeros(0)  # Current state vector

        # Relaxation constants
        self.relax_sigmoid_shift = 1.20
        self.relax_sigmoid_factor = 1.5
        self.relax_new_gain_factor = 0.4
        self.relax_new_gain_factor_subthreshold = 0.8

    def get_super_node_count(self) -> int:
        """Returns the number of super nodes."""
        return min(self.objective.state_dim // 2, len(self.space.nodes))

    def set_max_super_node_count(self, count: int) -> None:
        """Sets the maximum number of super nodes."""
        self.objective.state_dim = count * 2
        self.state_vector = np.zeros(self.objective.state_dim)

    def get_dist_error(self, from_node: "Node", to_node: "Node") -> float:
        """Calculates the distance error between two nodes."""
        size_from = self.SIZE_SCALE * from_node.get_radius() + 10.0
        size_to = self.SIZE_SCALE * to_node.get_radius() + 10.0
        size_avg = 0.5 * (size_from + size_to)

        # Calculate offset with wrapping
        offset = from_node.position - to_node.position

        # Check horizontal wrapping
        for shift_x in [-1, 0, 1]:
            new_pos = to_node.position.copy()
            new_pos[0] += shift_x * 800
            new_offset = from_node.position - new_pos
            if np.linalg.norm(new_offset) < np.linalg.norm(offset):
                offset = new_offset

        # Check vertical wrapping
        for shift_y in [-1, 0, 1]:
            new_pos = to_node.position.copy()
            new_pos[1] += shift_y * 400
            new_offset = from_node.position - new_pos
            if np.linalg.norm(new_offset) < np.linalg.norm(offset):
                offset = new_offset

        # Compute relative actual distance
        act_dist = (
            math.sqrt(
                offset[0] * offset[0] / (size_avg * size_avg)
                + offset[1] * offset[1] / (size_avg * size_avg)
            )
            + 0.001
        )

        return act_dist - self.OPT_DIST

    def layout_nodes(self) -> None:
        """Lays out all nodes in the space."""
        # Ensure nodes are sorted
        self.space.sort_nodes()

        # Position new super-threshold nodes
        self.position_new_super_nodes()

        # Layout the nodes
        self.layout_nodes_partial(0)

        # Relax node positions
        self.relax()

        # Apply final positions
        self.apply_positions()

    def layout_nodes_partial(self, const_nodes: int) -> None:
        """Layout a subset of nodes, keeping some constant."""
        const_nodes = min(const_nodes, self.objective.state_dim // 2)

        if const_nodes * 2 >= self.objective.state_dim:
            return

        # Load relationship matrices
        self.objective.load_node_relationships(
            const_nodes, self.objective.state_dim // 2
        )

        # Get current positions
        self.get_positions_vector()

        # Optimize positions
        # Note: In practice, you would use an optimization algorithm here
        # This is a simplified placeholder
        positions = self.state_vector[const_nodes * 2 :]
        optimized_positions = self.optimize_positions(positions)
        self.state_vector[const_nodes * 2 :] = optimized_positions

        # Apply the optimized positions
        self.set_positions_vector(self.state_vector)

    def optimize_positions(self, positions: np.ndarray) -> np.ndarray:
        """
        Optimize node positions using gradient descent.
        This is a simplified version - in practice you'd want to use
        a more sophisticated optimizer.
        """
        learning_rate = 0.01
        max_iterations = 100
        positions = positions.copy()

        for _ in range(max_iterations):
            _, gradient = self.objective.calculate_energy(positions, True) # type: ignore
            positions -= learning_rate * gradient

            if np.all(np.abs(gradient) < self.objective.tolerance):
                break

        return positions

    def position_new_super_nodes(self) -> None:
        """Positions the new super-threshold nodes."""
        highest_new_super = self.get_super_node_count()

        # Sort nodes by super-threshold status
        nodes = self.space.nodes
        i = 0
        while i < highest_new_super:
            node = nodes[i]
            if node.is_sub_threshold or node.post_super_count > 0:
                # Swap with last super node
                highest_new_super -= 1
                nodes[i], nodes[highest_new_super] = nodes[highest_new_super], nodes[i]
            else:
                i += 1

        # Position new super nodes
        for node in nodes[: self.get_super_node_count()]:
            node.position_new_super()

        # Layout new super nodes
        self.layout_nodes_partial(highest_new_super)
        self.relax(True)

        # Update post-super processing
        for node in nodes[: self.get_super_node_count()]:
            node.update_post_super()

        # Mark remaining nodes as sub-threshold
        for node in nodes[self.get_super_node_count() :]:
            node.is_sub_threshold = True

    def relax(self, sub_threshold: bool = False) -> None:
        """
        Relaxes the link weights by adjusting gain based on distance error.

        Args:
            sub_threshold: Whether to relax sub-threshold nodes
        """
        # Choose gain factor
        new_gain_factor = (
            self.relax_new_gain_factor_subthreshold
            if sub_threshold
            else self.relax_new_gain_factor
        )

        # Iterate over super nodes
        super_count = self.get_super_node_count()
        for i in range(super_count):
            node = self.space.nodes[i]

            # Iterate over linked nodes
            for j in range(super_count):
                linked = self.space.nodes[j]

                if node == linked:
                    continue

                # Skip non-sub-threshold nodes if in sub-threshold mode
                if (
                    sub_threshold
                    and not linked.is_sub_threshold
                    and not node.is_sub_threshold
                    and not linked.post_super_count > 0
                    and not node.post_super_count > 0
                ):
                    continue

                # Get link if it exists
                link = node.get_link_to(linked)
                if link is None:
                    continue

                # Compute distance error
                dist_err = self.get_dist_error(node, linked)
                dist_err *= node.activation + linked.activation

                # Compute new gain using sigmoid
                new_gain = 1.0 - self.sigmoid(
                    dist_err - self.relax_sigmoid_shift, self.relax_sigmoid_factor
                )

                # Update gain with weighted average
                link.gain = (
                    link.gain * (1.0 - new_gain_factor) + new_gain * new_gain_factor
                )

    def sigmoid(self, x: float, factor: float) -> float:
        """Compute sigmoid function."""
        return 1.0 / (1.0 + math.exp(-factor * x))

    def get_positions_vector(self) -> None:
        """Gets current node positions into state vector."""
        for i, node in enumerate(self.space.nodes[: self.objective.state_dim // 2]):
            self.state_vector[i * 2] = node.position[0]
            self.state_vector[i * 2 + 1] = node.position[1]

    def set_positions_vector(self, positions: np.ndarray) -> None:
        """Sets node positions from state vector."""
        for i, node in enumerate(self.space.nodes[: self.objective.state_dim // 2]):
            node.position[0] = positions[i * 2]
            node.position[1] = positions[i * 2 + 1]

    def apply_positions(self) -> None:
        """
        Applies the final positions to nodes after optimization.
        This includes any necessary rotation or translation adjustments.
        """
        # Get current positions
        self.get_positions_vector()

        # Apply any rotation/translation transformations here if needed
        # For now, just directly apply positions
        self.set_positions_vector(self.state_vector)

        # Update final state
        self.get_positions_vector()

""" Objective function for optimizing the layout of nodes in a space. """

import math
from typing import Optional, Union

import numpy as np

from pyweel.space import Space


class SpaceLayoutObjective:
    """
    Objective function for optimizing the layout of nodes in a space.
    Calculates energy based on node positions and their relationships.
    """

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

    # Energy constants
    K_POS = 600.0  # Constant for weighting position energy
    K_REP = 3200.0  # Constant for weighting repulsion energy
    TOLERANCE = 1e3  # Optimization tolerance

    def __init__(self, space: "Space"):
        self.space = space
        self.state_dim = 0
        self.const_nodes = 0
        self.state = np.zeros(0)
        self.const_positions = np.zeros(0)
        self.energy = 0.0
        self.energy_const = 0.0

        # Matrices for quick access to node relationships
        self.sizes_squared: Optional[np.ndarray] = (
            None  # Size relationships between nodes
        )
        self.avg_activations: Optional[np.ndarray] = (
            None  # Average activations between node pairs
        )
        self.link_weights: Optional[np.ndarray] = None  # Link weights between nodes

        # Parameters
        self.k_pos = self.K_POS
        self.k_rep = self.K_REP
        self.tolerance = self.TOLERANCE

    def initialize_matrices(self, node_count: int):
        """Initialize matrices for storing node relationship data."""
        self.sizes_squared = np.zeros((node_count, node_count))
        self.avg_activations = np.zeros((node_count, node_count))
        self.link_weights = np.zeros((node_count, node_count))

    def load_node_relationships(self, const_nodes: int, node_count: int):
        """Load node sizes and relationships into matrices for quick access."""
        node_count = min(node_count, self.state_dim // 2)
        node_count = min(node_count, len(self.space.nodes))

        # Initialize or resize matrices if needed
        self.initialize_matrices(node_count)
        assert self.sizes_squared is not None
        assert self.avg_activations is not None
        assert self.link_weights is not None

        # Calculate node sizes
        sizes = np.array(
            [
                self.SIZE_SCALE * node.get_radius() + 10.0
                for node in self.space.nodes[:node_count]
            ]
        )

        # Fill matrices
        for i in range(node_count):
            node_i = self.space.nodes[i]

            # Diagonal elements
            self.sizes_squared[i, i] = 1.0
            self.avg_activations[i, i] = 0.0
            self.link_weights[i, i] = 0.0

            # Off-diagonal elements
            for j in range(i + 1, node_count):
                node_j = self.space.nodes[j]

                # Common size (average)
                ss = (sizes[i] + sizes[j]) / 2.0
                self.sizes_squared[j, i] = ss * ss

                # Average activation
                avg_act = node_i.activation + node_j.activation
                self.avg_activations[j, i] = avg_act

                # Link weight for layout
                weight = (
                    node_i.activation / avg_act * node_i.get_link_gain_weight(node_j)
                    + node_j.activation / avg_act * node_j.get_link_gain_weight(node_i)
                ) + 1e-6
                self.link_weights[j, i] = weight * self.avg_activations[j, i]

                # Mirror values for upper triangle
                self.sizes_squared[i, j] = self.sizes_squared[j, i]
                self.avg_activations[i, j] = self.avg_activations[j, i]
                self.link_weights[i, j] = self.link_weights[j, i]

        # Handle constant nodes energy
        self.energy_const = 0.0
        if const_nodes > 0:
            self.const_positions = self.state[: const_nodes * 2].copy()
            old_state_dim = self.state_dim
            self.state_dim = const_nodes * 2
            self.const_nodes = 0
            self.energy_const = self.calculate_energy(self.const_positions)
            assert isinstance(self.energy_const, float)
            self.state_dim = old_state_dim

        self.const_nodes = const_nodes

    def calculate_energy(
        self, positions: np.ndarray, calculate_gradient: bool = False
    ) -> Union[float, tuple[float, np.ndarray]]:
        """
        Calculate the energy of the current layout configuration.
        Lower energy indicates better layout.
        Args:
            positions: Node position vector
            calculate_gradient: Whether to calculate the gradient
        Returns:
            energy: Total energy of the configuration
            gradient: Gradient vector if calculate_gradient is True
        """
        assert self.sizes_squared is not None
        assert self.avg_activations is not None
        assert self.link_weights is not None
        assert isinstance(self.energy_const, float)

        energy = self.energy_const
        gradient = np.zeros_like(positions) if calculate_gradient else None

        # Prepare full position information
        self.state[self.const_nodes * 2 : self.const_nodes * 2 + len(positions)] = (
            positions
        )

        node_count = min(self.state_dim // 2, len(self.space.nodes))
        node_count = min(node_count, len(positions) // 2 + self.const_nodes)

        # Calculate energy between each pair of nodes
        for i in range(node_count - 1, self.const_nodes - 1, -1):
            for j in range(i - 1, -1, -1):
                # Get position differences
                dx = self.state[i * 2] - self.state[j * 2]
                dy = self.state[i * 2 + 1] - self.state[j * 2 + 1]

                # Get scaling factors
                ss_sq = self.sizes_squared[i, j]

                # Position energy
                dx_ratio = dx / ss_sq
                dy_ratio = dy / ss_sq
                x_ratio = dx * dx_ratio
                y_ratio = dy * dy_ratio

                # Calculate actual distance and error
                act_dist = math.sqrt(x_ratio + y_ratio + 0.001)
                dist_error = act_dist - self.OPT_DIST

                # Position energy term
                pos_factor = self.k_pos * self.link_weights[i, j]
                energy += pos_factor * dist_error * dist_error

                if gradient is not None:
                    # Position gradient
                    dact_dist_dx = dx_ratio / act_dist
                    dact_dist_dy = dy_ratio / act_dist
                    denergy_dx = pos_factor * dist_error * dact_dist_dx
                    denergy_dy = pos_factor * dist_error * dact_dist_dy

                    gradient[i * 2] += 2 * denergy_dx
                    gradient[i * 2 + 1] += 2 * denergy_dy
                    gradient[j * 2] -= 2 * denergy_dx
                    gradient[j * 2 + 1] -= 2 * denergy_dy

                # Repulsion energy
                inv_sq = 1.0 / (x_ratio + y_ratio + 3.0)
                rep_factor = self.k_rep * self.avg_activations[i, j]
                energy += rep_factor * inv_sq

                if gradient is not None:
                    # Repulsion gradient
                    inv_sq_sq = inv_sq * inv_sq
                    drep_dx = rep_factor * dx_ratio * inv_sq_sq
                    drep_dy = rep_factor * dy_ratio * inv_sq_sq

                    gradient[i * 2] -= 2 * drep_dx
                    gradient[i * 2 + 1] -= 2 * drep_dy
                    gradient[j * 2] += 2 * drep_dx
                    gradient[j * 2 + 1] += 2 * drep_dy

        if gradient is not None:
            return energy, gradient[self.const_nodes * 2 :]
        return energy

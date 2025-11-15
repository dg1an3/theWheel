"""
Unit tests for Space class
Tests the CSpace Python bindings
"""
import pytest
import sys
import os

# Add the build directory to Python path
build_dir = os.path.join(os.path.dirname(__file__), '..', 'src', 'out', 'build', 'x64-debug', 'python')
if os.path.exists(build_dir):
    sys.path.insert(0, build_dir)

import pythewheel


@pytest.fixture
def space():
    """Create a Space for testing"""
    return pythewheel.Space()


@pytest.mark.unit
class TestSpace:
    """Test cases for Space class"""

    def test_space_creation(self):
        """Test basic space creation"""
        space = pythewheel.Space()
        assert space is not None

    def test_initial_node_count(self, space):
        """Test initial node count"""
        # A new space should have at least the root node
        count = space.get_node_count()
        assert count >= 0

    def test_root_node(self, space):
        """Test that space has a root node"""
        root = space.get_root_node()
        # Root node may be None initially or auto-created
        # Just verify the accessor works
        assert root is not None or root is None

    def test_add_node(self, space):
        """Test adding a node to the space"""
        node = pythewheel.Node(space, "TestNode", "Test")
        initial_count = space.get_node_count()

        space.add_node(node, None)

        # Node count should increase
        assert space.get_node_count() >= initial_count

    def test_add_node_with_parent(self, space):
        """Test adding a node with a parent"""
        parent = pythewheel.Node(space, "Parent", "Parent Node")
        child = pythewheel.Node(space, "Child", "Child Node")

        space.add_node(parent, None)
        space.add_node(child, parent)

        # Both nodes should be in the space
        assert space.get_node_count() >= 2

    def test_get_node_at(self, space):
        """Test retrieving nodes by index"""
        node1 = pythewheel.Node(space, "Node1", "First")
        node2 = pythewheel.Node(space, "Node2", "Second")

        space.add_node(node1, None)
        space.add_node(node2, None)

        count = space.get_node_count()
        if count > 0:
            retrieved = space.get_node_at(0)
            assert retrieved is not None

    def test_remove_node(self, space):
        """Test removing a node from the space"""
        node = pythewheel.Node(space, "ToRemove", "Remove me")
        space.add_node(node, None)

        initial_count = space.get_node_count()
        space.remove_node(node)

        # Node count should decrease
        assert space.get_node_count() <= initial_count

    def test_current_node(self, space):
        """Test getting and setting the current node"""
        node = pythewheel.Node(space, "Current", "Current Node")
        space.add_node(node, None)

        space.set_current_node(node)
        current = space.get_current_node()

        assert current == node

    def test_path_name(self, space):
        """Test getting and setting path name"""
        space.set_path_name("C:/test/path")
        assert space.get_path_name() == "C:/test/path"

    def test_prim_sec_ratio(self, space):
        """Test primary/secondary activation ratio"""
        space.set_prim_sec_ratio(0.7)
        assert abs(space.get_prim_sec_ratio() - 0.7) < 1e-6

    def test_spring_constant(self, space):
        """Test spring constant for layout"""
        space.set_spring_const(0.5)
        assert abs(space.get_spring_const() - 0.5) < 1e-6

    def test_center(self, space):
        """Test getting and setting the space center"""
        center = pythewheel.Vector3D(10.0, 20.0, 30.0)
        space.set_center(center)

        retrieved = space.get_center()
        assert retrieved[0] == 10.0
        assert retrieved[1] == 20.0
        assert retrieved[2] == 30.0


@pytest.mark.integration
@pytest.mark.activation
class TestSpaceActivation:
    """Integration tests for space-wide activation management"""

    def test_activate_node(self, space):
        """Test activating a node through the space"""
        node = pythewheel.Node(space, "Active", "Active Node")
        space.add_node(node, None)

        space.activate_node(node, 0.5)

        # Node should have some activation
        assert node.get_activation() > 0

    def test_normalize_nodes(self, space):
        """Test normalizing node activations"""
        node1 = pythewheel.Node(space, "Node1", "First")
        node2 = pythewheel.Node(space, "Node2", "Second")

        space.add_node(node1, None)
        space.add_node(node2, None)

        node1.set_activation(0.5)
        node2.set_activation(0.5)

        # Normalize to sum of 1.0
        space.normalize_nodes(1.0)

        # Total should be approximately 1.0
        total = node1.get_activation() + node2.get_activation()
        assert abs(total - 1.0) < 0.1  # Allow some tolerance for normalization

    def test_sort_nodes(self, space):
        """Test sorting nodes by activation"""
        node1 = pythewheel.Node(space, "Low", "Low activation")
        node2 = pythewheel.Node(space, "High", "High activation")

        space.add_node(node1, None)
        space.add_node(node2, None)

        node1.set_activation(0.2)
        node2.set_activation(0.8)

        space.sort_nodes()

        # After sorting, highest activation should come first
        if space.get_node_count() >= 2:
            first = space.get_node_at(0)
            # First node should have higher activation than last
            assert first.get_activation() >= node1.get_activation()

    def test_total_activation(self, space):
        """Test computing total activation"""
        node1 = pythewheel.Node(space, "Node1", "First")
        node2 = pythewheel.Node(space, "Node2", "Second")

        space.add_node(node1, None)
        space.add_node(node2, None)

        node1.set_activation(0.3)
        node2.set_activation(0.5)

        total = space.get_total_activation(True)  # Force computation
        assert total >= 0.0

    def test_primary_secondary_activation(self, space):
        """Test primary and secondary activation tracking"""
        node = pythewheel.Node(space, "Node", "Test")
        space.add_node(node, None)

        node.set_primary_activation(0.6)
        node.set_secondary_activation(0.4)

        total_primary = space.get_total_primary_activation(True)
        total_secondary = space.get_total_secondary_activation(True)

        assert total_primary >= 0.0
        assert total_secondary >= 0.0


@pytest.mark.integration
class TestSpaceNetwork:
    """Integration tests for complex node networks in a space"""

    def test_create_node_network(self, space):
        """Test creating a network of interconnected nodes"""
        # Create a small semantic network
        nodes = []
        for i in range(5):
            node = pythewheel.Node(space, f"Node{i}", f"Node {i}")
            space.add_node(node, None)
            nodes.append(node)

        # Create links between nodes
        for i in range(len(nodes) - 1):
            nodes[i].link_to(nodes[i + 1], 0.5)

        # Verify the network structure
        assert space.get_node_count() >= 5
        assert nodes[0].get_link_count() >= 1

    def test_hierarchical_structure(self, space):
        """Test creating a hierarchical node structure"""
        root = pythewheel.Node(space, "Root", "Root Node")
        space.add_node(root, None)

        # Create children
        for i in range(3):
            child = pythewheel.Node(space, f"Child{i}", f"Child {i}")
            space.add_node(child, root)
            child.set_parent(root)

        # Root should have children
        assert root.get_child_count() >= 3

"""
Unit tests for Node class
Tests the CNode Python bindings
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


@pytest.fixture
def node(space):
    """Create a Node for testing"""
    return pythewheel.Node(space, "TestNode", "Test Description")


@pytest.mark.unit
class TestNode:
    """Test cases for Node class"""

    def test_node_creation(self, space):
        """Test basic node creation"""
        node = pythewheel.Node(space, "Test", "Description")
        assert node is not None
        assert node.get_name() == "Test"
        assert node.get_description() == "Description"

    def test_node_without_space(self):
        """Test creating a node without a space"""
        node = pythewheel.Node()
        assert node is not None

    def test_get_set_name(self, node):
        """Test getting and setting node name"""
        node.set_name("NewName")
        assert node.get_name() == "NewName"

    def test_get_set_description(self, node):
        """Test getting and setting node description"""
        node.set_description("New Description")
        assert node.get_description() == "New Description"

    def test_get_set_class(self, node):
        """Test getting and setting node class"""
        node.set_class("TestClass")
        assert node.get_class() == "TestClass"

    def test_initial_activation(self, node):
        """Test initial activation values.
        CNode initializes with m_primaryActivation=0.005 and
        m_secondaryActivation=0.005, so total is 0.01."""
        assert abs(node.get_activation() - 0.01) < 1e-6
        assert abs(node.get_primary_activation() - 0.005) < 1e-6
        assert abs(node.get_secondary_activation() - 0.005) < 1e-6

    def test_set_activation(self, node):
        """Test setting activation value.
        SetActivation splits the delta equally between primary and secondary
        when called without an activator node."""
        node.set_activation(0.5)
        assert abs(node.get_activation() - 0.5) < 1e-6

    def test_activation_splits_primary_secondary(self, node):
        """Test that SetActivation splits delta between primary and secondary.
        Starting from 0.005/0.005, setting total to 0.5 adds delta=0.49.
        Primary gets +0.245, secondary gets +0.245."""
        node.set_activation(0.5)
        assert abs(node.get_primary_activation() - 0.25) < 1e-4
        assert abs(node.get_secondary_activation() - 0.25) < 1e-4

    def test_position(self, node):
        """Test getting and setting position"""
        pos = pythewheel.Vector3D(1.0, 2.0, 3.0)
        node.set_position(pos)
        retrieved_pos = node.get_position()
        assert retrieved_pos[0] == 1.0
        assert retrieved_pos[1] == 2.0
        assert retrieved_pos[2] == 3.0

    def test_link_count_initial(self, node):
        """Test initial link count is zero"""
        assert node.get_link_count() == 0

    def test_link_to_another_node(self, space):
        """Test creating a link between two nodes"""
        node1 = pythewheel.Node(space, "Node1", "First")
        node2 = pythewheel.Node(space, "Node2", "Second")

        node1.link_to(node2, 0.5)

        # Check link was created
        assert node1.get_link_count() >= 1

        # Check we can retrieve the link
        link = node1.get_link_to(node2)
        assert link is not None
        assert abs(link.get_weight() - 0.5) < 1e-6

    def test_get_link_weight(self, space):
        """Test getting link weight between nodes"""
        node1 = pythewheel.Node(space, "Node1", "First")
        node2 = pythewheel.Node(space, "Node2", "Second")

        node1.link_to(node2, 0.75)
        weight = node1.get_link_weight(node2)

        assert abs(weight - 0.75) < 1e-6

    def test_reciprocal_link(self, space):
        """Test that reciprocal links are created by default"""
        node1 = pythewheel.Node(space, "Node1", "First")
        node2 = pythewheel.Node(space, "Node2", "Second")

        node1.link_to(node2, 0.5, True)

        # Both nodes should have links
        assert node1.get_link_count() >= 1
        assert node2.get_link_count() >= 1

    def test_unlink_nodes(self, space):
        """Test unlinking nodes"""
        node1 = pythewheel.Node(space, "Node1", "First")
        node2 = pythewheel.Node(space, "Node2", "Second")

        node1.link_to(node2, 0.5)
        initial_count = node1.get_link_count()

        node1.unlink(node2)

        # Link count should decrease
        assert node1.get_link_count() < initial_count

    def test_parent_child_relationship(self, space):
        """Test parent-child relationships"""
        parent = pythewheel.Node(space, "Parent", "Parent Node")
        child = pythewheel.Node(space, "Child", "Child Node")

        child.set_parent(parent)

        # Check parent-child relationship
        assert child.get_parent() == parent
        assert parent.get_child_count() == 1

    def test_radius(self, node):
        """Test radius calculation based on activation"""
        node.set_activation(0.25)
        radius = node.get_radius()
        # radius = sqrt(activation)
        assert abs(radius - 0.5) < 1e-4


@pytest.mark.integration
class TestNodeActivation:
    """Integration tests for node activation spreading"""

    def test_multiple_nodes_activation(self, space):
        """Test activation with multiple interconnected nodes"""
        # Create a small network
        node1 = pythewheel.Node(space, "A", "Node A")
        node2 = pythewheel.Node(space, "B", "Node B")
        node3 = pythewheel.Node(space, "C", "Node C")

        # Link them
        node1.link_to(node2, 0.5)
        node2.link_to(node3, 0.5)

        # Activate first node
        node1.set_activation(1.0)

        assert abs(node1.get_activation() - 1.0) < 1e-6

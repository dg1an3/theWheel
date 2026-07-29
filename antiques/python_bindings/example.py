#!/usr/bin/env python3
"""
Example usage of theWheel Python bindings

This demonstrates the basic functionality of the semantic network system.
"""

import sys
import os

# Add the build directory to the path if the module isn't installed
# Adjust this path based on your build configuration
build_paths = [
    './build/python_bindings/Release',
    './build/python_bindings/Debug',
    './build/python_bindings',
    '../build/python_bindings/Release',
    '../build/python_bindings/Debug',
    '../build/python_bindings',
]

for path in build_paths:
    if os.path.exists(path):
        sys.path.insert(0, path)
        break

try:
    import thewheel
except ImportError as e:
    print(f"Error: Could not import thewheel module: {e}")
    print("\nPlease build the Python bindings first:")
    print("  mkdir build && cd build")
    print("  cmake .. -DBUILD_PYTHON_BINDINGS=ON")
    print("  cmake --build . --config Release")
    sys.exit(1)


def main():
    print("theWheel Python Bindings Example")
    print("=" * 50)
    print(f"Version: {thewheel.__version__}")
    print(f"Total activation constant: {thewheel.TOTAL_ACTIVATION}")
    print()

    # Create a new semantic space
    print("Creating semantic space...")
    space = thewheel.Space()

    # Create a simple test space
    print("Creating simple test space...")
    success = space.create_simple_space()

    if not success:
        print("Warning: create_simple_space() returned False")
        print("This might be expected - continuing anyway...")

    # Display basic information
    print(f"\nSpace contains {space.node_count} nodes")

    # Get the root node
    root = space.root_node
    if root:
        print(f"Root node: '{root.name}'")
        print(f"Root description: '{root.description}'")
        print(f"Root has {root.child_count} children")
    else:
        print("Warning: No root node found")

    # Display all nodes sorted by activation
    print("\nAll nodes (sorted by activation):")
    for i in range(space.node_count):
        node = space.get_node_at(i)
        if node:
            print(f"  [{i}] {node.name:20s} - Activation: {node.activation:.6f} "
                  f"(Links: {node.link_count}, Children: {node.child_count})")

    # Activate a specific node
    if space.node_count > 1:
        print("\n" + "=" * 50)
        print("Activating a node...")

        # Get the second node (first child of root, typically)
        node = space.get_node_at(1)
        if node:
            print(f"Activating node: '{node.name}'")
            space.activate_node(node, 1.0)

            # Normalize activations
            space.normalize_nodes(thewheel.TOTAL_ACTIVATION)

            print(f"\nAfter activation and normalization:")
            print(f"Total activation: {space.total_activation:.6f}")

            # Display top 5 activated nodes
            print("\nTop activated nodes:")
            for i in range(min(5, space.node_count)):
                n = space.get_node_at(i)
                if n:
                    print(f"  {i+1}. {n.name:20s} - {n.activation:.6f}")

    # Run layout algorithm
    print("\n" + "=" * 50)
    print("Running force-directed layout algorithm...")
    try:
        space.layout_nodes()
        print("Layout complete!")
    except Exception as e:
        print(f"Layout failed (this might be expected): {e}")

    # Set current node
    if space.node_count > 0:
        first_node = space.get_node_at(0)
        if first_node:
            space.current_node = first_node
            current = space.current_node
            if current:
                print(f"\nCurrent node set to: '{current.name}'")

    # Explore node hierarchy
    if root and root.child_count > 0:
        print("\n" + "=" * 50)
        print("Exploring node hierarchy:")
        print(f"\nRoot node: {root.name}")

        for i in range(min(3, root.child_count)):
            child = root.get_child_at(i)
            if child:
                print(f"  Child {i}: {child.name}")
                print(f"    Activation: {child.activation:.6f}")
                print(f"    Links: {child.link_count}")

                # Show grandchildren if any
                if child.child_count > 0:
                    print(f"    Has {child.child_count} children:")
                    for j in range(min(2, child.child_count)):
                        grandchild = child.get_child_at(j)
                        if grandchild:
                            print(f"      - {grandchild.name}")

    print("\n" + "=" * 50)
    print("Example complete!")


if __name__ == "__main__":
    main()

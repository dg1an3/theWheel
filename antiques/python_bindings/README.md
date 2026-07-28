# theWheel Python Bindings

Python bindings for the theWheel semantic network visualization system using pybind11.

## Building

### Prerequisites

- CMake 3.15 or higher
- C++ compiler with C++11 support (Visual Studio 2017+ on Windows)
- Python 3.7 or higher
- pybind11 (will be automatically downloaded if not found)

### Build Instructions

#### Windows (Visual Studio)

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DBUILD_PYTHON_BINDINGS=ON

# Build
cmake --build . --config Release

# The Python module will be in: build/python_bindings/Release/thewheel.pyd
```

#### Linux/macOS

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DBUILD_PYTHON_BINDINGS=ON -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)

# The Python module will be in: build/python_bindings/thewheel.so
```

### Installing the Python Module

After building:

```bash
# From the build directory
cmake --install python_bindings
```

Or manually copy the `.pyd` (Windows) or `.so` (Linux/macOS) file to your Python path.

## Usage

```python
import thewheel

# Create a new semantic space
space = thewheel.Space()

# Create a simple test space with example nodes
space.create_simple_space()

# Get the root node
root = space.root_node
print(f"Root node: {root.name}")
print(f"Number of nodes: {space.node_count}")

# Activate a node
if space.node_count > 1:
    node = space.get_node_at(1)
    space.activate_node(node, 1.0)
    print(f"Activated node: {node.name}")
    print(f"Activation level: {node.activation}")

# Normalize activations
space.normalize_nodes(thewheel.TOTAL_ACTIVATION)

# Run layout algorithm
space.layout_nodes()

# Iterate through most activated nodes
print("\nTop activated nodes:")
for i in range(min(5, space.node_count)):
    node = space.get_node_at(i)
    print(f"  {node.name}: {node.activation:.4f}")
```

## API Reference

### `Space` Class

The main container for the semantic network.

**Methods:**
- `Space()` - Create a new empty space
- `create_simple_space()` - Create a test space with example nodes
- `get_node_at(index)` - Get node at index (sorted by activation)
- `activate_node(node, scale)` - Activate a node with given scale
- `normalize_nodes(sum=1.0)` - Normalize total activation
- `layout_nodes()` - Run force-directed graph layout

**Properties:**
- `root_node` - The root node containing all other nodes
- `node_count` - Total number of nodes
- `current_node` - Currently selected node
- `total_activation` - Sum of all node activations

### `Node` Class

Represents a single semantic unit in the network.

**Methods:**
- `get_child_at(index)` - Get child node at index
- `link_to(target, weight, reciprocal=True)` - Create weighted link

**Properties:**
- `name` - Node name (read/write)
- `description` - Node description (read/write)
- `activation` - Current activation level (read-only)
- `link_count` - Number of outgoing links (read-only)
- `child_count` - Number of child nodes (read-only)

## Architecture

The theWheel system implements a semantic network with:

1. **Hierarchical Structure**: Nodes can contain child nodes
2. **Weighted Links**: Directed connections between nodes with weights
3. **Activation Spreading**: Neural-like activation propagation through links
4. **Force-Directed Layout**: Automatic 3D positioning using physics simulation

The Python bindings wrap the core C++ classes (`CSpace`, `CNode`, `CNodeLink`)
with Python-friendly interfaces, handling string conversions and memory management
automatically.

## License

Original theWheel code: Copyright (C) 1999-2002 Derek Graham Lane
U.S. Patent Pending

Python bindings: 2025

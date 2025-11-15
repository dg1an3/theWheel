# Python Testing Quick Start Guide

This is a condensed guide to get you up and running with Python-based testing for theWheel.

## Prerequisites

- Visual Studio 2017/2022 with C++ and MFC support
- Python 3.7 or higher
- CMake 3.10 or higher
- Git (for fetching pybind11)

## Quick Setup (5 minutes)

### 1. Build the Python Module

```bash
# Navigate to the src directory
cd C:\dev\theWheel\src

# Configure CMake (this will fetch pybind11 automatically)
cmake --preset x64-debug

# Build everything including the Python bindings
cmake --build out/build/x64-debug
```

The build will create `pythewheel.pyd` in `src/out/build/x64-debug/python/`

### 2. Install Python Dependencies

```bash
# Return to repository root
cd C:\dev\theWheel

# Install test dependencies
pip install -r requirements.txt
```

### 3. Run Tests

```bash
# Option 1: Use the convenience script
python run_tests.py

# Option 2: Run pytest directly
pytest -v

# Option 3: Run specific test files
pytest tests/test_node.py -v
```

## What Gets Tested?

The Python bindings expose core theWheel classes:

| Python Class | C++ Class | Description |
|--------------|-----------|-------------|
| `Vector3D` | `CVectorD<3>` | 3D position vectors |
| `NodeLink` | `CNodeLink` | Weighted links between nodes |
| `Node` | `CNode` | Semantic nodes with activation |
| `Space` | `CSpace` | Container managing node networks |

## Quick Example

```python
import pythewheel

# Create a space and nodes
space = pythewheel.Space()
node1 = pythewheel.Node(space, "Concept A", "First concept")
node2 = pythewheel.Node(space, "Concept B", "Second concept")

# Add nodes to space
space.add_node(node1, None)
space.add_node(node2, None)

# Create weighted link
node1.link_to(node2, 0.75)

# Set activation
node1.set_activation(1.0)

# Position in 3D space
pos = pythewheel.Vector3D(10.0, 20.0, 30.0)
node1.set_position(pos)

print(f"Node: {node1.get_name()}")
print(f"Activation: {node1.get_activation()}")
print(f"Links: {node1.get_link_count()}")
```

## Test Organization

```
tests/
├── test_vector3d.py    # Vector math operations
├── test_node.py        # Node creation, linking, activation
└── test_space.py       # Space management, network operations
```

## Running Specific Test Categories

```bash
# Unit tests only
pytest -m unit

# Integration tests
pytest -m integration

# Activation-related tests
pytest -m activation

# With coverage report
pytest --cov=pythewheel --cov-report=html
```

## Troubleshooting

**Problem**: `ImportError: No module named 'pythewheel'`
- **Solution**: Rebuild the project. The `.pyd` file must exist.

**Problem**: Build fails with "Python not found"
- **Solution**: Ensure Python is in your PATH and CMake can find it.

**Problem**: Tests fail with MFC errors
- **Solution**: Ensure Visual Studio has MFC components installed.

## Next Steps

- Read [TESTING.md](TESTING.md) for comprehensive documentation
- Examine existing tests in `tests/` for examples
- Add your own test cases as you develop features
- See [CLAUDE.md](CLAUDE.md) for theWheel architecture details

## Quick Commands Reference

```bash
# Build
cmake --preset x64-debug && cmake --build out/build/x64-debug

# Test
python run_tests.py
pytest -v
pytest -m unit
pytest tests/test_node.py::TestNode::test_node_creation

# Coverage
pytest --cov=pythewheel --cov-report=html
# Then open htmlcov/index.html

# Clean rebuild
rm -rf out/build/x64-debug
cmake --preset x64-debug
cmake --build out/build/x64-debug
```

---

For detailed information, see [TESTING.md](TESTING.md)

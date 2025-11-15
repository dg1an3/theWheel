# theWheel Python Testing Guide

This document describes the Python-based testing infrastructure for theWheel using pybind11 and pytest.

## Overview

The theWheel project now includes Python bindings for core C++ classes, enabling comprehensive testing of the semantic visualization and spreading activation functionality using Python's pytest framework.

## Architecture

### Python Bindings (`pythewheel`)

The `src/pythewheel/` directory contains pybind11 bindings that expose the following C++ classes to Python:

- **Vector3D**: 3D position vectors (CVectorD<3>)
- **NodeLink**: Weighted directed links between nodes (CNodeLink)
- **Node**: Semantic nodes with activation, position, and links (CNode)
- **Space**: Container managing the node network (CSpace)

### Test Structure

```
tests/
├── __init__.py          # Test package marker
├── conftest.py          # Pytest configuration and shared fixtures
├── test_vector3d.py     # Tests for Vector3D class
├── test_node.py         # Tests for Node class
├── test_space.py        # Tests for Space class
└── (future test files)
```

## Setup Instructions

### 1. Build the Project with CMake

First, ensure you have Python 3.x installed and visible to CMake.

```bash
# Configure the build (from the src directory)
cd src
cmake --preset x64-debug

# Build the project (this will fetch pybind11 and build the Python module)
cmake --build out/build/x64-debug
```

The Python module will be built to: `src/out/build/x64-debug/python/pythewheel.pyd` (Windows) or `pythewheel.so` (Linux/Mac)

### 2. Install Python Testing Dependencies

```bash
# From the repository root
pip install -r requirements.txt
```

This installs:
- pytest (testing framework)
- pytest-cov (coverage reporting)
- pytest-mock (mocking utilities)

### 3. Run Tests

```bash
# Run all tests
pytest

# Run with verbose output
pytest -v

# Run specific test file
pytest tests/test_node.py

# Run tests by marker
pytest -m unit           # Only unit tests
pytest -m integration    # Only integration tests
pytest -m activation     # Only activation tests

# Run with coverage report
pytest --cov=pythewheel --cov-report=html
```

## Test Categories

Tests are organized using pytest markers:

- **@pytest.mark.unit**: Unit tests for individual components
- **@pytest.mark.integration**: Integration tests across multiple components
- **@pytest.mark.activation**: Tests specifically for activation spreading
- **@pytest.mark.layout**: Tests for spatial layout algorithms
- **@pytest.mark.slow**: Long-running tests

## Writing New Tests

### Basic Test Structure

```python
import pytest
import pythewheel

@pytest.mark.unit
def test_my_feature():
    """Test description"""
    node = pythewheel.Node()
    node.set_name("Test")
    assert node.get_name() == "Test"
```

### Using Fixtures

```python
@pytest.fixture
def space():
    """Create a Space for testing"""
    return pythewheel.Space()

def test_with_space(space):
    """This test automatically gets a fresh Space"""
    node = pythewheel.Node(space, "Test", "Description")
    space.add_node(node, None)
    assert space.get_node_count() > 0
```

## Example Test Cases

### Test Node Creation and Attributes

```python
def test_node_attributes():
    space = pythewheel.Space()
    node = pythewheel.Node(space, "Concept", "A test concept")

    assert node.get_name() == "Concept"
    assert node.get_description() == "A test concept"

    node.set_class("Category")
    assert node.get_class() == "Category"
```

### Test Node Linking

```python
def test_node_linking():
    space = pythewheel.Space()
    node1 = pythewheel.Node(space, "Node1", "First")
    node2 = pythewheel.Node(space, "Node2", "Second")

    # Create weighted link
    node1.link_to(node2, 0.75)

    # Verify link exists
    assert node1.get_link_count() > 0
    assert node1.get_link_weight(node2) == 0.75
```

### Test Activation Spreading

```python
def test_activation():
    space = pythewheel.Space()
    node1 = pythewheel.Node(space, "Active", "Active node")

    # Set activation
    node1.set_activation(1.0)
    assert node1.get_activation() == 1.0

    # Set primary/secondary
    node1.set_primary_activation(0.7)
    node1.set_secondary_activation(0.3)

    assert node1.get_primary_activation() == 0.7
    assert node1.get_secondary_activation() == 0.3
```

## Troubleshooting

### Module Not Found Error

If you get `ImportError: No module named 'pythewheel'`:

1. Ensure you've built the project with CMake
2. Check that the `.pyd` or `.so` file exists in the build output directory
3. The `conftest.py` should automatically find it, but you can manually add the path:

```python
import sys
sys.path.insert(0, r'C:\dev\theWheel\src\out\build\x64-debug\python')
import pythewheel
```

### Build Errors

If the Python module fails to build:

1. Ensure Python development headers are installed
2. Check that CMake found Python: look for "Found Python" in CMake output
3. Verify pybind11 was fetched: check for "pybind11" in the build output
4. On Windows, ensure you're using the correct Visual Studio toolset

### MFC Dependency Issues

The theWheelModel library depends on MFC. If you encounter MFC-related errors:

1. Ensure Visual Studio is installed with MFC components
2. The CMake configuration should handle this automatically for MSVC
3. Some functions using MFC-specific types (like CDib) may not be fully exposed to Python

## CI/CD Integration

To integrate these tests into a CI/CD pipeline:

```yaml
# Example GitHub Actions workflow
- name: Build project
  run: |
    cd src
    cmake --preset x64-debug
    cmake --build out/build/x64-debug

- name: Install test dependencies
  run: pip install -r requirements.txt

- name: Run tests
  run: pytest -v --cov=pythewheel --cov-report=xml
```

## Future Enhancements

Planned improvements to the testing infrastructure:

- [ ] Add tests for CSpaceLayoutManager (layout optimization)
- [ ] Add tests for activation propagation through networks
- [ ] Performance benchmarks using pytest-benchmark
- [ ] Mock the MFC dependencies to enable cross-platform testing
- [ ] Add property-based testing with Hypothesis
- [ ] Integration with theWheelView rendering tests
- [ ] Automated regression testing with saved .spx files

## Contributing

When adding new Python bindings:

1. Update `src/pythewheel/bindings.cpp` with new class bindings
2. Add corresponding test file in `tests/`
3. Update this documentation
4. Ensure all tests pass before submitting PR

## References

- [pybind11 Documentation](https://pybind11.readthedocs.io/)
- [pytest Documentation](https://docs.pytest.org/)
- [theWheel Architecture](CLAUDE.md)

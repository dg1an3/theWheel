# Testing Guide

This document describes the testing infrastructure for theWheel, covering both the C++ Google Test suite and the Python pytest suite.

## Quick Start

### C++ Tests (Google Test)

```bash
# Build and run (either platform)
cmake --preset x64-debug          # or macos-debug
cmake --build build/x64-debug
cd build/x64-debug && ctest --output-on-failure
```

### Python Tests (pytest)

```bash
# Build the Python module first (see C++ step above)
pip install -r requirements.txt
python scripts/run_tests.py       # or: pytest -v
```

---

## C++ Tests (theWheelModelTests)

The primary test suite uses Google Test (v1.14.0, fetched via CMake FetchContent) and covers the core model layer.

### Test Files

| File | Tests | Coverage |
|------|-------|----------|
| `test_vectord.cpp` | 27 | `CVectorD<N>` construction, arithmetic, normalization |
| `test_node.cpp` | 25 | `CNode` construction, naming, linking, activation, position |
| `test_nodelink.cpp` | 11 | `CNodeLink` weight, target, stabilizer flag |
| `test_space.cpp` | 13 | `CSpace` node management, hierarchy, activation |
| `test_space_layout_manager.cpp` | 9 | `CSpaceLayoutManager` energy minimization, layout |
| **Total** | **85** | |

Source: [src/theWheelModelTests/](src/theWheelModelTests/)

### Running C++ Tests

```bash
# Full build + test (Windows)
cmake --preset x64-debug
cmake --build build/x64-debug
cd build/x64-debug && ctest --output-on-failure

# Full build + test (macOS)
cmake --preset macos-debug
cmake --build build/macos-debug
cd build/macos-debug && ctest --output-on-failure

# Run the test executable directly for verbose output
./build/x64-debug/src/theWheelModelTests/theWheelModelTests
```

### Adding C++ Tests

1. Add test cases to an existing `test_*.cpp` file, or create a new one
2. If creating a new file, add it to `TEST_SOURCES` in [src/theWheelModelTests/CMakeLists.txt](src/theWheelModelTests/CMakeLists.txt)
3. Tests are auto-discovered by CTest via `gtest_discover_tests()`

---

## Python Tests (pytest + pythewheel)

Python bindings (`pythewheel`) expose core C++ classes via pybind11, enabling testing with pytest.

### Exposed Classes

| Python Class | C++ Class | Description |
|--------------|-----------|-------------|
| `Vector3D` | `CVectorD<3>` | 3D position vectors |
| `NodeLink` | `CNodeLink` | Weighted links between nodes |
| `Node` | `CNode` | Semantic nodes with activation |
| `Space` | `CSpace` | Container managing node networks |

Bindings source: [src/pybind/bindings_simple.cpp](src/pybind/bindings_simple.cpp)

### Test Files

```
tests/
├── conftest.py          # Auto-locates pythewheel module, shared fixtures
├── test_vector3d.py     # Vector math operations
├── test_node.py         # Node creation, linking, activation
└── test_space.py        # Space management, network operations
```

### Setup

**Prerequisites**: Python 3.7+, CMake 3.10+, Visual Studio with MFC (Windows) or wxWidgets (macOS)

```bash
# 1. Build the Python module
cmake --preset x64-debug
cmake --build build/x64-debug

# 2. Install Python test dependencies
pip install -r requirements.txt
```

The build creates `pythewheel.pyd` (Windows) or `pythewheel.so` (macOS) in `build/<preset>/python/`.

### Running Python Tests

```bash
# Convenience script (auto-locates module)
python scripts/run_tests.py

# Or run pytest directly
pytest -v
pytest tests/test_node.py -v        # specific file
pytest -m unit                       # by marker
pytest -m integration                # by marker
pytest -m activation                 # by marker
pytest --cov=pythewheel --cov-report=html   # with coverage
```

### Test Markers

Defined in [pytest.ini](pytest.ini):

| Marker | Purpose |
|--------|---------|
| `@pytest.mark.unit` | Unit tests for individual components |
| `@pytest.mark.integration` | Integration tests across components |
| `@pytest.mark.activation` | Activation spreading tests |
| `@pytest.mark.layout` | Spatial layout tests |
| `@pytest.mark.slow` | Long-running tests |

### Writing Python Tests

```python
import pytest
import pythewheel

@pytest.mark.unit
def test_node_attributes():
    space = pythewheel.Space()
    node = pythewheel.Node(space, "Concept", "A test concept")
    assert node.get_name() == "Concept"
    assert node.get_description() == "A test concept"

@pytest.mark.activation
def test_activation():
    space = pythewheel.Space()
    node = pythewheel.Node(space, "Active", "Active node")
    node.set_activation(1.0)
    assert node.get_activation() == 1.0
```

Shared fixtures are defined in [tests/conftest.py](tests/conftest.py).

---

## Troubleshooting

### `ImportError: No module named 'pythewheel'`
1. Build the project with CMake first
2. Verify the `.pyd`/`.so` file exists in `build/<preset>/python/`
3. The `conftest.py` auto-adds the build path; if that fails, set `PYTHONPATH` manually

### Python module link error (`__imp__CrtDbgReportW`)
This is a debug/release CRT mismatch. Python extensions require `/MD` (release CRT), but MFC debug builds use `/MDd`. **Workaround**: build with a release preset, or use the C++ test suite for debug builds.

### Build fails with "Python not found"
Ensure Python is in your PATH and CMake can find it (look for "Found Python" in CMake output).

### MFC-related errors
Ensure Visual Studio is installed with MFC components. The CMake configuration handles `_AFXDLL` automatically for MSVC.

---

## Known Limitations

- **Simplified bindings**: `bindings_simple.cpp` exposes core functionality but not all `DECLARE_ATTRIBUTE`-generated methods (can be added with lambdas)
- **Debug builds**: Python module only works with release builds due to CRT mismatch
- **CDib/Images**: Image functionality not exposed to Python (MFC-dependent)
- **Windows-only for Python**: pybind module requires MFC; macOS port not yet supported for pythewheel

---

## CI/CD Integration

```yaml
# Example GitHub Actions workflow
- name: Build project
  run: |
    cmake --preset x64-debug
    cmake --build build/x64-debug

- name: Run C++ tests
  run: cd build/x64-debug && ctest --output-on-failure

- name: Install Python test dependencies
  run: pip install -r requirements.txt

- name: Run Python tests
  run: pytest -v --cov=pythewheel --cov-report=xml
```

# Building theWheel with CMake

This document describes how to build THEWHEEL_MODEL and its Python bindings using CMake.

## Prerequisites

### Required
- **CMake** 3.15 or higher
- **C++ Compiler** with C++11 support:
  - Windows: Visual Studio 2017 or higher
  - Linux: GCC 7+ or Clang 5+
  - macOS: Xcode 10+ or Clang 5+
- **Python** 3.7 or higher (for Python bindings)

### Optional
- **Intel IPP** (Intel Integrated Performance Primitives) for optimized math operations
- **pybind11** (will be automatically downloaded if not found)

## Quick Start

### Windows with Visual Studio

```powershell
# Navigate to the theWheel_antiques directory
cd c:\dev\DLaneAtElekta\theWheel_antiques

# Create build directory
mkdir build
cd build

# Configure the project
cmake .. -DBUILD_PYTHON_BINDINGS=ON

# Build (Release configuration recommended)
cmake --build . --config Release

# The Python module will be at:
# build\python_bindings\Release\thewheel.pyd
```

### Linux/macOS

```bash
# Navigate to the theWheel_antiques directory
cd /path/to/theWheel_antiques

# Create build directory
mkdir build
cd build

# Configure the project
cmake .. -DBUILD_PYTHON_BINDINGS=ON -DCMAKE_BUILD_TYPE=Release

# Build (use -j for parallel build)
cmake --build . -j$(nproc)

# The Python module will be at:
# build/python_bindings/thewheel.so
```

## CMake Build Options

You can customize the build with these options:

```bash
cmake .. -DOPTION_NAME=VALUE
```

Available options:

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_PYTHON_BINDINGS` | `ON` | Build Python bindings using pybind11 |
| `USE_INTEL_IPP` | `OFF` | Enable Intel IPP optimizations in MTL |
| `BUILD_TESTS` | `OFF` | Build test projects (not yet implemented) |
| `CMAKE_BUILD_TYPE` | `Release` | Build type: Debug, Release, RelWithDebInfo, MinSizeRel |

### Examples

**Build without Python bindings:**
```bash
cmake .. -DBUILD_PYTHON_BINDINGS=OFF
```

**Build with Intel IPP support:**
```bash
cmake .. -DUSE_INTEL_IPP=ON -DIPP_ROOT=/path/to/ipp
```

**Debug build:**
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

## Project Structure

The CMake build system includes these components:

```
theWheel_antiques/
├── CMakeLists.txt              # Root build configuration
├── XMLLogging/
│   └── CMakeLists.txt          # XML logging library
├── MTL/
│   └── CMakeLists.txt          # Math Template Library (header-only)
├── OptimizeN/
│   └── CMakeLists.txt          # Optimization algorithms
├── THEWHEEL_MODEL/
│   └── CMakeLists.txt          # Core semantic network model
└── python_bindings/
    ├── CMakeLists.txt          # Python bindings configuration
    ├── thewheel_bindings.cpp   # pybind11 wrapper code
    ├── example.py              # Example usage
    └── README.md               # Python bindings documentation
```

## Dependency Order

The build system automatically handles dependencies in this order:

1. **XMLLogging** - Basic logging library (no dependencies)
2. **MTL** - Math Template Library (no dependencies, header-only)
3. **OPTIMIZER_BASE** - Depends on: MTL, XMLLogging
4. **THEWHEEL_MODEL** - Depends on: OPTIMIZER_BASE, MTL, XMLLogging
5. **Python bindings** - Depends on: THEWHEEL_MODEL and all above

## Troubleshooting

### MFC Dependencies

The original code uses MFC (Microsoft Foundation Classes). For Python bindings, we avoid MFC dependencies by not using the `USE_MFC` option. If you need full MFC support for the original Windows GUI, you can enable it:

```bash
cmake .. -DUSE_MFC=ON
```

Note: This requires a Visual Studio installation with MFC components.

### pybind11 Not Found

If CMake can't find pybind11, it will automatically download it using FetchContent. If you want to use a specific version:

```bash
cmake .. -Dpybind11_DIR=/path/to/pybind11/share/cmake/pybind11
```

### DirectSound on Windows

THEWHEEL_MODEL uses DirectSound for audio. On Windows, the build system automatically links `dsound.lib` and `dxguid.lib`. These are included with the Windows SDK.

### Intel IPP

To use Intel IPP optimizations:

1. Install Intel IPP from: https://www.intel.com/content/www/us/en/developer/tools/oneapi/ipp.html
2. Set the `IPPROOT` environment variable or pass it to CMake:
   ```bash
   cmake .. -DUSE_INTEL_IPP=ON -DIPP_ROOT="C:/Program Files (x86)/Intel/oneAPI/ipp/latest"
   ```

## Testing the Build

After building, test the Python bindings:

```bash
# From the build directory
cd python_bindings

# Run the example (Windows)
python ..\python_bindings\example.py

# Run the example (Linux/macOS)
python ../python_bindings/example.py
```

## Installing

To install the Python module to your Python environment:

```bash
# From the build directory
cmake --install python_bindings

# Or manually copy the module file:
# Windows: copy python_bindings\Release\thewheel.pyd to your Python site-packages
# Linux/macOS: cp python_bindings/thewheel.so to your Python site-packages
```

## Build Artifacts

After a successful build, you'll have:

| Component | Location | Type |
|-----------|----------|------|
| XMLLogging | `build/XMLLogging/[Debug\|Release]/XMLLogging.lib` | Static library |
| MTL | Headers only | Interface library |
| OPTIMIZER_BASE | `build/OptimizeN/[Debug\|Release]/OPTIMIZER_BASE.lib` | Static library |
| THEWHEEL_MODEL | `build/THEWHEEL_MODEL/[Debug\|Release]/THEWHEEL_MODEL.lib` | Static library |
| Python module | `build/python_bindings/[Debug\|Release]/thewheel.pyd` (Windows)<br>`build/python_bindings/thewheel.so` (Linux/macOS) | Python extension |

## Next Steps

1. Build the project following the instructions above
2. Test the Python bindings with [example.py](python_bindings/example.py)
3. Read the [Python bindings documentation](python_bindings/README.md)
4. Explore the core model in [CLAUDE.md](CLAUDE.md)

## Known Issues

1. **MFC Dependency**: The original code uses MFC for serialization. The Python bindings work around this, but some features may not be available.

2. **Character Sets**: The original code uses multi-byte character sets (MBCS). String conversions are handled in the Python bindings.

3. **DirectSound**: Audio features (WAVE files, DirectSound) are Windows-specific and may not work on other platforms.

4. **Legacy Code**: This is a Visual Studio 2005-era codebase. Some modern compiler warnings are expected and suppressed with `_CRT_SECURE_NO_WARNINGS`.

## Support

For issues or questions:
- Check the main [CLAUDE.md](CLAUDE.md) for architecture details
- Review the [Python bindings README](python_bindings/README.md)
- Ensure all prerequisites are installed
- Try building in Debug mode for more diagnostic information

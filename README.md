# theWheel

**Interactive Knowledge Browser with Spreading Activation**

theWheel is a sophisticated semantic visualization and knowledge organization system that uses spreading activation networks to dynamically display and organize information in an interactive 3D space. It enables intuitive semantic browsing through force-directed graph layouts and weighted link propagation.

## Overview

theWheel represents knowledge as a network of **nodes** (information units) connected by **weighted links**. As users interact with nodes, **activation values** spread through the network, causing related content to emerge and less relevant information to fade. This creates a dynamic, context-sensitive visualization that adapts to user focus.

### Key Features

- **Spreading Activation Network**: Information propagates through weighted connections with configurable decay
- **Physics-Based Layout**: Force-directed graph positioning using energy minimization and spring forces
- **3D Visualization**: DirectX 9 (Windows) and wxWidgets/Modern GL (macOS/Cross-platform) rendering
- **Web Interface**: Modern React/Three.js web application for browser-based exploration
- **Cross-Platform Support**: Native Windows (MFC) and macOS (wxWidgets) desktop clients
- **Integrated Help**: Comprehensive documentation via MkDocs static site generator
- **Mature Algorithms**: 20+ years of refinement in semantic browsing and visualization

## Repository Structure

```
theWheel/
├── src/                    # C++ implementations (primary codebase)
│   ├── theWheel/          # MFC desktop application (Windows)
│   ├── theWheelWx/        # wxWidgets desktop application (macOS/Cross-platform)
│   ├── theWheelModel/     # Core data model and spreading activation logic
│   ├── theWheelView/      # DirectX 9 rendering layer
│   ├── OptimizeN/         # Multi-dimensional optimization library
│   ├── pybind/            # Python bindings (pythewheel) via pybind11
│   └── theWheelModelTests/ # C++ Google Test unit tests
│
├── weel-app/              # React/TypeScript/Three.js web application
│
├── pyweel/                # Legacy Python implementation (pure Python)
│
├── tests/                 # Python pytest suite for pybind11 bindings
│
├── scripts/               # Automation scripts (testing, .spx to Markdown)
│
├── data/                  # Sample serialized space (.spx) files
│
├── docs/                  # Documentation, ontologies, and MkDocs source
│
├── antiques/              # Historical source code and legacy projects
│
├── CLAUDE.md              # AI assistant development guide
├── TESTING.md             # Comprehensive testing documentation
├── BRANCH_SUMMARY.md      # Recent development and branch status
└── README.md              # This file
```

## Quick Start

### Modern Build (CMake) - Recommended

**Prerequisites:**
- CMake 3.15+
- Visual Studio 2022 (Windows) or Xcode/wxWidgets (macOS)
- Python 3.7+ (for bindings)
- vcpkg (optional, provides ANGLE for OpenGL ES rendering on macOS)

**macOS setup:**
```bash
brew install wxwidgets cmake

# Optional: install vcpkg for ANGLE (OpenGL ES renderer)
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh
export VCPKG_ROOT=~/vcpkg  # add to ~/.zshrc for persistence
```

**Build:**
```bash
# Using CMake presets
cmake --preset x64-debug          # Windows
cmake --preset macos-debug        # macOS

# Build all targets
cmake --build build/x64-debug     # Windows
cmake --build build/macos-debug   # macOS

# Run tests
build/macos-debug/src/theWheelModelTests/theWheelModelTests

# Launch the macOS app
open build/macos-debug/src/theWheelWx/theWheelWx.app
```

**Note:** On macOS without vcpkg, the theWheelGL (ANGLE) target is skipped automatically. The wxWidgets app still builds and runs using 2D rendering.

### Desktop Applications

- **Windows (MFC)**: Open `src/theWheel_src.sln` or build via CMake (target `theWheel`).
- **macOS/Cross-platform (wxWidgets)**: Build via CMake (target `theWheelWx`).

### Web Application (React)

```bash
cd weel-app
npm install
npm start
```

### Python Bindings (pythewheel)

The modern Python interface uses `pybind11` to wrap the C++ core.

```bash
# Build with CMake first, then:
pip install -r requirements.txt
python scripts/run_tests.py
```

### Documentation (MkDocs)

```bash
pip install mkdocs-material
mkdocs serve
```

## Architecture Highlights

### Core Model (C++)
- **CNode**: Semantic node with activation, position, links, and metadata.
- **CSpace**: Container managing node hierarchy and activation propagation.
- **CSpaceLayoutManager**: Physics-based optimization using Powell/ConjGrad algorithms.
- **CVectorD/CVectorN**: Template-based linear algebra library.

### Rendering Layers
- **MFC/DirectX 9**: Legacy high-performance rendering for Windows.
- **wxWidgets**: Cross-platform GUI and modern rendering for macOS/Linux.
- **Three.js**: Interactive 3D visualization for the web frontend.

### Numerical Optimization
- **OptimizeN**: Framework for multi-dimensional energy minimization.
- **CPowellOptimizer**: Powell's direction set method (primary algorithm).
- **CConjGradOptimizer**: Conjugate gradient descent.

## Technology Stack

| Component | Technologies |
|-----------|-------------|
| **C++ Desktop (Win)** | C++17, MFC, DirectX 9, Visual Studio |
| **C++ Desktop (macOS)** | C++17, wxWidgets, OpenGL |
| **Web Frontend** | React 19, Three.js, TypeScript |
| **Python Interface** | pybind11, pytest, NumPy |
| **Build System** | CMake 3.15+, MSBuild, VCPKG |
| **Documentation** | MkDocs, Material for MkDocs, Markdown |

## Documentation & Help

- **[CLAUDE.md](CLAUDE.md)** - Comprehensive developer guide for AI assistants and contributors.
- **[TESTING.md](TESTING.md)** - Detailed guide for running C++ and Python test suites.
- **[BRANCH_SUMMARY.md](BRANCH_SUMMARY.md)** - Overview of recent activity and repository structure.
- **[docs/](docs/)** - Project documentation and reference materials.

## Testing

theWheel includes a robust testing infrastructure:
- **C++ Tests**: Google Test suite in `src/theWheelModelTests/`.
- **Python Tests**: pytest suite in `tests/` validating `pybind11` bindings.

See **[TESTING.md](TESTING.md)** for detailed instructions on running and writing tests.

## History

theWheel has been in continuous development since approximately 2000, with core algorithms refined over two decades.

**Key Milestones:**
- **2000-2007**: Initial C++ implementation with DirectX and MFC.
- **2008-2015**: .NET web services and ActiveX controls (AxWheel).
- **2015-2020**: Pure Python reimplementation (pyweel).
- **2021-2024**: Modernization, CMake integration, and Python bindings (pybind11).
- **2025-2026**: wxWidgets macOS port, React/Three.js web app, MkDocs integration, and repository reorganization.

## License

Copyright 1996-2007, 2025-2026 Derek Graham Lane. All rights reserved.

*Note: U.S. Patent pending (as noted in source code)*

## Author

**Derek Graham Lane**
Email: dg1an3@users.github.com
GitHub: https://github.com/dg1an3/theWheel

---

**Status**: Active development with modern C++ and web technologies. Desktop clients are functional for Windows and macOS.

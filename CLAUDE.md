# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**theWheel** is a semantic visualization and knowledge organization system built around a spreading activation network. It dynamically displays nodes of information in 3D space, where activation values propagate through weighted links to enable semantic browsing and exploration. Development spans from ~2000 to present, with recent modernization efforts.

## Build Commands

### Visual Studio Build (Primary Method)

Open and build the main solution:
```bash
# Open in Visual Studio
start theWheel_src.sln

# Or build from command line using MSBuild
msbuild theWheel_src.sln /p:Configuration=Debug /p:Platform=Win32
msbuild theWheel_src.sln /p:Configuration=Release /p:Platform=Win32
```

**Build order** (dependencies are configured correctly in solution):
1. OptimizeN (static library)
2. theWheelModel (static library, depends on OptimizeN)
3. theWheelView (static library, depends on theWheelModel)
4. theWheel (executable, depends on all above)

### CMake Build (Cross-Platform)

The CMake build system supports both Windows and macOS:

```bash
# Windows (Visual Studio)
cmake --preset x64-debug
cmake --build out/build/x64-debug

# macOS
cmake --preset macos-debug
cmake --build out/build/macos-debug

# Run tests (either platform)
cd src/out/build/macos-debug  # or x64-debug
ctest --output-on-failure
```

### macOS Build (wxWidgets)

Prerequisites:
```bash
brew install wxwidgets cmake
```

Build and run:
```bash
cd src
cmake --preset macos-debug
cmake --build out/build/macos-debug
# Launch the app
open out/build/macos-debug/theWheelWx/theWheelWx.app
```

The macOS build produces:
- **OptimizeN** and **theWheelModel** static libraries (fully portable)
- **theWheelModelTests** (85 Google Test unit tests)
- **theWheelWx** (wxWidgets GUI application with 2D rendering)

**Note**: The Windows build also produces theWheelView (DirectX 9), theWheel (MFC app), and pythewheel (Python bindings) which are not yet ported to macOS.

## High-Level Architecture

### Four-Layer Structure

```
theWheel (MFC Application)     theWheelWx (wxWidgets Application)
    ↓                               ↓
theWheelView (DirectX 9)       [2D wxDC Rendering]
    ↓                               ↓
theWheelModel (Core Data & Logic)
    ↓
OptimizeN (Numerical Optimization)
```

### Core Concepts

#### theWheelModel - Data Layer
- **CNode**: Semantic node with activation values, hierarchical parent/child relationships, weighted links to other nodes, 3D position (CVectorD<3>), and metadata (name, description, image)
- **CSpace**: Container managing the node network hierarchy, activation propagation, layout optimization via CSpaceLayoutManager
- **CNodeLink**: Directed weighted edge with gain property; propagates activation between nodes
- **CSpaceLayoutManager**: Physics-based force-directed layout using energy minimization (extends CObjectiveFunction)
- **Spreading Activation**: Nodes propagate activation through weighted links with decay over time; supports primary and secondary activation values

#### theWheelView - Rendering Layer
- **CSpaceView**: Main DirectX 9 rendering window; manages device, creates CNodeView instances, handles mouse interaction
- **CNodeView**: Visual representation of a CNode with spring-based smooth animation, dynamic sizing based on activation
- **NodeViewSkin**: Manages visual appearance by creating and caching 3D "plaque" meshes at different activation levels
- **Plaque**: 3D mesh geometry based on RadialShape (parametric curves); creates Direct3D vertex/index buffers
- **Elliptangle**: Custom radial shape combining ellipse and rectangle properties with parametric evaluation
- **Physics-Based Animation**: Spring forces provide smooth position/size transitions; damping prevents oscillation

#### OptimizeN - Optimization Layer
- **COptimizer**: Abstract base for optimization algorithms (Powell, Conjugate Gradient, Brent)
- **CObjectiveFunction**: Interface for functions to minimize; used by CSpaceLayoutManager for layout energy
- **CVectorN/CMatrixNxM**: Dynamic-size linear algebra types
- **VectorOps.h**: Low-level vector operations with optional Intel IPP acceleration

### Key Patterns

1. **Model-View Separation**: Clear boundary between theWheelModel (no view dependencies) and theWheelView (holds pointers to model objects)
2. **Force-Directed Layout**: Energy minimization using spring attractions between linked nodes and repulsion between all nodes
3. **Level-of-Detail Rendering**: NodeViewSkin caches plaque meshes at discrete activation levels for performance
4. **Macro-Based Properties**: DECLARE_ATTRIBUTE macros generate consistent getter/setter methods throughout model classes
5. **Template Math Library**: CVectorD<DIM, TYPE> for fixed-dimension and CVectorN<TYPE> for dynamic-dimension vectors

## Technology Stack

- **Language**: C++ (C++17, CMake configured for C++17)
- **Platforms**: Windows Win32, macOS (via wxWidgets)
- **UI Framework (Windows)**: MFC (Microsoft Foundation Classes) with Document/View architecture
- **UI Framework (macOS)**: wxWidgets 3.3+ with 2D wxDC rendering
- **Graphics (Windows)**: DirectX 9 (D3D9, requires June 2010 DirectX SDK)
- **Graphics (macOS)**: wxWidgets 2D rendering (wxPaintDC, wxBufferedPaintDC)
- **Build Tools**: Visual Studio 2017/2022, CMake 3.10+, Ninja/Make
- **Cross-Platform**: `mfc_compat.h` provides MFC type stubs for non-MSVC platforms
- **Optional Libraries**: Intel IPP (for optimized vector operations, Windows only)

## Project Structure

```
src/
├── theWheel/           # MFC application (Document/View) [Windows only]
├── theWheelWx/         # wxWidgets application [macOS/Linux]
│   ├── WheelApp.cpp/h        # wxApp + wxFrame (main window)
│   ├── SpacePanel.cpp/h      # 2D rendering panel (wxPaintDC)
│   ├── SpaceTreeView.cpp/h   # Node hierarchy tree (wxTreeCtrl)
│   ├── PropertyDialogs.cpp/h # Node/Space property dialogs
│   ├── SpaceSerializer.cpp/h # JSON file I/O
│   ├── Spring.h               # Standalone spring physics (port)
│   └── wxcompat.h             # Force-include for mfc_compat types
├── theWheelModel/      # Core data model and activation logic
│   ├── include/        # Public headers (Node.h, Space.h, etc.)
│   └── *.cpp           # Implementation files
├── theWheelView/       # DirectX 9 rendering layer [Windows only]
│   ├── include/        # Public headers (SpaceView.h, NodeView.h, etc.)
│   └── *.cpp           # Rendering implementation
├── OptimizeN/          # Numerical optimization library
│   ├── include/        # Public headers (Optimizer.h, VectorN.h, etc.)
│   └── *.cpp           # Optimization algorithms
├── OptimizeND/         # CMake test project
├── theWheelModelTests/ # Google Test suite (85 tests)
└── pythewheel/         # Python bindings via pybind11 [Windows only]
```

## Common Development Tasks

### Understanding Activation Propagation
The spreading activation system is central to theWheel's behavior:
- Start in `theWheelModel/Node.cpp` → `CNode::PropagateActivation()` and `CNode::UpdateActivation()`
- Activation flows through `CNodeLink` objects with weighted propagation
- Decay and thresholding prevent runaway activation

### Modifying Node Rendering
Visual appearance is controlled by the skin system:
- `theWheelView/NodeViewSkin.cpp` → `NodeViewSkin::Render()` determines which plaque to use
- `theWheelView/Plaque.cpp` → Creates DirectX 9 vertex/index buffers for 3D shapes
- `theWheelView/Elliptangle.cpp` → Defines the parametric shape equation
- See `node-view-skin-design.md` for detailed pseudo-code and design rationale

### Adding New Optimization Algorithms
The optimizer framework is extensible:
- Extend `COptimizer` base class in `OptimizeN/include/Optimizer.h`
- Implement `Iterate()` method for your algorithm
- See `PowellOptimizer.cpp` or `ConjGradOptimizer.cpp` as examples
- `CSpaceLayoutManager` will automatically use new optimizers

### Working with the Math Library
Custom template-based linear algebra:
- **Fixed-dimension vectors**: `CVectorD<DIM, TYPE>` for 2D/3D positions (stack-allocated)
- **Dynamic vectors**: `CVectorN<TYPE>` for arbitrary-size vectors (heap-allocated)
- **Extents/Bounding boxes**: `CExtent<DIM, TYPE>` for rectangular regions
- **Matrices**: `CMatrixNxM<TYPE>` for transformations
- See `OptimizeN/include/VectorOps.h` for low-level operations

### File Format
- **Extension**: `.spx` (serialized space files)
- **Serialization**: CSpace/CNode/CNodeLink implement MFC serialization
- Loaded via theWheel's Document/View architecture

## Important Notes

### DirectX 9 Dependency
- Rendering is tightly coupled to legacy Direct3D 9 API
- Requires June 2010 DirectX SDK installation
- Modernization to D3D11/12 or Vulkan would require substantial theWheelView rewrite

### MFC Dependency
- Heavy use of MFC classes (CWnd, CDocument, CView, etc.) limits cross-platform portability
- COM/ActiveX variants exist (AxWheel, AxWheelServer) but are legacy

### Refactoring Plans (from TODO.txt)
- Remove XMLLogging dependency (move to OptimizeN)
- Get rid of ModelObject base class
- Replace MTL (Matrix Template Library) with modern alternatives (e.g., Eigen)
- Move utility classes to appropriate layers (Observer.h → theWheelModel, CExtent → theWheelView)

### Macro System
The codebase uses DECLARE_ATTRIBUTE macros extensively for property getters/setters:
- `DECLARE_ATTRIBUTE_VAL(Type, Name)` → by-value properties
- `DECLARE_ATTRIBUTE_PTR(Type, Name)` → pointer properties
- `DECLARE_ATTRIBUTE_GI(Type, Name)` → separate implementation
These enable consistent serialization and reduce boilerplate.

## Historical Context

- **Copyright**: 1996-2007 (main codebase), with 2025 updates
- **Author**: Derek Graham Lane
- **Patent**: U.S. Patent pending notation in code
- **Evolution**: Started in 2000, modernization ongoing (CMake, git)
- **Related Projects**: See `.NET web service (theWeelNet), ActiveX controls, self-organizing maps (SOM), tag mapping (TagMap08)

## Architecture References

- `node-view-skin-design.md`: Comprehensive rendering design with pseudo-code for Plaque/Elliptangle/RadialShape classes
- `TODO.txt`: Current refactoring priorities and technical debt

## Running Tests

No formal unit test framework is currently integrated. Testing is primarily done through:
- Building and running the main theWheel.exe application
- Loading .spx test files
- Verifying visual rendering and activation behavior
- OptimizeND project may serve as a test harness for modernized components

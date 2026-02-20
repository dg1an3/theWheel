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

### CMake Build (In Progress)

A CMake build system is being added for modernization. Presets are defined in `CMakePresets.json` at the repo root:
```bash
# Configure using presets (from repo root)
cmake --preset x64-debug
cmake --preset x64-release
cmake --preset macos-debug

# Build
cmake --build build/x64-debug
cmake --build build/x64-release
cmake --build build/macos-debug
```

Build output goes to `build/<preset-name>/` at the repo root.

**Note**: CMake currently covers OptimizeN, theWheelModel, theWheelModelTests, and pybind (Python bindings). The main theWheel application and theWheelView still use Visual Studio .vcxproj files.

## High-Level Architecture

### Four-Layer Structure

```
theWheel (MFC Application)
    ↓
theWheelView (DirectX 9 Rendering)
    ↓
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

- **Language**: C++ (targeting C++14/17, with CMake configured for C++20)
- **Platform**: Windows Win32
- **UI Framework**: MFC (Microsoft Foundation Classes) with Document/View architecture
- **Graphics**: DirectX 9 (D3D9, requires June 2010 DirectX SDK)
- **Build Tools**: Visual Studio 2017/2022 (v143 toolset), MSBuild, CMake 3.8+
- **Optional Libraries**: Intel IPP (for optimized vector operations)

## Project Structure

```
src/
├── theWheel/           # MFC application (Document/View)
├── theWheelModel/      # Core data model and activation logic
│   ├── include/        # Public headers (Node.h, Space.h, etc.)
│   └── *.cpp           # Implementation files
├── theWheelView/       # DirectX 9 rendering layer
│   ├── include/        # Public headers (SpaceView.h, NodeView.h, etc.)
│   └── *.cpp           # Rendering implementation
├── OptimizeN/          # Numerical optimization library
│   ├── include/        # Public headers (Optimizer.h, VectorN.h, etc.)
│   └── *.cpp           # Optimization algorithms
├── node-view-skin-design.md  # Detailed rendering design doc
└── TODO.txt            # Refactoring plans
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

### File Format (.spx)

The `.spx` files are MFC CArchive binary archives containing serialized CNode networks.

**Binary Format Overview:**
- File may start with a DWORD CSpace schema prefix (if first bytes ≠ 0xFFFF) or directly with CNode class tag
- MFC CArchive object tracking: classes and objects share a single map with 1-based indices
  - `0x0000` = NULL pointer
  - `0xFFFF` = NEW_CLASS_TAG (schema WORD, name length WORD, name bytes, then object data)
  - `0x8000 | idx` = OLD_CLASS_TAG (existing class, new object instance)
  - Plain WORD `idx` = back-reference to existing object
  - `0x7FFF` = BIG_OBJECT_TAG (followed by DWORD index)
- CString: BYTE length (or 0xFF + WORD, or 0xFF 0xFFFF + DWORD), then char data (ANSI/latin-1)
- CObArray: WORD count (via WriteCount), then serialized CObject* entries

**CNode Schema Versions:**
- **Schema 5**: name, description, image_filename, URL + children + links
- **Schema 6**: + class_name field
- **Schema 7**: + primary/secondary activation (doubles), position (3 doubles), max activator (CObject*), max delta (double)
- **Schema 8**: + optSSV (CObject*, always NULL in practice)

**CNodeLink Schema Versions:**
- **Schema 1**: weight (float), target (CObject* ref)
- **Schema 2**: + IsStabilizer (BOOL, 4 bytes DWORD)

**Eevorg (Evolutionary Automaton):**
Two historical versions exist in .spx files, auto-detected by the parser:
- **Standalone** (1995, `Eevorg : CObject`): 3 WORDs (maxVal, maxRule, score) + CByteArray rules
- **CNode subclass** (theWheel era): CNode::Serialize at file schema, then 3 WORDs + CByteArray

**Python Parser:** `scripts/spx_parser.py` — pure Python MFC archive parser
- `parse_spx(filepath)` → returns root `SpxNode` tree
- Handles schemas 5-8, CNodeLink schemas 1-2, both Eevorg variants
- Tested against all 14 known .spx files (6 theWheel_antiques + 8 OneDrive/Timeline)

**Markdown Converter:** `scripts/spx_to_markdown.py` — converts parsed nodes to markdown
- `convert_spx_to_markdown(spx_path, output_dir)` → generates one .md per category + index.md

**Known .spx Files:**
- `C:\dev\DLaneAtElekta\theWheel_antiques\theWheel\data\` — 4 files (schemas 5-6)
- `C:\dev\DLaneAtElekta\theWheel_antiques\theWheel2001\data\` — 1 file (schema 5)
- `C:\dev\DLaneAtElekta\theWheel_antiques\THEWHEEL_MODEL\THEWHEEL_MODEL_Test\` — 1 file (schema 8)
- `C:\Users\Derek\OneDrive\Timeline\35 theWheel\S200310-02 DGL Projects 10-2003 ~ SOM2001_20030428\` — 8 files:
  - `data\DerekLane\Derek.spx` (62 nodes — personal ToDo categories)
  - `data\RT\Radiation Treatment.spx` (51 nodes — radiation therapy)
  - `data\RT\RT.spx` (63 nodes — radiation therapy)
  - `data\SonicSpace\SonicSpace-schema6.spx` (185 nodes — music genres)
  - `data\SonicSpace\SonicSpace.spx` (169 nodes — music genres)
  - `data\TheWell\TheWell.spx` (90 nodes — topics)
  - `data\Topics\Corpus.spx` (54 nodes — evolution topics)
  - `theWheel-2002-06\TheWell3.spx` (89 nodes — topics)

## Important Notes

### DirectX 9 Dependency
- Rendering is tightly coupled to legacy Direct3D 9 API
- Requires June 2010 DirectX SDK installation
- Modernization to D3D11/12 or Vulkan would require substantial theWheelView rewrite

### MFC Dependency
- Heavy use of MFC classes (CWnd, CDocument, CView, etc.) limits cross-platform portability
- COM/ActiveX variants exist (AxWheel, AxWheelServer) but are legacy

### Refactoring Plans
- Remove XMLLogging dependency (move XMLLogging.h and UtilMacro.h to OptimizeN)
- Get rid of ModelObject base class
- Remove GradDescOptimizer and DFPOptimizer from OptimizeN
- Move Observer.h to theWheelModel (used in Space.h)
- Move CExtent to theWheelView
- Move MathUtil.h, VectorD.h, VectorN.h, VectorOps.h to OptimizeN/include
- Replace MTL (Matrix Template Library) with modern alternatives (e.g., Eigen)
  - CMatrixNxM is used in SpaceStateVector (SVD rotation) and ObjectiveFunction (hessian)
  - CMatrixD is used for CMolding — check if D3DMATRIX can replace it, and whether CMolding is still needed with Plaque

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

- `docs/node-view-skin-design.md`: Comprehensive rendering design with pseudo-code for Plaque/Elliptangle/RadialShape classes

## Running Tests

No formal unit test framework is currently integrated. Testing is primarily done through:
- Building and running the main theWheel.exe application
- Loading .spx test files
- Verifying visual rendering and activation behavior
- theWheelModelTests (Google Test) provides unit test coverage for core model classes

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**theWheel_antiques** is a Visual Studio 2005 C++ project implementing a semantic network visualization system. It combines graph theory, neural-like activation spreading, physics-based graph layout, and advanced rendering to create an interactive knowledge navigation system. This codebase is from circa 1999-2002 and contains U.S. patent-pending technology.

## Build Configuration

### Main Solution
- **File**: `theWheel\theWheel.sln`
- **Format**: Visual Studio 2005 (VS 8.0)
- **Build Configurations**:
  - `Debug|Win32` - Debug build with full symbols
  - `Release|Win32` - Optimized release build
  - `Release_f|Win32` - Release build variant

### Build Command
```bash
# Open solution in Visual Studio or use MSBuild:
MSBuild theWheel\theWheel.sln /p:Configuration=Debug /p:Platform=Win32

# Or for Release:
MSBuild theWheel\theWheel.sln /p:Configuration=Release /p:Platform=Win32
```

### Dependencies
The main application depends on these static libraries (built in order):
1. **XMLLogging** - Structured XML logging
2. **MTL** - Math Template Library with Intel IPP optimizations
3. **OPTIMIZER_BASE** - Optimization algorithms framework
4. **THEWHEEL_MODEL** - Core semantic network model
5. **THEWHEEL_VIEW** - DirectDraw-based visualization
6. **THEWHEEL_TREEVIEW** - Hierarchical tree editor

### Test Projects
Individual modules have test projects:
- `GEOM_MODEL\TestRegion\Test_GEOM_MODEL.sln` - Tests for geometry model
- `MTL\TestMTL\TestMTL.sln` - Tests for math library
- `OptimizeN\Test\OPTIMIZER_BASE_Test.sln` - Tests for optimizers
- `THEWHEEL_VIEW\TestNodeRenderer\TestNodeRenderer.sln` - Tests for rendering

## Architecture

### Layered Module Structure

```
┌─────────────────────────────────────────────────────┐
│            theWheel (Main Application)              │
│            MFC Document-View Architecture            │
└────────────────┬────────────────────────────────────┘
                 │
    ┌────────────┴────────────┬──────────────┐
    │                         │              │
┌───▼──────────────┐  ┌──────▼─────────┐  ┌─▼────────────────┐
│ THEWHEEL_VIEW    │  │ THEWHEEL_      │  │ THEWHEEL_MODEL   │
│ (DirectDraw)     │  │ TREEVIEW       │  │ (Core Model)     │
│ - CSpaceView     │  │ (Tree Editor)  │  │ - CSpace         │
│ - CNodeView      │  │                │  │ - CNode          │
│ - Spring         │  │                │  │ - CNodeLink      │
└───┬──────────────┘  └────────────────┘  └─┬────────────────┘
    │                                        │
    │                                        │
┌───▼──────────────┐  ┌────────────────────▼─────────────────┐
│ OGL_BASE         │  │ OPTIMIZER_BASE                        │
│ GUI_BASE         │  │ - COptimizer (base)                   │
└──────────────────┘  │ - BrentOptimizer, ConjGradOptimizer   │
                      │ - DFPOptimizer, PowellOptimizer       │
                      └─┬─────────────────────────────────────┘
                        │
    ┌───────────────────┴──────────────────┬──────────────┐
    │                                      │              │
┌───▼──────────┐  ┌──────────────────┐  ┌─▼────────────┐
│ GEOM_MODEL   │  │ MTL              │  │ XMLLogging   │
│ (Geometry)   │  │ (Math Library)   │  │              │
└───┬──────────┘  └──────────────────┘  └──────────────┘
    │
┌───▼──────────┐
│ GEOM_BASE    │
│ (Vectors/    │
│  Matrices)   │
└──────────────┘
```

### Key Components

#### THEWHEEL_MODEL (Core Semantic Network)
- **CSpace**: Container for entire node graph
  - Manages activation propagation across nodes
  - Fires observable events (NodeAddedEvent, CurrentNodeChangedEvent, etc.)
  - Serialization support via MFC
  - Total activation normalization (default: 0.55)

- **CNode**: Individual semantic units
  - Hierarchical parent-child relationships
  - Weighted directed links to other nodes (CNodeLink)
  - Dual activation model: primary + secondary activation
  - 3D position in space
  - Associated media: images (Dib), sounds (WAVE)
  - Attributes: Name, Description, Class, ImageFilename

- **CNodeLink**: Directed weighted connections
  - Source/target nodes
  - Gain weight for activation spreading
  - Stabilizer flag

- **CSpaceLayoutManager**: Force-directed graph layout
  - Inherits from CObjectiveFunction
  - Uses OPTIMIZER_BASE for energy minimization
  - Configurable repulsion/attraction forces

#### THEWHEEL_VIEW (Visualization)
- **CSpaceView**: Main visualization coordinator
  - DirectDraw back-buffered rendering
  - Timer-driven animation loop
  - Manages CNodeView instances
  - Drag-and-drop support

- **CNodeView**: Individual node renderer
  - Spring-based smooth position animation
  - Text and image rendering
  - Pending activation visualization
  - Skin-based visual styling (NodeViewSkin)

#### OPTIMIZER_BASE (Optimization Framework)
Multiple optimization algorithms sharing common interface:
- **Powell's Method** - Conjugate direction search
- **Conjugate Gradient** - Gradient-based optimization
- **DFP (Davidon-Fletcher-Powell)** - Quasi-Newton method
- **Brent's Method** - Line search
- **Gradient Descent** - Simple gradient following

Used primarily for graph layout energy minimization.

#### MTL (Math Template Library)
- Dynamic vector/matrix operations (CVectorN, CMatrixNxM)
- Intel IPP acceleration when available
- Pseudo-inverse calculations
- Vector space operations

#### GEOM_BASE & GEOM_MODEL
- **GEOM_BASE**: Template-based geometry primitives
  - `CVector<DIM,TYPE>` - N-dimensional vectors
  - `Matrix` - Matrix operations
  - `Polygon`, `VectorN`, `ScalarFunction`

- **GEOM_MODEL**: Higher-level geometry
  - Mesh, Pyramid representations
  - Transformations: AffineTransform, TPSTransform (Thin-Plate Spline)
  - Clustering algorithms
  - Image utilities and resampling

### Design Patterns

1. **Observer Pattern** (Pervasive)
   - `CObservableEvent` enables loose coupling
   - Model changes trigger view updates automatically
   - Events: NodeAddedEvent, CurrentNodeChangedEvent, NodeAttributeChangedEvent

2. **Model-View Separation**
   - Model: THEWHEEL_MODEL (no view dependencies)
   - Views: THEWHEEL_VIEW, THEWHEEL_TREEVIEW (multiple views of same model)
   - Controller: theWheel main app coordinates

3. **Template-Based Generic Programming**
   - GEOM_BASE and MTL use templates for flexibility
   - Compile-time dimension and type parameterization

4. **Strategy Pattern**
   - Interchangeable optimizer implementations
   - Common COptimizer interface

5. **Physics Simulation**
   - CSpring for smooth visual transitions
   - Force-directed layout using energy minimization

### Data Flow: Activation Spreading

This is the core patent-pending algorithm:

```
1. User Action → ActivateNode(pNode, scale)
2. CSpace::ActivateNode sets node's primary activation
3. Propagation through weighted links:
   - For each CNodeLink from activated node
   - Target.SecondaryActivation += Source.PrimaryActivation * Link.Gain
4. Accumulate secondary activation across all incoming links
5. NormalizeNodes(TOTAL_ACTIVATION) normalizes sum
6. Observable events fired → views update
7. CSpaceView creates/updates CNodeView instances
8. Spring animation smoothly transitions node positions
9. DirectDraw renders to back buffer
10. Timer triggers next frame
```

### Data Flow: Graph Layout

```
1. Node positions stored in CNode (CVectorD m_vPos)
2. CSpaceLayoutManager builds objective function:
   - Repulsion: nodes push apart (inverse square law)
   - Attraction: links pull connected nodes together (spring)
   - Energy = sum of repulsion + attraction forces
3. Optimizer (Powell, ConjGrad, etc.) minimizes energy
4. Updated positions set on nodes
5. CNodeView.Spring provides smooth animation to new positions
6. Render loop displays smooth transition
```

## MFC Integration

- **Document**: `CtheWheelDoc` contains `CSpace` instance
- **View**: `CtheWheelView` coordinates multiple views
- **Serialization**: MFC `DECLARE_SERIAL`/`IMPLEMENT_SERIAL` throughout
- **Character Set**: Multi-byte (not Unicode)
- **MFC Usage**: Shared DLL (`UseOfMFC="2"`)

## Key Include Paths

Main application includes:
```
..\THEWHEEL_TREEVIEW\include
..\THEWHEEL_VIEW\include
..\THEWHEEL_MODEL\include
..\OptimizeN\include
..\MTL
```

## Common Coding Patterns

### Attribute Macros
```cpp
// In header:
DECLARE_ATTRIBUTE_GI(Name, CString);          // Get/Set with invalidation
DECLARE_ATTRIBUTE_PTR_GI(Parent, CNode);      // Pointer attribute

// Expands to:
CString GetName() const;
void SetName(const CString& value);
```

### Observable Events
```cpp
// In class declaration:
CObservableEvent NodeAddedEvent;

// Subscribe:
pSpace->NodeAddedEvent.AddObserver(this, &CMyView::OnNodeAdded);

// Fire event:
NodeAddedEvent.Fire();
```

### Serialization
```cpp
// In header:
DECLARE_SERIAL(CSpace)

// In implementation:
IMPLEMENT_SERIAL(CSpace, CObject, VERSIONABLE_SCHEMA | VERSION_NUMBER)

void CSpace::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        ar << m_strPathName;
        // ... store members
    } else {
        ar >> m_strPathName;
        // ... load members
    }
}
```

## Historical Context

- **Copyright**: 1999-2002 Derek Graham Lane
- **Patent Status**: U.S. Patent Pending (as of 2002)
- **Technology Stack**: MFC, DirectDraw, DirectSound, Intel IPP
- **Version Control**: Originally CVS (CVSROOT directory present)
- **Target Platform**: Windows 32-bit

## Migration Notes

When modernizing this codebase:
- Visual Studio 2005 projects need upgrade to modern VS
- DirectDraw deprecated (consider Direct2D, Direct3D, or cross-platform alternatives)
- MFC still supported but consider modern UI frameworks
- Intel IPP integration may need updates
- Character set: currently MBCS, could migrate to Unicode

# theWheel

**Interactive Knowledge Browser with Spreading Activation**

theWheel is a sophisticated semantic visualization and knowledge organization system that uses spreading activation networks to dynamically display and organize information in an interactive 3D space. It enables intuitive semantic browsing through force-directed graph layouts and weighted link propagation.

## Overview

theWheel represents knowledge as a network of **nodes** (information units) connected by **weighted links**. As users interact with nodes, **activation values** spread through the network, causing related content to emerge and less relevant information to fade. This creates a dynamic, context-sensitive visualization that adapts to user focus.

### Key Features

- **Spreading Activation Network**: Information propagates through weighted connections with configurable decay
- **Physics-Based Layout**: Force-directed graph positioning using energy minimization and spring forces
- **3D Visualization**: DirectX 9-powered rendering with dynamic node sizing and smooth animations
- **Hierarchical Organization**: Nodes maintain parent-child relationships for structured knowledge
- **Multi-Implementation**: Available as C++ desktop app, Python library, and web application
- **20+ Years of Development**: Mature algorithms refined from 2000 to present

## Repository Structure

```
theWheel/
├── src/                    # C++ implementation (primary codebase)
│   ├── theWheel/          # MFC desktop application
│   ├── theWheelModel/     # Core data model and spreading activation logic
│   ├── theWheelView/      # DirectX 9 rendering layer
│   ├── OptimizeN/         # Multi-dimensional optimization library
│   └── OptimizeND/        # CMake modernization test project
│
├── pyweel/                # Python implementation
│   ├── space.py          # Space container with activation management
│   ├── node.py           # Node class with propagation logic
│   ├── node_link.py      # Weighted link connections
│   └── space_layout_*.py # Layout optimization algorithms
│
├── weel-app/              # Node.js/web application (in development)
│
├── docs/                  # Documentation and ontologies
│
├── CLAUDE.md              # AI assistant development guide
└── README.md              # This file
```

## Quick Start

### C++ Desktop Application (Windows)

**Prerequisites:**
- Visual Studio 2017 or 2022
- DirectX 9 SDK (June 2010)
- Windows SDK

**Build:**
```bash
# Open solution in Visual Studio
cd src
start theWheel_src.sln

# Or build from command line
msbuild theWheel_src.sln /p:Configuration=Release /p:Platform=Win32
```

**Run:**
```bash
cd src/Release
theWheel.exe
```

### Python Library

**Install (development mode):**
```bash
pip install -e ./pyweel
```

**Usage:**
```python
from pyweel import Space, Node

# Create a space
space = Space()

# Add nodes and links
root = Node(name="Root Concept")
child = Node(name="Related Concept")
space.add_node(root)
space.add_node(child)

# Activate and propagate
root.set_primary_activation(1.0)
space.propagate_activation()
```

## Architecture Highlights

### theWheelModel (C++)
Core data structures and algorithms:
- **CNode**: Semantic node with activation, position, links, and metadata
- **CSpace**: Container managing node hierarchy and activation propagation
- **CSpaceLayoutManager**: Physics-based optimization using Powell/ConjGrad algorithms
- **CVectorD/CVectorN**: Template-based linear algebra library

### theWheelView (C++)
DirectX 9 rendering pipeline:
- **CSpaceView**: Main rendering window with device management
- **CNodeView**: Visual representation with spring-based animation
- **NodeViewSkin**: 3D "plaque" mesh generation with level-of-detail caching
- **Elliptangle**: Custom parametric shape combining ellipse and rectangle

### OptimizeN (C++)
Numerical optimization framework:
- **COptimizer**: Abstract base for optimization algorithms
- **CPowellOptimizer**: Powell's direction set method (primary algorithm)
- **CConjGradOptimizer**: Conjugate gradient descent
- **CObjectiveFunction**: Energy function interface for layout optimization

## Technology Stack

| Component | Technologies |
|-----------|-------------|
| **C++ Desktop** | C++14/17, MFC, DirectX 9, Visual Studio, CMake |
| **Python** | Python 3.x, NumPy (for optimization) |
| **Web** | Node.js, JavaScript (under development) |
| **Build Tools** | MSBuild, CMake 3.8+, Visual Studio 2017/2022 |

## Use Cases

- **Knowledge Management**: Organize personal notes, research, and documentation
- **Semantic Browsing**: Explore large information spaces through associative navigation
- **Research Visualization**: Map relationships between papers, concepts, and citations
- **Educational Tools**: Interactive concept mapping and learning paths
- **Content Discovery**: Surface related content based on user focus and context

## Development

### Contributing

See [CLAUDE.md](CLAUDE.md) for comprehensive development guidance including:
- Detailed architecture documentation
- Build system configuration
- Common development tasks
- Code organization and patterns
- API reference with file locations

### Build Configurations

**C++ (Visual Studio):**
- Debug|Win32 - Development with full symbols
- Release|Win32 - Optimized production build

**CMake (In Progress):**
- x64-debug / x64-release
- x86-debug / x86-release

### Testing

Currently, testing is primarily done through:
- Running the main theWheel.exe application
- Loading and interacting with .spx (serialized space) files
- Visual verification of rendering and activation behavior

## Documentation

- **[CLAUDE.md](CLAUDE.md)** - Comprehensive developer guide for AI assistants and new contributors
- **[node-view-skin-design.md](src/node-view-skin-design.md)** - Detailed rendering design with pseudo-code
- **[TODO.txt](src/TODO.txt)** - Current refactoring priorities and technical debt
- **[docs/](docs/)** - Additional documentation and ontologies

## History

theWheel has been in continuous development since approximately 2000, with the core spreading activation and physics-based layout algorithms refined over two decades. Recent efforts focus on modernization (CMake, cross-platform support) while preserving the mature visualization algorithms.

**Key Milestones:**
- **2000-2007**: Initial C++ implementation with DirectX and MFC
- **2008-2015**: .NET web services and ActiveX controls (AxWheel, theWeelNet)
- **2015-2020**: Python reimplementation for portability
- **2020-Present**: CMake integration, git workflow, web application development

## License

Copyright 1996-2007, 2025 Derek Graham Lane. All rights reserved.

*Note: U.S. Patent pending (as noted in source code)*

## Author

**Derek Graham Lane**
Email: dg1an3@users.github.com
GitHub: https://github.com/dg1an3/theWheel

## Related Projects

This repository is part of a larger ecosystem of medical imaging, AI/ML, and knowledge visualization projects developed by the author.

---

**Status**: Active development with ongoing modernization efforts. The C++ desktop application is mature and functional. Python and web implementations are in various stages of completion.

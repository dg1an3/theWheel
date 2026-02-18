# Branch Summary

**Generated**: 2025-11-07 | **Updated**: 2026-02-18
**Repository**: theWheel

## Overview

This repository has **8 local branches** (including main) with corresponding remote tracking branches. Development activity spans from February 2025 to February 2026, with the current focus on CMake build modernization and comprehensive testing infrastructure.

---

## Main Branch

### `main`
- **Last Updated**: 2025-11-07
- **Status**: Primary development branch
- **Latest Commit**: Merge pull request #11 from dg1an3/docs/claude-md
- **Description**: Stable branch with merged documentation improvements

---

## Active Development Branches

### `feature/cmake` ⭐ Most Recent (Current Branch)
- **Last Updated**: 2026-02-18
- **Status**: Active development - Major build modernization with testing infrastructure
- **Commits Ahead of Main**: 9 commits
- **Latest Commit**: Add unit tests for CNode, CNodeLink, CSpace, and CVectorD classes; implement Google Test framework integration
- **Key Changes**:
  - CMake build system for OptimizeN, theWheelModel, and OptimizeND
  - CMakePresets.json with x64-debug and x64-release presets
  - MSVC compatibility layer with `mfc_compat.h` for cross-platform builds
  - Per-target MSVC runtime library selection in CMake
  - Python bindings via pybind11 (`pythewheel` module with `bindings.cpp` and `bindings_simple.cpp`)
  - Comprehensive pytest-based test suite (test_vector3d, test_node, test_space)
  - Google Test (gtest) C++ unit tests for CNode, CNodeLink, CSpace, CVectorD
  - Model source modifications to support non-MFC builds (Node.cpp, NodeLink.cpp, Space.cpp)
- **Purpose**: Modernize build system with CMake, enable cross-platform compilation without MFC dependency, establish dual testing infrastructure (Python + C++)
- **Files Added/Modified** (43 files, +3616/-41 lines):
  - `src/CMakeLists.txt`, `src/CMakePresets.json`
  - `src/OptimizeN/CMakeLists.txt`, `src/OptimizeN/include/mfc_compat.h`
  - `src/OptimizeND/CMakeLists.txt`, `src/OptimizeND/OptimizeND.cpp/h`
  - `src/theWheelModel/CMakeLists.txt`
  - `src/pythewheel/CMakeLists.txt`, `src/pythewheel/bindings.cpp`, `src/pythewheel/bindings_simple.cpp`
  - `src/theWheelModelTests/CMakeLists.txt`, `src/theWheelModelTests/test_*.cpp` (5 test files)
  - `tests/conftest.py`, `tests/test_vector3d.py`, `tests/test_node.py`, `tests/test_space.py`
  - `requirements.txt`, `pytest.ini`, `run_tests.py`
  - `TESTING.md`, `PYTHON_TESTING_QUICKSTART.md`, `PYTHON_TESTING_STATUS.md`

### `feature/wrap-distance`
- **Last Updated**: 2025-11-07
- **Status**: Active development
- **Commits Ahead of Main**: 9 commits
- **Latest Commit**: Refactor codebase and adopt modern C++ practices
- **Key Changes**:
  - Extensive refactoring with modern C++ practices
  - Updated project structure with new rendering classes
  - Improved floating-point handling in optimizer and node view
  - Modified layout constants and rendering logic
  - Changes to spacing and gain handling in node layout
  - Updates to VectorN.h, MatrixOps.h, VectorD.h
  - Modifications to NodeView.cpp, SpaceLayoutManager.cpp
- **Purpose**: Major modernization effort to adopt contemporary C++ standards and improve rendering architecture

### `docs/claude-md`
- **Last Updated**: 2025-11-07
- **Status**: Merged to main (can be deleted)
- **Latest Commit**: Move CLAUDE.md to repository root and add comprehensive README
- **Key Changes**: Documentation reorganization
- **Purpose**: Improve project documentation for Claude Code integration

---

## Feature Branches (March 2025)

### `feature/weel-app`
- **Last Updated**: 2025-03-22
- **Status**: Experimental
- **Commits Ahead of Main**: 3 commits
- **Key Changes**:
  - Created complete React TypeScript web application
  - Implemented 3D visualization components using React
  - Added geometry utilities (Elliptangle, RadialShape, Vector2D, Extent2D)
  - NodeGraph and Node visualization components
- **Purpose**: Web-based reimplementation of theWheel visualization using modern web technologies
- **Technology Stack**: React, TypeScript, Create-React-App
- **Files Added**: Entire `weel-app/` directory with full React project structure

### `feature/convert-space-to-md`
- **Last Updated**: 2025-03-22
- **Status**: Minimal changes, possibly abandoned
- **Commits Ahead of Main**: 0 unique commits
- **Key Changes**: Only README and CLAUDE.md changes (documentation)
- **Purpose**: Unclear - may have been for converting .spx format to markdown representation

### `feature/use_stl`
- **Last Updated**: 2025-03-22
- **Status**: Minimal changes, possibly abandoned
- **Commits Ahead of Main**: 0 unique commits
- **Key Changes**: Only README and CLAUDE.md changes (documentation)
- **Purpose**: Possibly intended to replace custom containers with STL containers

---

## Bugfix Branches (February 2025)

### `bugfix/cast-warnings`
- **Last Updated**: 2025-02-25
- **Status**: Ready for review/merge
- **Latest Commit**: Cleanup and refactor project files and source code
- **Key Changes**:
  - Fixed cast warnings throughout codebase
  - Updated stdafx.h files across projects
  - Cleaned up Node.cpp, Space.cpp, SpaceLayoutManager.cpp
  - Improved NodeView.cpp and SpaceView.cpp
  - Refactored dialog classes (ImagePropDlg, spaceclassdlg)
- **Files Modified**: 20 files across theWheel, theWheelModel, theWheelView, OptimizeN
- **Purpose**: Improve code quality by eliminating compiler warnings related to type casting

### `bugfix/clean-up-unused-files`
- **Last Updated**: 2025-02-21
- **Status**: Ready for review/merge
- **Latest Commit**: cleaning up unused files
- **Key Changes**:
  - Similar cleanup scope to cast-warnings branch
  - Removed unused code and files
  - Refactored MathUtil.h and Pseudoinverse.h
  - Updated dialog classes and document handling
- **Files Modified**: 20+ files across all projects
- **Purpose**: Reduce technical debt by removing dead code and unused files

---

## Branch Status Summary

| Branch | Status | Priority | Recommendation |
|--------|--------|----------|----------------|
| `main` | ✅ Stable | - | Current baseline |
| `feature/cmake` | 🚧 Active | High | Current focus - complete CMake migration and testing |
| `feature/wrap-distance` | 🚧 Active | High | Continue development, consider merging when stable |
| `docs/claude-md` | ✅ Merged | Low | Safe to delete |
| `feature/weel-app` | 🧪 Experimental | Medium | Evaluate if web version should continue |
| `feature/convert-space-to-md` | ⚠️ Stale | Low | Consider deleting if no longer needed |
| `feature/use_stl` | ⚠️ Stale | Low | Consider deleting if no longer needed |
| `bugfix/cast-warnings` | ✅ Ready | Medium | Review and merge to improve code quality |
| `bugfix/clean-up-unused-files` | ✅ Ready | Medium | Review and merge to reduce technical debt |

---

## Recommendations

### Active Development
1. **feature/cmake** - Current focus: CMake build modernization with Google Test and Python testing infrastructure (9 commits, 43 files changed)
2. **feature/wrap-distance** - Major modernization effort for modern C++ and rendering improvements

### Ready to Merge
3. **bugfix/cast-warnings** - Improves code quality with cast warning fixes
4. **bugfix/clean-up-unused-files** - Reduces technical debt

### Cleanup Candidates
5. **docs/claude-md** - Already merged, safe to delete
6. **feature/convert-space-to-md** - No unique changes, consider deleting
7. **feature/use_stl** - No unique changes, consider deleting

### Evaluate
8. **feature/weel-app** - Continue if web-based version is desired

---

## Notes

- All branches have corresponding remote tracking branches on origin
- Most feature branches created in March 2025 represent parallel modernization experiments
- Current focus (February 2026) is on `feature/cmake` for build modernization and test infrastructure
- `feature/wrap-distance` and `feature/cmake` are both 9 commits ahead of main, representing the two most active branches
- Two bugfix branches from February 2025 are complete and ready for integration
- Consider consolidating or cleaning up stale branches to reduce confusion

---

**Last Updated**: 2026-02-18

# Branch Summary

**Generated**: 2025-11-07
**Repository**: theWheel

## Overview

This repository has **8 local branches** (including main) with corresponding remote tracking branches. Development activity spans from February to November 2025, with active modernization and refactoring efforts.

---

## Main Branch

### `main`
- **Last Updated**: 2025-11-07
- **Status**: Primary development branch
- **Latest Commit**: Merge pull request #11 from dg1an3/docs/claude-md
- **Description**: Stable branch with merged documentation improvements

---

## Active Development Branches

### `feature/wrap-distance` ⭐ Most Recent
- **Last Updated**: 2025-11-07
- **Status**: Active development
- **Commits Ahead of Main**: 8 commits
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

### `feature/cmake`
- **Last Updated**: 2025-11-07
- **Status**: In progress - Major update with Python testing infrastructure
- **Commits Ahead of Main**: 2+ commits
- **Key Changes**:
  - Added CMakeLists.txt and CMakePresets.json to src/
  - Created new OptimizeND project with CMake configuration
  - **NEW: Complete Python testing infrastructure using pybind11**
  - **NEW: Added CMakeLists.txt for OptimizeN library**
  - **NEW: Added CMakeLists.txt for theWheelModel library**
  - **NEW: Created pythewheel Python bindings module**
  - **NEW: Comprehensive pytest-based test suite**
- **Purpose**: Modernize build system with CMake alongside existing Visual Studio solution, enable Python-based testing
- **Files Added**:
  - `src/CMakeLists.txt` (updated with pybind11 integration)
  - `src/CMakePresets.json`
  - `src/OptimizeN/CMakeLists.txt` (new)
  - `src/OptimizeND/CMakeLists.txt`
  - `src/OptimizeND/OptimizeND.cpp/h`
  - `src/theWheelModel/CMakeLists.txt` (new)
  - `src/pythewheel/CMakeLists.txt` (new)
  - `src/pythewheel/bindings.cpp` (new)
  - `requirements.txt` (Python dependencies)
  - `pytest.ini` (pytest configuration)
  - `TESTING.md` (comprehensive testing documentation)
  - `run_tests.py` (test runner script)
  - `tests/conftest.py` (pytest configuration)
  - `tests/test_vector3d.py` (Vector3D unit tests)
  - `tests/test_node.py` (Node unit and integration tests)
  - `tests/test_space.py` (Space unit and integration tests)

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
| `feature/wrap-distance` | 🚧 Active | High | Continue development, consider merging when stable |
| `docs/claude-md` | ✅ Merged | Low | Safe to delete |
| `feature/cmake` | ⏸️ Paused | Medium | Resume when ready to complete CMake migration |
| `feature/weel-app` | 🧪 Experimental | Medium | Evaluate if web version should continue |
| `feature/convert-space-to-md` | ⚠️ Stale | Low | Consider deleting if no longer needed |
| `feature/use_stl` | ⚠️ Stale | Low | Consider deleting if no longer needed |
| `bugfix/cast-warnings` | ✅ Ready | Medium | Review and merge to improve code quality |
| `bugfix/clean-up-unused-files` | ✅ Ready | Medium | Review and merge to reduce technical debt |

---

## Recommendations

### Ready to Merge
1. **bugfix/cast-warnings** - Improves code quality with cast warning fixes
2. **bugfix/clean-up-unused-files** - Reduces technical debt

### Active Development
3. **feature/wrap-distance** - Major modernization effort, continue development

### Cleanup Candidates
4. **docs/claude-md** - Already merged, safe to delete
5. **feature/convert-space-to-md** - No unique changes, consider deleting
6. **feature/use_stl** - No unique changes, consider deleting

### Resume When Ready
7. **feature/cmake** - Complete when modernizing build system
8. **feature/weel-app** - Continue if web-based version is desired

---

## Notes

- All branches have corresponding remote tracking branches on origin
- Most feature branches created in March 2025 represent parallel modernization experiments
- Recent focus (November 2025) is on `feature/wrap-distance` for modern C++ refactoring
- Two bugfix branches from February are complete and ready for integration
- Consider consolidating or cleaning up stale branches to reduce confusion

---

**Last Updated**: 2025-11-07

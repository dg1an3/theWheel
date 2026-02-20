# Branch Summary

**Generated**: 2025-11-07 | **Updated**: 2026-02-18
**Repository**: theWheel

## Overview

This repository has **4 local branches** (including main) with corresponding remote tracking branches. Development activity spans from February 2025 to February 2026. Five stale branches were deleted on 2026-02-18 after confirming they had been merged to main (see Deleted Branches section).

---

## Main Branch

### `main`
- **Last Updated**: 2025-11-07
- **Status**: Primary development branch
- **Latest Commit**: Update branch summary with current development status and focus on CMake modernization and testing infrastructure
- **Description**: Stable branch with CMake build modernization, testing infrastructure, and merged documentation/bugfix improvements

---

## Active Development Branches

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

---

## Deleted Branches (Merged)

The following branches were merged to main and deleted on 2026-02-18:

| Branch | Merged Via | Description |
|--------|-----------|-------------|
| `bugfix/cast-warnings` | PR #5 | Fixed cast warnings throughout codebase |
| `bugfix/clean-up-unused-files` | PR #4 | Removed dead code and unused files |
| `docs/claude-md` | PRs #10, #11 | Added CLAUDE.md and README documentation |
| `feature/cmake` | PR #8 | CMake build modernization, Google Test, Python bindings |
| `feature/use_stl` | PR #6 | Partial STL container migration (std::map for class colors) |

---

## Branch Status Summary

| Branch | Status | Priority | Recommendation |
|--------|--------|----------|----------------|
| `main` | ✅ Stable | - | Current baseline (includes merged CMake, bugfixes, docs) |
| `feature/wrap-distance` | 🚧 Active | High | Continue development, consider merging when stable |
| `feature/weel-app` | 🧪 Experimental | Medium | Evaluate if web version should continue |
| `feature/convert-space-to-md` | ⚠️ Stale | Low | Consider deleting if no longer needed |

---

## Recommendations

### Active Development
1. **feature/wrap-distance** - Major modernization effort for modern C++ and rendering improvements

### Cleanup Candidates
2. **feature/convert-space-to-md** - No unique changes, consider deleting

### Evaluate
4. **feature/weel-app** - Continue if web-based version is desired

---

## Notes

- All remaining branches have corresponding remote tracking branches on origin
- Most feature branches created in March 2025 represent parallel modernization experiments
- CMake build modernization, bugfixes, and documentation have been merged to main
- `feature/wrap-distance` is the most active development branch (9 commits ahead of main)
- One stale branch (`feature/convert-space-to-md`) could be cleaned up

---

**Last Updated**: 2026-02-18

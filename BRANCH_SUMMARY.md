# Branch Summary

**Generated**: 2025-11-07 | **Updated**: 2026-02-20
**Repository**: theWheel

## Overview

This repository has **3 local branches** (including main) with one remote tracking branch beyond main. Significant development activity since the last update: multiple feature branches merged to main via PRs, repository reorganized with antiques/ directory, wxWidgets macOS port added, and MkDocs help system integrated. Two additional branches (`feature/merge-antiques` and `feature/rearrangements`) exist locally but are fully merged to main.

---

## Main Branch

### `main`
- **Last Updated**: 2026-02-20
- **Status**: Primary development branch
- **Latest Commit**: Add TODO section and merge plans for elliptangle and weel-app into web frontend
- **Recent Activity** (since 2025-11-08):
  - Merged PR #12 (`feature/convert-space-to-md`) — space-to-markdown conversion
  - Merged PR #13 (`feature/node-title-rendering`) — improved node title rendering and overlay handling
  - Merged PR #9 (`feature/weel-app`) — React TypeScript web application
  - Merged PR #18 (`feature/mkdocs-help`) — replaced CHM help with MkDocs static site
  - Merged PR #16 (`feature/merge-elliptangle`) — merged elliptangle repository history
  - Merged PR #17 (`feature/macos-port`) — wxWidgets port for macOS
  - Merged `feature/rearrangements` — reorganized legacy projects to antiques/, cleaned build paths
  - Added NodeViewSkin and Plaque rendering classes
  - Added antiques/ directory with historical CVS-era source code
  - Moved output/ to data/ with cleaner subfolder names

---

## Active Development Branches

### `feature/wrap-distance`
- **Last Updated**: 2025-11-07
- **Status**: Active development
- **Commits Ahead of Main**: 5 commits
- **Remote**: `origin/feature/wrap-distance`
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

## Merged Branches (Pending Deletion)

These local branches are fully merged to main and can be safely deleted:

### `feature/merge-antiques`
- **Last Updated**: 2026-02-20
- **Status**: Fully merged to main (0 unique commits)
- **Purpose**: Added antiques/ directory with historical CVS-era source code

### `feature/rearrangements`
- **Last Updated**: 2026-02-20
- **Status**: Fully merged to main (0 unique commits)
- **Purpose**: Reorganized legacy projects from src/ to antiques/, cleaned build paths, removed OptimizeND module

---

## Deleted Branches (Previously Merged)

The following branches were merged to main and deleted:

| Branch | Merged Via | Date Deleted | Description |
|--------|-----------|--------------|-------------|
| `bugfix/cast-warnings` | PR #5 | 2026-02-18 | Fixed cast warnings throughout codebase |
| `bugfix/clean-up-unused-files` | PR #4 | 2026-02-18 | Removed dead code and unused files |
| `docs/claude-md` | PRs #10, #11 | 2026-02-18 | Added CLAUDE.md and README documentation |
| `feature/cmake` | PR #8 | 2026-02-18 | CMake build modernization, Google Test, Python bindings |
| `feature/use_stl` | PR #6 | 2026-02-18 | Partial STL container migration (std::map for class colors) |
| `feature/convert-space-to-md` | PR #12 | — | Space-to-markdown conversion |
| `feature/node-title-rendering` | PR #13 | — | Improved node title rendering and overlay |
| `feature/weel-app` | PR #9 | — | React TypeScript web application |
| `feature/mkdocs-help` | PR #18 | — | Replaced CHM help with MkDocs static site |
| `feature/merge-elliptangle` | PR #16 | — | Merged elliptangle repository history |
| `feature/macos-port` | PR #17 | — | wxWidgets port for macOS |

---

## Branch Status Summary

| Branch | Status | Priority | Recommendation |
|--------|--------|----------|----------------|
| `main` | ✅ Stable | — | Current baseline with macOS port, web app, MkDocs, reorganized structure |
| `feature/wrap-distance` | 🚧 Active | High | Continue development, consider merging when stable |
| `feature/merge-antiques` | ✅ Merged | — | Safe to delete |
| `feature/rearrangements` | ✅ Merged | — | Safe to delete |

---

## Recommendations

### Active Development
1. **feature/wrap-distance** — Major modernization effort for modern C++ and rendering improvements (5 commits ahead)

### Cleanup Candidates
2. **feature/merge-antiques** — Fully merged, no unique commits, delete local branch
3. **feature/rearrangements** — Fully merged, no unique commits, delete local branch

---

## Notes

- Only `feature/wrap-distance` has a remote tracking branch (beyond main)
- `feature/merge-antiques` and `feature/rearrangements` are local-only and fully merged
- Main has seen substantial activity since Nov 2025: 6 PRs merged, major repository reorganization
- The `weel-app/` and `elliptangle/` directories are now on main with a TODO to merge them into a single web frontend

---

**Last Updated**: 2026-02-20

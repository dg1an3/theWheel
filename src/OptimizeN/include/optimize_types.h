//////////////////////////////////////////////////////////////////////
// optimize_types.h: Minimal compatibility types for OptimizeN
//
// Provides the subset of Windows/MFC types and macros that OptimizeN
// actually needs, without pulling in CObject, CString, or other
// MFC class stubs.
//
// This header is only included on non-MSVC platforms via stdafx.h.
//////////////////////////////////////////////////////////////////////

#ifndef OPTIMIZE_TYPES_H
#define OPTIMIZE_TYPES_H

#include <cassert>
#include <cstdint>
#include <cmath>

//////////////////////////////////////////////////////////////////////
// Windows type aliases
//////////////////////////////////////////////////////////////////////

typedef int BOOL;
#define TRUE 1
#define FALSE 0

typedef uint32_t DWORD;
typedef unsigned int UINT;

//////////////////////////////////////////////////////////////////////
// MSVC intrinsics / macros
//////////////////////////////////////////////////////////////////////

#ifndef __max
#define __max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef __min
#define __min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// _finite -> std::isfinite
#ifndef _finite
#define _finite(x) std::isfinite(x)
#endif

// __forceinline -> inline (GCC/Clang have __attribute__((always_inline)))
#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif

// _T() text macro - no-op on Linux (no wide char)
#ifndef _T
#define _T(x) x
#endif

//////////////////////////////////////////////////////////////////////
// Debug macros
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define ASSERT(expr) assert(expr)
#else
#define ASSERT(expr) ((void)0)
#endif

#define ATLASSERT(expr) ASSERT(expr)
#define TRACE(...) ((void)0)
#define TRACE_VECTOR(...) ((void)0)

// USES_CONVERSION - ATL string conversion macro, no-op on non-MSVC
#define USES_CONVERSION

// DEBUG_NEW / THIS_FILE - used in #ifdef _DEBUG blocks
#define DEBUG_NEW new

//////////////////////////////////////////////////////////////////////
// Serialization macro stubs - no-ops on non-MSVC
//////////////////////////////////////////////////////////////////////

#define VERSIONABLE_SCHEMA 0
#define DECLARE_SERIAL(class_name)
#define IMPLEMENT_SERIAL(class_name, base_class, schema)
#define IMPLEMENT_DYNAMIC(class_name, base_class)
#define DECLARE_DYNAMIC(class_name)

//////////////////////////////////////////////////////////////////////
// Forward declarations for serialization (guarded by #ifdef _MSC_VER)
//////////////////////////////////////////////////////////////////////

class CArchive;
class CDumpContext;

#endif // OPTIMIZE_TYPES_H

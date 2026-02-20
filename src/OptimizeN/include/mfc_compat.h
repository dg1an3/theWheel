//////////////////////////////////////////////////////////////////////
// mfc_compat.h: Linux/non-MSVC compatibility header
//
// Provides minimal replacements for MFC types, macros, and classes
// used by OptimizeN and theWheelModel, allowing these libraries to
// build on Linux/GCC/Clang without MFC.
//
// This header is only included on non-MSVC platforms via stdafx.h.
//////////////////////////////////////////////////////////////////////

#ifndef MFC_COMPAT_H
#define MFC_COMPAT_H

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>

//////////////////////////////////////////////////////////////////////
// Windows type aliases
//////////////////////////////////////////////////////////////////////

typedef int BOOL;
#define TRUE 1
#define FALSE 0

typedef uint32_t DWORD;
typedef unsigned int UINT;
typedef uint32_t COLORREF;
typedef const char* LPCTSTR;

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
#include <cmath>
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

// VERSIONABLE_SCHEMA - used in IMPLEMENT_SERIAL
#define VERSIONABLE_SCHEMA 0

//////////////////////////////////////////////////////////////////////
// CObject - minimal base class stub
//////////////////////////////////////////////////////////////////////

class CDumpContext;

class CObject
{
public:
    virtual ~CObject() {}
};

//////////////////////////////////////////////////////////////////////
// CArchive - forward declaration only
// Serialization methods are guarded with #ifdef _MSC_VER
//////////////////////////////////////////////////////////////////////

class CArchive;

//////////////////////////////////////////////////////////////////////
// CString - minimal wrapper around std::string
// Provides the subset of CString API used in theWheelModel
//////////////////////////////////////////////////////////////////////

class CString
{
    std::string m_str;

public:
    CString() {}
    CString(const char* s) : m_str(s ? s : "") {}
    CString(const std::string& s) : m_str(s) {}
    CString(const CString& other) : m_str(other.m_str) {}

    CString& operator=(const CString& other) { m_str = other.m_str; return *this; }
    CString& operator=(const char* s) { m_str = (s ? s : ""); return *this; }

    // Conversion
    operator const char*() const { return m_str.c_str(); }
    const char* GetString() const { return m_str.c_str(); }

    // Length
    int GetLength() const { return (int)m_str.length(); }

    // Buffer access
    char* GetBuffer() { return &m_str[0]; }
    char* GetBuffer(int nMinBufLength) {
        m_str.resize(nMinBufLength);
        return &m_str[0];
    }
    void ReleaseBuffer(int nNewLength = -1) {
        if (nNewLength >= 0) m_str.resize(nNewLength);
        else m_str.resize(strlen(m_str.c_str()));
    }

    // Search
    int ReverseFind(char ch) const {
        size_t pos = m_str.rfind(ch);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }

    // Substring
    CString Left(int nCount) const {
        return CString(m_str.substr(0, nCount));
    }

    // Format (simplified - supports common printf-style formatting)
    void Format(const char* fmt, ...) {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        m_str = buf;
    }

    // Comparison
    bool operator==(const CString& other) const { return m_str == other.m_str; }
    bool operator==(const char* s) const { return m_str == (s ? s : ""); }
    bool operator!=(const CString& other) const { return m_str != other.m_str; }
    bool operator!=(const char* s) const { return m_str != (s ? s : ""); }

    // Ordering (needed for std::map<CString, ...>)
    bool operator<(const CString& other) const { return m_str < other.m_str; }

    // Concatenation
    CString operator+(const CString& other) const {
        return CString(m_str + other.m_str);
    }
    CString operator+(const char* s) const {
        return CString(m_str + (s ? s : ""));
    }
    friend CString operator+(const char* s, const CString& cs) {
        return CString(std::string(s ? s : "") + cs.m_str);
    }
    CString& operator+=(const CString& other) {
        m_str += other.m_str;
        return *this;
    }

    // Stream output support
    friend std::ostream& operator<<(std::ostream& os, const CString& cs) {
        return os << cs.m_str;
    }
};

//////////////////////////////////////////////////////////////////////
// Serialization macros - no-ops on Linux
//////////////////////////////////////////////////////////////////////

#define DECLARE_SERIAL(class_name)
#define IMPLEMENT_SERIAL(class_name, base_class, schema)
#define IMPLEMENT_DYNAMIC(class_name, base_class)
#define DECLARE_DYNAMIC(class_name)

//////////////////////////////////////////////////////////////////////
// CDocument - stub (CSpace inherits from it via IMPLEMENT_SERIAL)
//////////////////////////////////////////////////////////////////////

typedef CObject CDocument;

//////////////////////////////////////////////////////////////////////
// Misc stubs
//////////////////////////////////////////////////////////////////////

// RGB macro
#ifndef RGB
#define RGB(r,g,b) ((COLORREF)(((uint8_t)(r)|((uint16_t)((uint8_t)(g))<<8))|(((uint32_t)(uint8_t)(b))<<16)))
#endif

// HRESULT and related
#ifndef HRESULT
typedef long HRESULT;
#define S_OK ((HRESULT)0L)
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif

#endif // MFC_COMPAT_H

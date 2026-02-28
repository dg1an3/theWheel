//////////////////////////////////////////////////////////////////////
// mfc_compat.h: Linux/non-MSVC compatibility header
//
// Provides MFC class stubs (CObject, CString, CDocument, etc.)
// needed by theWheelModel. Builds on optimize_types.h which provides
// the basic Windows type aliases and macros.
//
// This header is only included on non-MSVC platforms via stdafx.h.
//////////////////////////////////////////////////////////////////////

#ifndef MFC_COMPAT_H
#define MFC_COMPAT_H

// Base types and macros shared with OptimizeN
#include <optimize_types.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>

//////////////////////////////////////////////////////////////////////
// Additional Windows type aliases (not needed by OptimizeN)
//////////////////////////////////////////////////////////////////////

typedef uint32_t COLORREF;
typedef const char* LPCTSTR;

//////////////////////////////////////////////////////////////////////
// CObject - minimal base class stub
//////////////////////////////////////////////////////////////////////

class CObject
{
public:
    virtual ~CObject() {}
};

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
    CString(const wchar_t* s) { // Support wxWidgets _T() producing wide strings
        if (s) {
            size_t len = wcslen(s);
            m_str.resize(len);
            for (size_t i = 0; i < len; i++) m_str[i] = (char)s[i];
        }
    }
    CString(const std::string& s) : m_str(s) {}
    CString(const CString& other) : m_str(other.m_str) {}

    CString& operator=(const CString& other) { m_str = other.m_str; return *this; }
    CString& operator=(const char* s) { m_str = (s ? s : ""); return *this; }
    CString& operator=(const wchar_t* s) { *this = CString(s); return *this; }

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
// CDocument - stub (CSpace inherits from it via IMPLEMENT_SERIAL)
//////////////////////////////////////////////////////////////////////

typedef CObject CDocument;

//////////////////////////////////////////////////////////////////////
// Misc stubs
//////////////////////////////////////////////////////////////////////

// RECT structure (used by CExtent)
#ifndef _WINDEF_
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};
#endif

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

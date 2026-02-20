// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__70763E28_0829_11D5_9E4C_00B0D0609AB0__INCLUDED_)
#define AFX_STDAFX_H__70763E28_0829_11D5_9E4C_00B0D0609AB0__INCLUDED_

#ifdef _MSC_VER

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afx.h>
#include <afxwin.h>

#include <afxtempl.h>

// ATL collection classes
#include <atlcoll.h>

// internet includes for web pages
#include <wininet.h>
#include <urlmon.h>
#include <mshtml.h>

// common definitions
#include <comdef.h>

// multimedia and sound includes
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
// #include <dsound.h>

#else // !_MSC_VER (Linux/GCC/Clang)

#include <mfc_compat.h>

#include <cmath>
#include <cstdio>
#include <cstdarg>
#include <cassert>

#endif // _MSC_VER

// STL - available on all platforms
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <functional>

// utility macros
#include <UtilMacros.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__70763E28_0829_11D5_9E4C_00B0D0609AB0__INCLUDED_)

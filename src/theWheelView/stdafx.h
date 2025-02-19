// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__0C8AA656_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_STDAFX_H__0C8AA656_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

//// Modify the following defines if you have to target a platform prior to the ones specified below.
//// Refer to MSDN for the latest info on corresponding values for different platforms.
//#ifndef WINVER				// Allow use of features specific to Windows XP or later.
//#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
//#endif
//
//#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
//#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
//#endif						
//
//#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
//#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
//#endif
//
//#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
//#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
//#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#include <afxtempl.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxmt.h>			// MFC support for multithreading
#include <afxhtml.h>		// MFC support for HTML browser

#include <afxole.h>			// MFC support for OLE

#include <afxconv.h>

// ATL collection classes
#include <atlcoll.h>

#include <dxgitype.h>

// internet includes for web pages
#include <wininet.h> 
#include <urlmon.h> 
#include <mshtml.h> 

// common definitions
#include <comdef.h>

// #ifndef _SECURE_SCL
// #define _SECURE_SCL 0
// #endif
// #define _HAS_ITERATOR_DEBUGGING 0


// STL
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <functional>

// utilities
#include <UtilMacros.h>

// multimedia and sound includes
#include <d3d9.h>
// #include <d3dx9.h>
#include "D3DXImpl.h"

#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
// #include <dsound.h>

typedef float D3DVALUE, * LPD3DVALUE;

// MTL
#include <VectorD.h>
#include <Extent.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0C8AA656_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)

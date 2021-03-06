// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
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

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit


#include <afxwin.h>
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>


// internet includes for web pages
#include <wininet.h> 
#include <urlmon.h> 
#include <mshtml.h> 

// common definitions
#include <comdef.h>


// #ifndef _SECURE_SCL
#define _SECURE_SCL 0
// #endif
#define _HAS_ITERATOR_DEBUGGING 0


// STL
#include <vector>
#include <hash_map>
#include <algorithm>
#include <numeric>

// multimedia and sound includes
#include <d3d9.h>
#include <d3dx9.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <dsound.h>

// IPP includes
#include <ipp.h>

// utility macros
#include <UtilMacros.h>


using namespace ATL;
#include "WebService.h"
[
  importidl(AxWheel.idl)
];

//#define CHECK_HR( x ) { HRESULT hr = (x); ASSERT(SUCCEEDED(hr)); }

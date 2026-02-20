// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__0C8AA656_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_STDAFX_H__0C8AA656_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#include <afxtempl.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxmt.h>
#include <afxhtml.h>

// internet includes for web pages
#include <wininet.h> 
#include <urlmon.h> 
#include <mshtml.h> 

#include <comdef.h>

// multimedia and sound includes
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <dsound.h>

void log_trace(const char *, ...);

#ifdef _DEBUG
#define LOG_TRACE TRACE
#else
#define LOG_TRACE log_trace
#endif

#define START_TIMER(EVENT_FLAG)			\
{										\
	LARGE_INTEGER freq;					\
	QueryPerformanceFrequency(&freq);	\
										\
	LARGE_INTEGER start;				\
	QueryPerformanceCounter(&start);	\

#define STOP_TIMER(EVENT_FLAG, LABEL)	\
	LARGE_INTEGER end;					\
	QueryPerformanceCounter(&end);		\
										\
	LOG_TRACE("Time: " LABEL " = %lf\n",	\
		(double) (end.QuadPart - start.QuadPart)	\
			/ (double) freq.QuadPart);	\
}										\

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0C8AA656_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)

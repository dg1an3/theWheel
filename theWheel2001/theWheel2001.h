// theWheel2001.h : main header file for the THEWHEEL2001 application
//

#if !defined(AFX_THEWHEEL2001_H__0C8AA654_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_THEWHEEL2001_H__0C8AA654_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CtheWheelApp:
// See theWheel2001.cpp for the implementation of this class
//

class CtheWheelApp : public CWinApp
{
public:
	CtheWheelApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CtheWheelApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CtheWheelApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_THEWHEEL2001_H__0C8AA654_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)

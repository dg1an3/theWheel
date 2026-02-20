//////////////////////////////////////////////////////////////////////
// theWheel2001.h: interface for the CtheWheelApp class.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THEWHEEL2001_H__0C8AA654_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_THEWHEEL2001_H__0C8AA654_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

//////////////////////////////////////////////////////////////////////
// class CtheWheelApp
//
// main application object for theWheel2001
//////////////////////////////////////////////////////////////////////
class CtheWheelApp : public CWinApp
{
public:
	// Construction/Destruction
	CtheWheelApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CtheWheelApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
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

// theWheel.h : main header file for the THEWHEEL application
//

#if !defined(AFX_THEWHEEL_H__EF3C7A67_A96A_46D7_85B1_EE95EE95BDF5__INCLUDED_)
#define AFX_THEWHEEL_H__EF3C7A67_A96A_46D7_85B1_EE95EE95BDF5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// theWheelApp:
// See theWheel.cpp for the implementation of this class
//

class theWheelApp : public CWinApp
{
public:
	theWheelApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(theWheelApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(theWheelApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_THEWHEEL_H__EF3C7A67_A96A_46D7_85B1_EE95EE95BDF5__INCLUDED_)

// theWheel.h : main header file for the theWheel application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "THEWHEEL_VIEW_resource.h"
#include "THEWHEEL_TREEVIEW_resource.h"

// CtheWheelApp:
// See theWheel.cpp for the implementation of this class
//

class CtheWheelApp : public CWinApp
{
public:
	CtheWheelApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CtheWheelApp theApp;
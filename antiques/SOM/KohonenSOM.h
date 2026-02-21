// KohonenSOM.h : main header file for the KohonenSOM application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CKohonenSOMApp:
// See KohonenSOM.cpp for the implementation of this class
//

class CKohonenSOMApp : public CWinApp
{
public:
	CKohonenSOMApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CKohonenSOMApp theApp;
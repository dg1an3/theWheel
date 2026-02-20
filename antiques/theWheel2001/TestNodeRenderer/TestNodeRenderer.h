// TestNodeRenderer.h : main header file for the TESTNODERENDERER application
//

#if !defined(AFX_TESTNODERENDERER_H__8D3A0D92_8AB3_11D5_ABAE_00B0D0AB90D6__INCLUDED_)
#define AFX_TESTNODERENDERER_H__8D3A0D92_8AB3_11D5_ABAE_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CTestNodeRendererApp:
// See TestNodeRenderer.cpp for the implementation of this class
//

class CTestNodeRendererApp : public CWinApp
{
public:
	CTestNodeRendererApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestNodeRendererApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CTestNodeRendererApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTNODERENDERER_H__8D3A0D92_8AB3_11D5_ABAE_00B0D0AB90D6__INCLUDED_)

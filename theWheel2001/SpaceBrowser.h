#if !defined(AFX_SPACEBROWSER_H__0DF7C232_7523_494A_B527_6CA921009735__INCLUDED_)
#define AFX_SPACEBROWSER_H__0DF7C232_7523_494A_B527_6CA921009735__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpaceBrowser.h : header file
//

#include "SpaceView.h"

/////////////////////////////////////////////////////////////////////////////
// CSpaceBrowser html view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
#include <afxhtml.h>

class CSpaceBrowser : public CHtmlView
{
protected:
	CSpaceBrowser();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSpaceBrowser)

// html Data
public:
	//{{AFX_DATA(CSpaceBrowser)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	CString m_strPrevUrl;
	CSpaceView * m_pSpaceView;
	CString m_strNavigatingToUrl;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpaceBrowser)
	public:
	virtual void OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel);
	virtual void OnNavigateComplete2(LPCTSTR strURL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSpaceBrowser();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CSpaceBrowser)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPACEBROWSER_H__0DF7C232_7523_494A_B527_6CA921009735__INCLUDED_)

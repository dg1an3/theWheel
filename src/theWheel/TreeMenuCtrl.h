#if !defined(AFX_TREEMENUCTRL_H__7160C52D_1C92_4F82_8BD1_8E2080D64D87__INCLUDED_)
#define AFX_TREEMENUCTRL_H__7160C52D_1C92_4F82_8BD1_8E2080D64D87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TreeMenuCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTreeMenuCtrl window

class CTreeMenuCtrl : public CTreeCtrl
{
// Construction
public:
	CTreeMenuCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTreeMenuCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	CWnd * m_pCmdTarget;
	UINT m_nMenuResourceID;
	void SetMenu(UINT nResourceID, CWnd *pCmdTarget);
	virtual ~CTreeMenuCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTreeMenuCtrl)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TREEMENUCTRL_H__7160C52D_1C92_4F82_8BD1_8E2080D64D87__INCLUDED_)

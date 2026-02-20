#if !defined(AFX_NODEPROPDLG_H__4EF79EA0_3D3A_431E_8C8E_3EE987593388__INCLUDED_)
#define AFX_NODEPROPDLG_H__4EF79EA0_3D3A_431E_8C8E_3EE987593388__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NodePropDlg.h : header file
//

#include "THEWHEEL_TREEVIEW_resource.h"

#include <Node.h>

/////////////////////////////////////////////////////////////////////////////
// CNodePropDlg dialog

class CNodePropDlg : public CDialog
{
// Construction
public:
	CNodePropDlg(CWnd* pParent = NULL);   // standard constructor

	// accessor for current node
	void SetCurNode(CNode *pNode);

	// over-ride to handle enter key
	void OnOK( ) { ::SetFocus(NULL); }

// Dialog Data
	//{{AFX_DATA(CNodePropDlg)
	enum { IDD = IDD_NODEPROP };
	CString	m_strName;
	CString	m_strDesc;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNodePropDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// pointer to current node
	CNode * m_pCurNode;

	// Generated message map functions
	//{{AFX_MSG(CNodePropDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChangeEditname();
	afx_msg void OnChangeEditdesc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NODEPROPDLG_H__4EF79EA0_3D3A_431E_8C8E_3EE987593388__INCLUDED_)

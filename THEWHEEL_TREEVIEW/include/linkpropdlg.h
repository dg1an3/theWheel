#if !defined(AFX_LINKPROPDLG_H__2177620C_294E_4D22_B4A0_3D593AE7572F__INCLUDED_)
#define AFX_LINKPROPDLG_H__2177620C_294E_4D22_B4A0_3D593AE7572F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LinkPropDlg.h : header file
//

#include <Node.h>

/////////////////////////////////////////////////////////////////////////////
// CLinkPropDlg dialog

class CLinkPropDlg : public CDialog
{
// Construction
public:
	CLinkPropDlg(CWnd* pParent = NULL);   // standard constructor

	// accessor for current node
	void SetCurNode(CNode *pNode);

// Dialog Data
	//{{AFX_DATA(CLinkPropDlg)
	enum { IDD = IDD_LINKPROP };
	CListCtrl	m_LinkList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLinkPropDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// pointer to current node
	CNode * m_pCurNode;

	// flag to indicate that the list needs sorting
	BOOL m_bSort;

	// helper function to add links from a neighboring node; recursive call
	//	while levels > 0
	void UpdateLinkList(CNode *pOtherNode, int nLevels = 5);

	// Generated message map functions
	//{{AFX_MSG(CLinkPropDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnClickLinklist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditLinklist(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LINKPROPDLG_H__2177620C_294E_4D22_B4A0_3D593AE7572F__INCLUDED_)

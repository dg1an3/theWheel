//////////////////////////////////////////////////////////////////////
// EditNodeDlg.h: interface for the CEditNodeDlg class.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDITNODEDLG_H__8E566675_F7C8_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_EDITNODEDLG_H__8E566675_F7C8_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// include for the CNode class
#include <Node.h>

//////////////////////////////////////////////////////////////////////
// class CEditNodeDlg
// 
// a dialog for editing a node; includes a list for the link weights
//////////////////////////////////////////////////////////////////////
class CEditNodeDlg : public CDialog
{
// Construction
public:
	CEditNodeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditNodeDlg)
	enum { IDD = IDD_NEWNODE };
	CListCtrl	m_LinkList;
	CString	m_strName;
	CString	m_strDesc;
	CString	m_strImageFilename;
	CString	m_strUrl;
	CString	m_strClass;
	//}}AFX_DATA

	// a pointer to the node being edited
	CNode *m_pNode;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditNodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// returns TRUE if a link is already present in the list to this node
	BOOL IsLinkPresent(CNode *pToNode);

	// helper function to add links from a neighboring node; recursive call
	//	while levels > 0
	void AddLinksFromOtherNode(CNode *pOtherNode, int nLevels = 1);

	// count of items in list
	int m_nItemCount;

	// array of nodes whose links have already been added
	CArray<CNode *, CNode *> m_arrNodes;

	// Generated message map functions
	//{{AFX_MSG(CEditNodeDlg)
	virtual BOOL OnInitDialog();
    afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITNODEDLG_H__8E566675_F7C8_11D4_9E3E_00B0D0609AB0__INCLUDED_)

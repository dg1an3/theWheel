#if !defined(AFX_NEWNODEDLG_H__8E566675_F7C8_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_NEWNODEDLG_H__8E566675_F7C8_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewNodeDlg.h : header file
//

#include <Node.h>

/////////////////////////////////////////////////////////////////////////////
// CNewNodeDlg dialog

class CNewNodeDlg : public CDialog
{
// Construction
public:
	CNewNodeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewNodeDlg)
	enum { IDD = IDD_NEWNODE };
	CListCtrl	m_LinkList;
	CString	m_strName;
	CString	m_strDesc;
	CString	m_strImageFilename;
	//}}AFX_DATA
	
	CNode *m_pNode;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewNodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewNodeDlg)
	virtual BOOL OnInitDialog();
    afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWNODEDLG_H__8E566675_F7C8_11D4_9E3E_00B0D0609AB0__INCLUDED_)

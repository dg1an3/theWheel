#if !defined(AFX_SPACECLASSDLG_H__E302AF6A_2EB4_48C3_9CD6_F644E1868D77__INCLUDED_)
#define AFX_SPACECLASSDLG_H__E302AF6A_2EB4_48C3_9CD6_F644E1868D77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpaceClassDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpaceClassDlg dialog

class CSpaceClassDlg : public CDialog
{
// Construction
public:
	CSpaceClassDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSpaceClassDlg)
	enum { IDD = IDD_SPACECLASSPROP };
	CListCtrl	m_ClassList;
	//}}AFX_DATA

	// the mapping from classes to colors
	CMapStringToPtr m_mapClassColors;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpaceClassDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// helper to populate the class color list
	void PopulateClassList();

	// Generated message map functions
	//{{AFX_MSG(CSpaceClassDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnItemactivatedClasslist(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPACECLASSDLG_H__E302AF6A_2EB4_48C3_9CD6_F644E1868D77__INCLUDED_)

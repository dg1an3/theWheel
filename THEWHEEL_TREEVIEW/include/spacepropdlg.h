#if !defined(AFX_SPACEPROPDLG_H__8E9735EF_DBC8_4DE3_8CE2_290115D689F2__INCLUDED_)
#define AFX_SPACEPROPDLG_H__8E9735EF_DBC8_4DE3_8CE2_290115D689F2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpacePropDlg.h : header file
//

class CSpace;

/////////////////////////////////////////////////////////////////////////////
// CSpacePropDlg dialog

class CSpacePropDlg : public CDialog
{
// Construction
public:
	CSpacePropDlg(CWnd* pParent = NULL);   // standard constructor

	// over-ride to handle enter key
	void OnOK( ) { ::SetFocus(NULL); }

// Dialog Data
	//{{AFX_DATA(CSpacePropDlg)
	enum { IDD = IDD_SPACEPROP };
	double	m_spring;
	double	m_primSec;
	//}}AFX_DATA

	// pointer to the space
	CSpace * m_pSpace;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpacePropDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpacePropDlg)
	afx_msg void OnKillfocusAny();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPACEPROPDLG_H__8E9735EF_DBC8_4DE3_8CE2_290115D689F2__INCLUDED_)

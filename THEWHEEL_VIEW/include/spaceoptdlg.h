#if !defined(AFX_SPACEOPTDLG_H__271AFAE1_22CA_4C53_BC2A_E307DF555E95__INCLUDED_)
#define AFX_SPACEOPTDLG_H__271AFAE1_22CA_4C53_BC2A_E307DF555E95__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpaceOptDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpaceOptDlg dialog

class CSpaceOptDlg : public CDialog
{
// Construction
public:
	CSpaceOptDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSpaceOptDlg)
	enum { IDD = IDD_SPACEOPTPROP };
	UINT	m_nSuperNodeCount;
	double	m_tolerance;
	double	m_kpos;
	double	m_krep;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpaceOptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpaceOptDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPACEOPTDLG_H__271AFAE1_22CA_4C53_BC2A_E307DF555E95__INCLUDED_)

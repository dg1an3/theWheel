#if !defined(AFX_ACTIVATIONCONTROLBAR_H__8D3A0DA0_8AB3_11D5_ABAE_00B0D0AB90D6__INCLUDED_)
#define AFX_ACTIVATIONCONTROLBAR_H__8D3A0DA0_8AB3_11D5_ABAE_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ActivationControlBar.h : header file
//

#include <NodeView.h>

/////////////////////////////////////////////////////////////////////////////
// CActivationControlBar dialog

class CActivationControlBar : public CDialogBar
{
// Construction
public:
	CActivationControlBar(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CActivationControlBar)
	enum { IDD = IDD_ACTIVATIONDIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// holds the current activation value (0.0 -> 1.0)
	double m_activation;

	CNode *m_pNode;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CActivationControlBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CActivationControlBar)
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTIVATIONCONTROLBAR_H__8D3A0DA0_8AB3_11D5_ABAE_00B0D0AB90D6__INCLUDED_)

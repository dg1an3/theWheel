#if !defined(AFX_IMAGEPROPDLG_H__3DCBE678_1B8F_4B56_BE40_9EC16A4032F0__INCLUDED_)
#define AFX_IMAGEPROPDLG_H__3DCBE678_1B8F_4B56_BE40_9EC16A4032F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImagePropDlg.h : header file
//

#include <Node.h>

/////////////////////////////////////////////////////////////////////////////
// CImagePropDlg dialog

class CImagePropDlg : public CDialog
{
// Construction
public:
	CImagePropDlg(CWnd* pParent = NULL);   // standard constructor

	// accessor for current node
	void SetCurNode(CNode *pNode);

// Dialog Data
	//{{AFX_DATA(CImagePropDlg)
	enum { IDD = IDD_IMAGEPROP };
	CString	m_strImageFilename;
	CString	m_strUrl;
	CString	m_strClass;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImagePropDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// pointer to current node
	CNode * m_pCurNode;

	// Generated message map functions
	//{{AFX_MSG(CImagePropDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEPROPDLG_H__3DCBE678_1B8F_4B56_BE40_9EC16A4032F0__INCLUDED_)

#if !defined(AFX_SPACEANDBROWSERVIEW_H__5E8C4228_0B8B_4A67_A9BF_5B2AE25D29AE__INCLUDED_)
#define AFX_SPACEANDBROWSERVIEW_H__5E8C4228_0B8B_4A67_A9BF_5B2AE25D29AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpaceAndBrowserView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpaceAndBrowserView view

class CSpaceAndBrowserView : public CView
{
protected:
	CSpaceAndBrowserView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSpaceAndBrowserView)

// Attributes
public:

// Operations
public:
	CSplitterWnd m_wndSplitter;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpaceAndBrowserView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSpaceAndBrowserView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CSpaceAndBrowserView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPACEANDBROWSERVIEW_H__5E8C4228_0B8B_4A67_A9BF_5B2AE25D29AE__INCLUDED_)

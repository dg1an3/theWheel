#if !defined(AFX_NODEVIEW_H__0C8AA66F_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_NODEVIEW_H__0C8AA66F_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NodeView.h : header file
//

#include <Vector.h>

#include "Node.h"

/////////////////////////////////////////////////////////////////////////////
// CNodeView window

class CNodeView : public CWnd, public CObserver
{
// Construction
public:
	CNodeView(CNode *pNode);

// Attributes
public:
	// accessor for the node
	CAssociation< CNode > forNode;

	// value for the window rectangle
	CValue< CRect > rectWindow;	// in parent coordinates

	// moves the nodeview slowly toward its actual center
	CValue< CVector<2> > center;
private:
	CValue< CVector<2> > privCenter;
public:

	// wave mode means activation on mouse movement
	CValue< BOOL > isWaveMode;

	// activation value for this window
	CValue< float > activation;
private:
	CValue< float > privActivation;
public:

	// helper to compute sigmoidal activation changes
	static float ActivationCurve(float a, float a_max);

// Operations
public:
	virtual void OnChange(CObservableObject *pSource, void *pOldValue);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNodeView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNodeView();

	// smooth updating of internally maintained private state 
	//		(center and activation)
	void UpdatePrivates();

protected:
	// accessor for the inner and outer rectangles 
	//		(defines the shape of the node view)
	CRect GetOuterRect();
	CRect GetInnerRect();

	// accessors for the left-right and top-bottom ellipse rectangles
	CRect GetTopBottomEllipseRect();
	CRect GetLeftRightEllipseRect();

	// Generated message map functions
	//{{AFX_MSG(CNodeView)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bDragging;
	BOOL m_bDragged;
	CPoint m_ptMouseDown;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NODEVIEW_H__0C8AA66F_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)

//////////////////////////////////////////////////////////////////////
// NodeView.h: interface for the CNodeView class.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODEVIEW_H__0C8AA66F_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_NODEVIEW_H__0C8AA66F_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Vector.h>

#include "Node.h"

//////////////////////////////////////////////////////////////////////
// class CNodeView
// 
// a view of a CNode object; child of a CSpaceView
//////////////////////////////////////////////////////////////////////
class CNodeView : public CWnd
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
	CValue< CVector<2> > springCenter;

	// wave mode means activation on mouse movement
	CValue< BOOL > isWaveMode;

	// activation value for this window
	CValue< float > thresholdedActivation;
	CValue< float > springActivation;

// Operations
public:
	// smooth updating of spring state variables
	void UpdateSprings();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNodeView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNodeView();

protected:
	// accessor for the inner and outer rectangles 
	//		(defines the shape of the node view)
	CRect GetOuterRect();
	CRect GetInnerRect();

	// accessors for the left-right and top-bottom ellipse rectangles
	CRect GetTopBottomEllipseRect();
	CRect GetLeftRightEllipseRect();

	// change event handler
	void OnRectChanged(CObservableObject *pSource, void *pOldValue);
	void OnSpringCenterChanged(CObservableObject *pSource, void *pOldValue);
	void OnThresholdedActivationChanged(CObservableObject *pSource, void *pOldValue);
	void OnSpringActivationChanged(CObservableObject *pSource, void *pOldValue);

	// Generated message map functions
	//{{AFX_MSG(CNodeView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// flags to indicate dragging state
	BOOL m_bDragging;
	BOOL m_bDragged;

	// stores the mouse-down point
	CPoint m_ptMouseDown;

	// the node view that this one is piggybacked on
	CNodeView *m_pPiggyBack;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NODEVIEW_H__0C8AA66F_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)

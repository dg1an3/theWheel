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

// the parent window class
class CSpaceView;

//////////////////////////////////////////////////////////////////////
// class CNodeView
// 
// a view of a CNode object; child of a CSpaceView
//////////////////////////////////////////////////////////////////////
class CNodeView : public CObject
{
// Construction
public:
	CNodeView(CNode *pNode, CSpaceView *pParent);
	virtual ~CNodeView();

// Attributes
public:
	// accessor for the node
	CAssociation< CNode > forNode;

	// moves the nodeview slowly toward its actual center
	CVector<2> GetCenter();
	CVector<2> GetSpringCenter();
	CVector<2> GetPiggybackCenter();
	void SetCenter(const CVector<2>& vCenter);
	
	// activation value for this window
	double GetThresholdedActivation();
	void SetThresholdedActivation(double activation);

	// accessors for the node view's rectangles
	CRect GetOuterRect();
	CRect GetInnerRect();

	// accessor to shape descriptions of the node view
	CRgn& GetShape();

// Operations
public:
	// smooth updating of spring state variables
	void UpdateSprings(double springConst = 0.90);

	// draws the node view
	void Draw(CDC *pDC);

// Implementation
protected:
	// accessors for the left-right and top-bottom ellipse rectangles
	CRect GetTopBottomEllipseRect();
	CRect GetLeftRightEllipseRect();

public:
	// drawing helper functions
	void DrawElliptangle(CDC *pDC);
	void DrawText(CDC *pDC, CRect& rectInner);
	void DrawImage(CDC *pDC, CRect& rectInner);

private:
	// stores the center and spring-connected center of the node view
	CVector<2> m_vCenter;
	CVector<2> m_vSpringCenter;

	// stores the thresholded activation and spring-connected activation
	double m_thresholdedActivation;
	double m_springActivation;

	// stores the node view's client rectangle
	CRect m_rectOuter;

	// stores the node view's region (shape)
	CRgn m_shape;

	// flags to indicate dragging state
	BOOL m_bDragging;
	BOOL m_bDragged;

	// stores the mouse-down point
	CPoint m_ptMouseDown;

	// stores the windows parent CSpaceView window
	CSpaceView *m_pParent;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NODEVIEW_H__0C8AA66F_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)

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
#include "NodeViewSkin.h"

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
	CNode *GetNode();

	// moves the nodeview slowly toward its actual center
	CVector<3> GetSpringCenter();
	
	// activation value for this window
	REAL GetThresholdedActivation();
	REAL GetSpringActivation() { return m_springActivation; }

	// accessors for the node view's rectangles
	CRect GetOuterRect();
	CRect GetInnerRect();

	// accessor to shape descriptions of the node view
	CRgn& GetShape(int nErode = 0);

	float m_pendingActivation;

// Operations
public:
	// smooth updating of spring state variables
	void UpdateSprings(REAL springConst = 0.95);

	// draws the node view
	void Draw(CDC *pDC, CNodeViewSkin *pSkin);

// Implementation
public:
	// drawing helper functions
	void DrawLinks(CDC *pDC);
	void DrawTitle(CDC *pDC, CRect& rectInner);
	void DrawText(CDC *pDC, CRect& rectInner);
	void DrawImage(CDC *pDC, CRect& rectInner);

private:
	// stores the node
	CNode* m_pNode;

public:
	// stores the center and spring-connected center of the node view
	CVector<3> m_vSpringCenter;

	// stores the thresholded activation and spring-connected activation
	// REAL m_thresholdedActivation;
	REAL m_springActivation;

	// pointer to the skin for this node view
	CNodeViewSkin *m_pSkin;

	// stores the node view's region (shape)
	CRgn m_shape;

	// flags to indicate dragging state
	BOOL m_bDragging;
	BOOL m_bDragged;

	// stores the mouse-down point
	CPoint m_ptMouseDown;

	// stores the windows parent CSpaceView window
	CSpaceView *m_pParent;

	// flag to indicate image is to be drawn in background
	BOOL m_bBackgroundImage;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NODEVIEW_H__0C8AA66F_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)

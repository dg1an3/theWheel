//////////////////////////////////////////////////////////////////////
// NodeView.h: interface for the CNodeView class.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(NODEVIEW_H)
#define NODEVIEW_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// vector manipulation
#include <VectorD.h>

// node class
#include <Node.h>

// the skin
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
	CVectorD<3> GetSpringCenter();
	
	// activation value for this window
	REAL GetThresholdedActivation();
	REAL GetSpringActivation();

	// accessors for the node view's rectangles
	CRect& GetOuterRect();
	CRect& GetInnerRect();

	// accessor to shape descriptions of the node view
	CRgn& GetShape();

// Operations
public:
	// smooth updating of spring state variables
	void UpdateSprings(REAL springConst = 0.95);

	// draws the node view
	void Draw(CDC *pDC);

	// draws the links for this nodeview
	void DrawLinks(CDC *pDC, CNodeViewSkin *pSkin);

	// adds an amount of pending activation
	void AddPendingActivation(double pending);

// Implementation
public:
	void ResetPendingActivation();
	REAL GetPendingActivation();

	// drawing helper functions
	void DrawTitleBand(CDC *pDC, CRect& rectInner);
	void DrawTitle(CDC *pDC, CRect& rectInner);
	void DrawText(CDC *pDC, CRect& rectInner);
	void DrawImage(CDC *pDC, CRect& rectInner);

private:
	// stores the node
	CNode* m_pNode;

	// stores the center and spring-connected center of the node view
	CVectorD<3> m_vSpringCenter;

	// stores the thresholded activation and spring-connected activation
	REAL m_springActivation;

	// stores the pending activation
	REAL m_pendingActivation;

	// stores the node view's region (shape)
	CRect m_rectOuter;
	CRect m_rectInner;

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

#endif // !defined(NODEVIEW_H)

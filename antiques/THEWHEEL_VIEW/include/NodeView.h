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

#include <Extent.h>
#include <Spring.h>

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
    // accessor for the node
    CNode *GetNode();

    // moves the nodeview slowly toward its actual center
    CVectorD<3> GetSpringCenter();
    CVectorD<3> GetScaledNodeCenter();

    // activation value for this window
    REAL GetThresholdedActivation();
    REAL GetSpringActivation();

    // accessor to shape descriptions of the node view
    CRgn& GetShape();

// Operations
public:
    // smooth updating of spring state variables
	void UpdateSpringPosition(REAL springConst = 0.95);
	void UpdateSpringActivation(REAL springConst = 0.95);

    // draws the node view
    void Draw(LPDIRECTDRAWSURFACE lpDDS);

    // draws the links for this nodeview
    void DrawLinks(CDC *pDC, CNodeViewSkin *pSkin);

    // adds an amount of pending activation
    void AddPendingActivation(REAL pending);

	void SetMaximized(BOOL bMax);

// Implementation
public:
	// helper functions for pending activation
    void ResetPendingActivation();
    REAL GetPendingActivation();

    // drawing helper functions
    void DrawTitle(CDC *pDC, CExtent<REAL>& rectInner);
    void DrawTitleBand(CDC *pDC, const CExtent<REAL>& rectInner);

	// returns cached title font
	CFont * GetTitleFont(int nDesiredHeight);

    void DrawImage(CDC *pDC, CExtent<REAL>& rectInner);

	// generates / retrieves pre-rendered text surface as DC
    void DrawText(CDC *pDC, CExtent<REAL>& rectInner);
	CDC * GetTextSurface(void);

private:
    // stores the node
    CNode* m_pNode;

    // stores the center and spring-connected center of the node view
    CVectorD<3> m_vSpringCenter;
	CSpring m_posSpring;

    // stores the thresholded activation and spring-connected activation
    REAL m_springActivation;
	CSpring m_actSpring;

	REAL m_layoutSelect;
	CSpring m_layoutSelectSpring;

public:
    // stores the pending activation
    REAL m_pendingActivation;

    // stores the node view's region (shape)
	CExtent<REAL> m_extOuter;
	CExtent<REAL> m_extInner;

    CRgn m_shape;
	CRgn m_shapeHit;

    // flags to indicate dragging state
    BOOL m_bDragging;
    BOOL m_bDragged;

    // stores the mouse-down point
    CPoint m_ptMouseDown;

    // stores the windows parent CSpaceView window
    CSpaceView *m_pParent;

    // flag to indicate image is to be drawn in background
    BOOL m_bBackgroundImage;

	// flag to indicate the view is currently maximized
	BOOL m_bMaximized;

	//////////////////////////////////////////////////////////////////
	// SMOOTH_TEXT variables

	// stores font of text
	static bool m_bStaticInit;
	static CFont m_font;

	// stores line height
	static int m_nSrcLineHeight;

	// stores size of text on bitmap
	CRect m_rectFrom;

	// stores the dc with 
	CDC m_dcText;
	CBitmap m_bitmapText;

	// array of cached title fonts
	static CArray<CFont *, CFont *> m_arrTitleFont;

};	// class CNodeView


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(NODEVIEW_H)

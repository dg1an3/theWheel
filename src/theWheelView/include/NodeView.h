//////////////////////////////////////////////////////////////////////
// NodeView.h: interface for the CNodeView class.
//
// Copyright (C) 1996-2001
// $Id: NodeView.h,v 1.11 2007/07/05 02:49:49 Derek Lane Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(NODEVIEW_H)
#define NODEVIEW_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// vector manipulation
#include <VectorD.h>

// helper for extent
#include <Extent.h>

// node class
#include <Node.h>

// the skin
#include "NodeViewSkin.h"
#include "NodeViewSkin2007.h"

// and springs for positioning
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

	// accessor to extents
	const CExtent<3,REAL>& GetExtOuter();

    // accessor to shape descriptions of the node view
    CRgn& GetShape();
	CRgn& GetShapeHit();

// Operations
public:
    // smooth updating of spring state variables
	void UpdateSpringPosition(REAL springConst = 0.95);
	void UpdateSpringActivation(REAL springConst = 0.95);

    // draws the node view
    void Draw(LPDIRECT3DDEVICE9 lpDDS); // LPDIRECTDRAWSURFACE lpDDS);

    // draws the links for this nodeview
    void DrawLinks(LPDIRECT3DDEVICE9 lpDDS, CNodeViewSkin *pSkin);

    // draws GDI title/text overlay (currently unused; title rendered via D3DXFont in Draw)
    void DrawOverlay(CDC *pDC);

    // adds an amount of pending activation
    void AddPendingActivation(REAL pending);
	void AttenuatePending(REAL atten);

	// sets maximized mode
	void SetMaximized(BOOL bMax);

// Implementation
public:
	// helper functions for pending activation
    void ResetPendingActivation();
    REAL GetPendingActivation();

    // drawing helper functions
    void DrawTitle(CDC *pDC, const CExtent<3,REAL>& extTitle);
    void DrawTitleBand(CDC *pDC, const CExtent<3,REAL>& extTitle);

	// returns cached title font
	CFont * GetTitleFont(int nDesiredHeight);

	// draws the image
    void DrawImage(CDC *pDC, const CExtent<3,REAL>& extImage);

	// generates / retrieves pre-rendered text surface as DC
    void DrawText(CDC *pDC, CExtent<3,REAL>& extDesc);
	CDC * GetTextSurface(void);

	static bool IsActivationGreater(CNodeView *pNodeView1, CNodeView *pNodeView2);
	static bool IsActDiffGreater(CNode *pNode1, CNode *pNode2);

	// returns the inner extent as a Win32 RECT (screen coordinates)
	RECT GetInnerRECT() const
	{
		RECT r;
		r.left   = (LONG) m_extInner.GetMin(0);
		r.top    = (LONG) m_extInner.GetMin(1);
		r.right  = (LONG) m_extInner.GetMax(0);
		r.bottom = (LONG) m_extInner.GetMax(1);
		return r;
	}

	// returns true if the inner extent has a drawable area
	bool HasDrawableArea() const { return m_extInner.GetSize(1) >= 1; }

protected:
    // stores the node view's region (shape)
	CExtent<3,REAL> m_extOuter;
	CExtent<3,REAL> m_extInner;

	// allow NVS access
	friend class CNodeViewSkin;
	friend class theWheel2007::NodeViewSkin;

private:
    // stores the node
    CNode* m_pNode;

    // stores the center and spring-connected center of the node view
    // CVectorD<3> m_vSpringCenter;
	CSpring m_posSpring;

    // stores the thresholded activation and spring-connected activation
    // REAL m_springActivation;
	CSpring m_actSpring;

	// the layout selection value & spring
	REAL m_layoutSelect;
	CSpring m_layoutSelectSpring;

    // stores the pending activation
    REAL m_pendingActivation;

	// stores the node's shapes as regions
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


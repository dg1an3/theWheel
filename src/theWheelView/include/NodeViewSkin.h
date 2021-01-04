//////////////////////////////////////////////////////////////////////
// NodeViewSkin.h: interface for the CNodeViewSkin class.
//
// Copyright (C) 1996-2003 Derek G Lane
// $Id: NodeViewSkin.h,v 1.10 2007/07/05 02:49:11 Derek Lane Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////


#if !defined(NODEVIEWSKIN_H)
#define NODEVIEWSKIN_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Direct3D includes
#include <D3D.h>

// vector class for position calculations
#include <VectorD.h>

#include <Extent.h>

// forward-declare CNodeView
class CNodeView;

//////////////////////////////////////////////////////////////////////
// class CNodeViewSkin
// 
// class to draw the skins of nodes
//////////////////////////////////////////////////////////////////////
class CNodeViewSkin  
{
public:
	CNodeViewSkin();
	virtual ~CNodeViewSkin();

	// sets the client area for this skin
	void SetClientArea(int nWidth, int nHeight, COLORREF colorBk);

	// computes the inner and outer rectangles for a node view, 
	//		given an activation
	void CalcInnerOuterRect(CNodeView *pNodeView);

	REAL GetAspect(REAL activation);
	CVectorD<3> GetNodeSize(CNodeView *pNodeView);

	// computes the shape of a node view, given an activation
	void CalcShape(CNodeView *pNodeView, CRgn& rgn, int nErode = 0);

// Operations
public:
	// blts the skin, if a surface has been prepared
	void BltSkin(LPDIRECT3DDEVICE9 lpDDS, CNodeView *pNodeView);

	// finds (or creates) the DDS for the skin of the specified width
	LPDIRECT3DSURFACE9 GetSkinDDS(CNodeView *pNodeView, CRect& rect);

	// draw the skin, using the direct-draw surface if available
	void DrawSkinGDI(CDC *pDC, CNodeView *pNodeView);

	// draw the skin, using the direct-draw surface if available
	void DrawSkinD3D(LPDIRECT3DDEVICE9 /*IDirect3DDevice2*/ lpD3DDev2, CNodeView *pNodeView);

	// draw a link
	void DrawLink(CDC *pDC, CVectorD<3>& vFrom, REAL actFrom,
		CVectorD<3>& vTo, REAL actTo);

	// initialize the drawing surface
	BOOL InitDDraw(LPDIRECT3DDEVICE9 pd3dDev);

// Implementation
public:
	// accessors for the left-right and top-bottom ellipse rectangles
	void CalcTopBottomEllipseExtent(CNodeView *pNodeView, CExtent<3,REAL>& extent);
	void CalcLeftRightEllipseExtent(CNodeView *pNodeView, CExtent<3,REAL>& extent);

/*	// Direct3D initialization
	BOOL InitD3DDevice(LPDIRECTDRAWSURFACE lpDDS, 
		LPDIRECT3DDEVICE9 *lpD3DDev);
	BOOL InitViewport(LPDIRECT3DDEVICE9 lpD3DDev, CRect rect,
		LPDIRECT3DVIEWPORT2 *lpD3DViewport); */
	BOOL InitLights(/*LPDIRECT3DVIEWPORT2 lpD3DDev, 
		LPDIRECT3DLIGHT *lpD3DLights*/);
	BOOL InitMaterial(); // LPDIRECT3DMATERIAL2 *lpD3DMat);

private:
	// stores the current client area
	int m_nWidth;
	int m_nHeight;

	// holds the background color -- for drawing links
	COLORREF m_colorBk;

	// stores pointer to the main DirectDraw object
	// LPDIRECTDRAW m_lpDD;
	// LPDIRECT3D2 
	LPDIRECT3DDEVICE9 m_lpD3D;

	// pointer to the skin drawing surface for blt
	// CArray<LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE&> m_arrlpSkinDDS;
	CArray<LPDIRECT3DSURFACE9, LPDIRECT3DSURFACE9&> m_arrlpSkinDDS;
	
};	// class CNodeViewSkin


#endif // !defined(NODEVIEWSKIN_H)

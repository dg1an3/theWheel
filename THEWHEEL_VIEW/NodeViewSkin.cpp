//////////////////////////////////////////////////////////////////////
// NodeViewSkin.cpp: implementation of the CNodeViewSkin class.
//
// Copyright (C) 1996-2003 Derek G Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// resources
#include "THEWHEEL_VIEW_resource.h"

// math stuff
#include <math.h>

// DirectDraw utilities
#include "DDUTIL.h"

// header file
#include "NodeViewSkin.h"

// node views
#include "NodeView.h"

// the molding for the skin
#include "Molding.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// Constants for the colors of the CNodeView
//
/////////////////////////////////////////////////////////////////////////////
const COLORREF BACKGROUND_COLOR = RGB(232, 232, 232);

const int THICK_PEN_WIDTH = 4;
const int THIN_PEN_WIDTH = 2;

const COLORREF UPPER_DARK_COLOR = RGB(240, 240, 240);
const COLORREF UPPER_LIGHT_COLOR = RGB(255, 255, 255);

const COLORREF LOWER_LIGHT_COLOR = RGB(160, 160, 160);
const COLORREF LOWER_DARK_COLOR = RGB(128, 128, 128);

const COLORREF COLORKEY = RGB(0, 255, 0);

const REAL BORDER_RADIUS = 10.0;

const int MIN_HEIGHT_ELLIPTANGLE = 15;
const int MAX_HEIGHT_GDI = MIN_HEIGHT_ELLIPTANGLE + 40;


/////////////////////////////////////////////////////////////////////////////
// ComputeEllipticalness
//
// helper function to compute the ellipitcalness given an activation
/////////////////////////////////////////////////////////////////////////////
REAL ComputeEllipticalness(REAL activation)
{
	// compute the r, which represents the amount of "elliptical-ness"
	REAL scale = (1.0 - 1.0 / sqrt(2.0));
	return 1.0 - scale * exp(-8.0 * activation + 0.01);

}	// ComputeEllipticalness


/////////////////////////////////////////////////////////////////////////////
// EvalElliptangle
//
// helper function to evaluate an elliptangle
/////////////////////////////////////////////////////////////////////////////
CVectorD<3, double> EvalElliptangle(double theta, double *p)
{
	// compute theta_prime for the ellipse
	double sign = cos(theta) > 0 ? 1.0 : -1.0;
	double theta_prime = atan2(sign * p[0] * tan(theta), sign * p[1]);

	return CVectorD<3>(p[0] * cos(theta_prime), p[1] * sin(theta_prime), 0.0);

}	// EvalElliptangle


/////////////////////////////////////////////////////////////////////////////
// BlendColors
//
// helper function to blend two colors
/////////////////////////////////////////////////////////////////////////////
COLORREF BlendColors(COLORREF color1, COLORREF color2, REAL frac1)
{
	// compute blended color
	REAL red = (REAL) GetRValue(color1) * frac1 
		+ (REAL) GetRValue(color2) * (1.0 - frac1);
	REAL grn = (REAL) GetGValue(color1) * frac1 
		+ (REAL) GetGValue(color2) * (1.0 - frac1);
	REAL blu = (REAL) GetBValue(color1) * frac1 
		+ (REAL) GetBValue(color2) * (1.0 - frac1);

	// return computed color
	return RGB(red, grn, blu);

}	// BlendColors


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CNodeViewSkin::CNodeViewSkin
//
// returns the outer (bounding) rectangle of the CNodeView
/////////////////////////////////////////////////////////////////////////////
CNodeViewSkin::CNodeViewSkin()
: m_nWidth(0),
	m_nHeight(0),
	m_lpDD(NULL),
	m_lpD3D(NULL),
	m_colorBk(RGB(192, 192, 192))
{
}	// CNodeViewSkin::CNodeViewSkin


/////////////////////////////////////////////////////////////////////////////
// CNodeViewSkin::~CNodeViewSkin
//
// destroys the CNodeViewSkin object
/////////////////////////////////////////////////////////////////////////////
CNodeViewSkin::~CNodeViewSkin()
{
	// sets the client area to 0, 0 (frees surfaces)
	SetClientArea(0, 0, m_colorBk);

	if (m_lpD3D)
		m_lpD3D->Release();

	if (m_lpDD)
		m_lpDD->Release();

}	// CNodeViewSkin::~CNodeViewSkin


/////////////////////////////////////////////////////////////////////////////
// CNodeViewSkin::InitDDraw
//
// initialize the DDraw surface
/////////////////////////////////////////////////////////////////////////////
BOOL CNodeViewSkin::InitDDraw(LPDIRECTDRAW lpDD)
{
	// store a pointer to the main DirectDraw object
	m_lpDD = lpDD;
	m_lpDD->AddRef();

	// get the Direct3D2 interface
	CHECK_HRESULT(m_lpDD->QueryInterface(IID_IDirect3D2, (void**)&m_lpD3D));

	return TRUE;

}	// CNodeViewSkin::InitDDraw


/////////////////////////////////////////////////////////////////////////////
// CNodeViewSkin::SetClientArea
//
// sets the client area dimensions and color for this skin
/////////////////////////////////////////////////////////////////////////////
void CNodeViewSkin::SetClientArea(int nWidth, int nHeight, COLORREF colorBk)
{
	// set the width and height parameters
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	// destroy the surfaces
	for (int nAt = 0; nAt < m_arrlpSkinDDS.GetSize(); nAt++)
	{
		if (m_arrlpSkinDDS[nAt] != NULL)
		{
			m_arrlpSkinDDS[nAt]->Release();
		}
	}

	// remove all elements
	m_arrlpSkinDDS.RemoveAll();

	// size to max client area
	m_arrlpSkinDDS.SetSize(m_nWidth);

	// set background color
	m_colorBk = colorBk;

}	// CNodeViewSkin::SetClientArea


/////////////////////////////////////////////////////////////////////////////
// CNodeViewSkin::CalcOuterRect
//
// calculates the outer (bounding) rectangle of the CNodeView
/////////////////////////////////////////////////////////////////////////////
CRect& CNodeViewSkin::CalcOuterRect(CNodeView *pNodeView)
{
	// compute the new width and height from the desired area and the desired
	//		aspect ratio
	REAL scale = (REAL) sqrt(m_nWidth * m_nHeight);

	// get the size for the node view, given the spring activation
	CVectorD<3> vSize = 
		pNodeView->GetNode()->GetSize(pNodeView->GetSpringActivation());

	// get the width and height for the node view
	int nWidth = (int) (1.1 * vSize[0] * scale); // * 0.75
		// + 0.25 * (REAL) rectOuterOld.Width());
	int nHeight = (int) (1.1 * vSize[1] * scale); // * 0.75
		// + 0.25 * (REAL) rectOuterOld.Height());

	// set the width and height of the window, keeping the center constant
	CRect& rectOuter = pNodeView->GetOuterRect();
	rectOuter.left =	- nWidth / 2;
	rectOuter.right =	+ nWidth / 2;
	rectOuter.top =		- nHeight / 2;
	rectOuter.bottom =	+ nHeight / 2;

	// position the rectangle
	rectOuter.OffsetRect((int) pNodeView->GetSpringCenter()[0], 
		(int) pNodeView->GetSpringCenter()[1]);

	return rectOuter;

}	// CNodeViewSkin::CalcOuterRect


/////////////////////////////////////////////////////////////////////////////
// CNodeViewSkin::CalcInnerRect
//
// calculates the inner rectangle of the CNodView
// uses the nodeview's existing OuterRect; does not recalc
//		these
/////////////////////////////////////////////////////////////////////////////
CRect& CNodeViewSkin::CalcInnerRect(CNodeView *pNodeView)
{
	// get a reference to the outer rectangle
	const CRect& rectOuter = pNodeView->GetOuterRect();

	// compute the r, which represents the amount of "elliptical-ness"
	//		(ratio of inner / outer)
	REAL r = ComputeEllipticalness(pNodeView->GetSpringActivation());

	// initialize the inner rectangle
	CRect& rectInner = pNodeView->GetInnerRect();
	rectInner = rectOuter;

	// adjust inner rectangle proportions
	int ndWidth = (1.0 - r) / 2.0 * rectOuter.Width();
	int ndHeight = (1.0 - r) / 2.0 * rectOuter.Height();
	rectInner.DeflateRect(ndWidth, ndHeight, ndWidth, ndHeight);

	return rectInner;

}	// CNodeViewSkin::CalcInnerRect


//////////////////////////////////////////////////////////////////////
// CNodeViewSkin::CalcShape
// 
// calculates the node view's region (shape)
// uses the nodeview's existing OuterRect and InnerRect; does not recalc
//		these
//////////////////////////////////////////////////////////////////////
void CNodeViewSkin::CalcShape(CNodeView *pNodeView, CRgn& rgn, int nErode)
{
	// re-create the region object as a rectangle (initially)
	rgn.DeleteObject();
	rgn.CreateRectRgnIndirect(pNodeView->GetInnerRect());

	// form the ellipse for top/bottom clipping
	CRect rectTopBottomEllipse = CalcTopBottomEllipseRect(pNodeView);
	rectTopBottomEllipse.DeflateRect(nErode, nErode, nErode, nErode);
	CRgn ellipseTopBottom;
	ellipseTopBottom.CreateEllipticRgnIndirect(&rectTopBottomEllipse);

	// form the ellipse for left/right clipping
	CRect rectLeftRightEllipse = CalcLeftRightEllipseRect(pNodeView);
	rectLeftRightEllipse.DeflateRect(nErode, nErode, nErode, nErode);
	CRgn ellipseLeftRight;
	ellipseLeftRight.CreateEllipticRgnIndirect(&rectLeftRightEllipse);

	// form the intersection of the two
	rgn.CombineRgn(&ellipseTopBottom, &ellipseLeftRight, RGN_AND);

	// delete the formant regions
	ellipseTopBottom.DeleteObject();
	ellipseLeftRight.DeleteObject();

}	// CNodeViewSkin::CalcShape


/////////////////////////////////////////////////////////////////////////////
// CNodeViewSkin::CalcLeftRightEllipseRect
//
// calculates the rectangle that bounds the ellipse for the left and right
//		sides of the elliptangle
// uses the nodeview's existing OuterRect and InnerRect; does not recalc
//		these
/////////////////////////////////////////////////////////////////////////////
CRect CNodeViewSkin::CalcLeftRightEllipseRect(CNodeView *pNodeView)
{
	const CRect& rectOuter = pNodeView->GetOuterRect();
	const CRect& rectInner = pNodeView->GetInnerRect();

	// compute the ellipse's b
	REAL rx = ((REAL) rectInner.Width())
			/ ((REAL) rectOuter.Width()); 
	REAL bx = (REAL) sqrt((REAL) (rectInner.Height() 
		* rectInner.Height()) * 0.25f
			/ (1.0f - rx * rx));

	// now compute the ellipse's rectangle
	CRect rectLeftRightEllipse = rectOuter;
	rectLeftRightEllipse.top = (rectOuter.top + rectOuter.Height() / 2) - (long) bx;
	rectLeftRightEllipse.bottom = (rectOuter.top + rectOuter.Height() / 2) + (long) bx;

	return rectLeftRightEllipse;

}	// CNodeViewSkin::CalcLeftRightEllipseRect


/////////////////////////////////////////////////////////////////////////////
// CNodeViewSkin::CalcTopBottomEllipseRect
//
// calculates the rectangle that bounds the ellipse for the top and bottom
//		sides of the elliptangle
// uses the nodeview's existing OuterRect and InnerRect; does not recalc
//		these
/////////////////////////////////////////////////////////////////////////////
CRect CNodeViewSkin::CalcTopBottomEllipseRect(CNodeView *pNodeView)
{
	const CRect& rectOuter = pNodeView->GetOuterRect();
	const CRect& rectInner = pNodeView->GetInnerRect();

	// compute the ellipse's b
	REAL ry = ((REAL) rectInner.Height())
			/ ((REAL) rectOuter.Height()); 
	REAL by = (REAL) sqrt((REAL) (rectInner.Width() * rectInner.Width()) * 0.25f
			/ (1.0f - ry * ry));

	// now compute the ellipse's rectangle
	CRect rectTopBottomEllipse = rectOuter;
	rectTopBottomEllipse.left = (rectOuter.left + rectOuter.Width() / 2) - (long) by;
	rectTopBottomEllipse.right = (rectOuter.left + rectOuter.Width() / 2) + (long) by;

	return rectTopBottomEllipse;

}	// CNodeViewSkin::CalcTopBottomEllipseRect


/////////////////////////////////////////////////////////////////////////////
// CNodeViewSkin::InitD3DDevice
//
// initializes the D3D device upon which rendering will occur
/////////////////////////////////////////////////////////////////////////////
BOOL CNodeViewSkin::InitD3DDevice(LPDIRECTDRAWSURFACE lpDDS,
							LPDIRECT3DDEVICE2 *lppD3DDev)
{
    // create a Direct3D device.
    CHECK_HRESULT(m_lpD3D->CreateDevice(IID_IDirect3DRGBDevice, 
		lpDDS, lppD3DDev));

    // Set default render state
    CHECK_HRESULT((*lppD3DDev)->SetRenderState(D3DRENDERSTATE_ZENABLE, 0));
    CHECK_HRESULT((*lppD3DDev)->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 0));
    CHECK_HRESULT((*lppD3DDev)->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD));
	CHECK_HRESULT((*lppD3DDev)->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE));
	CHECK_HRESULT((*lppD3DDev)->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));

	// set up an ambient light source
	CHECK_HRESULT((*lppD3DDev)->SetLightState(D3DLIGHTSTATE_AMBIENT, 
		(DWORD) D3DRGB(0.25, 0.25, 0.25)));

	return TRUE;

}	// CNodeViewSkin::InitD3DDevice


/////////////////////////////////////////////////////////////////////////////
// CNodeViewSkin::InitViewport
//
// initializes the viewport within which rendering will occur
/////////////////////////////////////////////////////////////////////////////
BOOL CNodeViewSkin::InitViewport(LPDIRECT3DDEVICE2 lpD3DDev, CRect rect,
						   LPDIRECT3DVIEWPORT2 *lppViewport)
{
    // now make a Viewport
    CHECK_HRESULT(m_lpD3D->CreateViewport(lppViewport, NULL));
    CHECK_HRESULT(lpD3DDev->AddViewport(*lppViewport));

	// set the current viewport
    CHECK_HRESULT(lpD3DDev->SetCurrentViewport(*lppViewport));

    // Setup the viewport for a reasonable viewing area
    D3DVIEWPORT2 viewData;
    memset(&viewData, 0, sizeof(D3DVIEWPORT2));
    viewData.dwSize = sizeof(D3DVIEWPORT2);
    viewData.dwX = 0;
    viewData.dwY = 0;
    viewData.dwWidth  = rect.Width();
    viewData.dwHeight = rect.Height();
    viewData.dvClipX = -1.0f;
    viewData.dvClipWidth = 2.0f;
    viewData.dvClipHeight = (D3DVALUE)(rect.Height() * 2.0 / rect.Width());
    viewData.dvClipY = viewData.dvClipHeight / 2.0f;
    viewData.dvMinZ = 0.0f;
    viewData.dvMaxZ = 1.0f;
    CHECK_HRESULT((*lppViewport)->SetViewport2(&viewData));

	return TRUE;

}	// CNodeViewSkin::InitViewport

    
/////////////////////////////////////////////////////////////////////////////
// CNodeViewSkin::InitLights
//
// initializes the viewport within which rendering will occur
/////////////////////////////////////////////////////////////////////////////
BOOL CNodeViewSkin::InitLights(LPDIRECT3DVIEWPORT2 lpViewport, 
						 LPDIRECT3DLIGHT *lppLights)
{
	D3DLIGHT2 light;
	ZeroMemory(&light, sizeof(D3DLIGHT2));
	light.dwSize = sizeof(D3DLIGHT2);
    light.dltType = D3DLIGHT_DIRECTIONAL;
    light.dcvColor.r = (D3DVALUE) 0.8;
    light.dcvColor.g = (D3DVALUE) 0.8;
    light.dcvColor.b = (D3DVALUE) 0.8;
    light.dvDirection.x = (D3DVALUE) 1.0;
    light.dvDirection.y = (D3DVALUE) -1.0;
    light.dvDirection.z = (D3DVALUE) -0.1;
	light.dvRange = D3DLIGHT_RANGE_MAX;
	light.dwFlags = D3DLIGHT_ACTIVE;
	CHECK_HRESULT(m_lpD3D->CreateLight(&lppLights[0], NULL));
	CHECK_HRESULT(lppLights[0]->SetLight((LPD3DLIGHT) &light));
	CHECK_HRESULT(lpViewport->AddLight(lppLights[0]));

    light.dcvColor.r = (D3DVALUE) 0.6;
    light.dcvColor.g = (D3DVALUE) 0.6;
    light.dcvColor.b = (D3DVALUE) 0.6;
    light.dvDirection.x = (D3DVALUE) 0.0;
    light.dvDirection.y = (D3DVALUE) 0.0;
    light.dvDirection.z = (D3DVALUE) -1.0;
	CHECK_HRESULT(m_lpD3D->CreateLight(&lppLights[1], NULL));
	CHECK_HRESULT(lppLights[1]->SetLight((LPD3DLIGHT) &light));
	CHECK_HRESULT(lpViewport->AddLight(lppLights[1]));

	return TRUE;

}	// CNodeViewSkin::InitLights


/////////////////////////////////////////////////////////////////////////////
// CNodeViewSkin::InitMaterial
//
// initializes the material for rendering
/////////////////////////////////////////////////////////////////////////////
BOOL CNodeViewSkin::InitMaterial(LPDIRECT3DMATERIAL2 *lppD3DMat)
{
	// initialize material
    CHECK_HRESULT(m_lpD3D->CreateMaterial(lppD3DMat, NULL)); 

    D3DMATERIAL mat;
    memset(&mat, 0, sizeof(D3DMATERIAL));
    mat.dwSize = sizeof(D3DMATERIAL);
    mat.diffuse.r = (D3DVALUE)1.0;
    mat.diffuse.g = (D3DVALUE)1.0;
    mat.diffuse.b = (D3DVALUE)1.0;
    mat.ambient.r = (D3DVALUE)1.0;
    mat.ambient.g = (D3DVALUE)1.0;
    mat.ambient.b = (D3DVALUE)1.0;
    mat.specular.r = (D3DVALUE)1.0;
    mat.specular.g = (D3DVALUE)1.0;
    mat.specular.b = (D3DVALUE)1.0;
    mat.power = (float)1.0;
    mat.hTexture = 0; // TextureHandle;
    mat.dwRampSize = 1;
    CHECK_HRESULT((*lppD3DMat)->SetMaterial(&mat));

    return TRUE;

}	// CNodeViewSkin::InitMaterial


//////////////////////////////////////////////////////////////////////
// CNodeViewSkin::BltSkin
// 
// blts the skin for the node view from the prepared surface
//////////////////////////////////////////////////////////////////////
void CNodeViewSkin::BltSkin(LPDIRECTDRAWSURFACE lpDDS, CNodeView *pNodeView)
{
	// calculate the rectangles
	const CRect& rectOuter = CalcOuterRect(pNodeView);
	const CRect& rectInner = CalcInnerRect(pNodeView);

	// calculate the node-views shape rgn, for hit-testing
	CalcShape(pNodeView, pNodeView->GetShape(), THICK_PEN_WIDTH);

	if (rectOuter.Height() < 6)
	{
		return;
	}

	// gets the skin, plus the actual rectangle for the skin
	CRect rectDest;
	LPDIRECTDRAWSURFACE lpSkinDDS = GetSkinDDS(pNodeView, rectDest);

	// form the client rectangle
	CRect rectClient(0, 0, m_nWidth, m_nHeight);

	// form the intersection of the destination and client
	CRect rectDestIntersect;
	BOOL bIntersect = rectDestIntersect.IntersectRect(rectClient, rectDest);

	// if the destination is completely within the client,
	if (rectDestIntersect == rectDest)
	{
		// perform a fast blt
		ASSERT_HRESULT(lpDDS->BltFast(rectDest.left, rectDest.top, 
			lpSkinDDS, rectDest - rectDest.TopLeft(),
			DDBLTFAST_SRCCOLORKEY));
	}
	// was there at least some intersection?
	else if (bIntersect)
	{
		// perform a slower blt
		ASSERT_HRESULT(lpDDS->Blt(rectDestIntersect, lpSkinDDS,
			rectDestIntersect - rectDest.TopLeft(), DDBLT_KEYSRC, NULL));
	}

}	// CNodeViewSkin::BltSkin


//////////////////////////////////////////////////////////////////////
// CNodeViewSkin::GetSkinDDS
// 
// blts the skin for the node view from the prepared surface
// assumes the node view's outer and inner rectangles have been
//		computed
//////////////////////////////////////////////////////////////////////
LPDIRECTDRAWSURFACE CNodeViewSkin::GetSkinDDS(CNodeView *pNodeView, 
											  CRect& rectSrc)
{
	// get the skin rectangle
	rectSrc = pNodeView->GetOuterRect();

	// compute width (= index into cache)
	int nWidth = rectSrc.Width();

	// stores the new DDS
	LPDIRECTDRAWSURFACE lpDDS = m_arrlpSkinDDS.GetAt(nWidth);

	// generate it, if needed 
	if (!lpDDS)
	{
		// inflate rectangle to account for margin
		rectSrc.InflateRect(20, 20, 20, 20); 

		// create a new drawing surface
		DDSURFACEDESC	ddsd;
		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
		ddsd.dwWidth = rectSrc.Width();
		ddsd.dwHeight = rectSrc.Height();
		ASSERT_HRESULT(m_lpDD->CreateSurface(&ddsd, &lpDDS, NULL));

		// fill the surface
		DDBLTFX ddBltFx;
		ddBltFx.dwSize = sizeof(DDBLTFX);
		ddBltFx.dwFillColor = (DWORD) COLORKEY; 
		ASSERT_HRESULT(lpDDS->Blt(rectSrc - rectSrc.TopLeft(), NULL, 
			rectSrc - rectSrc.TopLeft(), DDBLT_COLORFILL, &ddBltFx));

		// set the color key
		ASSERT_HRESULT(DDSetColorKey(lpDDS, COLORKEY));

		// use GDI calls for small
		if (rectSrc.Height() < MAX_HEIGHT_GDI)
		{
			// form a CDC for the surface
			CDC dc;
			GET_ATTACH_DC(lpDDS, dc);

			// offset to the top of the rectangle
			dc.OffsetWindowOrg(rectSrc.left, rectSrc.top);

			// draw the skin
			DrawSkinGDI(&dc, pNodeView);

			// release and detach
			RELEASE_DETACH_DC(lpDDS, dc);
		}
		else
		{
			// Direct3D rendering -- initialize the objects first
			LPDIRECT3DDEVICE2 lpD3DDev = NULL;
			ASSERT_BOOL(InitD3DDevice(lpDDS, &lpD3DDev));

			LPDIRECT3DVIEWPORT2	lpViewport = NULL;
			ASSERT_BOOL(InitViewport(lpD3DDev, rectSrc, &lpViewport));

			// set up the zoom transform, accounting for rectangle
			//		inflation
			D3DMATRIX mat;
			ZeroMemory(&mat, sizeof(D3DMATRIX));
			mat(0, 0) = (D3DVALUE) 1.0 / (rectSrc.Width() + 10.0);
			mat(1, 1) = (D3DVALUE) 1.0 / (rectSrc.Width() + 10.0);
			mat(2, 2) = (D3DVALUE) 1.0; 
			mat(3, 3) = (D3DVALUE) 1.0;
			CHECK_HRESULT(lpD3DDev->SetTransform(D3DTRANSFORMSTATE_VIEW, &mat));

			LPDIRECT3DLIGHT lpLights[2];
			ASSERT_BOOL(InitLights(lpViewport, lpLights));

			// create the material and attach to the device's state
			LPDIRECT3DMATERIAL2	lpMaterial = NULL;
			ASSERT_BOOL(InitMaterial(&lpMaterial));

			D3DTEXTUREHANDLE hMat;
			ASSERT_HRESULT(lpMaterial->GetHandle(lpD3DDev, &hMat));
			ASSERT_HRESULT(lpD3DDev->SetLightState(D3DLIGHTSTATE_MATERIAL, hMat));

			// render the skin
			ASSERT_HRESULT(lpD3DDev->BeginScene());
			DrawSkinD3D(lpD3DDev, pNodeView);
			ASSERT_HRESULT(lpD3DDev->EndScene());

			// release the interface
			ASSERT_HRESULT(lpMaterial->Release());
			ASSERT_HRESULT(lpLights[0]->Release());
			ASSERT_HRESULT(lpLights[1]->Release());
			ASSERT_HRESULT(lpViewport->Release());
			ASSERT_HRESULT(lpD3DDev->Release());
		}

		// store the newly-formed DDS
		m_arrlpSkinDDS.SetAt(nWidth, lpDDS);
	}
	else
	{
		// make sure rectSrc is the correct size
		DDSURFACEDESC sd;
		sd.dwSize = sizeof(DDSURFACEDESC);
		lpDDS->GetSurfaceDesc(&sd);

		// store the current center point
		CPoint ptCenter = rectSrc.CenterPoint();

		// initialize to a correctly sized rectangle
		rectSrc = CRect(0, 0, sd.dwWidth, sd.dwHeight);

		// offset to correct center point
		rectSrc.OffsetRect(ptCenter - rectSrc.CenterPoint());
		ASSERT(rectSrc.CenterPoint() == ptCenter);
	}

	// return the DDS
	return lpDDS;

}	// CNodeViewSkin::GetSkinDDS


//////////////////////////////////////////////////////////////////////
// CNodeViewSkin::DrawSkinGDI
// 
// draws the skin for the node view using GDI calls
// assumes the node view's outer and inner rectangles have been
//		computed
//////////////////////////////////////////////////////////////////////
void CNodeViewSkin::DrawSkinGDI(CDC *pDC, CNodeView *pNodeView)
{	
	// calculate the rectangles
	const CRect& rectOuter = pNodeView->GetOuterRect();
	const CRect& rectInner = pNodeView->GetInnerRect();

	// get the guide rectangles
	CRect rectLeftRightEllipse = CalcLeftRightEllipseRect(pNodeView);
	CRect rectTopBottomEllipse = CalcTopBottomEllipseRect(pNodeView);

	// store the old pen and brush
	CPen *pOldPen = NULL;
	CBrush *pOldBrush = NULL;

	// select attributes for background
	// if height > MIN_HEIGHT_ELLIPTANGLE, draw true elliptangle
	if (rectOuter.Height() > MIN_HEIGHT_ELLIPTANGLE)
	{
		CBrush backBrush(BACKGROUND_COLOR);
		pOldBrush = pDC->SelectObject(&backBrush);
		pOldPen = (CPen *)pDC->SelectStockObject(NULL_PEN);

		// now draw the background
		pDC->FillSolidRect(rectInner, BACKGROUND_COLOR);
		pDC->Chord(&rectLeftRightEllipse, rectInner.TopLeft(), 
			CPoint(rectInner.left, rectInner.bottom));
		pDC->Chord(&rectTopBottomEllipse, CPoint(rectInner.right, rectInner.top), 
			rectInner.TopLeft());
		pDC->Chord(&rectLeftRightEllipse, rectInner.BottomRight(), 
			CPoint(rectInner.right, rectInner.top));
		pDC->Chord(&rectTopBottomEllipse, CPoint(rectInner.left, rectInner.bottom), 
			rectInner.BottomRight());

		// draw the upper-left half of the outline
		CPen penUpperLeftThick(PS_SOLID, THICK_PEN_WIDTH, UPPER_DARK_COLOR);
		pDC->SelectObject(&penUpperLeftThick);
		pDC->Arc(&rectLeftRightEllipse, rectInner.TopLeft(), 
			CPoint(rectInner.left, rectInner.bottom));
		pDC->Arc(&rectTopBottomEllipse, CPoint(rectInner.right, rectInner.top), 
			rectInner.TopLeft());

		CPen penUpperLeftThin(PS_SOLID, THIN_PEN_WIDTH, UPPER_LIGHT_COLOR);
		pDC->SelectObject(&penUpperLeftThin);
		pDC->Arc(&rectLeftRightEllipse, rectInner.TopLeft(), 
			CPoint(rectInner.left, rectInner.bottom));
		pDC->Arc(&rectTopBottomEllipse, CPoint(rectInner.right, rectInner.top), 
			rectInner.TopLeft());

		// draw the lower-right half of the outline
		CPen penLowerRightThick(PS_SOLID, THICK_PEN_WIDTH, LOWER_LIGHT_COLOR);
		pDC->SelectObject(&penLowerRightThick);
		pDC->Arc(&rectLeftRightEllipse, rectInner.BottomRight(), 
			CPoint(rectInner.right, rectInner.top));
		pDC->Arc(&rectTopBottomEllipse, CPoint(rectInner.left, rectInner.bottom), 
			rectInner.BottomRight());

		CPen penLowerRightThin(PS_SOLID, THIN_PEN_WIDTH, LOWER_DARK_COLOR);
		pDC->SelectObject(&penLowerRightThin);
		pDC->Arc(&rectLeftRightEllipse, rectInner.BottomRight(), 
			CPoint(rectInner.right, rectInner.top));
		pDC->Arc(&rectTopBottomEllipse, CPoint(rectInner.left, rectInner.bottom), 
			rectInner.BottomRight());
	}
	else
	{
		// fraction of ellipse color
		REAL fracEllipseColor = (REAL) rectOuter.Height() 
			/ (REAL) MIN_HEIGHT_ELLIPTANGLE;

		// form the brush and pen
		CBrush backBrush(BlendColors(BACKGROUND_COLOR, m_colorBk, fracEllipseColor));
		pOldBrush = pDC->SelectObject(&backBrush);
		pOldPen = (CPen *)pDC->SelectStockObject(NULL_PEN);

		pDC->Ellipse(rectOuter);

		// restore the old pen 
		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);
	}

}	// CNodeViewSkin::DrawSkinGDI


//////////////////////////////////////////////////////////////////////
// CNodeViewSkin::DrawSkinD3D
// 
// draws the skin for the node view using Direct3D calls
// assumes the node view's outer and inner rectangles have been
//		computed
//////////////////////////////////////////////////////////////////////
void CNodeViewSkin::DrawSkinD3D(IDirect3DDevice2 *lpD3DDev, 
								CNodeView *pNodeView)
{
	// stores the node's aspect ratio
	const CRect& rectOuter = pNodeView->GetOuterRect();

	// compute the angle dividing the sections of the elliptangle
	double sectionAngle = atan2(rectOuter.Height(), rectOuter.Width());

	// compute the parameters for the vertical sides
	CRect rectLR = CalcLeftRightEllipseRect(pNodeView);
	double pVert[2] = {rectLR.Width(), rectLR.Height()};

	// compute the parameters for the horizontal sides
	CRect rectTB = CalcTopBottomEllipseRect(pNodeView);
	double pHoriz[2] = {rectTB.Width(), rectTB.Height()};

	// create a previous molding, to connect to next
	CMolding* pPrev = new CMolding(2*PI, EvalElliptangle(2*PI, pVert), 
		(rectOuter.Height() > BORDER_RADIUS * 4) ? BORDER_RADIUS 
			: rectOuter.Height() / 4);

	// now render the four sections of the molding
	pPrev = CMolding::RenderMoldingSection(lpD3DDev, 
		2*PI, 2*PI + sectionAngle, EvalElliptangle, pVert, pPrev);
	pPrev = CMolding::RenderMoldingSection(lpD3DDev,        
		sectionAngle,   PI - sectionAngle, EvalElliptangle, pHoriz, pPrev);
	pPrev = CMolding::RenderMoldingSection(lpD3DDev, 
		PI - sectionAngle,   PI + sectionAngle, EvalElliptangle, pVert, pPrev);
	pPrev = CMolding::RenderMoldingSection(lpD3DDev, 
		PI + sectionAngle, 2*PI - sectionAngle, EvalElliptangle, pHoriz, pPrev);
	pPrev = CMolding::RenderMoldingSection(lpD3DDev, 
		2*PI - sectionAngle, 2*PI + 0.1, EvalElliptangle, pVert, pPrev);

	// delete the connecting molding
	delete pPrev;

}	// CNodeViewSkin::DrawSkinD3D


//////////////////////////////////////////////////////////////////////
// CNodeViewSkin::DrawLink
// 
// draw a link
//////////////////////////////////////////////////////////////////////
void CNodeViewSkin::DrawLink(CDC *pDC, CVectorD<3>& vFrom, REAL actFrom,
			  CVectorD<3>& vTo, REAL actTo)
{
	// brushes for drawing links
	CBrush brushDark(RGB(0.85 * GetRValue(m_colorBk),
		0.85 * GetGValue(m_colorBk),
		0.85 * GetBValue(m_colorBk)));
	CBrush brushMid(RGB(0.90 * GetRValue(m_colorBk),
		0.90 * GetGValue(m_colorBk),
		0.90 * GetBValue(m_colorBk)));
	CBrush brushLight(RGB(0.95 * GetRValue(m_colorBk),
		0.95 * GetGValue(m_colorBk),
		0.95 * GetBValue(m_colorBk)));

	CBrush *pOldBrush;
	if (actFrom + actTo < 0.025)
		pOldBrush = pDC->SelectObject(&brushLight);
	else if (actFrom + actTo < 0.05)
		pOldBrush = pDC->SelectObject(&brushMid);
	else
		pOldBrush = pDC->SelectObject(&brushDark);
	CPen *pOldPen = (CPen *)pDC->SelectStockObject(NULL_PEN);

	CVectorD<3> vOffset = vTo - vFrom;
	CVectorD<3> vNormal(vOffset[1], -vOffset[0]);
	vNormal.Normalize();

	REAL fromSize = (REAL) sqrt(actFrom);
	REAL toSize = (REAL) sqrt(actTo);
	REAL midSize = 0.5f * (fromSize + toSize);

	vOffset *= (REAL) 0.1;

	CPoint ptPoly[8];
	ptPoly[0] = vFrom;
	ptPoly[1] = vFrom +				 vOffset + fromSize * 60.0f * vNormal;
	ptPoly[2] = vFrom + (REAL) 5.0 * vOffset +  midSize * 15.0f * vNormal;
	ptPoly[3] = vFrom + (REAL) 9.0 * vOffset +   toSize * 60.0f * vNormal;
	ptPoly[4] = vTo;
	ptPoly[5] = vFrom + (REAL) 9.0 * vOffset -   toSize * 60.0f * vNormal;
	ptPoly[6] = vFrom + (REAL) 5.0 * vOffset -  midSize * 15.0f * vNormal;
	ptPoly[7] = vFrom +				 vOffset - fromSize * 60.0f * vNormal;

	// now draw the polygon
	pDC->Polygon(ptPoly, 8);

	// unselect the brsuh and pen for drawing the links
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

}	// CNodeViewSkin::DrawLink


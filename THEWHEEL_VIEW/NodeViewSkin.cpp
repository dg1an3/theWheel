// NodeViewSkin.cpp: implementation of the CNodeViewSkin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
// #include "thewheel2001.h"
#include "NodeViewSkin.h"

#include "NodeView.h"

#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// Constants for the colors of the CNodeView
//
/////////////////////////////////////////////////////////////////////////////
COLORREF BACKGROUND_COLOR = RGB(232, 232, 232);

COLORREF UPPER_DARK_COLOR = RGB(240, 240, 240);
COLORREF UPPER_LIGHT_COLOR = RGB(255, 255, 255);

COLORREF LOWER_LIGHT_COLOR = RGB(160, 160, 160);
COLORREF LOWER_DARK_COLOR = RGB(128, 128, 128);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNodeViewSkin::CNodeViewSkin()
: m_nWidth(0),
	m_nHeight(0)
{
}

CNodeViewSkin::~CNodeViewSkin()
{

}


// sets the client area for this skin
void CNodeViewSkin::SetClientArea(int nWidth, int nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}


/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetOuterRect
//
// returns the outer (bounding) rectangle of the elliptangle
/////////////////////////////////////////////////////////////////////////////
CRect CNodeViewSkin::CalcOuterRect(CNodeView *pNodeView)
{
	// compute the new width and height from the desired area and the desired
	//		aspect ratio
	REAL scale = (REAL) sqrt(m_nWidth * m_nHeight);

	// get the size for the node view, given the spring activation
	CVector<3> vSize = 
		pNodeView->GetNode()->GetSize(pNodeView->GetSpringActivation());

	// get the width and height for the node view
	int nWidth = (int) (vSize[0] * scale);
	int nHeight = (int) (vSize[1] * scale);

	// set the width and height of the window, keeping the center constant
	CRect rect;
	rect.left =		- nWidth / 2;
	rect.right =	+ nWidth / 2;
	rect.top =		- nHeight / 2;
	rect.bottom =	+ nHeight / 2;

	return rect;
}

/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetInnerRect
//
// returns the inner rectangle of the elliptangle
/////////////////////////////////////////////////////////////////////////////
CRect CNodeViewSkin::CalcInnerRect(CNodeView *pNodeView)
{
	// compute the r, which represents the amount of "elliptical-ness"
	REAL r = 1.0f - (1.0f - 1.0f / (REAL) sqrt(2.0f))
		* (REAL) exp(-8.0f * pNodeView->GetSpringActivation());

	CRect rectOuter = CalcOuterRect(pNodeView);
	CRect rectInner;
	rectInner.top = (long) ((REAL) (rectOuter.top + rectOuter.Height() / 2) 
		- r * (REAL) rectOuter.Height() / 2.0f);
	rectInner.bottom = (long) ((REAL) (rectOuter.top + rectOuter.Height() / 2) 
		+ r * (REAL) rectOuter.Height() / 2.0f);
	rectInner.left = (long) ((REAL) (rectOuter.left + rectOuter.Width() / 2) 
		- r * (REAL) rectOuter.Width() / 2.0f);
	rectInner.right = (long) ((REAL) (rectOuter.left + rectOuter.Width() / 2) 
		+ r * (REAL) rectOuter.Width() / 2.0f);

	return rectInner;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::GetShape
// 
// returns and computes (if needed) the node view's region (shape)
//////////////////////////////////////////////////////////////////////
void CNodeViewSkin::CalcShape(CNodeView *pNodeView, CRgn *pRgn, int nErode)
{
	// delete the region object
	pRgn->DeleteObject();

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
	pRgn->CreateRectRgnIndirect(CalcInnerRect(pNodeView));
	pRgn->CombineRgn(&ellipseTopBottom, &ellipseLeftRight, RGN_AND);

	// delete the formant regions
	ellipseTopBottom.DeleteObject();
	ellipseLeftRight.DeleteObject();
}

/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetLeftRightEllipseRect
//
// returns the rectangle that bounds the ellipse for the left and right
//		sides of the elliptangle
/////////////////////////////////////////////////////////////////////////////
CRect CNodeViewSkin::CalcLeftRightEllipseRect(CNodeView *pNodeView)
{
	CRect rectOuter = CalcOuterRect(pNodeView);
	CRect rectInner = CalcInnerRect(pNodeView);

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
}

/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetTopBottomEllipseRect
//
// returns the rectangle that bounds the ellipse for the top and bottom
//		sides of the elliptangle
/////////////////////////////////////////////////////////////////////////////
CRect CNodeViewSkin::CalcTopBottomEllipseRect(CNodeView *pNodeView)
{
	CRect rectOuter = CalcOuterRect(pNodeView);
	CRect rectInner = CalcInnerRect(pNodeView);

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
}

//////////////////////////////////////////////////////////////////////
// CNodeView::DrawElliptangle
// 
// draws the elliptangle for the node
//////////////////////////////////////////////////////////////////////
void CNodeViewSkin::DrawSkin(CDC *pDC, CNodeView *pNodeView)
{
	// reset the inner rectangle
	CRect rectInner = CalcInnerRect(pNodeView);

	// get the guide rectangles
	CRect rectLeftRightEllipse = CalcLeftRightEllipseRect(pNodeView);
	CRect rectTopBottomEllipse = CalcTopBottomEllipseRect(pNodeView);

	// store the old pen and brush
	CPen *pOldPen = NULL;
	CBrush *pOldBrush = NULL;

	// select attributes for background
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
	CPen penUpperLeftThick(PS_SOLID, 4, UPPER_DARK_COLOR);
	// pOldPen = (CPen *)
		pDC->SelectObject(&penUpperLeftThick);
	pDC->Arc(&rectLeftRightEllipse, rectInner.TopLeft(), 
		CPoint(rectInner.left, rectInner.bottom));
	pDC->Arc(&rectTopBottomEllipse, CPoint(rectInner.right, rectInner.top), 
		rectInner.TopLeft());

	CPen penUpperLeftThin(PS_SOLID, 2, UPPER_LIGHT_COLOR);
	pDC->SelectObject(&penUpperLeftThin);
	pDC->Arc(&rectLeftRightEllipse, rectInner.TopLeft(), 
		CPoint(rectInner.left, rectInner.bottom));
	pDC->Arc(&rectTopBottomEllipse, CPoint(rectInner.right, rectInner.top), 
		rectInner.TopLeft());

	// draw the lower-right half of the outline
	CPen penLowerRightThick(PS_SOLID, 4, LOWER_LIGHT_COLOR);
	pDC->SelectObject(&penLowerRightThick);
	pDC->Arc(&rectLeftRightEllipse, rectInner.BottomRight(), 
		CPoint(rectInner.right, rectInner.top));
	pDC->Arc(&rectTopBottomEllipse, CPoint(rectInner.left, rectInner.bottom), 
		rectInner.BottomRight());

	CPen penLowerRightThin(PS_SOLID, 2, LOWER_DARK_COLOR);
	pDC->SelectObject(&penLowerRightThin);
	pDC->Arc(&rectLeftRightEllipse, rectInner.BottomRight(), 
		CPoint(rectInner.right, rectInner.top));
	pDC->Arc(&rectTopBottomEllipse, CPoint(rectInner.left, rectInner.bottom), 
		rectInner.BottomRight());

	// restore the old pen 
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}

// draw a link
void CNodeViewSkin::DrawLink(CDC *pDC, CVector<3>& vFrom, REAL actFrom,
			  CVector<3>& vTo, REAL actTo)
{
	// grey brush for drawing links
	static CBrush greyBrushDark(RGB(160, 160, 160));
	static CBrush greyBrushMid(RGB(172, 172, 172));
	static CBrush greyBrushLight(RGB(184, 184, 184));

	CBrush *pOldBrush;
	if (actFrom + actTo < 0.025)
		pOldBrush = pDC->SelectObject(&greyBrushLight);
	else if (actFrom + actTo < 0.05)
		pOldBrush = pDC->SelectObject(&greyBrushMid);
	else
		pOldBrush = pDC->SelectObject(&greyBrushDark);
	CPen *pOldPen = (CPen *)pDC->SelectStockObject(NULL_PEN);

	CVector<3> vOffset = vTo - vFrom;
	CVector<3> vNormal(vOffset[1], -vOffset[0]);
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
}
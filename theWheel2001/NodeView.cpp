//////////////////////////////////////////////////////////////////////
// NodeView.cpp: implementation of the CNodeView class.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "NodeView.h"

// parent class
#include "SpaceView.h"

// the displayed model object
#include <Node.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
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

//////////////////////////////////////////////////////////////////////
// CNodeView::CNodeView
// 
// constructs a new CNodeView object for the passed CNode
//////////////////////////////////////////////////////////////////////
CNodeView::CNodeView(CNode *pNode, CSpaceView *pParent)
: forNode(pNode),
	m_vCenter(CVector<2>(0.0, 0.0)),
	m_vSpringCenter(m_vCenter),
	m_thresholdedActivation(pNode->GetActivation()),
	m_springActivation(m_thresholdedActivation),
	m_ptMouseDown(-1, -1),
	m_bDragging(FALSE),
	m_bDragged(FALSE),
	m_pParent(pParent)
{
	// set the view pointer for the node
	forNode->SetView(this);
}

//////////////////////////////////////////////////////////////////////
// CNodeView::~CNodeView
// 
// destroys the node view
//////////////////////////////////////////////////////////////////////
CNodeView::~CNodeView()
{
}

//////////////////////////////////////////////////////////////////////
// CNodeView::GetCenter
// 
// returns the center point of the node view
//////////////////////////////////////////////////////////////////////
CVector<2> CNodeView::GetCenter()
{
	return m_vCenter;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::GetSpringCenter
// 
// returns the spring-loaded center point of the node view
//////////////////////////////////////////////////////////////////////
CVector<2> CNodeView::GetSpringCenter()
{
	return m_vSpringCenter;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::GetPiggybackCenter
// 
// returns the piggy-backed center (for sub-threshold node views)
//////////////////////////////////////////////////////////////////////
CVector<2> CNodeView::GetPiggybackCenter()
{
	CSpaceView *pSpaceView = (CSpaceView *)m_pParent;

	// get the parent rectangle
	CRect rectParent;
	ASSERT(::IsWindow(m_pParent->m_hWnd));
	m_pParent->GetClientRect(&rectParent);

	// initialize the new center to the center of the parent window
	CVector<2> vNewCenter = rectParent.CenterPoint();

	// find the max activator node that is super-threshold
	CNode *pMaxActNode = forNode.Get();
	while (pMaxActNode != NULL)
	{
		// get the current max activator
		pMaxActNode = pMaxActNode->GetMaxActivator();

		// if one is found...
		if (pMaxActNode != NULL)
		{
			// find the view for the max activator node
			CNodeView *pMaxActView = 
				pSpaceView->GetViewForNode(pMaxActNode);

			// make sure one is found
			ASSERT(pMaxActView != NULL);

			// if the max activator's view is super-threshold,
			if (pMaxActView->GetThresholdedActivation() > 0.0) 
			{
				// set the new center to it
				vNewCenter = pMaxActView->GetCenter();

				// and break the loop
				break;
			}
		} 
	}

	return vNewCenter;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::SetCenter
// 
// sets the center point of the node view
//////////////////////////////////////////////////////////////////////
void CNodeView::SetCenter(const CVector<2>& vCenter)
{
	m_vCenter = vCenter;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::GetThresholdedActivation
// 
// returns the thresholded activation value for the node view
//////////////////////////////////////////////////////////////////////
double CNodeView::GetThresholdedActivation()
{
	return m_thresholdedActivation;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::SetThresholdedActivation
// 
// sets the thresholded activation value for the node view
//////////////////////////////////////////////////////////////////////
void CNodeView::SetThresholdedActivation(double activation)
{
	m_thresholdedActivation = activation;
}

/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetOuterRect
//
// returns the outer (bounding) rectangle of the elliptangle
/////////////////////////////////////////////////////////////////////////////
CRect CNodeView::GetOuterRect()
{
	return m_rectOuter;
}

/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetInnerRect
//
// returns the inner rectangle of the elliptangle
/////////////////////////////////////////////////////////////////////////////
CRect CNodeView::GetInnerRect()
{
	CRect rectOuter = GetOuterRect();

	// compute the r, which represents the amount of "elliptical-ness"
	float r = 1.0f - (1.0f - 1.0f / (float) sqrt(2.0f))
		* (float) exp(-8.0f * m_springActivation);

	CRect rectInner = rectOuter;
	rectInner.top = (long) ((float) (rectOuter.top + rectOuter.Height() / 2) 
		- r * (float) rectOuter.Height() / 2.0f);
	rectInner.bottom = (long) ((float) (rectOuter.top + rectOuter.Height() / 2) 
		+ r * (float) rectOuter.Height() / 2.0f);
	rectInner.left = (long) ((float) (rectOuter.left + rectOuter.Width() / 2) 
		- r * (float) rectOuter.Width() / 2.0f);
	rectInner.right = (long) ((float) (rectOuter.left + rectOuter.Width() / 2) 
		+ r * (float) rectOuter.Width() / 2.0f);

	return rectInner;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::GetShape
// 
// returns and computes (if needed) the node view's region (shape)
//////////////////////////////////////////////////////////////////////
CRgn& CNodeView::GetShape()
{
	// re-compute the region, if needed
	if (m_shape.GetSafeHandle() == NULL)
	{
		// form the ellipse for top/bottom clipping
		CRect rectTopBottomEllipse = GetTopBottomEllipseRect();
		CRgn ellipseTopBottom;
		ellipseTopBottom.CreateEllipticRgnIndirect(&rectTopBottomEllipse);

		// form the ellipse for left/right clipping
		CRect rectLeftRightEllipse = GetLeftRightEllipseRect();
		CRgn ellipseLeftRight;
		ellipseLeftRight.CreateEllipticRgnIndirect(&rectLeftRightEllipse);

		// form the intersection of the two
		m_shape.CreateRectRgnIndirect(GetInnerRect());
		m_shape.CombineRgn(&ellipseTopBottom, &ellipseLeftRight, RGN_AND);

		// delete the formant regions
		ellipseTopBottom.DeleteObject();
		ellipseLeftRight.DeleteObject();
	}

	// return the computed region
	return m_shape;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::UpdateSprings
// 
// updates the spring-dynamic variables -- should be called on a 
//		regular basis
//////////////////////////////////////////////////////////////////////
void CNodeView::UpdateSprings(double springConst)
{
	// get the parent window
	CSpaceView *pSpaceView = (CSpaceView *)m_pParent;

	// get the parent rectangle
	CRect rectParent;
	ASSERT(::IsWindow(m_pParent->m_hWnd));
	m_pParent->GetClientRect(&rectParent);

	// update spring activation
	m_springActivation = GetThresholdedActivation() * (1.0 - springConst)
		+ m_springActivation * springConst;

	// if we are sub-threshold, set our center to the max activator's
	//		center
	if (GetThresholdedActivation() == 0.0)
	{
		CVector<2> vNewCenter = GetPiggybackCenter();

		// set the new center point
		SetCenter(vNewCenter * (1.0 - springConst)
			+ GetCenter() * springConst);
	}

	// update spring center
	m_vSpringCenter = GetCenter() * (1.0 - springConst)
		+ m_vSpringCenter * springConst;

	// compute the area interpreting springActivation as the fraction of the 
	//		parent's total area
	float area = (float) m_springActivation 
		* (rectParent.Width() * rectParent.Height());

	// compute the desired aspect ratio (maintain the current aspect ratio)
	float aspectRatio = 0.75f - 0.375f * (float) exp(-8.0f * m_springActivation);

	// compute the new width and height from the desired area and the desired
	//		aspect ratio
	int nWidth = (int) sqrt(area / aspectRatio);
	int nHeight = (int) sqrt(area * aspectRatio);

	// set the width and height of the window, keeping the center constant
	CRect rect;
	rect.left = (long) m_vSpringCenter[0] - nWidth / 2;
	rect.right = (long) m_vSpringCenter[0] + nWidth / 2;
	rect.top = (long) m_vSpringCenter[1] - nHeight / 2;
	rect.bottom = (long) m_vSpringCenter[1] + nHeight / 2;

	// move the window -- re-computes window region
	m_shape.DeleteObject();	// trigger re-computing the shape
	m_rectOuter = rect;

	// and invalidate the parent window
	m_pParent->Invalidate(FALSE);
}

//////////////////////////////////////////////////////////////////////
// CNodeView::Draw
// 
// draws the entire node view
//////////////////////////////////////////////////////////////////////
void CNodeView::Draw(CDC *pDC)
{
	// get the inner rectangle for drawing the text
	CRect rectInner = GetInnerRect();

	// only draw if it has a substantial area
	if (rectInner.Height() > 1)
	{
		// draw the elliptangle
		DrawElliptangle(pDC);

		// draw the image (if any)
		DrawImage(pDC, rectInner);

		// draw the text
		DrawText(pDC, rectInner);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetLeftRightEllipseRect
//
// returns the rectangle that bounds the ellipse for the left and right
//		sides of the elliptangle
/////////////////////////////////////////////////////////////////////////////
CRect CNodeView::GetLeftRightEllipseRect()
{
	CRect rectOuter = GetOuterRect();
	CRect rectInner = GetInnerRect();

	// compute the ellipse's b
	float rx = ((float) rectInner.Width())
			/ ((float) rectOuter.Width()); 
	float bx = (float) sqrt((float) (rectInner.Height() 
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
CRect CNodeView::GetTopBottomEllipseRect()
{
	CRect rectOuter = GetOuterRect();
	CRect rectInner = GetInnerRect();

	// compute the ellipse's b
	float ry = ((float) rectInner.Height())
			/ ((float) rectOuter.Height()); 
	float by = (float) sqrt((float) (rectInner.Width() * rectInner.Width()) * 0.25f
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
void CNodeView::DrawElliptangle(CDC *pDC)
{
	// reset the inner rectangle
	CRect rectInner = GetInnerRect();

	// get the guide rectangles
	CRect rectLeftRightEllipse = GetLeftRightEllipseRect();
	CRect rectTopBottomEllipse = GetTopBottomEllipseRect();

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

//////////////////////////////////////////////////////////////////////
// CNodeView::DrawText
// 
// draws the node's text in the given rectangle
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawText(CDC *pDC, CRect& rectInner)
{
	pDC->SetBkMode(TRANSPARENT);

	int nDesiredHeight = min(rectInner.Height() / 3, 20);
	int nDesiredWidth = rectInner.Width() / 80;

	CFont font;
	BOOL bResult = font.CreateFont(nDesiredHeight, 0, //nDesiredWidth,
		0, 0, FW_BOLD, 
		FALSE, FALSE, 0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		VARIABLE_PITCH,
		"Arial");
	ASSERT(bResult);

	CFont *pOldFont = pDC->SelectObject(&font);

	CRect rectText(rectInner);
	rectText.DeflateRect(5, 5, 5, -5);
	pDC->DrawText(forNode->name.Get(), rectText, 
		DT_CENTER | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);

	// now calculate the height of the drawn text
	rectText = rectInner;
	rectText.DeflateRect(5, 5, 5, 5);
	int nHeight = pDC->DrawText(forNode->name.Get(), rectText, 
		DT_CALCRECT | DT_CENTER | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);

	// now draw the description body
	if (m_springActivation >= 0.05)
	{
		rectText = rectInner;
		rectText.DeflateRect(5, 5, 5, 5);
		rectText.top += nHeight + 5;

		if (forNode->GetDib() != NULL)
		{
			CRect rectImage = rectText;
			rectImage.right = rectImage.left + rectImage.Height();
			// rectImage.left += rectImage.Height();

			rectImage.DeflateRect(5, 5, 5, 5);
			forNode->GetDib()->Draw(*pDC, &rectImage);

			// draw the bitmap next to the title
			// pDC->Ellipse(rectImage);

			// adjust the rectangle to account for the bitmap
			rectText.left += rectText.Height();
		}

		nDesiredHeight = max(nDesiredHeight / 2, 12);
		CFont smallFont;
		bResult = smallFont.CreateFont(nDesiredHeight, 0, //nDesiredWidth,
			0, 0, FW_NORMAL, 
			FALSE, FALSE, 0,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			VARIABLE_PITCH,
			"Arial");
		ASSERT(bResult);

		pDC->SelectObject(&smallFont);
		pDC->DrawText(forNode->description.Get(), rectText, 
			DT_LEFT | DT_END_ELLIPSIS | DT_WORDBREAK);

		pDC->SelectObject(pOldFont);
		smallFont.DeleteObject();
	}
	else
		pDC->SelectObject(pOldFont);

	font.DeleteObject();
}

//////////////////////////////////////////////////////////////////////
// CNodeView::DrawImage
// 
// draws the image in the given rectangle, updating the rectangle
//		to reflect the actual image size
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawImage(CDC *pDC, CRect& rectInner)
{
	if (forNode->GetDib() != NULL && m_springActivation < 0.05)
	{
		// calculate the bitmap size 
		float bitmapSize = 
			rectInner.Height() * 0.03f / (float) m_springActivation;
		bitmapSize = min(bitmapSize, rectInner.Height());
		float margin = rectInner.Height() - bitmapSize;

		CRect rectImage = rectInner;
		rectImage.top += (int) margin / 2;
		rectImage.right = rectImage.left + (int) bitmapSize;
		rectImage.bottom -= (int) margin / 2;

		rectImage.DeflateRect(5, 5, 5, 5);

		// draw the bitmap next to the title
		forNode->GetDib()->Draw(*pDC, &rectImage);

		// adjust the rectangle to account for the bitmap
		rectInner.left += (int) bitmapSize;
	}
}

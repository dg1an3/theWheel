//////////////////////////////////////////////////////////////////////
// NodeView.cpp: implementation of the CNodeView class.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

#include <float.h>

// the class definition
#include "NodeView.h"

// parent class
#include "SpaceView.h"

#include "NodeViewEnergyFunction.h"
#include <PowellOptimizer.h>

// the displayed model object
#include <Node.h>

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CObArray CNodeView::m_arrNodeViewsToDraw;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CNodeView::CNodeView
// 
// constructs a new CNodeView object for the passed CNode
//////////////////////////////////////////////////////////////////////
CNodeView::CNodeView(CNode *pNode, CSpaceView *pParent)
: m_pNode(pNode),
	m_pSkin(NULL),
	m_vSpringCenter(m_pNode->GetPosition()),
	m_thresholdedActivation(pNode->GetActivation()),
	m_springActivation(m_thresholdedActivation),
	m_ptMouseDown(-1, -1),
	m_bDragging(FALSE),
	m_bDragged(FALSE),
	m_pParent(pParent),
	m_bBackgroundImage(FALSE),
	m_pendingActivation(0.0f)
{
	// set the view pointer for the node
	GetNode()->SetView(this);
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
// CNodeView::GetNode
// 
// returns a pointer to the node for this node view
//////////////////////////////////////////////////////////////////////
CNode *CNodeView::GetNode()
{
	return m_pNode;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::GetSpringCenter
// 
// returns the spring-loaded center point of the node view
//////////////////////////////////////////////////////////////////////
CVector<3> CNodeView::GetSpringCenter()
{
	return m_vSpringCenter;
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
	CRect rectOuter;
	if (m_pSkin)
		rectOuter = m_pSkin->CalcOuterRect(this);
	rectOuter.OffsetRect(GetSpringCenter()[0], GetSpringCenter()[1]);

	return rectOuter;
}

/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetInnerRect
//
// returns the inner rectangle of the elliptangle
/////////////////////////////////////////////////////////////////////////////
CRect CNodeView::GetInnerRect()
{
	CRect rectInner;
	if (m_pSkin)
		rectInner = m_pSkin->CalcInnerRect(this);
	rectInner.OffsetRect(GetSpringCenter()[0], GetSpringCenter()[1]);

	return rectInner;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::GetShape
// 
// returns and computes (if needed) the node view's region (shape)
//////////////////////////////////////////////////////////////////////
CRgn& CNodeView::GetShape(int nErode)
{
	// no shape if the rectangle is too small
	if (GetOuterRect().Height() < 5)
	{
		m_shape.DeleteObject();
		return m_shape;
	}

	// re-compute the region, if needed
	if (m_shape.GetSafeHandle() == NULL && m_pSkin != NULL)
	{
		m_pSkin->CalcShape(this, &m_shape, nErode);
		m_shape.OffsetRgn(GetSpringCenter()[0], GetSpringCenter()[1]);
	}

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

	// update spring center
	springConst = 0.925;
	springConst *= 1.02;
	m_vSpringCenter = GetNode()->GetPosition() * (1.0 - springConst)
		+ m_vSpringCenter * (springConst);

	// trigger re-computing the shape
	m_shape.DeleteObject();	

	// and invalidate the parent window
	m_pParent->Invalidate(FALSE);
}

//////////////////////////////////////////////////////////////////////
// CNodeView::Draw
// 
// draws the entire node view
//////////////////////////////////////////////////////////////////////
void CNodeView::Draw(CDC *pDC, CNodeViewSkin *pSkin)
{
	// store the skin
	m_pSkin = pSkin;

	// get the inner rectangle for drawing the text
	CRect rectInner = GetInnerRect();

	// only draw if it has a substantial area
	if (rectInner.Height() > 5)
	{
		// draw the links
		// DrawLinks(pDC);

		// draw the skin
		pDC->OffsetWindowOrg(-GetSpringCenter()[0], -GetSpringCenter()[1]);
		pSkin->DrawSkin(pDC, this);
		pDC->OffsetWindowOrg(GetSpringCenter()[0], GetSpringCenter()[1]);

		rectInner.DeflateRect(5, 5, 5, 5);

		// draw the image (if any)
		if (m_bBackgroundImage)
		{
			DrawImage(pDC, rectInner);
			DrawTitle(pDC, rectInner);
			DrawText(pDC, rectInner);
		}
		else
		{
			// draw the title first, adjusting the rectangle
			DrawTitle(pDC, rectInner);
			DrawImage(pDC, rectInner);
			DrawText(pDC, rectInner);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CNodeView::DrawLinks
// 
// draws the links for this node view
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawLinks(CDC *pDC)
{
	pDC->SetBkMode(TRANSPARENT);

	// draw the links only if the activation is above 0.0
	if (GetThresholdedActivation() > 0.0)
	{
		// for each link
		for (int nAtLink = 0; nAtLink < GetNode()->GetLinkCount(); nAtLink++)
		{
			CNodeLink *pLink = GetNode()->GetLinkAt(nAtLink);
			CNodeView *pLinkedView = (CNodeView *)pLink->GetTarget()->GetView();
			ASSERT(pLinkedView != NULL);

			// see if its on the array of node views to be drawn
//			for (int nAtToDraw = 0; nAtToDraw < m_arrNodeViewsToDraw.GetSize(); nAtToDraw++)
//			{
//				if (pLinkedView == (CNodeView *) m_arrNodeViewsToDraw[nAtToDraw])
				
			// only draw the link to node view's with activations greater than the current
			if (pLink->GetWeight() > 0.1
				&& pLinkedView != NULL
				&& pLinkedView->GetThresholdedActivation() > 0.0
				&& pLinkedView->GetSpringActivation() > GetSpringActivation())
			{
				// draw the link
				CVector<3> vFrom = GetSpringCenter();
				CVector<3> vTo = pLinkedView->GetSpringCenter();
				
				m_pSkin->DrawLink(pDC, vFrom, m_springActivation, 
					vTo, pLinkedView->m_springActivation);
			}
//			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CNodeView::DrawText
// 
// draws the node's text in the given rectangle
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawTitle(CDC *pDC, CRect& rectInner)
{
	pDC->SetBkMode(TRANSPARENT);

	int nDesiredHeight = min(rectInner.Height() / 4, 30);
	nDesiredHeight = max(nDesiredHeight, 15);
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

	// calculate the height of the drawn text
	CRect rectText(rectInner);
	rectText = rectInner;
	int nHeight = pDC->DrawText(GetNode()->GetName(), rectText, 
		DT_CALCRECT | DT_LEFT | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);

	// draw the background for the title
	CBrush backBrush(RGB(149, 205, 208));
	CBrush *pOldBrush = pDC->SelectObject(&backBrush);
	CPen *pOldPen = (CPen *)pDC->SelectStockObject(NULL_PEN);
	CRect rectTitle = GetOuterRect();
	rectTitle.top -= 5;
	if (m_springActivation >= 0.01)
		rectTitle.bottom = rectText.bottom + 3;

	// clip to the node view
	pDC->SelectClipRgn(&GetShape(5));

	pDC->Rectangle(rectTitle);

	// unselect clipping to node view
	pDC->SelectClipRgn(NULL);

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);

	for (int nAt = 0; nAt < 4 && rectText.bottom > GetInnerRect().bottom; nAt++)
	{
		rectText.bottom = rectText.top +
			(rectText.Height() / nDesiredHeight - 1) * nDesiredHeight;
	}
	// CRect rectText(rectInner);
	pDC->DrawText(GetNode()->GetName(), rectText, 
		DT_LEFT | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);

	// now draw the description body
	if (m_springActivation >= 0.01)
	{
		rectText = rectInner;
		// rectText.DeflateRect(5, 5, 5, 5);
		rectText.top += nHeight + 5;
	}
	else
	{
		rectText.bottom = rectText.top;
	}

	rectInner = rectText;

	pDC->SelectObject(pOldFont);
	font.DeleteObject();
}

//////////////////////////////////////////////////////////////////////
// CNodeView::DrawText
// 
// draws the node's text in the given rectangle
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawText(CDC *pDC, CRect& rectInner)
{
	pDC->SetBkMode(TRANSPARENT);

	int nDesiredHeight = min(rectInner.Height() / 4, 30);
	nDesiredHeight = max(nDesiredHeight, 15);
	int nDesiredWidth = rectInner.Width() / 80;

	// now draw the description body
	if (m_springActivation >= 0.01)
	{
		nDesiredHeight = max(nDesiredHeight / 2, 14);

		if (rectInner.bottom > GetOuterRect().bottom)

			// adjust height of rectangle for text
			rectInner.bottom -= rectInner.Height() -
				(rectInner.Height() / nDesiredHeight) * nDesiredHeight;

		CFont smallFont;
		BOOL bResult = smallFont.CreateFont(nDesiredHeight, 0,
			0, 0, FW_NORMAL, 
			FALSE, FALSE, 0,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			VARIABLE_PITCH,
			"Arial");
		ASSERT(bResult);

		CFont *pOldFont = pDC->SelectObject(&smallFont);

		pDC->DrawText(GetNode()->GetDescription(), rectInner, 
			DT_LEFT | DT_END_ELLIPSIS | DT_WORDBREAK);

		pDC->SelectObject(pOldFont);
		smallFont.DeleteObject();
	}
}

//////////////////////////////////////////////////////////////////////
// CNodeView::DrawImage
// 
// draws the image in the given rectangle, updating the rectangle
//		to reflect the actual image size
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawImage(CDC *pDC, CRect& rectInner)
{
	if (GetNode()->GetDib() != NULL)
	{
		if (!m_bBackgroundImage)
		{
			CRect rectImage = rectInner;
			int nActualWidth;
	
			CSize sizeImage = GetNode()->GetDib()->GetSize();
			nActualWidth = sizeImage.cx * rectImage.Height() / sizeImage.cy;
			rectImage.right = rectImage.left + nActualWidth;

			if (rectImage.Height() > 20 && rectImage.Width() > 20)
			{
				// rectImage.DeflateRect(5, 5, 5, 5);
				GetNode()->GetDib()->Draw(*pDC, &rectImage);

				// adjust the rectangle to account for the bitmap
				rectInner.left += nActualWidth + 5;
			}
		}
		else
		{
			CRect rectImage(GetOuterRect());
			CSize sizeImage = GetNode()->GetDib()->GetSize();

			BOOL m_bPreserveWidth = FALSE;
			if (m_bPreserveWidth)
			{
				int nHeight = rectImage.Width() * sizeImage.cy / sizeImage.cx;
				int nMidHeight = rectImage.CenterPoint().y;
				rectImage.top = nMidHeight - nHeight / 2;
				rectImage.bottom = nMidHeight + nHeight / 2;
			}
			else
			{
				int nWidth = rectImage.Height() * sizeImage.cx / sizeImage.cy;
				int nMidWidth = rectImage.CenterPoint().x;
				rectImage.left = nMidWidth - nWidth / 2;
				rectImage.right = nMidWidth + nWidth / 2;
			}

			// clip to the node view
			pDC->SelectClipRgn(&GetShape(1));

			GetNode()->GetDib()->Draw(*pDC, &rectImage);

			// unselect clipping to node view
			pDC->SelectClipRgn(NULL);
		}
	}
}

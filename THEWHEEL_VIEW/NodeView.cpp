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

// the displayed model object
#include <Node.h>

#include <Eevorg.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const COLORREF DEFAULT_TITLE = RGB(149, 205, 208);

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
	// m_pSkin(NULL),
	m_springActivation(pNode->GetActivation()),
	m_ptMouseDown(-1, -1),
	m_bDragging(FALSE),
	m_bDragged(FALSE),
	m_pParent(pParent),
	m_bBackgroundImage(FALSE),
	m_pendingActivation(0.0f)
{
	// set the view pointer for the node
	GetNode()->SetView(this);

	CRect rectParent;
	pParent->GetClientRect(&rectParent);

	m_vSpringCenter = GetNode()->GetPosition();
		// CVector<3>(rectParent.Width() / 2, 
		// rectParent.Height() / 2);
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
CVectorD<3> CNodeView::GetSpringCenter()
{
	return m_vSpringCenter;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::GetThresholdedActivation
// 
// returns the thresholded activation value for the node view
//////////////////////////////////////////////////////////////////////
REAL CNodeView::GetThresholdedActivation()
{
	// compute the threshold
	int nSuperNodeCount = 
		m_pParent->GetDocument()->GetSuperNodeCount();
	
	if (nSuperNodeCount > 0)
	{
		CNode *pLastSuperNode = 
			m_pParent->GetDocument()->GetNodeAt(nSuperNodeCount-1);
		REAL activationThreshold = pLastSuperNode->GetActivation();

		// compute the normalization factor for super-threshold node views
		REAL superThresholdScale = TOTAL_ACTIVATION * 1.2f
			/ m_pParent->GetDocument()->GetTotalActivation();

		// get this node's activation
		REAL activation = GetNode()->GetActivation();

		// for super-threshold nodes,
		if (activation >= activationThreshold)
		{
			// return the scaled, thresholded activation
			return activation * superThresholdScale * superThresholdScale;

		}
	}

	// otherwise, the thresholded activation is 0.0
	return 0.0f;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::GetSpringActivation
// 
// returns the thresholded activation value for the node view
//////////////////////////////////////////////////////////////////////
REAL CNodeView::GetSpringActivation() 
{
	REAL dist = (GetSpringCenter() - GetNode()->GetPosition()).GetLength();
	REAL factor = 0.8 + 0.5 * (1.0 / (0.5 * dist * dist + 1.0));

	return factor * m_springActivation; 
}


/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetOuterRect
//
// returns the outer (bounding) rectangle of the elliptangle
/////////////////////////////////////////////////////////////////////////////
CRect& CNodeView::GetOuterRect()
{
	return m_rectOuter;
}

/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetInnerRect
//
// returns the inner rectangle of the elliptangle
/////////////////////////////////////////////////////////////////////////////
CRect& CNodeView::GetInnerRect()
{
	return m_rectInner;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::GetShape
// 
// returns and computes (if needed) the node view's region (shape)
//////////////////////////////////////////////////////////////////////
CRgn& CNodeView::GetShape()
{
	return m_shape;
}

//////////////////////////////////////////////////////////////////////
// CNodeView::UpdateSprings
// 
// updates the spring-dynamic variables -- should be called on a 
//		regular basis
//////////////////////////////////////////////////////////////////////
void CNodeView::UpdateSprings(REAL springConst)
{
	// update spring activation
 	m_springActivation 
		= GetThresholdedActivation() * (1.0f - springConst * 0.90)
			+ m_springActivation * springConst * 0.90;

	// compute the scale
	CRect rectParent;
	m_pParent->GetClientRect(&rectParent);
	REAL scale 
		= (REAL) sqrt(rectParent.Width() * rectParent.Height()) 
			/ 250.0f;

	// compute the center
	CVectorD<3> vCenter(
		(REAL) rectParent.Width() / 2, 
		(REAL) rectParent.Height() / 2,
		0.0f);

	// compute the scaled position
	CVectorD<3> vScaledPos 
		= scale * (GetNode()->GetPosition() - vCenter) + vCenter;

	// compute a new spring constant
	REAL factor = exp(GetThresholdedActivation() - GetSpringActivation())
		/ (exp(GetThresholdedActivation() - GetSpringActivation()) + 0.01);
	springConst = springConst * (0.2 + 0.8 * factor);

	// update the center spring
	m_vSpringCenter = vScaledPos 
		* (1.0f - springConst) + m_vSpringCenter * (springConst);

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
	// only draw if it has a substantial area
	if (GetOuterRect().Height() >= 10)
	{
		// get the inner rectangle for drawing 
		CRect rectInner = GetInnerRect();
		rectInner.DeflateRect(5, 5, 5, 5);

		// draw the title band first
		DrawTitleBand(pDC, rectInner);

		// draw the image (if any)
		if (m_bBackgroundImage)
		{
			DrawImage(pDC, rectInner);
			DrawTitle(pDC, rectInner);
			DrawText(pDC, rectInner);
		}
		else if (rectInner.Height() > 30)
		{
			// draw the title first, adjusting the rectangle
			DrawTitle(pDC, rectInner);
			DrawImage(pDC, rectInner);
			DrawText(pDC, rectInner);
		}
		else
		{
			// draw the image first
			DrawImage(pDC, rectInner);
			DrawTitle(pDC, rectInner);
			DrawText(pDC, rectInner);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CNodeView::DrawLinks
// 
// draws the links for this node view
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawLinks(CDC *pDC, CNodeViewSkin *pSkin)
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

			// only draw the link to node view's with activations greater than the current
			if (!pLink->IsStabilizer()
				&& pLink->GetWeight() > 0.1
				&& pLinkedView != NULL
				&& pLinkedView->GetSpringActivation() > GetSpringActivation())
			{
				// draw the link
				CVectorD<3> vFrom = GetSpringCenter();
				CVectorD<3> vTo = pLinkedView->GetSpringCenter();
				
				pSkin->DrawLink(pDC, vFrom, GetSpringActivation(), 
					vTo, pLinkedView->GetSpringActivation());
			}
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

	int nDesiredHeight = __min(rectInner.Height() / 4, 30);
	nDesiredHeight = __max(nDesiredHeight, 12);
	int nDesiredWidth = rectInner.Width() / 100;

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
	rectText.right += 500;
	int nHeight = pDC->DrawText(GetNode()->GetName(), rectText, 
		DT_CALCRECT | DT_LEFT | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);

	for (int nAt = 0; nAt < 4 && rectText.bottom > GetInnerRect().bottom; nAt++)
	{
		rectText.bottom = rectText.top +
			(rectText.Height() / nDesiredHeight - 1) * nDesiredHeight;
	}
	// CRect rectText(rectInner);
	pDC->DrawText(GetNode()->GetName(), rectText, 
		DT_LEFT | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);

	// now draw the description body
	if (GetSpringActivation() >= 0.01)
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

	int nDesiredHeight = __min(rectInner.Height() / 4, 30);
	nDesiredHeight = __max(nDesiredHeight, 15);
	int nDesiredWidth = rectInner.Width() / 80;

	// now draw the description body
	if (GetSpringActivation() >= 0.01)
	{
		nDesiredHeight = __max(nDesiredHeight / 2, 14);

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

			if (rectImage.Height() > 10 || rectImage.Width() > 10)
			{
				rectImage.InflateRect(2, 2, 2, 2);
				GetNode()->GetDib()->Draw(*pDC, &rectImage);

				// adjust the rectangle to account for the bitmap
				rectInner.left += nActualWidth + 10;
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
			pDC->SelectClipRgn(&GetShape());

			GetNode()->GetDib()->Draw(*pDC, &rectImage);

			// unselect clipping to node view
			pDC->SelectClipRgn(NULL);
		}
	}
}

void CNodeView::DrawTitleBand(CDC *pDC, CRect& rectInner)
{
	int nDesiredHeight = __min(rectInner.Height() / 4, 30);
	nDesiredHeight = __max(nDesiredHeight, 15);
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
	rectText.right += 500;
	int nHeight = pDC->DrawText(GetNode()->GetName(), rectText, 
		DT_CALCRECT | DT_LEFT | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);

	// draw the background for the title
	CBrush backBrush;
	if (GetNode()->GetClass() == "Genre")
	{
		backBrush.CreateSolidBrush(RGB(176, 176, 160)); // 174, 220, 154));
	}
	else if (GetNode()->GetClass() == "Artist")
	{
		backBrush.CreateSolidBrush(RGB(182, 139, 224));
	}
	else
	{
		backBrush.CreateSolidBrush(DEFAULT_TITLE);
	}

	CBrush *pOldBrush = pDC->SelectObject(&backBrush);
	CPen *pOldPen = (CPen *)pDC->SelectStockObject(NULL_PEN);
	CRect rectTitle = GetOuterRect();
	rectTitle.top -= 5;
	if (GetSpringActivation() >= 0.01)
	{
		rectTitle.bottom = rectText.bottom + 3;
	}

	// clip to the node view
	pDC->SelectClipRgn(&GetShape());

	pDC->Rectangle(rectTitle);

	// unselect clipping to node view
	pDC->SelectClipRgn(NULL);

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);

	pDC->SelectObject(pOldFont);
	font.DeleteObject();
}

void CNodeView::AddPendingActivation(double pending)
{
	m_pendingActivation += pending;
}

REAL CNodeView::GetPendingActivation()
{
	return m_pendingActivation;
}

void CNodeView::ResetPendingActivation()
{
	m_pendingActivation = (REAL) 0.0;
}

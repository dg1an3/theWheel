//////////////////////////////////////////////////////////////////////
// NodeView.cpp: implementation of the CNodeView class.
//
// Copyright (C) 1996-2003 Derek G Lane
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


//////////////////////////////////////////////////////////////////////
// constants
//////////////////////////////////////////////////////////////////////

const COLORREF DEFAULT_TITLE = RGB(149, 205, 208);

const REAL MAX_SPRING_CONST = 0.99;
const REAL MIN_POST_SUPER = 0.0014;
const REAL VIEW_SCALE = 290.0; // 770.0;

const REAL MIN_WEIGHT_TO_DRAW_LINK = 0.01;

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
	m_springActivation(pNode->GetActivation()),
	m_ptMouseDown(-1, -1),
	m_bDragging(FALSE),
	m_bDragged(FALSE),
	m_pParent(pParent),
	m_bBackgroundImage(FALSE),
	m_pendingActivation(0.0f),
	m_bMaximized(FALSE)
{

	ZeroMemory(m_rectOuter, sizeof(CRect));
	ZeroMemory(m_rectInner, sizeof(CRect));

	// set the view pointer for the node
	GetNode()->SetView(this);

	CRect rectParent;
	pParent->GetClientRect(&rectParent);

	m_vSpringCenter = GetNode()->GetPosition();

}	// CNodeView::CNodeView


//////////////////////////////////////////////////////////////////////
// CNodeView::~CNodeView
// 
// destroys the node view
//////////////////////////////////////////////////////////////////////
CNodeView::~CNodeView()
{
}	// CNodeView::~CNodeView


//////////////////////////////////////////////////////////////////////
// CNodeView::GetNode
// 
// returns a pointer to the node for this node view
//////////////////////////////////////////////////////////////////////
CNode *CNodeView::GetNode()
{
	return m_pNode;

}	// CNodeView::GetNode


//////////////////////////////////////////////////////////////////////
// CNodeView::GetSpringCenter
// 
// returns the spring-loaded center point of the node view
//////////////////////////////////////////////////////////////////////
CVectorD<3> CNodeView::GetSpringCenter()
{
	return m_vSpringCenter;

}	// CNodeView::GetSpringCenter


//////////////////////////////////////////////////////////////////////
// CNodeView::GetScaledNodeCenter
// 
// computes the center in the CSpaceView's coordinates, given
//		the CNode's position
//////////////////////////////////////////////////////////////////////
CVectorD<3> CNodeView::GetScaledNodeCenter()
{
	// compute the scale
	CRect rectParent;
	m_pParent->GetClientRect(&rectParent);
	REAL scale = (REAL) sqrt((REAL)(rectParent.Width() * rectParent.Height())) 
			/ VIEW_SCALE;

	// compute the center
	CVectorD<3> vCenter(
		(REAL) rectParent.Width() / 2, 
		(REAL) rectParent.Height() / 2, 0.0);

	CVectorD<3> vPos = GetNode()->GetPosition();
	if (m_bMaximized)
	{
		vPos = (REAL) 0.5 * (vPos + GetNode()->GetSpace()->m_vCenter);
	}
	vPos -= vCenter;
	vPos[0] /= 13.0 / 16.0;
	vPos[1] *= 13.0 / 16.0;

	// compute the scaled position
	return scale * vPos + vCenter;

}	// CNodeView::GetScaledNodeCenter


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
		REAL superThresholdScale = TOTAL_ACTIVATION // * 1.1
			/ m_pParent->GetDocument()->GetTotalActivation();

		// get this node's activation
		REAL activation = GetNode()->GetActivation();

		// for super-threshold nodes,
		if (activation >= activationThreshold)
		{
			// return the scaled, thresholded activation
			return 	_cpp_min<REAL>(activation * superThresholdScale * superThresholdScale, 
				TOTAL_ACTIVATION * 0.2);
		}
	}

	// otherwise, the thresholded activation is 0.0
	return 0.0;

}	// CNodeView::GetThresholdedActivation


//////////////////////////////////////////////////////////////////////
// CNodeView::GetSpringActivation
// 
// returns the thresholded activation value for the node view
//////////////////////////////////////////////////////////////////////
REAL CNodeView::GetSpringActivation() 
{

	return m_springActivation; 

}	// CNodeView::GetSpringActivation


/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetOuterRect
//
// returns the outer (bounding) rectangle of the elliptangle
/////////////////////////////////////////////////////////////////////////////
CRect& CNodeView::GetOuterRect()
{
	return m_rectOuter;

}	// CNodeView::GetOuterRect


/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetInnerRect
//
// returns the inner rectangle of the elliptangle
/////////////////////////////////////////////////////////////////////////////
CRect& CNodeView::GetInnerRect()
{
	return m_rectInner;

}	// CNodeView::GetInnerRect


//////////////////////////////////////////////////////////////////////
// CNodeView::GetShape
// 
// returns and computes (if needed) the node view's region (shape)
//////////////////////////////////////////////////////////////////////
CRgn& CNodeView::GetShape()
{
	return m_shape;

}	// CNodeView::GetShape


//////////////////////////////////////////////////////////////////////
// CNodeView::UpdateSprings
// 
// updates the spring-dynamic variables -- should be called on a 
//		regular basis
//////////////////////////////////////////////////////////////////////
void CNodeView::UpdateSpringPosition(REAL springConst)
{
	// compute the scaled position
	CVectorD<3> vScaledPos = GetScaledNodeCenter();

	if ((vScaledPos - m_vSpringCenter).GetLength() < 10.0)
		springConst = 0.999; 
	else if ((vScaledPos - m_vSpringCenter).GetLength() < 20.0)
		springConst = 0.99;
	else if ((vScaledPos - m_vSpringCenter).GetLength() < 30.0)
		springConst = 0.95;

	// update the center spring
	m_vSpringCenter = vScaledPos 
		* ((REAL) 1.0 - springConst) + m_vSpringCenter * (springConst);

	// and invalidate the parent window
	m_pParent->Invalidate(FALSE);

}	// CNodeView::UpdateSprings


//////////////////////////////////////////////////////////////////////
// CNodeView::UpdateSpringActivation
// 
// updates the spring-dynamic variables -- should be called on a 
//		regular basis
//////////////////////////////////////////////////////////////////////
void CNodeView::UpdateSpringActivation(REAL springConst)
{
	if (m_bMaximized)
	{
		// update spring activation
 		m_springActivation = TOTAL_ACTIVATION * 0.35 * (1.0 - springConst)
			+ m_springActivation * springConst;

		return;
	}

	// see if the activation is below threshold
	// compute the threshold
	int nSuperNodeCount = 
		m_pParent->GetDocument()->GetSuperNodeCount();
	
	if (nSuperNodeCount > 0)
	{
		CNode *pLastSuperNode = 
			m_pParent->GetDocument()->GetNodeAt(nSuperNodeCount-1);
		REAL activationThreshold = pLastSuperNode->GetActivation();

		// linearly scale the spring constant if we are below
		//		the activation threshold
		if (m_springActivation < activationThreshold
			&& GetNode()->IsSubThreshold())
		{
			springConst += (MAX_SPRING_CONST - springConst) 
				* (1.0 - m_springActivation / activationThreshold);

			// set the post-super flag
			GetNode()->SetPostSuper(m_springActivation > MIN_POST_SUPER);
		}
	}

	// update spring activation
 	m_springActivation = GetThresholdedActivation() * (1.0 - springConst)
			+ m_springActivation * springConst;

	// and invalidate the parent window
	m_pParent->Invalidate(FALSE);

}	// CNodeView::UpdateSpringActivation


//////////////////////////////////////////////////////////////////////
// CNodeView::AddPendingActivation
// 
// draws the band upon which the title is displayed
//////////////////////////////////////////////////////////////////////
void CNodeView::AddPendingActivation(REAL pending)
{
	m_pendingActivation += pending;

}	// CNodeView::AddPendingActivation


//////////////////////////////////////////////////////////////////////
// CNodeView::GetPendingActivation
// 
// draws the band upon which the title is displayed
//////////////////////////////////////////////////////////////////////
REAL CNodeView::GetPendingActivation()
{
	return m_pendingActivation;

}	// CNodeView::GetPendingActivation


//////////////////////////////////////////////////////////////////////
// CNodeView::ResetPendingActivation
// 
// draws the band upon which the title is displayed
//////////////////////////////////////////////////////////////////////
void CNodeView::ResetPendingActivation()
{
	m_pendingActivation = (REAL) 0.0;

}	// CNodeView::ResetPendingActivation


//////////////////////////////////////////////////////////////////////
// CNodeView::SetMaximized
// 
// sets the node maximized flag
//////////////////////////////////////////////////////////////////////
void CNodeView::SetMaximized(BOOL bMax)
{
	m_bMaximized = bMax;

	if (!m_bMaximized)
	{
		// m_pParent->m_btnGo.MoveWindow(-30, -30, 0, 0, FALSE);
	}

}	// CNodeView::SetMaximized


//////////////////////////////////////////////////////////////////////
// CNodeView::Draw
// 
// draws the entire node view
//////////////////////////////////////////////////////////////////////
void CNodeView::Draw(CDC *pDC)
{
	// only draw if it has a substantial area
	if (GetOuterRect().Height() >= 3)
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

}	// CNodeView::Draw


//////////////////////////////////////////////////////////////////////
// CNodeView::DrawLinks
// 
// draws the links for this node view
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawLinks(CDC *pDC, CNodeViewSkin *pSkin)
{
	const int WIDTH_MULTIPLIER = 4;
	// check if node is significantly far from where it should be
	if ((GetScaledNodeCenter() - GetSpringCenter()).GetLength() > 
		GetOuterRect().Width() * WIDTH_MULTIPLIER)
	{
		// and skip if so
		return;
	}

	pDC->SetBkMode(TRANSPARENT);

	// draw the links only if the activation is above 0.0
	if (!GetNode()->IsSubThreshold())
		// && GetNode()->GetActivation() > MIN_POST_SUPER)
	{
		// for each link
		for (int nAtLink = 0; nAtLink < GetNode()->GetLinkCount(); nAtLink++)
		{
			CNodeLink *pLink = GetNode()->GetLinkAt(nAtLink);
			CNodeView *pLinkedView = (CNodeView *)pLink->GetTarget()->GetView();

			// check to see if the node is significantly far from where it should be
			if ((pLinkedView->GetScaledNodeCenter() - pLinkedView->GetSpringCenter()).GetLength() > 
				pLinkedView->GetOuterRect().Width() * WIDTH_MULTIPLIER)
			{
				// and skip if so
				continue;
			}

			// only draw the link to node view's with activations greater than the current
			if (!pLink->IsStabilizer()
				// && pLink->GetWeight() > MIN_WEIGHT_TO_DRAW_LINK
				&& pLinkedView != NULL
				// && pLinkedView->GetSpringActivation() >= GetSpringActivation()
				&& !pLinkedView->GetNode()->IsSubThreshold()
				// && pLinkedView->GetNode()->GetActivation() > MIN_POST_SUPER
				)
			{
				// draw the link
				CVectorD<3> vFrom = GetSpringCenter();
				CVectorD<3> vTo = pLinkedView->GetSpringCenter();
				REAL gain = GetNode()->GetLinkTo(pLinkedView->GetNode())->GetGain();

				pSkin->DrawLink(pDC, vFrom, 
					gain * GetSpringActivation(), 
					vTo,
					gain * pLinkedView->GetSpringActivation());
			}

			PROFILE_FLAG("Display", "Gain")
			{
				if (// pLink->GetWeight() < MIN_WEIGHT_TO_DRAW_LINK
					// 
					GetNode()->GetActivation() > pLink->GetTarget()->GetActivation()
					&& pLinkedView != NULL
					&& !pLinkedView->GetNode()->IsSubThreshold())
				{
					// draw the link
					CVectorD<3> vFrom = GetSpringCenter();
					CVectorD<3> vTo = pLinkedView->GetSpringCenter();
					REAL gain = GetNode()->GetLinkTo(pLinkedView->GetNode())->GetGain();

					CVectorD<3> vMid = (REAL) 0.5 * (vFrom + vTo);
					CString strGain;
					strGain.Format("%0.2lf", (double) pLink->GetGain());
					pDC->TextOut(vMid[0], vMid[1], strGain);
				}
			}
		}
	}

}	// CNodeView::DrawLinks


//////////////////////////////////////////////////////////////////////
// CNodeView::DrawTitle
// 
// draws the node's title in the given rectangle
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

	CString strName = GetNode()->GetName();
	if (GetNode()->GetOptimalStateVector() != NULL)
		strName += "*";

	PROFILE_FLAG("Display", "Activation")
	{
		CString strAct;
		strAct.Format(" (%2.1lf/%2.1lf)", 
			100.0 * GetSpringActivation(),
			100.0 * GetThresholdedActivation());
		strName += strAct;
	}

	int nHeight = pDC->DrawText(strName, rectText, 
		DT_CALCRECT | DT_LEFT | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);

	for (int nAt = 0; nAt < 4 && rectText.bottom > GetInnerRect().bottom; nAt++)
	{
		rectText.bottom = rectText.top +
			(rectText.Height() / nDesiredHeight - 1) * nDesiredHeight;
	}
	pDC->DrawText(strName, rectText, 
		DT_LEFT | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);

	// now draw the description body
	if (GetSpringActivation() >= 0.01)
	{
		rectText = rectInner;
		rectText.top += nHeight + 5;
	}
	else
	{
		rectText.bottom = rectText.top;
	}

	rectInner = rectText;

	pDC->SelectObject(pOldFont);
	font.DeleteObject();

}	// CNodeView::DrawText


//////////////////////////////////////////////////////////////////////
// CNodeView::DrawText
// 
// draws the node's text in the given rectangle
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawText(CDC *pDC, CRect& rectInner)
{
	pDC->SetBkMode(TRANSPARENT);

	int nDesiredHeight = __min(rectInner.Height() / 3, 40);
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

}	// CNodeView::DrawText


//////////////////////////////////////////////////////////////////////
// CNodeView::DrawImage
// 
// draws the image in the given rectangle, updating the rectangle
//		to reflect the actual image size
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawImage(CDC *pDC, CRect& rectInner)
{
	if (GetNode()->GetDib() != NULL 
		|| GetNode()->GetIcon() != NULL)
	{
		CSize sizeImage;
		if (GetNode()->GetDib())
		{
			sizeImage = GetNode()->GetDib()->GetSize();
		}
		else 
		{
			sizeImage = CSize(48, 48);
		}

		if (!m_bBackgroundImage)
		{
			CRect rectImage = rectInner;
			rectImage.bottom = rectImage.top + __min(rectImage.Height(), 64);

			int nActualWidth;
			nActualWidth = sizeImage.cx * rectImage.Height() / sizeImage.cy;
			rectImage.right = rectImage.left + nActualWidth;

			if (rectImage.Height() > 10 || rectImage.Width() > 10)
			{
				rectImage.InflateRect(2, 2, 2, 2);
				if (GetNode()->GetDib())
				{
					GetNode()->GetDib()->Draw(*pDC, &rectImage);
				}
				else
				{
					::DrawIconEx(*pDC, rectImage.left, rectImage.top,
							GetNode()->GetIcon(), 
							rectImage.Width(), rectImage.Height(), 
							0, NULL, DI_NORMAL);
				}

/*				if (m_bMaximized)
				{
					CRect rectBtn(rectImage);
					rectBtn.top = rectBtn.bottom + 5;
					rectBtn.bottom = rectBtn.top + 30;
					m_pParent->m_btnGo.MoveWindow(&rectBtn, FALSE);
					CPoint ptOrig = pDC->SetViewportOrg(rectBtn.TopLeft());
					m_pParent->m_btnGo.PrintClient(pDC, PRF_CLIENT);
					pDC->SetViewportOrg(ptOrig);
				}
*/
				// adjust the rectangle to account for the bitmap
				rectInner.left += nActualWidth + 10;
			}
		}
		else
		{
			CRect rectImage(GetOuterRect());

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

			if (GetNode()->GetDib())
			{
				GetNode()->GetDib()->Draw(*pDC, &rectImage);
			}
			else
			{
				::DrawIconEx(*pDC, rectImage.left, rectImage.top,
						GetNode()->GetIcon(), 
						rectImage.Width(), rectImage.Height(), 
						0, NULL, DI_NORMAL);
			}

			// unselect clipping to node view
			pDC->SelectClipRgn(NULL);
		}
	}

}	// CNodeView::DrawImage


//////////////////////////////////////////////////////////////////////
// CNodeView::DrawTitleBand
// 
// draws the band upon which the title is displayed
//////////////////////////////////////////////////////////////////////
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
	COLORREF backColor = DEFAULT_TITLE;
	if (GetNode()->GetSpace() != NULL)
	{
		GetNode()->GetSpace()->GetClassColorMap().Lookup(
			GetNode()->GetClass(), backColor);
	}

	CBrush backBrush;
	backBrush.CreateSolidBrush(backColor);

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

}	// CNodeView::DrawTitleBand




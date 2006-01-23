//////////////////////////////////////////////////////////////////////
// NodeView.cpp: implementation of the CNodeView class.
//
// Copyright (C) 1996-2006 Derek G Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

#include <float.h>

// the class definition
#include <NodeView.h>

// parent class
#include <SpaceView.h>

// the displayed model object
#include <Node.h>
#include ".\include\nodeview.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////
// constants
//////////////////////////////////////////////////////////////////////

const COLORREF DEFAULT_TITLE = RGB(149, 205, 208);

const REAL MAX_SPRING_CONST = 4.0; // 0.99;
const REAL MIN_POST_SUPER = 0.0014;
const REAL VIEW_SCALE = 290.0; // 770.0;

const REAL MIN_WEIGHT_TO_DRAW_LINK = 0.01;

const int TEXT_WIDTH = 400;

const COLORREF BACK_COLOR = RGB(228, 228, 228);

// statics for storing the font for smooth text

bool CNodeView::m_bStaticInit = false;
CFont CNodeView::m_font;
int CNodeView::m_nSrcLineHeight;

CArray<CFont *, CFont *> CNodeView::m_arrTitleFont;


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
	m_layoutSelect(0),
	m_ptMouseDown(-1, -1),
	m_bDragging(FALSE),
	m_bDragged(FALSE),
	m_pParent(pParent),
	m_bBackgroundImage(FALSE),
	m_pendingActivation(0.0f),
	m_bMaximized(FALSE)
{
	// set the view pointer for the node
	GetNode()->SetView(this);

	CRect rectParent;
	pParent->GetClientRect(&rectParent);

	m_vSpringCenter = GetNode()->GetPosition();

	m_posSpring.m_b = 8.0;
	m_actSpring.m_b = 6.0;
	m_layoutSelectSpring.m_b = 7.0;

	if (!m_bStaticInit)
	{
		// initialization of font
		m_nSrcLineHeight = 32;
		m_font.CreateFont(
			m_nSrcLineHeight, 
			0,					// nWidth
			0,					// nEscapement
			0,					// nOrientation
			FW_REGULAR,			// nWeight 
			0,					// bItalic
			0,					// bUnderline
			0,					// bStrikeout
			DEFAULT_CHARSET,	// nCharSet 
			OUT_TT_PRECIS,		// nOutPrecision
			CLIP_DEFAULT_PRECIS,// nClipPrecision
			PROOF_QUALITY,		// nQuality
			FF_DONTCARE,		// nPitchAndFamily
			"Trebeuchet MS"		// lpszFacename
			);

		m_bStaticInit = true;
	}

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

	// check if the node has been completely re-positioned (requiring reset of spring set point)
	if (GetNode()->IsPositionReset(true))
	{
		// just set the node position
		m_vSpringCenter = vScaledPos;
	}
	else
	{
		// otherwise, update the sping
		m_posSpring.m_vPosition = m_vSpringCenter - vScaledPos;
		m_posSpring.m_mass = GetNode()->GetSize(GetNode()->GetActivation()).GetLength() + 0.5;
		m_posSpring.UpdateSpring(0.1);
		m_vSpringCenter = m_posSpring.m_vPosition + vScaledPos;
	}

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
			// springConst 
			// m_actSpring.m_b = m_actSpring.m_b * 0.75
			//	+ 0.25 * 10.0 * m_springActivation / activationThreshold;
				//(MAX_SPRING_CONST - m_actSpring.m_b) 
				// * (1.0 - m_springActivation / activationThreshold);

			// set the post-super flag
			GetNode()->SetPostSuper(m_springActivation > MIN_POST_SUPER);
		}
	}

	// update spring activation
	m_actSpring.m_vPosition[0] = m_springActivation - GetThresholdedActivation();
	m_actSpring.UpdateSpring(0.1);
	m_springActivation = m_actSpring.m_vPosition[0] + GetThresholdedActivation();
 	// m_springActivation = GetThresholdedActivation() * (1.0 - springConst)
	//		+ m_springActivation * springConst;

	// update layout selection spring

	// first, update extInner
	m_pParent->m_skin.CalcInnerOuterRect(this);

	// now, determine integer layout selection (set point), based on m_extCurrent
	REAL layoutSelectInt = __max(m_extInner.Height() - 10.0, 0.0);		// SYNCH with Draw code

	// now determine the integer layout selection (where it should be at) 
	CNodeLayoutManager *pNLM = m_pParent->m_pNLM;
	while (layoutSelectInt < pNLM->GetNextLayoutManager()->GetMinLayoutSelection())
	{
		pNLM = pNLM->GetNextLayoutManager();
	}
	layoutSelectInt = pNLM->GetMinLayoutSelection();

	// now update spring
	m_layoutSelectSpring.m_vPosition[0] = m_layoutSelect - layoutSelectInt;
	m_layoutSelectSpring.UpdateSpring(0.1);
	m_layoutSelect = m_layoutSelectSpring.m_vPosition[0] + layoutSelectInt;

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
void CNodeView::Draw(LPDIRECTDRAWSURFACE lpDDS)
{
	// render the skin
	m_pParent->m_skin.BltSkin(lpDDS, this);

	// get a DC for the drawing surface
	CDC dc;
	GET_ATTACH_DC(lpDDS, dc);

	// only draw if it has a substantial area
	if (m_extOuter.Height() >= 3)
	{
		// get the inner rectangle for drawing 
		CExtent<REAL> extCurrent = m_extInner;

#ifdef NOTHING
		extCurrent.Deflate(5, 5, 5, 5);

		// draw the title band first
		DrawTitleBand(&dc, extCurrent);

		// draw the image (if any)
		if (m_bBackgroundImage)
		{
			DrawImage(&dc, extCurrent);
			DrawTitle(&dc, extCurrent);
			DrawText(&dc, extCurrent);
		}
		else if (extCurrent.Height() > 30)
		{
			// draw the title first, adjusting the rectangle
			DrawTitle(&dc, extCurrent);
			DrawImage(&dc, extCurrent);
			DrawText(&dc, extCurrent);
		}
		else
		{
			// draw the image first
			DrawImage(&dc, extCurrent);
			DrawTitle(&dc, extCurrent);
			DrawText(&dc, extCurrent);
		}
#endif

		///////////////////////////////////////////////////////////////////

		// TESTING NLM

		extCurrent = m_extInner;
		extCurrent.Deflate(5, 5, 5, 5);

		if (extCurrent.Height() >= 0.0)
		{
			// now set up layout
			CNodeLayoutManager *pNLM = m_pParent->m_pNLM;
/*			while (extCurrent.Height() < pNLM->GetMinLayoutSelection())
			{
				pNLM = pNLM->GetNextLayoutManager();
			} */

			CExtent<REAL> extTitle;
			CExtent<REAL> extImage;
			CExtent<REAL> extDesc;

			REAL layoutSelection = extCurrent.Height();
			pNLM->CalcExtent(extCurrent, this, 
				m_layoutSelect > 0.0 ? m_layoutSelect : 0.0, 
					// layoutSelection,		
					// layout selection -- use 10000.0 to trigger for now
				extTitle,
				extImage,
				extDesc);

			// draw the title band first
			DrawTitleBand(&dc, extTitle);
			DrawImage(&dc, extImage);
			DrawTitle(&dc, extTitle);
			DrawText(&dc, extDesc);

/*
			dc.SelectStockObject(NULL_BRUSH);

			CPen penRed(PS_SOLID, 2, RGB(255, 0, 0));
			CPen *pOldPen = dc.SelectObject(&penRed);
			dc.Rectangle((CRect) extTitle);
			dc.SelectObject(pOldPen);
			penRed.DeleteObject();

			if (extImage.Width() > 0.0)
			{
				CPen penGreen(PS_SOLID, 2, RGB(0, 255, 0));
				pOldPen = dc.SelectObject(&penGreen);
				dc.Rectangle((CRect) extImage);
				dc.SelectObject(pOldPen);
				penGreen.DeleteObject();
			}

			if (extDesc.Width() > 0.0)
			{
				CPen penBlue(PS_SOLID, 2, RGB(0, 0, 255));
				pOldPen = dc.SelectObject(&penBlue);
				dc.Rectangle((CRect) extDesc);
				dc.SelectObject(pOldPen);
				penBlue.DeleteObject(); 
			}
			*/
		}
	}

	// release the DC
	RELEASE_DETACH_DC(lpDDS, dc);

}	// CNodeView::Draw


//////////////////////////////////////////////////////////////////////
// CNodeView::DrawLinks
// 
// draws the links for this node view
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawLinks(CDC *pDC, CNodeViewSkin *pSkin)
{
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

			// only draw the link to node view's with activations greater than the current
			if (!pLink->IsStabilizer()
				&& pLinkedView != NULL
				&& !pLinkedView->GetNode()->IsSubThreshold()
				)
			{
				// draw the link
				CVectorD<3> vFrom = GetSpringCenter();
				CVectorD<3> vTo = pLinkedView->GetSpringCenter();
				REAL gain = 0.01 + sqrt(GetNode()->GetLinkTo(pLinkedView->GetNode())->GetGain());

				pSkin->DrawLink(pDC, vFrom, 
					gain * GetSpringActivation(), 
					vTo,
					gain * pLinkedView->GetSpringActivation());
			}

			PROFILE_FLAG("Display", "Gain")
			{
				if (GetNode()->GetActivation() > pLink->GetTarget()->GetActivation()
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
					pDC->TextOut(Round<int>(vMid[0]), Round<int>(vMid[1]), strGain);
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
void CNodeView::DrawTitle(CDC *pDC, CExtent<REAL>& rectInner)
{
	pDC->SetBkMode(TRANSPARENT);

	REAL desiredHeight = rectInner.Height(); // __min(rectInner.Height() / 4.0, 30.0);
	desiredHeight = __max(desiredHeight, 9.0);
	REAL desiredWidth = rectInner.Width() / 80.0;

	CFont *pFont = GetTitleFont(Round<int>(desiredHeight));
	CFont *pOldFont = pDC->SelectObject(pFont);

	// calculate the height of the drawn text
	CRect rectText = (CRect) rectInner;
	rectText.right = rectText.right + 500;

	CString strName = GetNode()->GetName();
	if (GetNode()->GetOptimalStateVector() != NULL)
	{
		strName += "*";
	}

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

	// TODO: does this handle wrapping of title (title shouldn't wrap)
/*	for (int nAt = 0; nAt < 4 
			&& rectText.bottom > m_extInner.Bottom(); nAt++)
	{
		rectText.bottom = rectText.top +
			(rectText.Height() / desiredHeight - 1) * desiredHeight;
	} */
	pDC->DrawText(strName, rectText, 
		DT_LEFT | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);

	// now draw the description body
	if (m_extInner.Height() > 25.0) // GetSpringActivation() >= 0.01)
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

}	// CNodeView::DrawText


//////////////////////////////////////////////////////////////////////
// CNodeView::DrawTitleBand
// 
// draws the band upon which the title is displayed
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawTitleBand(CDC *pDC, const CExtent<REAL>& rectInner)
{
	REAL desiredHeight = rectInner.Height(); // 	REAL desiredHeight = __min(rectInner.Height() / 4.0, 30.0);
	desiredHeight = __max(desiredHeight, 15.0);
	REAL desiredWidth = rectInner.Width() / 80.0;

	CFont *pFont = GetTitleFont(Round<int>(desiredHeight));
	CFont *pOldFont = pDC->SelectObject(pFont);

	// calculate the height of the drawn text
	CRect rectText = (CRect) rectInner;
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
	CExtent<REAL> rectTitle = m_extOuter; 
	rectTitle.SetTop(rectTitle.Top() - 5.0);

	if (m_extInner.Height() > 25.0) // GetSpringActivation() >= 0.01)
	{
		rectTitle.SetBottom(rectText.top + nHeight + 3.0);
	}

	// clip to the node view
	pDC->SelectClipRgn(&GetShape());

	pDC->Rectangle((CRect) rectTitle);

	// unselect clipping to node view
	pDC->SelectClipRgn(NULL);

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);

	pDC->SelectObject(pOldFont);

	// TODO: clean memory leak in m_arrTitleFont
	// font.DeleteObject();

}	// CNodeView::DrawTitleBand


// returns cached title font
CFont * CNodeView::GetTitleFont(int nDesiredHeight)
{
	if (m_arrTitleFont.GetSize() <= nDesiredHeight
			|| m_arrTitleFont[nDesiredHeight] == NULL)
	{
		CFont * pFont = new CFont();
		BOOL bResult = pFont->CreateFont(
			Round<int>(nDesiredHeight), 
			0, //nDesiredWidth,
			0, 0, FW_BOLD, 
			FALSE, FALSE, 0,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			VARIABLE_PITCH,
			"Trebeuchet MS" /* "Arial" */);
		ASSERT(bResult);

		m_arrTitleFont.SetAtGrow(nDesiredHeight, pFont);
	}

	return m_arrTitleFont[nDesiredHeight];
}



//////////////////////////////////////////////////////////////////////
// CNodeView::DrawImage
// 
// draws the image in the given rectangle, updating the rectangle
//		to reflect the actual image size
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawImage(CDC *pDC, CExtent<REAL>& rectInner)
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
			CRect rectImage = (CRect) rectInner;
			int nDeflate = Round<int>(
				0.5 * 
					(rectImage.Height() - (REAL) sizeImage.cy * rectImage.Width() 
						/ (REAL) sizeImage.cx));

			rectImage.DeflateRect(0, 0, 0, 2 * nDeflate);
			// rectImage.right = rectImage.left + nActualWidth;

			if (rectImage.Height() > 2 || rectImage.Width() > 2)
			{
				// rectImage.InflateRect(2, 2, 2, 2);
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
				// rectInner.SetLeft(rectInner.Left() + nActualWidth + 10);
			}
		}
		else
		{
			CRect rectImage(m_extOuter); // GetOuterRect());

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
// CNodeView::DrawText
// 
// draws the node's text in the given rectangle
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawText(CDC *pDC, CExtent<REAL>& rectInner)
{
#define SMOOTH_TEXT
#ifdef SMOOTH_TEXT

	if (rectInner.Bottom() < rectInner.Top())
	{
		return;
	}

	// set max text width
	if (rectInner.Width() > TEXT_WIDTH)
	{
		rectInner.SetRight(rectInner.Left() + TEXT_WIDTH);
	}

	// generates the text surface
	CDC *pTextDC = GetTextSurface();
	
	// now draw smooth text
	// calculate scale from dest to source
	double scale = (double)  m_rectFrom.Width() / (double) rectInner.Width();

	// calculate dest rectangle height, in source units
	double height = scale * (double) rectInner.Height();
	if (height > m_rectFrom.Height())
		height = m_rectFrom.Height();

	// calculate full lines to render
	int nLines = (int) floor(height / (double) m_nSrcLineHeight);

	// compute actual src rectangle
	CRect actualRectFrom = m_rectFrom;
	actualRectFrom.bottom = actualRectFrom.top + nLines * m_nSrcLineHeight;

	// compute actual dest rectangle
	CRect actualRectTo = (CRect) rectInner;
	actualRectTo.bottom = actualRectTo.top + 
		(int) ((double) (nLines * m_nSrcLineHeight) / scale);

	// now blt the first nLines
	pDC->SetStretchBltMode(HALFTONE);
	pDC->StretchBlt(
		actualRectTo.left, actualRectTo.top, actualRectTo.Width(), actualRectTo.Height(), 
		&m_dcText, 
		actualRectFrom.left, actualRectFrom.top, actualRectFrom.Width(), actualRectFrom.Height(), 
		SRCCOPY);

	// move source rectangle to last line
	actualRectFrom.top = actualRectFrom.bottom;
	actualRectFrom.bottom += m_nSrcLineHeight;

	// move dest rectangle to last line
	actualRectTo.top = actualRectTo.bottom;
	actualRectTo.bottom = Round<int>(rectInner.Bottom());

	// compute width and horizontal position of dest rectangle
	scale = (double)  actualRectFrom.Height() / (double) actualRectTo.Height();
	int nRectToWidth = (int)((double) actualRectFrom.Width() / scale);
	actualRectTo.right = actualRectTo.left + nRectToWidth;

	// now blt the last line
	pDC->StretchBlt(
		actualRectTo.left, actualRectTo.top, actualRectTo.Width(), actualRectTo.Height(), 
		&m_dcText, 
		actualRectFrom.left, actualRectFrom.top, actualRectFrom.Width(), actualRectFrom.Height(), 
		SRCCOPY);

#else
	pDC->SetBkMode(TRANSPARENT);

	int nDesiredHeight = __min(rectInner.Height() / 3, 40);
	nDesiredHeight = __max(nDesiredHeight, 15);
	int nDesiredWidth = rectInner.Width() / 80;

	// now draw the description body
	if (GetSpringActivation() >= 0.01)
	{
		nDesiredHeight = __max(nDesiredHeight / 2, 14);

		if (rectInner.bottom > GetOuterRect().bottom)
		{
			// adjust height of rectangle for text
			rectInner.bottom -= rectInner.Height() -
				(rectInner.Height() / nDesiredHeight) * nDesiredHeight;
		}

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
#endif

}	// CNodeView::DrawText



// generates / retrieves pre-rendered text surface as DC
CDC * CNodeView::GetTextSurface(void)
{
	if (m_dcText.m_hDC == NULL)
	{
		// get drawing context
		CDC dcScreen;
		dcScreen.Attach(::GetDC(NULL));

		// determine initial rectangle
		m_rectFrom = CRect(0, 0, TEXT_WIDTH, 0);

		// now compute full rectangle
		int nOptions = DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP;

		// select the font
		dcScreen.SelectObject(&m_font);

		// ref to text
		CString& strText = GetNode()->GetDescription();
		int nLength = strText.GetLength();

		// draw the text
		m_rectFrom.bottom = ::DrawTextEx(dcScreen.GetSafeHdc(), 
			strText.GetBuffer(nLength), nLength,
			m_rectFrom, nOptions | DT_CALCRECT, NULL);
		strText.ReleaseBuffer();

		// allocate the ddraw surface
		m_dcText.CreateCompatibleDC(&dcScreen);

		// creat the bitmap
		m_bitmapText.CreateCompatibleBitmap(&dcScreen, 
			m_rectFrom.Width(), m_rectFrom.Height());

		m_dcText.SelectObject(&m_bitmapText);

		// clear the surface
		CBrush brush(BACK_COLOR);
		CBrush *pOldBrush = m_dcText.SelectObject(&brush);
		m_dcText.SelectStockObject(NULL_PEN);
		m_rectFrom.InflateRect(10, 10, 10, 10);
		m_dcText.Rectangle(m_rectFrom);
		m_rectFrom.DeflateRect(10, 10, 10, 10);
		m_dcText.SelectObject(pOldBrush);

		m_dcText.SetBkMode(OPAQUE);
		m_dcText.SetBkColor(BACK_COLOR);

		// select the font
		m_dcText.SelectObject(&m_font);

		// draw the text
		::DrawTextEx(m_dcText.GetSafeHdc(), 
			strText.GetBuffer(nLength), nLength,
			m_rectFrom, nOptions, NULL);
		strText.ReleaseBuffer();

		// release the screen device context
		::ReleaseDC(NULL, dcScreen.Detach());
	}

	return &m_dcText;
}


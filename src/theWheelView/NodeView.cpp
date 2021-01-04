//////////////////////////////////////////////////////////////////////
// NodeView.cpp: implementation of the CNodeView class.
//
// Copyright (C) 1996-2006 Derek G Lane
// $Id: NodeView.cpp,v 1.26 2007/07/05 02:50:24 Derek Lane Exp $
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


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//////////////////////////////////////////////////////////////////////
// constants
//////////////////////////////////////////////////////////////////////

const COLORREF DEFAULT_TITLE = RGB(166, 190, 191);

// view scaling factor
const REAL VIEW_SCALE = 650.0; // 
		// 550.0;		

// maximum for spring constant
const REAL MAX_SPRING_CONST = 4.0;

// minimum for a post-super node
const REAL MIN_POST_SUPER = 0.0014;

// min weight to draw link
const REAL MIN_WEIGHT_TO_DRAW_LINK = 0.01;

// width of text block
const int TEXT_WIDTH = 400;

// default background color
const COLORREF BACK_COLOR = RGB(228, 228, 228);

// TODO: get this from NodeViewSkin
const int THICK_PEN_WIDTH = 4;

//////////////////////////////////////////////////////////////////////
// statics for storing the font for smooth text

// base font
CFont CNodeView::m_font;

// source line height variable
int CNodeView::m_nSrcLineHeight;

// array of fonts
CArray<CFont *, CFont *> CNodeView::m_arrTitleFont;

// flag to indicate statics initialized
bool CNodeView::m_bStaticInit = false;



//////////////////////////////////////////////////////////////////////////////
class ProfileFlag
{
public:
	ProfileFlag(CString strSection, CString strKey) 
		: m_bInit(false)
		, m_strKey(strKey)
		, m_strSection(strSection)
	{
	}

	bool IsEnabled()
	{
		if (!m_bInit)
		{
			m_bEnabled = (bool) ::AfxGetApp()->GetProfileInt(m_strSection, m_strKey, 0);
			m_bInit = true;
		}
		return m_bEnabled;
	}

private:
	CString m_strSection;
	CString m_strKey;
	bool m_bInit;
	bool m_bEnabled;
};



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

	// set up the spring constants
	m_posSpring.SetB(8.0);
	m_posSpring.SetMass(0.6);
	m_posSpring.SetOrigin(GetNode()->GetPosition());
	m_posSpring.SetPosition(GetNode()->GetPosition());

	m_actSpring.SetB(6.0);
	m_actSpring.SetPosition(CVectorD<3>(pNode->GetActivation(), 0., 0.));

	m_layoutSelectSpring.SetB(4.0); // 7.0);

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
			_T("Trebeuchet MS")	// lpszFacename
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
	return m_posSpring.GetPosition();

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
		vPos = (REAL) 0.5 * (vPos + GetNode()->GetSpace()->GetCenter());
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
		m_pParent->GetSpace()->GetLayoutManager()->GetSuperNodeCount();
	
	if (nSuperNodeCount > 0)
	{
		CNode *pLastSuperNode = 
			m_pParent->GetSpace()->GetNodeAt(nSuperNodeCount-1);
		REAL activationThreshold = pLastSuperNode->GetActivation();

		// compute the normalization factor for super-threshold node views
		REAL superThresholdScale = TOTAL_ACTIVATION // * 1.1
			/ m_pParent->GetSpace()->GetTotalActivation();

		// get this node's activation
		REAL activation = GetNode()->GetActivation();

		// for super-threshold nodes,
		if (activation >= activationThreshold
			|| GetNode()->GetPostSuperCount() > 0)
		{
			// return the scaled, thresholded activation
			return std::min<REAL>(activation * superThresholdScale * superThresholdScale, 
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
	return 	m_actSpring.GetPosition()[0];

}	// CNodeView::GetSpringActivation


//////////////////////////////////////////////////////////////////////
const CExtent<3,REAL>& 
	CNodeView::GetExtOuter()
{
	return m_extOuter;

}	// CNodeView::GetExtOuter


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
CRgn& 
	CNodeView::GetShapeHit()
{
	return m_shapeHit;

}	// CNodeView::GetShapeHit


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
	m_posSpring.SetOrigin(vScaledPos);

	// check if the node has been completely re-positioned (requiring reset of spring set point)
	if (GetNode()->GetIsResetPositionValid()) // GetNode()->IsPositionReset(true))
	{
		if (GetNode()->GetMaxActivator())
		{
			CNodeView *pView = (CNodeView *)GetNode()->GetMaxActivator()->GetView();
			if (pView->GetNode()->GetIsResetPositionValid())
				pView->UpdateSpringPosition(springConst);
			vScaledPos = pView->m_posSpring.GetPosition();

			// just set the node position
			m_posSpring.SetPosition(vScaledPos); 
		} 
		else
		{
			ASSERT(GetNode()->GetMaxActivator() != NULL);

			CVectorD<3> vTemp = GetNode()->GetPosition();
			GetNode()->SetPosition(GetNode()->GetResetPosition());
			vScaledPos = GetScaledNodeCenter();
			GetNode()->SetPosition(vTemp);

			// just set the node position
			m_posSpring.SetPosition(vScaledPos); 
		}

		GetNode()->SetIsResetPositionValid(false);
	}
	// else
	{
		// otherwise, update the sping
		m_posSpring.UpdateSpring(0.01, 10);
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
		REAL m_springActivation = m_actSpring.GetPosition()[0];

		// update spring activation
 		m_springActivation = TOTAL_ACTIVATION * 0.35 * (1.0 - springConst)
			+ m_springActivation * springConst;

		m_actSpring.SetPosition(CVectorD<3>(m_springActivation, 0., 0.));

		return;
	}


	// update spring activation
	m_actSpring.SetOrigin(CVectorD<3>(GetThresholdedActivation(), 0., 0.));
	m_actSpring.UpdateSpring(0.01, 10);

	// update layout selection spring

	// first, update extInner
	m_pParent->m_skin.CalcInnerOuterRect(this);
	m_pParent->m_skin.CalcShape(this, m_shape, 7); // THICK_PEN_WIDTH);
	m_pParent->m_skin.CalcShape(this, m_shapeHit, 10);

	// now, determine integer layout selection (set point), based on m_extCurrent
	REAL layoutSelectInt = __max(m_extInner.GetSize(1) - 10.0, 0.0);		// SYNCH with Draw code

	// now determine the integer layout selection (where it should be at) 
	CNodeLayoutManager *pNLM = m_pParent->m_pNLM;
	while (layoutSelectInt < pNLM->GetNextLayoutManager()->GetMinLayoutSelection())
	{
		pNLM = pNLM->GetNextLayoutManager();
	}
	layoutSelectInt = pNLM->GetMinLayoutSelection();

	// now update spring
	m_layoutSelectSpring.SetPosition(CVectorD<3>(m_layoutSelect - layoutSelectInt, 0., 0.));
	m_layoutSelectSpring.UpdateSpring(0.01, 10);
	m_layoutSelect = m_layoutSelectSpring.GetPosition()[0] + layoutSelectInt;

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
void 
	CNodeView::AttenuatePending(REAL atten)
{
	m_pendingActivation *= atten;
}

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
void CNodeView::Draw(LPDIRECT3DDEVICE9 lpDDS)
{
	D3DXMATRIX mat;
	D3DXMatrixTranslation(&mat, 
		-m_extInner.GetCenter()[0], 
		-m_extInner.GetCenter()[1], 0.0);
		// m_extInner.GetCenter()[2]);
	ASSERT_HRESULT(lpDDS->SetTransform(D3DTS_WORLD, &mat));

	// render the skin
	// m_pParent->m_skin.BltSkin(lpDDS, this);
	m_pParent->m_pSkin->Render(this);

/*	LPDIRECT3DSURFACE9 lpd3dSurf = NULL;
	ASSERT_HRESULT(lpDDS->GetRenderTarget(0, &lpd3dSurf));

	HDC hdc;
	ASSERT_HRESULT(lpd3dSurf->GetDC(&hdc));

	// get a DC for the drawing surface
	CDC dc;
	dc.Attach(hdc);
	// GET_ATTACH_DC(lpDDS, dc);

	// only draw if it has a substantial area
	if (m_extOuter.GetSize(1) >= 1)
	{
		// get the inner rectangle for drawing 
		CExtent<3,REAL> extCurrent = m_extInner;
		if (extCurrent.GetSize(1) > 10)
			extCurrent.Deflate(5, 5, 5, 5);
		
		if (extCurrent.GetSize(1) >= 0.0)
		{
			// now set up layout
			CNodeLayoutManager *pNLM = m_pParent->m_pNLM;

			// store the calculated extents
			CExtent<3,REAL> extTitle;
			CExtent<3,REAL> extImage;
			CExtent<3,REAL> extDesc;

			// current height = NLM selector
			pNLM->CalcExtent(extCurrent, this, 
				m_layoutSelect > 0.0 ? m_layoutSelect : 0.0, 
				extTitle,
				extImage,
				extDesc);

			// draw the node elements
			DrawTitleBand(&dc, extTitle);
			DrawImage(&dc, extImage);
			DrawTitle(&dc, extTitle);
			DrawText(&dc, extDesc);
		}
	}

	// release the DC
	// RELEASE_DETACH_DC(lpDDS, dc);
	dc.Detach();

	lpd3dSurf->ReleaseDC(hdc); 

	lpd3dSurf->Release(); */

}	// CNodeView::Draw

static ProfileFlag g_DisplayGainFlag(_T("Display"), _T("Gain"));

//////////////////////////////////////////////////////////////////////
// CNodeView::DrawLinks
// 
// draws the links for this node view
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawLinks(LPDIRECT3DDEVICE9 lpDDS, // CDC *pDC, 
						  CNodeViewSkin *pSkin)
{
	LPDIRECT3DSURFACE9 lpd3dSurf = NULL;
	ASSERT_HRESULT(lpDDS->GetRenderTarget(0, &lpd3dSurf));

	HDC hdc;
	ASSERT_HRESULT(lpd3dSurf->GetDC(&hdc));
	CDC dc;
	dc.Attach(hdc);
	CDC *pDC = &dc;

	pDC->SetBkMode(TRANSPARENT);

	// draw the links only if the activation is above 0.0
	if (!GetNode()->GetIsSubThreshold())
	{
		// for each link
		for (int nAtLink = 0; nAtLink < GetNode()->GetLinkCount(); nAtLink++)
		{
			CNodeLink *pLink = GetNode()->GetLinkAt(nAtLink);
			CNodeView *pLinkedView = (CNodeView *)pLink->GetTarget()->GetView();

			// only draw the link to node view's with activations greater than the current
			if (!pLink->GetIsStabilizer()
				&& pLinkedView != NULL
				&& !pLinkedView->GetNode()->GetIsSubThreshold()
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

			// check if gain should be displayed
			// PROFILE_FLAG(_T("Display"), _T("Gain"))
			if (g_DisplayGainFlag.IsEnabled())
			{
				if (GetNode()->GetActivation() > pLink->GetTarget()->GetActivation()
					&& pLinkedView != NULL
					&& !pLinkedView->GetNode()->GetIsSubThreshold())
				{
					// draw the link
					CVectorD<3> vFrom = GetSpringCenter();
					CVectorD<3> vTo = pLinkedView->GetSpringCenter();
					REAL gain = GetNode()->GetLinkTo(pLinkedView->GetNode())->GetGain();

					CVectorD<3> vMid = (REAL) 0.5 * (vFrom + vTo);
					CString strGain;
					strGain.Format(_T("%0.2lf"), (double) pLink->GetGain());
					pDC->TextOut(Round<int>(vMid[0]), Round<int>(vMid[1]), strGain);
				}
			}
		}
	}

	dc.Detach();

	lpd3dSurf->ReleaseDC(hdc);

	lpd3dSurf->Release();

}	// CNodeView::DrawLinks


//////////////////////////////////////////////////////////////////////
// CNodeView::DrawTitle
// 
// draws the node's title in the given rectangle
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawTitle(CDC *pDC, const CExtent<3,REAL>& rectTitle)
{
	pDC->SetBkMode(TRANSPARENT);

	// calc desired height
	REAL desiredHeight = rectTitle.GetSize(1);
	desiredHeight = __max(desiredHeight, 11.0);
	REAL desiredWidth = rectTitle.GetSize(0) / 80.0;

	// get font
	CFont *pFont = GetTitleFont(Round<int>(desiredHeight));
	CFont *pOldFont = pDC->SelectObject(pFont);

	// calculate the height of the drawn text
	CRect rectText = (CRect) rectTitle;
	rectText.right = rectText.right + 500;
	if (rectText.Height() < Round<int>(desiredHeight))
		rectText.bottom = rectText.top + Round<int>(desiredHeight);

	// copy the name string
	// const CString& strName = GetNode()->GetName();
	CString strName = GetNode()->GetName();
	strName += GetNode()->GetIsSubThreshold() ? _T("*") : _T("");
	if (GetNode()->GetPostSuperCount() > 0)
	{
		CString strCount;
		strCount.Format(_T("%i"), GetNode()->GetPostSuperCount());
		strName += strCount;
	}

	// draw the text
	pDC->DrawText(strName, rectText, 
		DT_LEFT | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);

	// select out font
	pDC->SelectObject(pOldFont);

}	// CNodeView::DrawText


//////////////////////////////////////////////////////////////////////
// CNodeView::DrawTitleBand
// 
// draws the band upon which the title is displayed
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawTitleBand(CDC *pDC, const CExtent<3,REAL>& extTitle)
{
	REAL desiredHeight = extTitle.GetSize(1);
	desiredHeight = __max(desiredHeight, 9.0);
	REAL desiredWidth = extTitle.GetSize(0) / 80.0;

	// get font
	CFont *pFont = GetTitleFont(Round<int>(desiredHeight));
	CFont *pOldFont = pDC->SelectObject(pFont);

	// calculate the height of the drawn text
	CRect rectText = (CRect) extTitle;
	rectText.right += 500;
	int nHeight = pDC->DrawText(GetNode()->GetName(), rectText, 
		DT_CALCRECT | DT_LEFT | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);
	nHeight = __max(nHeight, Round<int>(desiredHeight));

	// draw the background for the title
	COLORREF backColor = DEFAULT_TITLE;
	if (GetNode()->GetSpace() != NULL)
	{
		GetNode()->GetSpace()->GetClassColorMap().Lookup(
			GetNode()->GetClass(), backColor);
	}

	// form background brush
	CBrush backBrush;
	backBrush.CreateSolidBrush(backColor);
	CBrush *pOldBrush = pDC->SelectObject(&backBrush);

	// use null pen
	CPen *pOldPen = (CPen *)pDC->SelectStockObject(NULL_PEN);

	// now calculate useful title rectangle
	CExtent<3,REAL> rectTitle = m_extOuter; 
	rectTitle.SetMin(1, rectTitle.GetMin(1) - 5.0);

	// only band if height greater than min
	if (m_extInner.GetSize(1) > 25.0) 
	{
		rectTitle.SetMax(1, rectText.top + nHeight + 3.0);
	}

	// clip to the node view
	pDC->SelectClipRgn(&GetShape());

	// draw the rectangle
	pDC->Rectangle((CRect) rectTitle);

	// unselect clipping to node view
	pDC->SelectClipRgn(NULL);

	// select brush and pen
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);

	// select out font
	pDC->SelectObject(pOldFont);

}	// CNodeView::DrawTitleBand


//////////////////////////////////////////////////////////////////////
CFont * 
	CNodeView::GetTitleFont(int nDesiredHeight)
	// returns cached title font
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
			_T("Trebeuchet MS") /* "Arial" */);
		ASSERT(bResult);

		m_arrTitleFont.SetAtGrow(nDesiredHeight, pFont);
	}

	return m_arrTitleFont[nDesiredHeight];

}	// CNodeView::GetTitleFont


//////////////////////////////////////////////////////////////////////
// CNodeView::DrawImage
// 
// draws the image in the given rectangle, updating the rectangle
//		to reflect the actual image size
//////////////////////////////////////////////////////////////////////
void CNodeView::DrawImage(CDC *pDC, const CExtent<3,REAL>& extImage)
{
	if (GetNode()->GetDib() != NULL)
	{
		// get image size
		CSize sizeImage(48, 48);
		if (GetNode()->GetDib())
		{
			sizeImage = GetNode()->GetDib()->GetSize();
		}

		// calculate rectangle
		if (!m_bBackgroundImage)
		{
			// form image rectangle
			CRect rectImage = (CRect) extImage;

			// compute deflate value
			int nDeflate = Round<int>(
				0.5 * 
					(rectImage.Height() - (REAL) sizeImage.cy * rectImage.Width() 
						/ (REAL) sizeImage.cx));

			// deflat the rectangle
			rectImage.DeflateRect(0, 0, 0, 2 * nDeflate);

			// only draw if substantial
			if (rectImage.Height() > 2 || rectImage.Width() > 2)
			{
				// is there a DIB?
				if (GetNode()->GetDib())
				{
					// yes, draw
					GetNode()->GetDib()->Draw(*pDC, &rectImage);
				}
			}
		}
		else
		{
			// background, so draw in outer rectangle
			CRect rectImage(m_extOuter);

			// do we preserve width???
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

			// is there a DIB?
			if (GetNode()->GetDib())
			{
				// yes, draw
				GetNode()->GetDib()->Draw(*pDC, &rectImage);
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
void CNodeView::DrawText(CDC *pDC, CExtent<3,REAL>& extDesc)
{
	if (extDesc.GetMax(1) < extDesc.GetMin(1))
	{
		return;
	}

	// set max text width
	if (extDesc.GetSize(0) > TEXT_WIDTH)
	{
		extDesc.SetMax(0, extDesc.GetMin(0) + TEXT_WIDTH);
	}

	// generates the text surface
	CDC *pTextDC = GetTextSurface();
	
	// now draw smooth text
	// calculate scale from dest to source
	double scale = (double)  m_rectFrom.Width() / (double) extDesc.GetSize(0);

	// calculate dest rectangle height, in source units
	double height = scale * (double) extDesc.GetSize(1);
	if (height > m_rectFrom.Height())
	{
		height = m_rectFrom.Height();
	}

	// calculate full lines to render
	int nLines = (int) floor(height / (double) m_nSrcLineHeight);

	// compute actual src rectangle
	CRect actualRectFrom = m_rectFrom;
	actualRectFrom.bottom = actualRectFrom.top + nLines * m_nSrcLineHeight;

	// compute actual dest rectangle
	CRect actualRectTo = (CRect) extDesc;
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
	actualRectTo.bottom = Round<int>(extDesc.GetMax(1));

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

}	// CNodeView::DrawText


//////////////////////////////////////////////////////////////////////////////
CDC * 
	CNodeView::GetTextSurface(void)
	// generates / retrieves pre-rendered text surface as DC
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
		CString& strText = const_cast<CString&>(GetNode()->GetDescription());
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

}	// CNodeView::GetTextSurface


//////////////////////////////////////////////////////////////////////
bool 
	CNodeView::IsActivationGreater(CNodeView *pNodeView1, CNodeView *pNodeView2)
	// comparison function for two CNodeView pointers, based on 
	//		the activation value of both
{
	// return difference in activation; > 0 if act2 > act1
	return // pNodeView1->GetNode()->GetActivation() > pNodeView2->GetNode()->GetActivation();
		pNodeView1->GetNode()->IsActivationGreater(pNodeView2->GetNode());

}	// CNodeView::IsActivationGreater


//////////////////////////////////////////////////////////////////////
bool
	CNodeView::IsActDiffGreater(CNode *pNode1, CNode *pNode2)
	// comparison function for two CNodeView pointers, based on 
	//		the activation factor value of both
{
	CNodeView *pNodeView1 = (CNodeView *) pNode1->GetView();

	// factor for assessing the drawing order
	REAL factor1 = exp(pNodeView1->GetNode()->GetPrimaryActivation())
			* (pNodeView1->GetThresholdedActivation() 
				- pNodeView1->GetSpringActivation());

	CNodeView *pNodeView2 = (CNodeView *) pNode2->GetView();

	REAL factor2 = exp(pNodeView2->GetNode()->GetPrimaryActivation())
			* (pNodeView2->GetThresholdedActivation() 
				- pNodeView2->GetSpringActivation());

	// return difference in factors; > 0 if factor1 > factor2
	return factor1 < factor2; // 
		// factor2 > factor1;

}	// NodeView::IsActDiffGreater



////////////////////////////////////
// Copyright (C) 1996-2000 DG Lane
// U.S. Patent Pending
////////////////////////////////////

// NodeView.cpp : implementation file
//

#include "stdafx.h"
#include "theWheel2001.h"
#include "NodeView.h"

#include "SpaceView.h"

#include "NewNodeDlg.h"

#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNodeView

CNodeView::CNodeView(CNode *pNode)
: forNode(pNode),
	center(CPoint(0, 0)),
	springCenter(CPoint(0, 0)),
	springActivation(pNode->activation.Get()),
	m_ptMouseDown(-1, -1),
	m_bDragging(FALSE),
	m_bDragged(FALSE),
	isWaveMode(FALSE),
	m_pPiggyBack(NULL)
{
	rectWindow.AddObserver(this, (ChangeFunction) OnChange);
	springCenter.AddObserver(this, (ChangeFunction) OnChange);
	springActivation.AddObserver(this, (ChangeFunction) OnChange);
}

CNodeView::~CNodeView()
{
}

// TODO: break this up
void CNodeView::OnChange(CObservableObject *pSource, void *pOldValue)
{
	if ((pSource == &rectWindow) && ::IsWindow(m_hWnd))
	{
		CRect rectWnd;
		GetWindowRect(&rectWnd);	// get the old window rectangle

		// get the parent window and transform to its coordinates
		GetParent()->ScreenToClient(&rectWnd);

		// invalidate the old region
		CRgn rgn;
		rgn.CreateRectRgn(0, 0, 1, 1);
		GetWindowRgn(rgn);
		rgn.OffsetRgn(rectWnd.TopLeft());
		GetParent()->InvalidateRgn(&rgn, FALSE);

		// move the window
		MoveWindow(rectWindow.Get(), FALSE);

		// set the springCenter point
		springCenter.Set(rectWindow.Get().CenterPoint());

		// and invalidate
		Invalidate(FALSE);
	}
	else if (pSource == &springCenter)
	{
		// compute the offset
		CPoint ptOffset = rectWindow.Get().CenterPoint() - springCenter.Get();
		rectWindow.Set(rectWindow.Get() - ptOffset);
	}
	else if ((pSource == &springActivation) && ::IsWindow(m_hWnd))
	{
		// CSpaceView *pParent = (CSpaceView *)GetParent();
		CRect rectParent;
		GetParent()->GetWindowRect(&rectParent);

		// compute the area interpreting springActivation as the fraction of the 
		//		parent's total area
		float area = springActivation.Get() * (rectParent.Width() * rectParent.Height());

		// compute the desired aspect ratio (maintain the current aspect ratio)
		float aspectRatio = 0.75f - 0.375f * (float) exp(-8.0f * springActivation.Get());

		// compute the new width and height from the desired area and the desired
		//		aspect ratio
		int nWidth = (int) sqrt(area / aspectRatio);
		int nHeight = (int) sqrt(area * aspectRatio);

		// set the width and height of the window, keeping the center constant
		CRect rect;
		rect.left = (long) springCenter.Get()[0] - nWidth / 2;
		rect.right = (long) springCenter.Get()[0] + nWidth / 2;
		rect.top = (long) springCenter.Get()[1] - nHeight / 2;
		rect.bottom = (long) springCenter.Get()[1] + nHeight / 2;

		rectWindow.Set(rect);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetOuterRect
//
//
/////////////////////////////////////////////////////////////////////////////
CRect CNodeView::GetOuterRect()
{
	// outer rectangle = client rectangle
	CRect rectOuter;
	GetClientRect(&rectOuter);

	return rectOuter;
}


/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetInnerRect
//
//
/////////////////////////////////////////////////////////////////////////////
CRect CNodeView::GetInnerRect()
{
	CRect rectOuter = GetOuterRect();

	// compute the r, which represents the amount of "elliptical-ness"
	float r = 1.0f - (1.0f - 1.0f / (float) sqrt(2.0f))
		* (float) exp(-8.0f * springActivation.Get());

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


/////////////////////////////////////////////////////////////////////////////
// CNodeView::GetLeftRightEllipseRect
//
//
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
//
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

BEGIN_MESSAGE_MAP(CNodeView, CWnd)
	//{{AFX_MSG_MAP(CNodeView)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNodeView message handlers

/////////////////////////////////////////////////////////////////////////////
// Constants for the colors of the CNodeView
//
/////////////////////////////////////////////////////////////////////////////
COLORREF BACKGROUND_COLOR = RGB(232, 232, 232);

COLORREF UPPER_DARK_COLOR = RGB(240, 240, 240);
COLORREF UPPER_LIGHT_COLOR = RGB(255, 255, 255);

COLORREF LOWER_LIGHT_COLOR = RGB(160, 160, 160);
COLORREF LOWER_DARK_COLOR = RGB(128, 128, 128);

void CNodeView::OnPaint() 
{
	// device context for painting
	CPaintDC dc(this); 
	
	// create a memory device context
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	// get the inner rectangle for drawing the text
	CRect rectClient;
	GetClientRect(&rectClient);

	// Draw the image to the back-buffer
	CBitmap bitmapBuffer;
	bitmapBuffer.CreateBitmap(rectClient.Width(), rectClient.Height(), 1, 32, NULL); 
	dcMem.SelectObject(&bitmapBuffer);

	CBrush brush;
	brush.CreateSolidBrush(BACKGROUND_COLOR);
	dcMem.FillRect(&rectClient, &brush);

	// get the inner rectangle for drawing the text
	CRect rectInner = GetInnerRect();

	if (forNode->GetDib() != NULL && springActivation.Get() < 0.05)
	{
		// calculate the bitmap size 
		float bitmapSize = rectInner.Height() * 0.03f / springActivation.Get();
		bitmapSize = min(bitmapSize, rectInner.Height());
		float margin = rectInner.Height() - bitmapSize;

		CRect rectImage = rectInner;
		rectImage.top += (int) margin / 2;
		rectImage.right = rectImage.left + (int) bitmapSize;
		rectImage.bottom -= (int) margin / 2;

		rectImage.DeflateRect(5, 5, 5, 5);

		// draw the bitmap next to the title
		forNode->GetDib()->Draw(dcMem, &rectImage);

		// dcMem.Ellipse(rectImage);

		// adjust the rectangle to account for the bitmap
		rectInner.left += (int) bitmapSize;
	}

	// draw the text
	dcMem.SetBkMode(TRANSPARENT);

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

	CFont *pOldFont = dcMem.SelectObject(&font);

	CRect rectText(rectInner);
	rectText.DeflateRect(5, 5, 5, -5);
	dcMem.DrawText(forNode->name.Get(), rectText, 
		DT_CENTER | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);

	// now calculate the height of the drawn text
	rectText = rectInner;
	rectText.DeflateRect(5, 5, 5, 5);
	int nHeight = dcMem.DrawText(forNode->name.Get(), rectText, 
		DT_CALCRECT | DT_CENTER | DT_END_ELLIPSIS | DT_VCENTER | DT_WORDBREAK);

	// stores the old pen that was selected into the context
	CPen *pOldPen;

	// now draw the description body
//	CPen penTemp(PS_SOLID, 1, RGB(0, 0, 0));
//	pOldPen = dcMem.SelectObject(&penTemp);
	if (springActivation.Get() >= 0.05)
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
			forNode->GetDib()->Draw(dcMem, &rectImage);

			// draw the bitmap next to the title
			// dcMem.Ellipse(rectImage);

			// adjust the rectangle to account for the bitmap
			rectText.left += rectText.Height();
		}
//	}

//	dcMem.Rectangle( rectText );

//	dcMem.SelectObject(pOldPen);

//	if (nDesiredHeight > 12)
//	{
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

		dcMem.SelectObject(&smallFont);
		dcMem.DrawText(forNode->description.Get(), rectText, 
			DT_LEFT | DT_END_ELLIPSIS | DT_WORDBREAK);

		dcMem.SelectObject(pOldFont);
		smallFont.DeleteObject();
	}
	else
		dcMem.SelectObject(pOldFont);

	font.DeleteObject();

	// reset the inner rectangle
	rectInner = GetInnerRect();

	// get the guide rectangles
	CRect rectLeftRightEllipse = GetLeftRightEllipseRect();
	CRect rectTopBottomEllipse = GetTopBottomEllipseRect();

	// draw the upper-left half of the outline
	CPen penUpperLeftThick(PS_SOLID, 6, UPPER_DARK_COLOR);
	pOldPen = dcMem.SelectObject(&penUpperLeftThick);
#ifdef NO_CURVE
	dcMem.MoveTo(rectInner.left, rectInner.bottom);
	dcMem.LineTo(rectInner.TopLeft());
	dcMem.LineTo(rectInner.right, rectInner.top);
#else
	dcMem.Arc(&rectLeftRightEllipse, rectInner.TopLeft(), 
		CPoint(rectInner.left, rectInner.bottom));
	dcMem.Arc(&rectTopBottomEllipse, CPoint(rectInner.right, rectInner.top), 
		rectInner.TopLeft());
#endif

	CPen penUpperLeftThin(PS_SOLID, 3, UPPER_LIGHT_COLOR);
	dcMem.SelectObject(&penUpperLeftThin);
#ifdef NO_CURVE
	dcMem.MoveTo(rectInner.left, rectInner.bottom);
	dcMem.LineTo(rectInner.TopLeft());
	dcMem.LineTo(rectInner.right, rectInner.top);
#else
	dcMem.Arc(&rectLeftRightEllipse, rectInner.TopLeft(), 
		CPoint(rectInner.left, rectInner.bottom));
	dcMem.Arc(&rectTopBottomEllipse, CPoint(rectInner.right, rectInner.top), 
		rectInner.TopLeft());
#endif

	// draw the lower-right half of the outline
	CPen penLowerRightThick(PS_SOLID, 6, LOWER_LIGHT_COLOR);
	dcMem.SelectObject(&penLowerRightThick);
#ifdef NO_CURVE
	dcMem.MoveTo(rectInner.right, rectInner.top);
	dcMem.LineTo(rectInner.BottomRight());
	dcMem.LineTo(rectInner.left, rectInner.bottom);
#else
	dcMem.Arc(&rectLeftRightEllipse, rectInner.BottomRight(), 
		CPoint(rectInner.right, rectInner.top));
	dcMem.Arc(&rectTopBottomEllipse, CPoint(rectInner.left, rectInner.bottom), 
		rectInner.BottomRight());
#endif

	CPen penLowerRightThin(PS_SOLID, 3, LOWER_DARK_COLOR);
	dcMem.SelectObject(&penLowerRightThin);
#ifdef NO_CURVE
	dcMem.MoveTo(rectInner.right, rectInner.top);
	dcMem.LineTo(rectInner.BottomRight());
	dcMem.LineTo(rectInner.left, rectInner.bottom);
#else
	dcMem.Arc(&rectLeftRightEllipse, rectInner.BottomRight(), 
		CPoint(rectInner.right, rectInner.top));
	dcMem.Arc(&rectTopBottomEllipse, CPoint(rectInner.left, rectInner.bottom), 
		rectInner.BottomRight());
#endif

	// restore the old pen 
	dcMem.SelectObject(pOldPen);

	// Now blit the backbuffer to the screen
	dc.BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dcMem, 0, 0, SRCCOPY);

	// clean up
	dcMem.DeleteDC();
	bitmapBuffer.DeleteObject();

	// Do not call CWnd::OnPaint() for painting messages
}

void CNodeView::OnSize(UINT nType, int cx, int cy) 
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
	CRect rectInner = GetInnerRect();
	CRgn rgnWindow;
	rgnWindow.CreateRectRgnIndirect(rectInner);
#ifndef NO_CURVE
	rgnWindow.CombineRgn(&ellipseTopBottom, &ellipseLeftRight, RGN_AND);
#endif

	// set the window's region to the intersection
	SetWindowRgn(rgnWindow, FALSE);	

	CWnd::OnSize(nType, cx, cy);

	// retrieve the new window rectangle
	CRect rectWnd;
	GetWindowRect(&rectWnd);

	// get the parent window and transform to its coordinates
	GetParent()->ScreenToClient(&rectWnd);

	// set the window rect value
	rectWindow.Set(rectWnd);
}

BOOL CNodeView::PreCreateWindow(CREATESTRUCT& cs) 
{
#ifdef NO_NODEVIEWS
	LONG mask = ~WS_VISIBLE;
	cs.style &= mask;
#endif

	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_HANDPOINT)),
		CreateSolidBrush(BACKGROUND_COLOR), NULL);

	return TRUE;
}

BOOL CNodeView::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE; // CWnd::OnEraseBkgnd(pDC);
}

void CNodeView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (!m_bDragging)
	{
		m_bDragging = TRUE;
		m_ptMouseDown = point;
	
		m_bDragged = FALSE;

		// compute the new activation
		// float oldActivation = forNode->activation.Get();
		// float newActivation = ActivationCurve(oldActivation * 1.6f,
		//	0.3f);
		// forNode->activation.Set(newActivation);

		CSpaceView *pParent = (CSpaceView *)GetParent();
		
		// propagate activation
		pParent->ActivateNode(this, 0.25);
	}

	SetCapture();

	CWnd::OnLButtonDown(nFlags, point);
}

void CNodeView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bDragging = FALSE;

	::ReleaseCapture();

	// redraw the parent
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	
	CWnd::OnLButtonUp(nFlags, point);
}

void CNodeView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bDragging)
	{
		CPoint ptOffset = (point - m_ptMouseDown);

		if ((ptOffset.x * ptOffset.x + ptOffset.y * ptOffset.y) > 10)
		{
			m_bDragged = TRUE;
		}

		springCenter.Set(springCenter.Get() + CVector<2>(ptOffset));
		center.Set(springCenter.Get());

		CSpaceView *pParent = (CSpaceView *)GetParent();

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (isWaveMode.Get())
	{
		CPoint ptOffset = (point - m_ptMouseDown);
		float lengthSq = (float) sqrt(ptOffset.x * ptOffset.x + ptOffset.y * ptOffset.y);

		if (lengthSq > 10.0f)
		{
			// compute the new activation
			CSpaceView *pParent = (CSpaceView *)GetParent();
			pParent->ActivateNode(this, 0.15);

			// do some idle time processing
			CSpaceView *pSpaceView = (CSpaceView *)GetParent();
			pSpaceView->LayoutNodeViews();

			// update the privates
			int nAt;
			for (nAt = 0; nAt < pSpaceView->nodeViews.GetSize(); nAt++)
			{
				pSpaceView->nodeViews.Get(nAt)->UpdateSprings();
			}

			pSpaceView->RedrawWindow();
			for (nAt = 0; nAt < pSpaceView->nodeViews.GetSize(); nAt++)
			{
				pSpaceView->nodeViews.Get(nAt)->RedrawWindow();
			}

			m_ptMouseDown = point;
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CNodeView::UpdateSprings()
{
	CSpaceView *pSpaceView = (CSpaceView *)GetParent();

	CRect rect;
	pSpaceView->GetClientRect(&rect);
	CVector<2> vNewCenter(rect.CenterPoint());

	float newActivation;
	newActivation = thresholdedActivation.Get() * 0.125f
		+ springActivation.Get() * 0.875f;

	// if (forNode->activation.Get() >= activationThreshold)
	if (thresholdedActivation.Get() > 0.0f)
	{
		vNewCenter = center.Get() * 0.125 + springCenter.Get() * 0.875;
	}
	else 
	{
		if (m_pPiggyBack == NULL)
		{
			CNodeLink *pMaxLink = forNode->links.Get(0);
			if (pMaxLink != NULL)
			{
				m_pPiggyBack = pSpaceView->
					GetViewForNode(pMaxLink->forTarget.Get());
				ASSERT(m_pPiggyBack->forNode.Get() == pMaxLink->forTarget.Get());
			}
		}
		if (m_pPiggyBack != NULL)
		{
			vNewCenter = center.Get() * 0.9375 
				+ m_pPiggyBack->center.Get() * 0.0625;
			center.Set(vNewCenter);
		}
	}
	springCenter.Set(vNewCenter);

	springActivation.Set(newActivation);
}

void CNodeView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CNewNodeDlg newDlg(GetParent());

	newDlg.m_strName = forNode->name.Get();
	newDlg.m_strDesc = forNode->description.Get();
	newDlg.m_strImageFilename = forNode->imageFilename.Get();
	newDlg.m_pNode = forNode.Get();

	if (newDlg.DoModal() == IDOK)
	{
		forNode->name.Set(newDlg.m_strName);
		forNode->description.Set(newDlg.m_strDesc);
		forNode->imageFilename.Set(newDlg.m_strImageFilename);
		
		Invalidate();
	}
	
	CWnd::OnLButtonDblClk(nFlags, point);
}

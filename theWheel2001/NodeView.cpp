// NodeView.cpp : implementation file
//

#include "stdafx.h"
#include "theWheel2001.h"
#include "NodeView.h"

#include "SpaceView.h"

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
	privActivation(0.0f),
	m_ptMouseDown(-1, -1),
	m_bDragging(FALSE),
	m_bDragged(FALSE),
	isWaveMode(FALSE)
{
	rectWindow.AddObserver(this);
	privCenter.AddObserver(this);
	privActivation.AddObserver(this);
}

CNodeView::~CNodeView()
{
}

void CNodeView::OnChange(CObservableObject *pSource)
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

		// set the privCenter point
		privCenter.Set(rectWindow.Get().CenterPoint());

		// and invalidate
		Invalidate(FALSE);
	}
	else if (pSource == &privCenter)
	{
		// compute the offset
		CPoint ptOffset = rectWindow.Get().CenterPoint() - privCenter.Get();
		rectWindow.Set(rectWindow.Get() - ptOffset);
	}
	else if ((pSource == &privActivation) && ::IsWindow(m_hWnd))
	{
		// CSpaceView *pParent = (CSpaceView *)GetParent();
		CRect rectParent;
		GetParent()->GetWindowRect(&rectParent);

		// compute the area interpreting privActivation as the fraction of the 
		//		parent's total area
		float area = privActivation.Get() * (rectParent.Width() * rectParent.Height());

		// compute the desired aspect ratio (maintain the current aspect ratio)
		float aspectRatio = 0.75f - 0.375f * (float) exp(-8.0f * privActivation.Get());

		// compute the new width and height from the desired area and the desired
		//		aspect ratio
		int nWidth = (int) sqrt(area / aspectRatio);
		int nHeight = (int) sqrt(area * aspectRatio);

		// set the width and height of the window, keeping the center constant
		CRect rect;
		rect.left = (long) privCenter.Get()[0] - nWidth / 2;
		rect.right = (long) privCenter.Get()[0] + nWidth / 2;
		rect.top = (long) privCenter.Get()[1] - nHeight / 2;
		rect.bottom = (long) privCenter.Get()[1] + nHeight / 2;

		rectWindow.Set(rect);
	}
}

float CNodeView::ActivationCurve(float a, float a_max)
{
	float frac = a / (a + a_max);

	return (1 - frac) * a + (frac * a_max);
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
		* (float) exp(-8.0f * privActivation.Get());

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
	CPaintDC dc(this); // device context for painting
	
	// get the inner rectangle for drawing the text
	CRect rectClient;
	GetClientRect(&rectClient);
	CBrush brush;
	brush.CreateSolidBrush(BACKGROUND_COLOR);
	dc.FillRect(&rectClient, &brush);

	// get the inner rectangle for drawing the text
	CRect rectInner = GetInnerRect();

	// draw the text
	dc.SetBkMode(TRANSPARENT);

	int nDesiredHeight = rectClient.Height() / 5;
	int nDesiredWidth = rectClient.Width() / 100;

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

	CFont *pOldFont = dc.SelectObject(&font);

	dc.DrawText(forNode->name.Get(), rectInner, 
		DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	dc.SelectObject(pOldFont);

	// stores the old pen that was selected into the context
	CPen *pOldPen;

	// get the guide rectangles
	CRect rectLeftRightEllipse = GetLeftRightEllipseRect();
	CRect rectTopBottomEllipse = GetTopBottomEllipseRect();

	// draw the upper-left half of the outline
	CPen penUpperLeftThick(PS_SOLID, 6, UPPER_DARK_COLOR);
	pOldPen = dc.SelectObject(&penUpperLeftThick);
#ifdef NO_CURVE
	dc.MoveTo(rectInner.left, rectInner.bottom);
	dc.LineTo(rectInner.TopLeft());
	dc.LineTo(rectInner.right, rectInner.top);
#else
	dc.Arc(&rectLeftRightEllipse, rectInner.TopLeft(), 
		CPoint(rectInner.left, rectInner.bottom));
	dc.Arc(&rectTopBottomEllipse, CPoint(rectInner.right, rectInner.top), 
		rectInner.TopLeft());
#endif

	CPen penUpperLeftThin(PS_SOLID, 3, UPPER_LIGHT_COLOR);
	dc.SelectObject(&penUpperLeftThin);
#ifdef NO_CURVE
	dc.MoveTo(rectInner.left, rectInner.bottom);
	dc.LineTo(rectInner.TopLeft());
	dc.LineTo(rectInner.right, rectInner.top);
#else
	dc.Arc(&rectLeftRightEllipse, rectInner.TopLeft(), 
		CPoint(rectInner.left, rectInner.bottom));
	dc.Arc(&rectTopBottomEllipse, CPoint(rectInner.right, rectInner.top), 
		rectInner.TopLeft());
#endif

	// draw the lower-right half of the outline
	CPen penLowerRightThick(PS_SOLID, 6, LOWER_LIGHT_COLOR);
	dc.SelectObject(&penLowerRightThick);
#ifdef NO_CURVE
	dc.MoveTo(rectInner.right, rectInner.top);
	dc.LineTo(rectInner.BottomRight());
	dc.LineTo(rectInner.left, rectInner.bottom);
#else
	dc.Arc(&rectLeftRightEllipse, rectInner.BottomRight(), 
		CPoint(rectInner.right, rectInner.top));
	dc.Arc(&rectTopBottomEllipse, CPoint(rectInner.left, rectInner.bottom), 
		rectInner.BottomRight());
#endif

	CPen penLowerRightThin(PS_SOLID, 3, LOWER_DARK_COLOR);
	dc.SelectObject(&penLowerRightThin);
#ifdef NO_CURVE
	dc.MoveTo(rectInner.right, rectInner.top);
	dc.LineTo(rectInner.BottomRight());
	dc.LineTo(rectInner.left, rectInner.bottom);
#else
	dc.Arc(&rectLeftRightEllipse, rectInner.BottomRight(), 
		CPoint(rectInner.right, rectInner.top));
	dc.Arc(&rectTopBottomEllipse, CPoint(rectInner.left, rectInner.bottom), 
		rectInner.BottomRight());
#endif

	// restore the old pen 
	dc.SelectObject(pOldPen);

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

#define PROPAGATE_ON_DOWN
#ifdef PROPAGATE_ON_DOWN
		// compute the new activation
		float oldActivation = activation.Get();
		float newActivation = ActivationCurve(oldActivation * 1.3f,
			0.2f);
		activation.Set(newActivation);

		CSpaceView *pParent = (CSpaceView *)GetParent();

		// propagate activation
		pParent->PropagateActivation(this, newActivation, 1.3f);
		pParent->ResetForPropagation();

		// now normalize all children
		pParent->NormalizeNodeViews();
#endif
	}

	SetCapture();

	CWnd::OnLButtonDown(nFlags, point);
}

void CNodeView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bDragging = FALSE;

	::ReleaseCapture();

#ifndef PROPAGATE_ON_DOWN
	if (!m_bDragged)
	{
		// compute the new activation
		float oldActivation = activation.Get();
		float newActivation = ActivationCurve(oldActivation * 1.3f,
			0.2f);
		activation.Set(newActivation);

		CSpaceView *pParent = (CSpaceView *)GetParent();

		// propagate activation
		pParent->PropagateActivation(this, newActivation);
		pParent->ResetForPropagation();

		// now normalize all children
		pParent->NormalizeNodeViews();
	}
#endif

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

		privCenter.Set(privCenter.Get() + CVector<2>(ptOffset));
		center.Set(privCenter.Get());

		CSpaceView *pParent = (CSpaceView *)GetParent();

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (isWaveMode.Get())
	{
		CPoint ptOffset = (point - m_ptMouseDown);
		float lengthSq = (float) sqrt(ptOffset.x * ptOffset.x + ptOffset.y * ptOffset.y);

		// compute the new activation
		float factor = 1.0f + lengthSq * 1e-6f;
		float oldActivation = activation.Get();
		float newActivation = ActivationCurve(oldActivation * factor, 0.2f);

		activation.Set(newActivation);

		CSpaceView *pParent = (CSpaceView *)GetParent();

		// propagate activation
		pParent->PropagateActivation(this, newActivation, factor);
		pParent->ResetForPropagation();

		// now normalize all children
		pParent->NormalizeNodeViews();
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CNodeView::UpdatePrivates()
{
	CVector<2> vNewCenter = center.Get() * 0.125 + privCenter.Get() * 0.875;
	privCenter.Set(vNewCenter);

	float newActivation = activation.Get() * 0.25f + privActivation.Get() * 0.75f;
	privActivation.Set(newActivation);
}

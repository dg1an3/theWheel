//////////////////////////////////////////////////////////////////////
// SpaceView.cpp: implementation of the CSpaceView class.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// resource includes
#include "THEWHEEL_VIEW_resource.h"

// the class definition
#include "SpaceView.h"

// include for some constants
#include <float.h>

// the displayed model object
#include <Space.h>

// child node views
#include "NodeView.h"

// the new node dialog
#include "EditNodeDlg.h"

#include <HTMLNode.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// CheckNodeViews
// 
// helper function to check for the existence of node views for 
//		every node
//////////////////////////////////////////////////////////////////////
BOOL CheckNodeViews(CNode *pNode)
{
	CNodeView *pView = (CNodeView *)pNode->GetView();
	BOOL bOK = (pView != NULL); // pView->IsKindOf(RUNTIME_CLASS(CNodeView));
	for (int nAt = 0; nAt < pNode->GetChildCount(); nAt++)
		bOK = bOK && CheckNodeViews(pNode->GetChildAt(nAt));
	return bOK;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CSpaceView::CSpaceView
// 
// constructs a new CSpaceView object 
//////////////////////////////////////////////////////////////////////
CSpaceView::CSpaceView()
: m_pBrowser(NULL),
	m_bWaveMode(TRUE),
	m_lpDD(NULL),			// DirectDraw object
	m_lpDDSPrimary(NULL),	// DirectDraw primary surface
	m_lpDDSOne(NULL),		// Offscreen surface 1
	m_lpDDSBitmap(NULL),	// Offscreen surface 1
	m_lpClipper(NULL)		// clipper for primary
{
	// initialize the recent click list
	for (int nAt = 0; nAt < 2; nAt++)
		m_pRecentClick[nAt] = NULL;
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::CSpaceView
// 
// constructs a new CSpaceView object 
//////////////////////////////////////////////////////////////////////
CSpaceView::~CSpaceView()
{
	// get rid of the node views
	for (int nAt = 0; nAt < m_arrNodeViews.GetSize(); nAt++)
		delete m_arrNodeViews[nAt];
	m_arrNodeViews.RemoveAll();
}

//////////////////////////////////////////////////////////////////////
// implements the dynamic creation mechanism for the CSpaceView
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CSpaceView, CView)


//////////////////////////////////////////////////////////////////////
// CSpaceView::GetNodeViewCount
// 
// returns the number of node views
//////////////////////////////////////////////////////////////////////
int CSpaceView::GetNodeViewCount()
{
	return m_arrNodeViews.GetSize();
}


//////////////////////////////////////////////////////////////////////
// CSpaceView::GetNodeView
// 
// returns the node view at the given index position
//////////////////////////////////////////////////////////////////////
CNodeView *CSpaceView::GetNodeView(int nAt)
{
	return (CNodeView *) m_arrNodeViews.GetAt(nAt);
}


//////////////////////////////////////////////////////////////////////
// CSpaceView::GetVisibleNodeCount
// 
// returns the number of visible nodes
//////////////////////////////////////////////////////////////////////
int CSpaceView::GetVisibleNodeCount()
{
	int nNumVizNodeViews = __min(m_arrNodeViews.GetSize(), 
		STATE_DIM / 2 - GetDocument()->GetClusterCount());

	return nNumVizNodeViews;
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::ActivateNodeView
// 
// activates and propagates on a particular node view
//////////////////////////////////////////////////////////////////////
void CSpaceView::ActivateNodeView(CNodeView *pNodeView, REAL scale)
{
	// first activate the node
	GetDocument()->ActivateNode(pNodeView->GetNode(), scale);

	// now sort the node views
	SortNodeViews();
}


//////////////////////////////////////////////////////////////////////
// CSpaceView::FindNodeViewAt
// 
// finds the topmost node view containing the point
//////////////////////////////////////////////////////////////////////
CNodeView *CSpaceView::FindNodeViewAt(CPoint pt)
{
	// search throught the node views
	for (int nAt = 0; nAt < GetVisibleNodeCount(); nAt++)
	{
		// get the current node view
		CNodeView *pNodeView = GetNodeView(nAt);

		// see if the mouse-click was within it
		const CRgn& rgnShape = pNodeView->GetShape();
		if (rgnShape.m_hObject && rgnShape.PtInRegion(pt))
		{
			return pNodeView;
		}
	}

	// return NULL for no result
	return (CNodeView *)NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CSpaceView drawing

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnDraw
// 
// draws the individual node views, using a back-buffer
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnDraw(CDC* pDC)
{
	// get the inner rectangle for drawing the text
	CRect rectClient;
	GetClientRect(&rectClient);

	// erase the drawing area
	CBrush brush(RGB(192, 192, 192));
	pDC->FillRect(&rectClient, &brush);

// #define NO_DRAW_SPACEVIEW
#ifdef NO_DRAW_SPACEVIEW
	return; 
#endif

	static BOOL bFirst = TRUE;

	if (!bFirst)
	{
		// draw the node views
		int nAtNodeView;
		for (nAtNodeView = 0; nAtNodeView < GetNodeViewCount(); nAtNodeView++)
		{
			// get the current node view
			CNodeView *pNodeView = GetNodeView(nAtNodeView);

			// draw the node view
			pNodeView->DrawLinks(pDC);
		} 
	}
	bFirst = FALSE;

/*	// draw the node views
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < GetNodeViewCount(); nAtNodeView++)
	{
		// get the current node view
		CNodeView *pNodeView = GetNodeView(nAtNodeView);

		// draw the node view
		pNodeView->Draw(pDC, &m_skin);
	} 
*/
	CObArray arrNodeViewsToDraw;
	arrNodeViewsToDraw.Copy(m_arrNodeViews);

	while (arrNodeViewsToDraw.GetSize() > 0)
	{
		REAL min_diff = FLT_MAX;
		int nMinIndex = 0;
		CNodeView *pMinNodeView;

		// draw the node views
		int nAtNodeView;
		for (nAtNodeView = 0; nAtNodeView < arrNodeViewsToDraw.GetSize(); nAtNodeView++)
		{
			// get the current node view
			CNodeView *pNodeView = (CNodeView *)arrNodeViewsToDraw[nAtNodeView];

			REAL diff = pNodeView->GetSpringActivation() * pNodeView->GetSpringActivation() * 
				(pNodeView->GetThresholdedActivation() - pNodeView->GetSpringActivation());

			if (diff < min_diff)
			{
				min_diff = diff;
				nMinIndex = nAtNodeView;
				pMinNodeView = pNodeView;
			}
		} 

		arrNodeViewsToDraw.RemoveAt(nMinIndex);

		// draw the node view
		pMinNodeView->Draw(pDC, &m_skin);
	}

#ifdef USE_EIGENVECTOR_DOT
	// draw the central moment and principle eigenvector
	CVector<3> vCenter = GetDocument()->GetCentralMoment();
	CVector<2> vPrinEigenvector;
	REAL prinEigenvalue = Eigen(GetDocument()->GetInertiaTensor(), 1, &vPrinEigenvector);

	// draw the node view
	pDC->Ellipse((int) vCenter[0] - 10, (int) vCenter[1] - 10, 
		(int) vCenter[0] + 10, (int) vCenter[1] + 10);
	pDC->MoveTo(vCenter[0], vCenter[1]);
	pDC->LineTo(vCenter[0] + vPrinEigenvector[0] * prinEigenvalue, 
		vCenter[1] + vPrinEigenvector[1] * prinEigenvalue);
#endif

#ifdef USE_CLUSTER_DOT
	// draw the clusters
	int nAtCluster;
	for (nAtCluster = 0; nAtCluster < GetDocument()->GetClusterCount(); nAtCluster++)
	{
		// get the current node view
		CVector<3> vCenter = GetDocument()->GetClusterAt(nAtCluster)->GetPosition();

		// draw the node view
		pDC->Ellipse((int) vCenter[0] - 10, (int) vCenter[1] - 10, 
			(int) vCenter[0] + 10, (int) vCenter[1] + 10);
	}
#endif
}


//////////////////////////////////////////////////////////////////////
// CSpaceView::OnInitialUpdate
// 
// called when a CSpace is first loaded
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

#if defined(_DEBUG)
	GetDocument()->Dump(afxDump);
#endif

	// make sure super node count + clusters = STATE_DIM
	GetDocument()->SetMaxSuperNodeCount(
		STATE_DIM / 2 - GetDocument()->GetClusterCount());

	// get rid of the node views
	for (int nAt = 0; nAt < m_arrNodeViews.GetSize(); nAt++)
		delete m_arrNodeViews[nAt];
	m_arrNodeViews.RemoveAll();

	// initialize the recent click list
	for (nAt = 0; nAt < 2; nAt++)
		m_pRecentClick[nAt] = NULL;

	// create the child node views
	CRect rect;
	GetClientRect(&rect);
	CreateNodeViews(GetDocument()->GetRootNode(), rect.CenterPoint());

	// check to make sure all nodes have views
	ASSERT(CheckNodeViews(GetDocument()->GetRootNode()));

	// now snap the node views to the parameter values
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < GetNodeViewCount(); nAtNodeView++)
	{
		CNodeView *pNodeView = GetNodeView(nAtNodeView);
		pNodeView->UpdateSprings(0.0);
	}
}


//////////////////////////////////////////////////////////////////////
// CSpaceView::OnUpdate
// 
// called when a CSpace has changed; if a new node has been created,
//		pass it as the hint object
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (pHint != NULL)
	{
		// cast the hint object to a CNode
		CNode *pNewNode = (CNode *)pHint;
		// ASSERT(pNewNode->IsKindOf(RUNTIME_CLASS(CNode)));

		if (pNewNode->GetView() == NULL)
		{
			// construct a new node view for this node, and add to the array
			CNodeView *pNewNodeView = new CNodeView(pNewNode, this);
			m_arrNodeViews.Add(pNewNodeView);

			// activate the node to propagate the activation
			ActivateNodeView(pNewNodeView, (float) 1.4);

			// and lay them out and center them
			GetDocument()->LayoutNodes();
			CenterNodeViews();
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CSpaceView printing


//////////////////////////////////////////////////////////////////////
// CSpaceView::OnPreparePrinting
// 
// prepares for printing the CSpaceView
//////////////////////////////////////////////////////////////////////
BOOL CSpaceView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}


//////////////////////////////////////////////////////////////////////
// CSpaceView::OnBeginPrinting
// 
// begins printing the CSpaceView
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}


//////////////////////////////////////////////////////////////////////
// CSpaceView::OnEndPrinting
// 
// finishes printing the CSpaceView
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


/////////////////////////////////////////////////////////////////////////////
// CSpaceView diagnostics

#ifdef _DEBUG
void CSpaceView::AssertValid() const
{
	CView::AssertValid();
}

void CSpaceView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSpace* CSpaceView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSpace)));
	return (CSpace*)m_pDocument;
}
#endif //_DEBUG


//////////////////////////////////////////////////////////////////////
// CSpaceView::CreateNodeViews
// 
// creates the node view for the parent, as well as all child node 
//		views
//////////////////////////////////////////////////////////////////////
void CSpaceView::CreateNodeViews(CNode *pParentNode, CPoint pt)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	// construct a new node view for this node
	CNodeView *pNewNodeView = new CNodeView(pParentNode, this);
	pNewNodeView->GetNode()->SetPosition(CVector<3>(rectClient.CenterPoint()));
	pParentNode->SetView(pNewNodeView);

	// and add to the array
	m_arrNodeViews.Add(pNewNodeView);

	// and finally, create the child node views
	int nAtNode;
	for (nAtNode = 0; nAtNode < pParentNode->GetChildCount(); nAtNode++)
	{
		// construct a new node view for this node
		CreateNodeViews(pParentNode->GetChildAt(nAtNode), pt);
	}

	// activate this node view after creating children (to maximize size)
	ActivateNodeView(pNewNodeView, 0.5);
}


//////////////////////////////////////////////////////////////////////
// CSpaceView::SortNodeViews
// 
// sorts the children node views
//////////////////////////////////////////////////////////////////////
void CSpaceView::SortNodeViews()
{
	BOOL bRearrange;
	do 
	{
		bRearrange = FALSE;
		for (int nAt = 0; nAt < GetNodeViewCount()-1; nAt++)
		{
			CNodeView *pThisNodeView = GetNodeView(nAt);
			CNodeView *pNextNodeView = GetNodeView(nAt+1);

			if (pNextNodeView->GetNode()->GetActivation() >
				pThisNodeView->GetNode()->GetActivation())
			{
				m_arrNodeViews.SetAt(nAt, pNextNodeView);
				m_arrNodeViews.SetAt(nAt+1, pThisNodeView);

				bRearrange = TRUE;
			}
		}
	} while (bRearrange);
}


/////////////////////////////////////////////////////////////////////////////
// CSpaceView::CenterNodeViews
//
// Centering algorithm 
//		Compute vector mean for all node view centers 
//		Offset = mean of centers - center of window 
//		For all node views, SetWindowCenter to the offset + GetWindowCenter 
//
/////////////////////////////////////////////////////////////////////////////
void CSpaceView::CenterNodeViews()
{
	// only center if there are children
	if (GetNodeViewCount() == 0)
		return;
	
	CVector<3> vMeanCenter;

	// compute the vector sum of all node views' centers
	int nAt = 0;
	REAL totalScaleFactor = 0.0f;

	for (nAt = 0; nAt < GetVisibleNodeCount(); nAt++)
	{
		CNodeView *pView = GetNodeView(nAt);
		REAL scaleFactor = 10.0f * // sqrt
			(pView->GetNode()->GetPrimaryActivation());

		if (pView == m_pRecentClick[0])
			scaleFactor *= 2.0;
		else if (pView == m_pRecentClick[1])
			scaleFactor *= 1.5; 
		vMeanCenter = vMeanCenter 
			+ pView->GetNode()->GetPosition() * scaleFactor; 
		totalScaleFactor += scaleFactor;
	}

	// divide by number of node views
	vMeanCenter *= 1.0f / totalScaleFactor;

	// compute the vector offset for the node views
	//		window center
	CRect rectWnd;
	GetClientRect(&rectWnd);
	CVector<3> vOffset = vMeanCenter 
		- CVector<3>(rectWnd.CenterPoint()); // rectWnd.Width() / 2, rectWnd.Height() / 2);

	// offset each node view by the difference between the mean and the 
	//		window center
	for (nAt = 0; nAt < GetNodeViewCount(); nAt++)
	{
		CNodeView *pView = GetNodeView(nAt);
		pView->GetNode()->SetPosition(pView->GetNode()->GetPosition() - vOffset);
	}
}


//////////////////////////////////////////////////////////////////////
// CSpaceView::InitDDraw
// 
// activates and propagates on a particular node view
//////////////////////////////////////////////////////////////////////
BOOL CSpaceView::InitDDraw()
{
    DDSURFACEDESC	ddsd;
    HRESULT		ddrval;

    // create the main DirectDraw object
    ddrval = DirectDrawCreate( NULL, &m_lpDD, NULL );
    if( ddrval != DD_OK )
    {
		return FALSE;
    }
    ddrval = m_lpDD->SetCooperativeLevel( m_hWnd, DDSCL_NORMAL );

    // Create the primary surface
    ddsd.dwSize = sizeof( ddsd );
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    ddrval = m_lpDD->CreateSurface( &ddsd, &m_lpDDSPrimary, NULL );
    if( ddrval != DD_OK )
    {
		return FALSE;
    }

    // create a clipper for the primary surface
    ddrval = m_lpDD->CreateClipper( 0, &m_lpClipper, NULL );
    if( ddrval != DD_OK )
    {
		return FALSE;
    }
    
    ddrval = m_lpClipper->SetHWnd( 0, m_hWnd);
    if( ddrval != DD_OK )
    {
		return FALSE;
    }

    ddrval = m_lpDDSPrimary->SetClipper( m_lpClipper );
    if( ddrval != DD_OK )
    {
		return FALSE;
    }

#ifdef USE_SKINS
	lpDDSBitmap = DDLoadBitmap(lpDD, MAKEINTRESOURCE(IDB_NODEBUBBLE), 0, 0);
	if (lpDDSBitmap == NULL)
	{
		return FALSE;
	}

	ddrval = DDSetColorKey(lpDDSBitmap, RGB(0, 0, 0));
    if( ddrval != DD_OK )
    {
		return FALSE;
    }
#endif

    return TRUE;
}


//////////////////////////////////////////////////////////////////////
// CSpaceView::WaveActivate
// 
// processes a mouse movement to activate node views
//////////////////////////////////////////////////////////////////////
void CSpaceView::WaveActivate(CPoint pt)
{
	// determine the closest node to the mouse point, and the distance
	float minDist = FLT_MAX;
	CNodeView *pClosestNodeView = NULL;
	for (int nAt = 0; nAt < GetVisibleNodeCount(); nAt++)
	{
		// compute the distance from the cursor position
		//		to the center of this node view
		CVector<3> vOffset = 
			GetNodeView(nAt)->GetNode()->GetPosition() - CVector<3>(pt);
		float dist = (float) vOffset.GetLength();

		// see if this is closer than any so far
		if (minDist > dist)
		{
			minDist = dist;
			pClosestNodeView = GetNodeView(nAt);
		}
	}

	// if a closest node view was found
	if (pClosestNodeView != NULL)
	{
		// compute "diameter" of closest node view
		CRect rectOuter = pClosestNodeView->GetOuterRect();
		float diameter = 
			(float) (rectOuter.Width() + rectOuter.Height()) 
				/ 1.0f;

		// compute the proportion of activation, based on
		//		how close to the center
		float propActivate = (diameter - minDist) / diameter;
		// if (propActivate < 0.5f)
			propActivate *= 0.05f;
		// else
		//	propActivate = 0.075f;

		// now compute the length of the previous mouse move
		// CPoint ptOffset = (m_ptPrev - pt);
		// float length = 
		//	(float) sqrt(ptOffset.x * ptOffset.x + ptOffset.y + ptOffset.y);

		// and adjust the activation level based on the length
		// propActivate *= length / 200.0f;

		// and finally activate
		if (propActivate > 0.0f)
			pClosestNodeView->m_pendingActivation += propActivate;
			// ActivateNodeView(pClosestNodeView, propActivate);
	}

	// store the point for previous
	m_ptPrev = pt;
}


/////////////////////////////////////////////////////////////////////////////
// CSpaceView Message Map
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSpaceView, CView)
	//{{AFX_MSG_MAP(CSpaceView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_TIMER()
	ON_COMMAND(ID_NEW_NODE, OnNewNode)
	ON_COMMAND(ID_VIEW_WAVE, OnViewWave)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WAVE, OnUpdateViewWave)
	ON_COMMAND(ID_VIEW_LAYOUT, OnViewLayout)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpaceView message handlers

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnCreate
// 
// called to create the view
//////////////////////////////////////////////////////////////////////
int CSpaceView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// initialize the direct-draw routines
	InitDDraw();

	// create a timer
 	m_nTimerID = SetTimer(7, 10, NULL);
	ASSERT(m_nTimerID != 0);

	return 0;
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnSize
// 
// on resize, regenerate the direct draw surface
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	if (cx == 0 || cy == 0)
		return;

	if (m_lpDDSOne != NULL)
	{
		// free the surface
		m_lpDDSOne->Release();
	}

	// create other surface
    DDSURFACEDESC	ddsd;
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsd.dwWidth = cx;
    ddsd.dwHeight = cy;

    HRESULT ddrval = m_lpDD->CreateSurface(&ddsd, &m_lpDDSOne, NULL);
	ASSERT(ddrval == DD_OK);

	// tell the skin the new client area
	m_skin.SetClientArea(cx, cy);
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnPaint
// 
// over-ride to draw to the DDraw surface, then blt to the screen
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnPaint() 
{
	if (m_lpDDSOne)
	{
		CRect rcRect;
		GetClientRect(&rcRect);

		CRect destRect;
		GetClientRect( &destRect );

		CPoint pt;
		pt.x = pt.y = 0;
		ClientToScreen( &pt );
		destRect.OffsetRect(pt);
		// OffsetRect(&destRect, pt.x, pt.y);

		// draw some stuff
		HDC hdc;
		HRESULT ddrval = m_lpDDSOne->GetDC(&hdc);
		ASSERT(ddrval == DD_OK);
	
#ifdef LOG_DRAW_TIME
		START_TIMER(TIME_DRAW)
#endif
		OnDraw(CDC::FromHandle(hdc));

#ifdef LOG_DRAW_TIME
		STOP_TIMER(TIME_DRAW, "Draw")
#endif

		ddrval = m_lpDDSOne->ReleaseDC(hdc);
		ASSERT(ddrval == DD_OK);

#ifdef USE_SKINS
		ddrval = lpDDSOne->BltFast(100, 100, lpDDSBitmap, CRect(0, 0, 233, 176), // DDBLTFAST_NOCOLORKEY); 
			DDBLTFAST_SRCCOLORKEY);
		ASSERT(ddrval == DD_OK);
#endif
		// now blit the buffer to the screen
		ddrval = m_lpDDSPrimary->Blt( &destRect, m_lpDDSOne, &rcRect, 0, NULL );

		ASSERT(ddrval == DD_OK);
	}

	// validate the client rectangle
	ValidateRect(NULL);
	
	// Do not call CView::OnPaint() for painting messages
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnMouseMove
// 
// changes the cursor as the user moves the mouse around
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// get the sync object
	if (!m_sync.Lock())
	{
		CView::OnMouseMove(nFlags, point);
		return;
	}

	// find the node view containing the point
	CNodeView *pSelectedView = FindNodeViewAt(point);

	// set the cursor based on whether the point is in a view
	if (pSelectedView != NULL) 
	{
		::SetCursor(::LoadCursor(GetModuleHandle(NULL), 
			MAKEINTRESOURCE(IDC_HANDPOINT)));
	}
	else
	{
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}

	// if we are in wave mode
	if (m_bWaveMode)
	{
// #define USE_PROP_ACT
#ifdef USE_PROP_ACT
		WaveActivate(point);
#else
		// activate the node view
		if (pSelectedView != NULL)
		{
			pSelectedView->m_pendingActivation += 0.075f;
			// ActivateNodeView(pSelectedView, 0.05f);
		}
#endif
		// activate the node view
		if (pSelectedView != NULL)
		{
			// learning rule for nodes w/ act > this
			for (int nAt = 0; nAt < GetVisibleNodeCount(); nAt++)
			{
				pSelectedView->GetNode()->LearnFromNode(
					GetNodeView(nAt)->GetNode());
			}
		}
	}

	// if we are in wave mode, update the recent click list
	if (m_bWaveMode)
	{
		// update the recent click list
		m_pRecentClick[1] = m_pRecentClick[0];
		m_pRecentClick[0] = pSelectedView;
	}

	// standard processing of mouse move
	CView::OnMouseMove(nFlags, point);

	m_sync.Unlock();
}


//////////////////////////////////////////////////////////////////////
// CSpaceView::OnLButtonDown
// 
// activates a node when the user left-clicks on the node view
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// get the sync object
	if (!m_sync.Lock())
	{
		CView::OnLButtonDown(nFlags, point);
		return;
	}

	// find the node view containing the point
	CNodeView *pSelectedView = FindNodeViewAt(point);

	// was a node view found?
	if (pSelectedView != NULL)
	{
		// if so, activate it
		ActivateNodeView(pSelectedView, 0.45f);

		// and then navigate to its URL, if it has one
		if (m_pBrowser && pSelectedView->GetNode()->GetUrl() != "")
		{
			m_pBrowser->Navigate2(pSelectedView->GetNode()->GetUrl());
		}
	}

	// update the recent click list
	m_pRecentClick[1] = m_pRecentClick[0];
	m_pRecentClick[0] = pSelectedView;

	// standard processing of button down
	CView::OnLButtonDown(nFlags, point);

	m_sync.Unlock();
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnLButtonDblClk
// 
// Left button double-click calls up the edit box for the node
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// get the sync object
	if (!m_sync.Lock())
	{
		CView::OnLButtonDblClk(nFlags, point);
		return;
	}

	// find the node view containing the point
	CNodeView *pSelectedView = FindNodeViewAt(point);

	// was a node view found?
	if (pSelectedView != NULL)
	{
		// kill the the timer
		KillTimer(m_nTimerID);

		// do the edit box
		CEditNodeDlg editDlg(this);
		editDlg.m_pNode = pSelectedView->GetNode();

		// now do the modal dialog
		editDlg.DoModal();

		// restart the timer
 		m_nTimerID = SetTimer(7, 10, NULL);
		ASSERT(m_nTimerID != 0);

		// notify all views of the change
		GetDocument()->UpdateAllViews(NULL, 0L, editDlg.m_pNode);
	}
	
	// update the recent click list
	m_pRecentClick[1] = m_pRecentClick[0];
	m_pRecentClick[0] = pSelectedView;

	// standard processing of button double-click
	CView::OnLButtonDblClk(nFlags, point);

	m_sync.Unlock();
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnRButtonDown
// 
// displays the right-click menu on the right-button down
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// get the sync object
	if (!m_sync.Lock())
	{
		CView::OnRButtonDown(nFlags, point);
		return;
	}

	ClientToScreen(&point);
	CMenu *pMenu = new CMenu();
	pMenu->LoadMenu(IDR_SPACE_POPUP);
	pMenu->GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);

	delete pMenu;

	CView::OnRButtonDown(nFlags, point);

	m_sync.Unlock();
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnTimer
// 
// Timer call updates the springs and performs a layout
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnTimer(UINT nIDEvent) 
{
	// get the sync object
	if (!m_sync.Lock())
	{
		CView::OnTimer(nIDEvent);
		return;
	}

	///////////////////////////////////////////////////////////////////
	// Saccade Processing

	// compute the current cursor position
	CPoint point;
	::GetCursorPos(&point);
	ScreenToClient(&point);
	
	// retrieve the client rectangle
	CRect rect;
	GetClientRect(&rect);

	// test if the point is in the client rectangle
	if (rect.PtInRect(point))
	{
		// add the mouse point to the array
		m_arrPoints.Add(point);

		// add the time stamp to the array
		m_arrTimeStamps.Add(::GetTickCount());

		// determine if its a saccade only if enough points have been stored
		int nSize = m_arrPoints.GetSize();
		if (nSize > 2)
		{
			// now compute the adjusted deceleration
			double saccadeFactor = GetSaccadeFactor(nSize-3);
			int nSaccadeFactor = (int) saccadeFactor;

			// and put an ellipse at the point
			CPoint ptAt = (m_arrPoints[nSize-3]);
			GetDC()->Ellipse(ptAt.x - nSaccadeFactor, ptAt.y - nSaccadeFactor, 
				ptAt.x + nSaccadeFactor, ptAt.y + nSaccadeFactor);	

			// find the node view containing the point
			CNodeView *pSelectedView = FindNodeViewAt(ptAt);
			if (NULL != pSelectedView)
			{
				pSelectedView->m_pendingActivation += 0.0; // saccadeFactor;
			}
		}
	}
	else
	{
		// not in window, so clear points
		m_arrPoints.RemoveAll();
		m_arrTimeStamps.RemoveAll();
	}

	//////////////////////////////////////////////////////////////////////
	// Layout

#ifdef LOG_LAYOUT_TIME
	START_TIMER(TIME_LAYOUT)
#endif

	// perform any pending activations
	for (int nAt = 0; nAt < this->GetNodeViewCount(); nAt++)
	{
		CNodeView *pNodeView = GetNodeView(nAt);
		if (pNodeView->m_pendingActivation > 0.0f)
		{
			ActivateNodeView(pNodeView, pNodeView->m_pendingActivation);
			pNodeView->m_pendingActivation = 0.0f;
		}
	}

	// layout the nodes and center them
	GetDocument()->LayoutNodes();
	CenterNodeViews();

#ifdef LOG_LAYOUT_TIME
	STOP_TIMER(TIME_LAYOUT, "Layout")
#endif

	// update the privates
	for (nAt = 0; nAt < GetNodeViewCount(); nAt++)
	{
		GetNodeView(nAt)->UpdateSprings();
	}

	// update the clusters
	int nAtCluster;
	for (nAtCluster = 0; nAtCluster < GetDocument()->GetClusterCount(); nAtCluster++)
	{
		GetDocument()->GetClusterAt(nAtCluster)->UpdateSpring();
	}

#define LOG_SORTNODEVIEW_TIME
#ifdef LOG_SORTNODEVIEW_TIME
	START_TIMER(TIME_SORTNODEVIEW)
#endif

	SortNodeViews();

#ifdef LOG_SORTNODEVIEW_TIME
	STOP_TIMER(TIME_SORTNODEVIEW, "Sort Node Views")
#endif

#ifdef LOG_REDRAW_TIME
	START_TIMER(TIME_REDRAW)
#endif

	// redraw the window
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

#ifdef LOG_REDRAW_TIME
	STOP_TIMER(TIME_REDRAW, "Redraw")
#endif

	// standard processing
	CView::OnTimer(nIDEvent);

	// unlock the timer
	m_sync.Unlock();
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnNewNode
// 
// launches the new node dialog box
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnNewNode() 
{
	// kill the the timer for a modal dialog
	KillTimer(m_nTimerID);

	// do the edit box
	CEditNodeDlg newDlg(this);
	newDlg.m_pNode = new CNode();
	if (newDlg.DoModal() == IDOK)
	{
		// sort the node views, so that the max node view is first
		SortNodeViews();
		CNode *pParentNode = GetNodeView(0)->GetNode();

		// add the node to the space, with the given parent
		GetDocument()->AddNode(newDlg.m_pNode, pParentNode);
	}
	else
	{
		delete newDlg.m_pNode;
	}

	// restart the timer
 	m_nTimerID = SetTimer(7, 10, NULL);
	ASSERT(m_nTimerID != 0);
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnViewWave
// 
// changes the wave mode view state
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnViewWave() 
{
	m_bWaveMode = !m_bWaveMode;	
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnUpdateViewWave
// 
// sets/unsets the wave mode toggle button
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnUpdateViewWave(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bWaveMode ? 1 : 0);		
}

void CSpaceView::OnViewLayout() 
{
// 	GetDocument()->ComputeClusters();	

	CHTMLNode *pTestNode = new CHTMLNode();
	pTestNode->SetUrl("http://www.microsoft.com");

	// init sets up the connection point
	HRESULT hr;
	
	if (SUCCEEDED(hr = pTestNode->Init()))
	{
		if (SUCCEEDED(hr = pTestNode->Run()))
		{
			// hr = testNode.Term();
		}
	}
}

CVector<2> CSpaceView::GetVelocity(int nIndex)
{
	int nSize = m_arrPoints.GetSize();
	if (nIndex > 1 && nIndex < nSize-1)
	{
		CVector<2> vm1(m_arrPoints[nIndex-1]);
		CVector<2> vp1(m_arrPoints[nIndex+1]);

		return 0.5 * (vp1 - vm1);
	}

	return CVector<2>(0, 0);
}

CVector<2> CSpaceView::GetAvgVelocity(int nIndex)
{
	return 0.25 * GetVelocity(nIndex+1) 
		+  0.5  * GetVelocity(nIndex) 
		+  0.25 * GetVelocity(nIndex-1);
}

CVector<2> CSpaceView::GetAccel(int nIndex)
{
	int nSize = m_arrPoints.GetSize();
	if (nIndex > 1 && nIndex < nSize-1)
	{
		CVector<2> vm1(m_arrPoints[nIndex-1]);
		CVector<2> v0(m_arrPoints[nIndex]);
		CVector<2> vp1(m_arrPoints[nIndex+1]);

		return (vm1 + vp1) - 2.0 * v0;
	}

	return CVector<2>(0, 0);
}

CVector<2> CSpaceView::GetAvgAccel(int nIndex)
{
	return 0.25 * GetAccel(nIndex+1) 
		+  0.5  * GetAccel(nIndex) 
		+  0.25 * GetAccel(nIndex-1);
}

double CSpaceView::GetSaccadeFactor(int nIndex)
{
	// compute the velocity
	CVector<2> vVelocity = GetAvgVelocity(nIndex);

	// compute the velocity scale factor
	double velocityScale = 500.0 
		/ (vVelocity.GetLength() * vVelocity.GetLength() + 0.002);

	// compute the acceleration
	CVector<2> vAccel = GetAvgAccel(nIndex);

	// compute the signed acceleration
	double signedAccel = velocityScale * vVelocity * vAccel;

	// pass through the inverse sigmoidal function
	const double A = log(3) / 1000.0;		// determines the half-point of the sigmoidal
	double saccadeFactor = (exp(-A * signedAccel) - exp(A * signedAccel)) 
		/ (exp(-A * signedAccel) + exp(A * signedAccel));

	// now compute the adjusted deceleration
	return (saccadeFactor > 0.0) ? saccadeFactor : 0.0;
}

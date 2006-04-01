#include "include\SpaceView.h"
//////////////////////////////////////////////////////////////////////
// SpaceView.cpp: implementation of the CSpaceView class.
//
// Copyright (C) 1996-2002
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
	CNodeView *pView = (CNodeView *) pNode->GetView();
	// ASSERT(pView->IsKindOf(RUNTIME_CLASS(CNodeView)));

	BOOL bOK = (pView != NULL); 
	for (int nAt = 0; nAt < pNode->GetChildCount(); nAt++)
	{
		bOK = bOK && CheckNodeViews(pNode->GetChildAt(nAt));
	}

	return bOK;

}	// CheckNodeViews


//////////////////////////////////////////////////////////////////////
// IntersectLineSegments
// 
// helper function to find the intersection of two line segments
//////////////////////////////////////////////////////////////////////
BOOL IntersectLineSegments(CVectorD<2> vP1, CVectorD<2> vO1, 
						   CVectorD<2> vP2, CVectorD<2> vO2)
{
	// form the normal of the second line segment
	CVectorD<2> vN2(-vO2[1], vO2[0]);
	vN2.Normalize();

	// compute lambda -- distance from the second point to the first
	//		along the second's direction
	REAL lambda = (vP2 - vP1) * vN2 / (vO1 * vN2);

	// intersection if lambda is between 0.0 and 1.0
	return (lambda >= 0.0 && lambda <= 1.0);

}	// IntersectLineSegments


//////////////////////////////////////////////////////////////////////
// CompareNodeViewActivation
// 
// comparison function for two CNodeView pointers, based on 
//		the activation value of both
//////////////////////////////////////////////////////////////////////
int __cdecl CompareNodeViewActivation(const void *elem1, const void *elem2 )
{
	// extract pointers to the node views
	CNodeView *pNodeView1 = *(CNodeView **) elem1;
	CNodeView *pNodeView2 = *(CNodeView **) elem2;

	// return difference in activation; > 0 if act2 > act1
	return Round<int>(1e+5 * (pNodeView2->GetNode()->GetActivation() 
		- pNodeView1->GetNode()->GetActivation()));

}	// CompareNodeViewActivation


//////////////////////////////////////////////////////////////////////
// CompareNodeViewActDiff
// 
// comparison function for two CNodeView pointers, based on 
//		the activation factor value of both
//////////////////////////////////////////////////////////////////////
int __cdecl CompareNodeViewActDiff(const void *elem1, const void *elem2 )
{
	// extract pointers to the node views
	CNodeView *pNodeView1 = *(CNodeView **) elem1;
	CNodeView *pNodeView2 = *(CNodeView **) elem2;

	// factor for assessing the drawing order
	REAL factor1 = exp(pNodeView1->GetNode()->GetPrimaryActivation())
			* (pNodeView1->GetThresholdedActivation() 
				- pNodeView1->GetSpringActivation());

	REAL factor2 = exp(pNodeView2->GetNode()->GetPrimaryActivation())
			* (pNodeView2->GetThresholdedActivation() 
				- pNodeView2->GetSpringActivation());

	// return difference in factors; > 0 if factor1 > factor2
	return Round<int>(1e+5 * (factor1 - factor2));

}	// CompareNodeViewActDiff


//////////////////////////////////////////////////////////////////////
// constants for the window's timer
//////////////////////////////////////////////////////////////////////
const int TIMER_ELAPSED = 20;	// ms per tick
const int TIMER_ID = 7;			// luck 7

// initialize to the main module state, or to static module state
AFX_MODULE_STATE *CSpaceView::m_pModuleState = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CSpaceView::CSpaceView
// 
// constructs a new CSpaceView object 
//////////////////////////////////////////////////////////////////////
CSpaceView::CSpaceView()
	: m_lpDD(NULL)				// DirectDraw object
		, m_lpDDSPrimary(NULL)	// DirectDraw primary surface
		, m_lpDDSOne(NULL)		// Offscreen surface 1
		, m_lpClipper(NULL)		// clipper for primary
		, m_pTracker(NULL)
		, m_pMaximizedView(NULL)
		, m_bDragging(FALSE)
		, m_pNLM(NULL)
		, m_pSpace(NULL)
{
	DWORD dwBkColor = ::AfxGetApp()->GetProfileInt("Settings", "BkColor", 
		(DWORD) RGB(115, 158, 206));

	// set default background color
	SetBkColor((COLORREF) dwBkColor);

	m_pRecentActivated[0] = NULL;
	m_pRecentActivated[1] = NULL;

}	// CSpaceView::CSpaceView


//////////////////////////////////////////////////////////////////////
// CSpaceView::~CSpaceView
// 
// destroys the CSpaceView object 
//////////////////////////////////////////////////////////////////////
CSpaceView::~CSpaceView()
{
	// get rid of the node views
	for (int nAt = 0; nAt < m_arrNodeViews.GetSize(); nAt++)
	{
		delete m_arrNodeViews[nAt];
	}
	m_arrNodeViews.RemoveAll();

	// delete the DirectDraw surfaces
	if (m_lpDDSOne != NULL)
	{
		ASSERT_HRESULT(m_lpDDSOne->Release());
	}

	if (m_lpClipper != NULL)
	{
		ASSERT_HRESULT(m_lpClipper->Release());
	}

	if (m_lpDDSPrimary != NULL)
	{
		ASSERT_HRESULT(m_lpDDSPrimary->Release());
	}

	if (m_lpDD != NULL)
	{
		ASSERT_HRESULT(m_lpDD->Release());
	}

	// delete the tracker
	delete m_pTracker;

}	// CSpaceView::~CSpaceView


//////////////////////////////////////////////////////////////////////
// implements the dynamic creation mechanism for the CSpaceView
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CSpaceView, CWnd)


//////////////////////////////////////////////////////////////////////
// CSpaceView::GetNodeViewCount
// 
// returns the number of node views
//////////////////////////////////////////////////////////////////////
int CSpaceView::GetNodeViewCount()
{
	return m_arrNodeViews.GetSize();

}	// CSpaceView::GetNodeViewCount


//////////////////////////////////////////////////////////////////////
// CSpaceView::GetNodeView
// 
// returns the node view at the given index position
//////////////////////////////////////////////////////////////////////
CNodeView *CSpaceView::GetNodeView(int nAt)
{
	return (CNodeView *) m_arrNodeViews.GetAt(nAt);

}	// CSpaceView::GetNodeView


//////////////////////////////////////////////////////////////////////
// CSpaceView::GetVisibleNodeCount
// 
// returns the number of visible nodes
//////////////////////////////////////////////////////////////////////
int CSpaceView::GetVisibleNodeCount()
{
	int nNumVizNodeViews = __min(m_arrNodeViews.GetSize(), 
		GetSpace()->GetSuperNodeCount()); 

	return nNumVizNodeViews;

}	// CSpaceView::GetVisibleNodeCount


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
		const CRgn& rgnShape = pNodeView->m_shapeHit;
		if (rgnShape.m_hObject && rgnShape.PtInRegion(pt))
		{
			return pNodeView;
		}
	}

	// return NULL for no result
	return (CNodeView *)NULL;

}	// CSpaceView::FindNodeViewAt


//////////////////////////////////////////////////////////////////////
// CSpaceView::FindNearestNodeView
// 
// finds the CNodeView nearest to the point
//////////////////////////////////////////////////////////////////////
CNodeView *CSpaceView::FindNearestNodeView(CPoint pt)
{
	CNodeView *pNearest = NULL;
	REAL minDistSq = FLT_MAX;

	// search throught the node views
	for (int nAt = 0; nAt < GetVisibleNodeCount() / 2; nAt++)
	{
		// get the current node view
		CNodeView *pNodeView = GetNodeView(nAt);
		// CRect rect = pNodeView->GetInnerRect();
		CSize sz = // rect.CenterPoint()
			(CPoint) (pNodeView->GetSpringCenter()) - pt;
		REAL distSq = sz.cx * sz.cx + sz.cy * sz.cy;
		if (distSq < minDistSq)
		{
			pNearest = pNodeView;
			minDistSq = distSq;
		}
	}

	// return NULL for no result
	return pNearest;

}	// CSpaceView::FindNearestNodeView


//////////////////////////////////////////////////////////////////////
// CSpaceView::FindLink
// 
// Finds the largest link intersecting the given line segment
//////////////////////////////////////////////////////////////////////
BOOL CSpaceView::FindLink(CPoint ptFrom, CPoint ptTo, 
						  CNodeView **pLinkingView, CNodeView **pLinkedView)
{
	CVectorD<2> vFrom(ptFrom);
	CVectorD<2> vOffset = CVectorD<2>(ptTo) - vFrom;

	for (int nAtNodeView = 0; nAtNodeView < GetNodeViewCount(); nAtNodeView++)
	{
		CVectorD<2> vNodeCenter(GetNodeView(nAtNodeView)->GetSpringCenter()); // GetInnerRect().CenterPoint());
		for (int nAtLinkedView = nAtNodeView+1; nAtLinkedView < GetNodeViewCount(); nAtLinkedView++)
		{
			CVectorD<2> vLinkedFrom(GetNodeView(nAtLinkedView)->GetSpringCenter()); // GetInnerRect().CenterPoint());
			CVectorD<2> vLinkedOffset = vLinkedFrom - vNodeCenter;

			BOOL bIntersect = IntersectLineSegments(vFrom, vOffset, vLinkedFrom, vLinkedOffset)
				|| IntersectLineSegments(vLinkedFrom, vLinkedOffset, vFrom, vOffset);
			if (bIntersect)
			{
				// found the link
				(*pLinkingView) = GetNodeView(nAtNodeView);
				(*pLinkedView) = GetNodeView(nAtLinkedView);

				// return
				return TRUE;
			}
		}
	}

	// didn't find the link
	(*pLinkingView) = NULL;
	(*pLinkedView) = NULL;

	return FALSE;

}	// CSpaceView::FindLink


//////////////////////////////////////////////////////////////////////
// CSpaceView::SetTracker
// 
// Sets the tracker for processing mouse events
//////////////////////////////////////////////////////////////////////
void CSpaceView::SetTracker(CTracker *pTracker)
{
	// destroy old tracker, if present
	if (NULL != m_pTracker)
	{
		delete m_pTracker;
	}

	m_pTracker = pTracker;

}	// CSpaceView::SetTracker


//////////////////////////////////////////////////////////////////////
// CSpaceView::SetBkColor
// 
// Sets the background color for the SpaceView
//////////////////////////////////////////////////////////////////////
void CSpaceView::SetBkColor(COLORREF color)
{
	// store background color
	m_colorBk = color;

	// get the client rectangle, if available
	CRect rect(0, 0, 0, 0);
	if (m_hWnd)
	{
		GetClientRect(&rect);
	}

	// tell skin about background color
	m_skin.SetClientArea(rect.Width(), rect.Height(), m_colorBk);

}	// CSpaceView::SetBkColor


//////////////////////////////////////////////////////////////////////
// CSpaceView::ActivateNodeView
// 
// activates and propagates on a particular node view
//////////////////////////////////////////////////////////////////////
void CSpaceView::ActivateNodeView(CNodeView *pNodeView, REAL scale)
{
	// first activate the node
	GetSpace()->ActivateNode(pNodeView->GetNode(), scale);

	// now sort the node views
	SortNodeViews();

}	// CSpaceView::ActivateNodeView


//////////////////////////////////////////////////////////////////////
// CSpaceView::InitDDraw
// 
// activates and propagates on a particular node view
//////////////////////////////////////////////////////////////////////
BOOL CSpaceView::InitDDraw()
{
    // create the main DirectDraw object
    CHECK_HRESULT(DirectDrawCreate( NULL, &m_lpDD, NULL ));
    CHECK_HRESULT(m_lpDD->SetCooperativeLevel( m_hWnd, DDSCL_NORMAL ));

	// check display mode
    DDSURFACEDESC	ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof( ddsd );
	CHECK_HRESULT(m_lpDD->GetDisplayMode(&ddsd));

	// ensure true-color 32-bit mode
	if ((ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB) == 0
		|| ddsd.ddpfPixelFormat.dwRGBBitCount != 32)
	{
		::AfxMessageBox(IDS_ERR_TRUECOLOR32, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

    // create the primary surface
	ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof( ddsd );
	ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
    CHECK_HRESULT(m_lpDD->CreateSurface( &ddsd, &m_lpDDSPrimary, NULL ));

    // create a clipper for the primary surface
    CHECK_HRESULT(m_lpDD->CreateClipper( 0, &m_lpClipper, NULL ));
    CHECK_HRESULT(m_lpClipper->SetHWnd(0, m_hWnd));
    CHECK_HRESULT(m_lpDDSPrimary->SetClipper(m_lpClipper));

    return TRUE;

}	// CSpaceView::InitDDraw


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
	pParentNode->SetView(pNewNodeView);
	m_skin.CalcInnerOuterRect(pNewNodeView);

	// trigger loading of the DIB
	pParentNode->GetDib();

	// and add to the array
	m_arrNodeViews.Add(pNewNodeView);

	// and finally, create the child node views
	int nAtNode;
	for (nAtNode = 0; nAtNode < pParentNode->GetChildCount(); nAtNode++)
	{
		// construct a new node view for this node
		CreateNodeViews(pParentNode->GetChildAt(nAtNode), pt);
	}

	// sort the new node views
	SortNodeViews();

}	// CSpaceView::CreateNodeViews


/////////////////////////////////////////////////////////////////////////////
// CSpaceView::SortNodeViews
// 
// sorts the children node views by activation
/////////////////////////////////////////////////////////////////////////////
void CSpaceView::SortNodeViews()
{
	qsort(m_arrNodeViews.GetData(), GetNodeViewCount(), sizeof(CNodeView*), 
		CompareNodeViewActivation);

}	// CSpaceView::SortNodeViews


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
	
	CVectorD<3> vMeanCenter;

	// compute the vector sum of all node views' centers
	int nAt = 0;
	REAL totalScaleFactor = 0.0f;

	for (nAt = 0; nAt < GetVisibleNodeCount(); nAt++)
	{
		// get the node view
		CNodeView *pView = GetNodeView(nAt);

		// scale by the activation of the node view
		REAL scaleFactor = pView->GetNode()->GetPrimaryActivation();

		// weight recently activated node views more than others
		if (pView == m_pRecentActivated[0])
		{
			scaleFactor *= 6.0;
		}
		else if (pView == m_pRecentActivated[1])
		{
			scaleFactor *= 3.0; 
		}

		// add to the mean center
		vMeanCenter = vMeanCenter 
			+ pView->GetNode()->GetPosition() * scaleFactor; 

		// keep up with the total scale factor
		totalScaleFactor += scaleFactor;
	}

	// divide by number of node views
	vMeanCenter *= 1.0f / totalScaleFactor;

	// compute the vector offset for the node views
	//		window center
	CRect rectWnd;
	GetClientRect(&rectWnd);
	CVectorD<3> vOffset = vMeanCenter 
		- CVectorD<3>(rectWnd.CenterPoint());

	GetSpace()->SetCenter(CVectorD<3>(rectWnd.CenterPoint())); 

	// offset each node view by the difference between the mean and the 
	//		window center
	for (nAt = 0; nAt < GetVisibleNodeCount(); nAt++)
	{
		CNodeView *pView = GetNodeView(nAt);
		pView->GetNode()->SetPosition(pView->GetNode()->GetPosition() - vOffset);
	}

}	// CSpaceView::CenterNodeViews


//////////////////////////////////////////////////////////////////////
// CSpaceView::ActivatePending
// 
// Performs all pending node activations
//////////////////////////////////////////////////////////////////////
void CSpaceView::ActivatePending()
{
	// m_pRecentActivated[1] = NULL;
	// m_pRecentActivated[0] = NULL;
	REAL maxPending = 0.0;
	CNodeView *pMaxPending = NULL;
	REAL secMaxPending = 0.0;
	CNodeView *pSecMaxPending = NULL;

	// perform any pending activations
	for (int nAt = 0; nAt < GetVisibleNodeCount(); nAt++)
	{
		CNodeView *pNodeView = GetNodeView(nAt);

		if (pNodeView->GetPendingActivation() > 1e-6)
		{
			const REAL frac_act = 1.0 / 2.0;
			ActivateNodeView(pNodeView, pNodeView->GetPendingActivation() * frac_act);
			if (pNodeView->GetPendingActivation() > maxPending)
			{
				secMaxPending = maxPending;
				pSecMaxPending = pMaxPending;
				maxPending = pNodeView->GetPendingActivation();
				pMaxPending = pNodeView;
			}
			else if (pNodeView->GetPendingActivation() > secMaxPending)
			{
				secMaxPending = pNodeView->GetPendingActivation();
				pSecMaxPending = pNodeView;
			}
			
			pNodeView->m_pendingActivation *= (1.0 - frac_act);
		}
	}

	if (maxPending > 0.0)
	{
		// update the recent click list
		m_pRecentActivated[1] = pSecMaxPending;
		m_pRecentActivated[0] = pMaxPending;


		// normalize the nodes
		GetSpace()->NormalizeNodes();
	}

}	// CSpaceView::ActivatePending



// called when a new node is added to the space
void CSpaceView::OnNodeAdded(CObservableEvent * pEvent, void * pParam)
{
	CNode *pNode = (CNode *) pParam;
	ASSERT(pNode->IsKindOf(RUNTIME_CLASS(CNode)));

	if (pNode->GetView() == NULL)
	{
		// construct a new node view for this node, and add to the array
		CNodeView *pNewNodeView = new CNodeView(pNode, this);
		m_arrNodeViews.Add(pNewNodeView);

		// activate the node to propagate the activation
		ActivateNodeView(pNewNodeView, (float) 1.4);

		// and lay them out and center them
		GetSpace()->LayoutNodes();
		CenterNodeViews();
	}
}


// called when a new node is added to the space
void CSpaceView::OnNodeRemoved(CObservableEvent * pEvent, void * pParam)
{
	CNode *pNode = (CNode *) pParam;
	ASSERT(pNode->IsKindOf(RUNTIME_CLASS(CNode)));

	// find the node view
	for (int nAt = 0; nAt < m_arrNodeViews.GetSize(); nAt++)
	{
		// deleting a node
		CNodeView *pNodeView = (CNodeView *) pNode->GetView();

		if (m_arrNodeViews[nAt] == pNodeView)
		{
			m_arrNodeViews.RemoveAt(nAt);
		}
	}
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
	// draw the tracker
	if (NULL != m_pTracker)
	{
		m_pTracker->OnDraw(pDC);
	}

	static BOOL bDrawEnergy = FALSE;
	static BOOL bReadFlagFromRegistry = TRUE;
	if (bReadFlagFromRegistry)
	{
		bDrawEnergy = 
			::AfxGetApp()->GetProfileInt("Settings", "DrawEnergy", 0) == 1;

		bReadFlagFromRegistry = FALSE;
	}

	// extract the flag from the registry
	if (bDrawEnergy)
	{
		// draw the energy
		CString strEnergy;
		strEnergy.Format("%lf", 
			// GetSpace()->GetStateVector()->m_rmse);

			GetSpace()->GetLayoutManager()->GetEnergy());

		// get the inner rectangle for drawing the text
		CRect rectClient;
		GetClientRect(&rectClient);

		CRect rect;
		rect.bottom = rectClient.bottom - 5;
		rect.left = rectClient.right - 200;
		rect.right = rectClient.right - 5;
		rect.top = rectClient.bottom - 50;

		pDC->DrawText(strEnergy, rect, DT_CENTER);
	}

}	// CSpaceView::OnDraw


//////////////////////////////////////////////////////////////////////
// CSpaceView::OnInitialUpdate
// 
// called when a CSpace is first loaded
//////////////////////////////////////////////////////////////////////
void CSpaceView::SetSpace(CSpace *pSpace)
{
	// CWnd::OnInitialUpdate();
	m_pSpace = pSpace;

	ASSERT(GetSpace() != NULL);

	// get rid of the node views
	for (int nAt = 0; nAt < m_arrNodeViews.GetSize(); nAt++)
	{
		delete m_arrNodeViews[nAt];
	}
	m_arrNodeViews.RemoveAll();

	// create the child node views
	CRect rect;
	GetClientRect(&rect);
	CreateNodeViews(GetSpace()->GetRootNode(), rect.CenterPoint());

	// check to make sure all nodes have views
	ASSERT(CheckNodeViews(GetSpace()->GetRootNode()));

	// center the node views
	CenterNodeViews();

	// now snap the node views to the parameter values
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < GetNodeViewCount(); nAtNodeView++)
	{
		CNodeView *pNodeView = GetNodeView(nAtNodeView);
		for (int nAtCount = 0; nAtCount < 100; nAtCount++)
		{
			pNodeView->UpdateSpringPosition(0.1);
			pNodeView->UpdateSpringActivation(0.1);
		}
	}

	// initialize the recent click list
	for (int nAt = 0; nAt < 2; nAt++)
	{
		m_pRecentActivated[nAt] = NULL;
	}

	// now add event listeners
	GetSpace()->NodeAddedEvent.AddObserver(this, 
		(ListenerFunction) &CSpaceView::OnNodeAdded);
	GetSpace()->NodeRemovedEvent.AddObserver(this, 
		(ListenerFunction) &CSpaceView::OnNodeRemoved);

}	// CSpaceView::OnInitialUpdate




/////////////////////////////////////////////////////////////////////////////
// CSpaceView diagnostics

#ifdef _DEBUG
void CSpaceView::AssertValid() const
{
	CWnd::AssertValid();
}

void CSpaceView::Dump(CDumpContext& dc) const
{
	CWnd::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CSpaceView Message Map
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSpaceView, CWnd)
	//{{AFX_MSG_MAP(CSpaceView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDB_GO, OnGoClicked)
	//}}AFX_MSG_MAP
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
	// AFX_MANAGE_STATE(m_pModuleState);

	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// initialize the direct-draw routines
	if (!InitDDraw())
		return -1;

	// initialze the direct-draw for the skin
	m_skin.InitDDraw(m_lpDD);

	// initialize CNodeLayoutManager for large layout
	m_pNLM = new CNodeLayoutManager();
	m_pNLM->m_minLayoutSelection = 60.0;
	m_pNLM->m_bImageBesideTitle = true; // false;
	m_pNLM->m_bShowDescription = true;
	m_pNLM->m_maxTitleHeight = 30.0;
	m_pNLM->m_titleLogWeight = 5.0;
	m_pNLM->m_imageLogWeight = 3.0;
	m_pNLM->m_maxImageWidth = 200.0;
	m_pNLM->m_maxDescWidth = 400.0;  // TODO: synch with TEXT_WIDTH in CNodeView

	// set up medium layout
	CNodeLayoutManager *pNLM = m_pNLM->GetNextLayoutManager();
	pNLM->m_minLayoutSelection = 25.0;
	pNLM->m_bImageBesideTitle = true;
	pNLM->m_imageLogWeight = 1.0;

	// set up small layout
	pNLM = pNLM->GetNextLayoutManager();
	pNLM->m_minLayoutSelection = 15.0;
	pNLM->m_imageLogWeight = 0.0;
	// pNLM->m_bShowDescription = false;

	// set up small layout
	pNLM = pNLM->GetNextLayoutManager();
	pNLM->m_maxImageWidth = 0.0;

	// create a timer
 	m_nTimerID = SetTimer(TIMER_ID, TIMER_ELAPSED, NULL);
	ASSERT(m_nTimerID != 0);

	if (!m_dropTarget.Register(this))
		return -1;

#ifdef USE_GO_BUTTON
	if (!m_btnGo.Create("Go", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 
		CRect(-30, -30, 0, 0), this, IDB_GO))
		return -1;

	if (!m_btnGo.LoadBitmaps(IDB_GO_BUTTON, IDB_GO_BUTTON_DOWN))
		return -1;
#endif

	return 0;

}	// CSpaceView::OnCreate


//////////////////////////////////////////////////////////////////////
// CSpaceView::OnSize
// 
// on resize, regenerate the direct draw surface
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnSize(UINT nType, int cx, int cy) 
{
	// AFX_MANAGE_STATE(m_pModuleState);

	CWnd::OnSize(nType, cx, cy);

	// check for zero size
	if (cx == 0 || cy == 0)
	{
		return;
	}

	// release the existing drawing surface
	if (m_lpDDSOne != NULL)
	{
		// free the surface
		m_lpDDSOne->Release();
	}

	// create a new drawing surface
    DDSURFACEDESC	ddsd;
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN 
		| /* DGL: adding for direct 3d rendering */ DDSCAPS_3DDEVICE;
    ddsd.dwWidth = cx;
    ddsd.dwHeight = cy;
    ASSERT_HRESULT(m_lpDD->CreateSurface(&ddsd, &m_lpDDSOne, NULL));

	// tell the skin the new client area
	m_skin.SetClientArea(cx, cy, m_colorBk);

}	// CSpaceView::OnSize


//////////////////////////////////////////////////////////////////////
// CSpaceView::OnPaint
// 
// over-ride to draw to the DDraw surface, then blt to the screen
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnPaint() 
{
	// AFX_MANAGE_STATE(m_pModuleState);

	if (m_lpDDSOne)
	{
		// get the inner rectangle for drawing the text
		CRect rectClient;
		GetClientRect(&rectClient);

		// fill the surface
		DDBLTFX ddBltFx;
		ddBltFx.dwSize = sizeof(DDBLTFX);
		ddBltFx.dwFillColor = (DWORD) RGB(GetBValue(m_colorBk),
			GetGValue(m_colorBk), GetRValue(m_colorBk));
		ASSERT_HRESULT(m_lpDDSOne->Blt(rectClient, NULL,
			rectClient, DDBLT_COLORFILL, &ddBltFx));

		static BOOL bDrawNodes = FALSE;
		static BOOL bReadFlagFromRegistry = TRUE;
		if (bReadFlagFromRegistry)
		{
			bDrawNodes = 
				::AfxGetApp()->GetProfileInt("Settings", "DrawNodes", 1) == 1;

			bReadFlagFromRegistry = FALSE;
		}

		// extract the flag from the registry
		if (bDrawNodes)
		{

			// get a DC for the drawing surface
			CDC dc;
			GET_ATTACH_DC(m_lpDDSOne, dc);

			// draw the node view links
			int nAtNodeView;
			for (nAtNodeView = __min(GetVisibleNodeCount() * 2, m_arrNodeViews.GetSize() - 1); 
					nAtNodeView >= 0; nAtNodeView--)
			{
				// get the current node view
				CNodeView *pNodeView = GetNodeView(nAtNodeView);
				
				if (!pNodeView->GetNode()->GetIsSubThreshold()
					|| pNodeView->GetNode()->GetIsPostSuper())
				{
					// draw the node view
					pNodeView->DrawLinks(&dc, &m_skin);
				}
			}

			// release the DC
			RELEASE_DETACH_DC(m_lpDDSOne, dc);

			// now create an array to hold the drawing-order for the nodeviews
			CObArray arrNodeViewsToDraw;
			arrNodeViewsToDraw.SetSize(__min(GetVisibleNodeCount() * 2, m_arrNodeViews.GetSize()));
			memcpy(arrNodeViewsToDraw.GetData(), m_arrNodeViews.GetData(), 
				arrNodeViewsToDraw.GetSize() * sizeof(CObject *));

			// sort by activation difference comparison
			qsort(arrNodeViewsToDraw.GetData(), arrNodeViewsToDraw.GetSize(), 
				sizeof(CObject *), CompareNodeViewActDiff);

			// see if there is a maximized view
			if (m_pMaximizedView)
			{
				arrNodeViewsToDraw.Add(m_pMaximizedView);
			}

#ifdef SKIN_RENDER_3D
			///////////////////////////////////////////////////////////////
			// 3D rendering
			///////////////////////////////////////////////////////////////

			// create context

			// Direct3D rendering -- initialize the objects first
			LPDIRECT3DDEVICE2 lpD3DDev = NULL;
			ASSERT_BOOL(m_skin.InitD3DDevice(m_lpDDSOne, &lpD3DDev));

			CRect rectClient;
			GetClientRect(rectClient);
			LPDIRECT3DVIEWPORT2	lpViewport = NULL;
			ASSERT_BOOL(m_skin.InitViewport(lpD3DDev, rectClient, &lpViewport));

			// set up the zoom transform, accounting for rectangle
			//		inflation
			D3DMATRIX mat;
			ZeroMemory(&mat, sizeof(D3DMATRIX));
			mat(0, 0) = (D3DVALUE) (1.0 / (rectClient.Width())); //  + 10.0));
			mat(1, 1) = (D3DVALUE) (1.0 / (rectClient.Width())); // + 10.0));
			mat(2, 2) = (D3DVALUE) 1.0; 
			mat(3, 3) = (D3DVALUE) 1.0;
			ASSERT_HRESULT(lpD3DDev->SetTransform(D3DTRANSFORMSTATE_VIEW, &mat));

			LPDIRECT3DLIGHT lpLights[2];
			ASSERT_BOOL(m_skin.InitLights(lpViewport, lpLights));

			// create the material and attach to the device's state
			LPDIRECT3DMATERIAL2	lpMaterial = NULL;
			ASSERT_BOOL(m_skin.InitMaterial(&lpMaterial));

			D3DTEXTUREHANDLE hMat;
			ASSERT_HRESULT(lpMaterial->GetHandle(lpD3DDev, &hMat));
			ASSERT_HRESULT(lpD3DDev->SetLightState(D3DLIGHTSTATE_MATERIAL, hMat));

			// render the skin
			ASSERT_HRESULT(lpD3DDev->BeginScene());

			// render in sorted order
			for (nAtNodeView = 0; nAtNodeView < arrNodeViewsToDraw.GetSize(); 
					nAtNodeView++)
			{
				CNodeView *pNodeView = ((CNodeView *)arrNodeViewsToDraw[nAtNodeView]);
				if (!pNodeView->GetNode()->IsSubThreshold()
					|| pNodeView->GetNode()->IsPostSuper())
				{
					// draw the min_diff node view
					// TODO: fix this
					const int THICK_PEN_WIDTH = 4;
					m_skin.CalcShape(pNodeView, pNodeView->GetShape(), THICK_PEN_WIDTH);

					if (pNodeView->m_extOuter // GetOuterRect()
						.Height() > 1)
					{
						mat(0, 0) = (D3DVALUE) 1.0;
						mat(1, 1) = (D3DVALUE) 1.0;
						mat(2, 2) = (D3DVALUE) 1.0;
						mat(3, 3) = (D3DVALUE) 1.0;

						mat(3, 0) = (D3DVALUE) (pNodeView->GetSpringCenter()[0] * 2.0 - rectClient.Width());
						mat(3, 1) = (D3DVALUE) (-pNodeView->GetSpringCenter()[1] * 2.0 + rectClient.Height());

						ASSERT_HRESULT(lpD3DDev->SetTransform(D3DTRANSFORMSTATE_WORLD, &mat));

						m_skin.DrawSkinD3D(lpD3DDev, pNodeView);
					}

					// pNodeView->Draw3D(m_lpDDSOne);
				}
			} 

			ASSERT_HRESULT(lpD3DDev->EndScene());

			// release the interface
			ASSERT_HRESULT(lpMaterial->Release());
			ASSERT_HRESULT(lpLights[0]->Release());
			ASSERT_HRESULT(lpLights[1]->Release());
			ASSERT_HRESULT(lpViewport->Release());
#endif

			///////////////////////////////////////////////////////////////
			// directdraw rendering
			///////////////////////////////////////////////////////////////

			// draw in sorted order
			for (nAtNodeView = 0; nAtNodeView < arrNodeViewsToDraw.GetSize(); 
					nAtNodeView++)
			{
				CNodeView *pNodeView = ((CNodeView *)arrNodeViewsToDraw[nAtNodeView]);
				if (!pNodeView->GetNode()->GetIsSubThreshold()
					|| pNodeView->GetNode()->GetIsPostSuper())
				{
					// draw the min_diff node view
					pNodeView->Draw(m_lpDDSOne);
				}
			} 

			// get a DC for the drawing surface
			GET_ATTACH_DC(m_lpDDSOne, dc);

			// now draw the space
			OnDraw(&dc);

			// release the DC
			RELEASE_DETACH_DC(m_lpDDSOne, dc);
		}

		// form the destination (screen) rectangle
		CRect rectDest = rectClient;
		CPoint ptOrigin(0, 0);
		ClientToScreen(&ptOrigin);
		rectDest.OffsetRect(ptOrigin);

		// now blit the buffer to the screen
		ASSERT_HRESULT(m_lpDDSPrimary->Blt(&rectDest, m_lpDDSOne, 
			&rectClient, 0, NULL));
	}

	// validate the client rectangle
	ValidateRect(NULL);
	
	// Do not call CWnd::OnPaint() for painting messages

}	// CSpaceView::OnPaint


//////////////////////////////////////////////////////////////////////
// CSpaceView::OnMouseMove
// 
// changes the cursor as the user moves the mouse around
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// AFX_MANAGE_STATE(m_pModuleState);

	if (NULL != m_pTracker)
	{
		if (m_bDragging)
		{
			m_pTracker->OnMouseDrag(nFlags, point);
		}
		else
		{
			m_pTracker->OnMouseMove(nFlags, point);
		}
	}

	// standard processing of mouse move
	CWnd::OnMouseMove(nFlags, point);

}	// CSpaceView::OnMouseMove


//////////////////////////////////////////////////////////////////////
// CSpaceView::OnLButtonDown
// 
// activates a node when the user left-clicks on the node view
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// AFX_MANAGE_STATE(m_pModuleState);

	if (NULL != m_pTracker)
	{
		m_pTracker->OnButtonDown(nFlags, point);
	}

	// set the dragging flag
	m_bDragging = TRUE;

	// and capture mouse events
	SetCapture();

	// standard processing of button down
	CWnd::OnLButtonDown(nFlags, point);

}	// CSpaceView::OnLButtonDown


//////////////////////////////////////////////////////////////////////
// CSpaceView::OnLButtonUp
// 
// activates a node when the user left-clicks on the node view
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// AFX_MANAGE_STATE(m_pModuleState);

	if (NULL != m_pTracker)
	{
		m_pTracker->OnButtonUp(nFlags, point);
	}

	// set the dragging flag
	m_bDragging = FALSE;

	// and release mouse capture
	::ReleaseCapture();

	// standard processing of button down
	CWnd::OnLButtonUp(nFlags, point);

}	// CSpaceView::OnLButtonUp


void CSpaceView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// AFX_MANAGE_STATE(m_pModuleState);

	if (NULL != m_pTracker)
	{
		m_pTracker->OnButtonDblClk(nFlags, point);
	}
	
	CWnd::OnLButtonDblClk(nFlags, point);
}

static bool bLayout = true;

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnTimer
// 
// Timer call updates the springs and performs a layout
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnTimer(UINT nIDEvent) 
{
	// AFX_MANAGE_STATE(m_pModuleState);

    // check the ID to see if it is the CSpaceView timer
    if (nIDEvent != m_nTimerID)
    {
    	CWnd::OnTimer(nIDEvent);
    	return;
    }

	if (NULL == m_pSpace)
	{
		return;
	}

	// send OnTimer message to any tracker
	if (NULL != m_pTracker)
	{
		CPoint point;
		::GetCursorPos(&point);
		ScreenToClient(&point);
		
		m_pTracker->OnTimer(TIMER_ELAPSED, point);
	}

	// perform any pending activation
	ActivatePending();
	
	if (bLayout)
	{
		// layout the nodes and center them
		GetSpace()->LayoutNodes();

		// now center based on the new positions
		CenterNodeViews();
	}
	// bLayout = ~bLayout;

	// update the privates
	for (int nAt = 0; nAt < GetNodeViewCount(); nAt++)
	{
		GetNodeView(nAt)->UpdateSpringPosition(GetSpace()->GetSpringConst());
		GetNodeView(nAt)->UpdateSpringActivation(GetSpace()->GetSpringConst());

		// TODO: fix this
		m_skin.CalcInnerOuterRect(GetNodeView(nAt));
	}

	// redraw the window
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	// standard processing
	CWnd::OnTimer(nIDEvent);

}	// CSpaceView::OnTimer


void CSpaceView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// AFX_MANAGE_STATE(m_pModuleState);

	if (NULL != m_pTracker)
	{
		m_pTracker->OnKeyDown(nChar, nFlags);
	}
	
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CSpaceView::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
	if (pDataObject->IsDataAvailable(CF_HDROP))
	{
		HGLOBAL hMem = pDataObject->GetGlobalData(CF_HDROP);
		LPDROPFILES pDropFiles = (LPDROPFILES) ::GlobalLock(hMem);

		USES_CONVERSION;
		LPWSTR pszWFilePath = 
			(LPWSTR) ((LPBYTE) pDropFiles + pDropFiles->pFiles);
		LPCSTR pszFilePath = W2A(pszWFilePath);


		SHFILEINFO info;
		ZeroMemory(&info, sizeof(info));

		SHGetFileInfo(pszFilePath, 0, &info, sizeof(info), SHGFI_DISPLAYNAME);

		CNode *pNewNode = new CNode();
		pNewNode->SetName(info.szDisplayName);
		pNewNode->SetUrl(pszFilePath);

		GetSpace()->AddNode(pNewNode, GetSpace()->GetNodeAt(0));

		::GlobalUnlock(hMem);
	}
		
	return FALSE;
}

DROPEFFECT CSpaceView::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	return OnDragOver(pDataObject, dwKeyState, point);
}

void CSpaceView::OnDragLeave() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	// CWnd::OnDragLeave();
}

DROPEFFECT CSpaceView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	DROPEFFECT de = DROPEFFECT_NONE;
	if (pDataObject->IsDataAvailable(CF_TEXT)
		|| pDataObject->IsDataAvailable(CF_BITMAP)
		|| pDataObject->IsDataAvailable(CF_HDROP))
	{
		de = DROPEFFECT_COPY;
	}

	return de;	
}


void CSpaceView::OnGoClicked()
{
	::AfxMessageBox("Go Clicked", MB_OK);
}

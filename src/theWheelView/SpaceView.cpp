#include "include\SpaceView.h"
//////////////////////////////////////////////////////////////////////
// SpaceView.cpp: implementation of the CSpaceView class.
//
// Copyright (C) 1996-2002
// $Id: SpaceView.cpp,v 1.24 2007/07/05 02:50:56 Derek Lane Exp $
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
const int TIMER_ELAPSED = 30; // 20;	// ms per tick
const int TIMER_ID = 7;			// luck 7

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CSpaceView::CSpaceView
// 
// constructs a new CSpaceView object 
//////////////////////////////////////////////////////////////////////
CSpaceView::CSpaceView()
	: //m_lpDD(NULL)				// DirectDraw object
		//, m_lpDDSPrimary(NULL)	// DirectDraw primary surface
		//, m_lpDDSOne(NULL)		// Offscreen surface 1
		//, m_lpClipper(NULL)		// clipper for primary
		 m_pd3d(NULL)
		, m_pd3dDev(NULL)
		, m_pTracker(NULL)
		, m_pMaximizedView(NULL)
		, m_bDragging(FALSE)
		, m_pNLM(NULL)
		, m_pSkin(NULL)
		, m_pSpace(NULL)
{
	DWORD dwBkColor = ::AfxGetApp()->GetProfileInt(_T("Settings"), _T("BkColor"), 
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
#ifdef STL_COLL
	for (int nAt = 0; nAt < m_arrNodeViews.size(); nAt++)
	{
		delete m_arrNodeViews.at(nAt);
	}
	m_arrNodeViews.clear(); // RemoveAll();
#else
	for (int nAt = 0; nAt < m_arrNodeViews.GetCount(); nAt++)
	{
		delete m_arrNodeViews[nAt];
	}
	m_arrNodeViews.RemoveAll();
#endif

	if (m_pd3dDev != NULL)
		m_pd3dDev->Release();

	if (m_pd3d != NULL)
		m_pd3d->Release();

	delete m_pSkin;

/*	// delete the DirectDraw surfaces
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
	} */

	// delete the tracker
	delete m_pTracker;

	if (m_pNLM != NULL)
	{
		delete m_pNLM;
	}

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
	return (int) GetSpace()->GetNodeCount(); // m_arrNodeViews.size();

}	// CSpaceView::GetNodeViewCount


//////////////////////////////////////////////////////////////////////
// CSpaceView::GetNodeView
// 
// returns the node view at the given index position
//////////////////////////////////////////////////////////////////////
CNodeView *CSpaceView::GetNodeView(int nAt)
{
	return (CNodeView *) GetSpace()->GetNodeAt(nAt)->GetView(); // m_arrNodeViews.at(nAt);

}	// CSpaceView::GetNodeView


//////////////////////////////////////////////////////////////////////
// CSpaceView::GetVisibleNodeCount
// 
// returns the number of visible nodes
//////////////////////////////////////////////////////////////////////
int CSpaceView::GetVisibleNodeCount()
{
	int nNumVizNodeViews = __min((int) GetNodeViewCount(), // m_arrNodeViews.size(), 
		GetSpace()->GetLayoutManager()->GetSuperNodeCount()); 

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
		if (pNodeView == NULL)
		{
			continue;
		}

		// see if the mouse-click was within it
		const CRgn& rgnShape = pNodeView->GetShapeHit();
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
//	SortNodeViews();

}	// CSpaceView::ActivateNodeView


//////////////////////////////////////////////////////////////////////////////
HRESULT 
	CSpaceView::ResetDevice()
	// called to either create or reset the device
{
	if (!m_pd3d)
	{
		// Create the D3D object, which is needed to create the D3DDevice.
		m_pd3d = Direct3DCreate9( D3D_SDK_VERSION );
		if (NULL == m_pd3d)
			return E_FAIL;
	}

	// free the device, if it is still present
	if (m_pd3dDev)
		m_pd3dDev->Release();

    // Set up the structure used to create the D3DDevice. Most parameters are
    // zeroed out. We set Windowed to TRUE, since we want to do D3D in a
    // window, and then set the SwapEffect to "discard", which is the most
    // efficient method of presenting the back buffer to the display.  And 
    // we request a back buffer format that matches the current desktop display 
    // format.
    D3DPRESENT_PARAMETERS d3dpp; 
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

    // Create the Direct3D device. Here we are using the default adapter (most
    // systems only have one, unless they have multiple graphics hardware cards
    // installed) and requesting the HAL (which is saying we want the hardware
    // device rather than a software one). Software vertex processing is 
    // specified since we know it will work on all cards. On cards that support 
    // hardware vertex processing, though, we would see a big performance gain 
    // by specifying hardware vertex processing.
	CHECK_HRESULT(m_pd3d->CreateDevice(D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL, 
		m_hWnd,                              
		D3DCREATE_HARDWARE_VERTEXPROCESSING, // 
		// D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, 
		&m_pd3dDev));

	// Turn off culling, so we see the front and back of the triangle
	m_pd3dDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	//// auto-rescale normals
	///m_pd3dDev->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	m_pd3dDev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

    ASSERT_HRESULT(m_pd3dDev->SetRenderState(D3DRS_ZENABLE, 0));
    ASSERT_HRESULT(m_pd3dDev->SetRenderState(D3DRS_ZWRITEENABLE, 0));
    ASSERT_HRESULT(m_pd3dDev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD));
	ASSERT_HRESULT(m_pd3dDev->SetRenderState(D3DRS_SPECULARENABLE, FALSE));
	ASSERT_HRESULT(m_pd3dDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	ASSERT_HRESULT(m_pd3dDev->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE));
	ASSERT_HRESULT(m_pd3dDev->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_RGBA(64, 64, 64, 255))); // (DWORD) D3DCOLOR_COLORVALUE(0.25, 0.25, 0.25, 1.0)));

	// now call OnResetDevice for skin and node views

	m_skin.InitDDraw(m_pd3dDev);

	if (m_pSkin)
		delete m_pSkin;

	/// TODO: fix this memory leak
	m_pSkin = new theWheel2007::NodeViewSkin(m_pd3dDev);

	return S_OK;
}


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
	// pParentNode->SetView(pNewNodeView);
	// m_skin.CalcInnerOuterRect(pNewNodeView);
	// TODO: calc shape???  Call UpdateSpringActivation

	// trigger loading of the DIB
	pParentNode->GetDib();

	// and add to the array
#ifdef STL_COLL
	m_arrNodeViews.push_back(pNewNodeView);
#else
	m_arrNodeViews.Add(pNewNodeView);
#endif


	// and finally, create the child node views
	int nAtNode;
	for (nAtNode = 0; nAtNode < pParentNode->GetChildCount(); nAtNode++)
	{
		// construct a new node view for this node
		CreateNodeViews(pParentNode->GetChildAt(nAtNode), pt);
	}

	// sort the new node views
//	SortNodeViews();

}	// CSpaceView::CreateNodeViews

/*
/////////////////////////////////////////////////////////////////////////////
// CSpaceView::SortNodeViews
// 
// sorts the children node views by activation
/////////////////////////////////////////////////////////////////////////////
void CSpaceView::SortNodeViews()
{
	/*
	if (m_arrNodeViews.size() > 0)
		sort(m_arrNodeViews.begin(), m_arrNodeViews.end(), &CNodeView::IsActivationGreater);
		// qsort(&m_arrNodeViews.at(0), GetNodeViewCount(), sizeof(CNodeView*), 
		//	CompareNodeViewActivation);
* /

}	// CSpaceView::SortNodeViews
*/

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
		auto pView = GetNodeView(nAt);
		if (pView == NULL)
		{
			continue;
		}

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
		auto pView = GetNodeView(nAt);
		if (pView == NULL)
		{
			continue;
		}

		pView->GetNode()->SetPosition(pView->GetNode()->GetPosition() - vOffset,
			pView->GetNode()->IsPositionReset(false));
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
		if (pNodeView == NULL)
		{
			continue;
		}

		if (pNodeView->GetPendingActivation() > 1e-6)
		{
			const REAL frac_act = 1.0 / 4.0;
			REAL pendAct = pNodeView->GetPendingActivation() * frac_act;
			// pendAct = __min(pendAct, 0.01);
			ActivateNodeView(pNodeView, pendAct);
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
			
			pNodeView->AttenuatePending(1.0 - frac_act);
			// pNodeView->m_pendingActivation *= (1.0 - frac_act);
		}
	}

	if (maxPending > 0.0)
	{
		// update the recent click list
		m_pRecentActivated[1] = pSecMaxPending;
		m_pRecentActivated[0] = pMaxPending;


		// normalize the nodes
		// GetSpace()->NormalizeNodes();
	}

}	// CSpaceView::ActivatePending



// called when a new node is added to the space
void CSpaceView::OnNodeAdded(/*CObservableEvent * pEvent, void * pParam*/)
{
	//CNode *pNode = (CNode *) pParam;
	//ASSERT(pNode->IsKindOf(RUNTIME_CLASS(CNode)));

	//if (pNode->GetView() == NULL)
	//{
	//	// construct a new node view for this node, and add to the array
	//	CNodeView *pNewNodeView = new CNodeView(pNode, this);
	//	m_arrNodeViews.push_back(pNewNodeView);

	//	// activate the node to propagate the activation
	//	ActivateNodeView(pNewNodeView, (float) 1.4);

	//	// and lay them out and center them
	//	GetSpace()->GetLayoutManager()->LayoutNodes();
	//	CenterNodeViews();
	//}
}


// called when a new node is added to the space
void CSpaceView::OnNodeRemoved(/*CObservableEvent * pEvent, void * pParam*/)
{
	//CNode *pNode = (CNode *) pParam;
	//ASSERT(pNode->IsKindOf(RUNTIME_CLASS(CNode)));

	//// find the node view
	//for (int nAt = 0; nAt < m_arrNodeViews.size(); nAt++)
	//{
	//	// deleting a node
	//	CNodeView *pNodeView = (CNodeView *) pNode->GetView();

	//	if (m_arrNodeViews[nAt] == pNodeView)
	//	{
	//		m_arrNodeViews.erase(m_arrNodeViews.begin()+nAt);
	//		nAt--;
	//	}
	//}
}


/////////////////////////////////////////////////////////////////////////////
// CSpaceView drawing


//////////////////////////////////////////////////////////////////////
void 
	CSpaceView::SetSpace(CSpace *pSpace)
	// called when a CSpace is first loaded
{
	// CWnd::OnInitialUpdate();
	m_pSpace = pSpace;

	ASSERT(GetSpace() != NULL);

	// get rid of the node views
#ifdef STL_COLL
	for (int nAt = 0; nAt < m_arrNodeViews.size(); nAt++)
	{
		delete m_arrNodeViews.at(nAt);
	}
	m_arrNodeViews.clear();
#else
	for (int nAt = 0; nAt < m_arrNodeViews.GetCount(); nAt++)
	{
		delete m_arrNodeViews[nAt];
	}
	m_arrNodeViews.RemoveAll();
#endif

	// create the child node views
	CRect rect;
	GetClientRect(&rect);
	CreateNodeViews(GetSpace()->GetRootNode(), rect.CenterPoint());

	// check to make sure all nodes have views
	// ASSERT(CheckNodeViews(GetSpace()->GetRootNode()));

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
	//GetSpace()->NodeAddedEvent.AddObserver(this, 
	//	(ListenerFunction) &CSpaceView::OnNodeAdded);
	//GetSpace()->NodeRemovedEvent.AddObserver(this, 
	//	(ListenerFunction) &CSpaceView::OnNodeRemoved);

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
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// initialize CNodeLayoutManager for large layout
	m_pNLM = new CNodeLayoutManager();
	m_pNLM->SetMinLayoutSelection(60.0);
	m_pNLM->SetImageBesideTitle(true); 
	m_pNLM->SetShowDescription(true);
	m_pNLM->SetMaxTitleHeight(30.0);
	m_pNLM->SetTitleLogWeight(5.0);
	m_pNLM->SetImageLogWeight(3.0);
	m_pNLM->SetMaxImageWidth(200.0);
	m_pNLM->SetMaxDescWidth(400.0);  
		// TODO: synch with TEXT_WIDTH in CNodeView

	// set up medium layout
	CNodeLayoutManager *pNLM = m_pNLM->GetNextLayoutManager();
	pNLM->SetMinLayoutSelection(25.0);
	pNLM->SetImageBesideTitle(true);
	pNLM->SetImageLogWeight(1.0);

	// set up small layout
	pNLM = pNLM->GetNextLayoutManager();
	pNLM->SetMinLayoutSelection(15.0);
	pNLM->SetImageLogWeight(0.0);

	// set up small layout
	pNLM = pNLM->GetNextLayoutManager();
	pNLM->SetMaxImageWidth(0.0);

	// create a timer
 	m_nTimerID = (int) SetTimer(TIMER_ID, TIMER_ELAPSED, NULL);
	ASSERT(m_nTimerID != 0);

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

	// use this to re-initialize for new window size
	ASSERT_HRESULT(ResetDevice());

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
	// check that the device is available
	if (!m_pd3dDev)
		ASSERT_HRESULT(ResetDevice());

	// test whether the device is OK
	HRESULT hr = m_pd3dDev->TestCooperativeLevel();
	if (SUCCEEDED(hr))
	{
		// reset background
		m_pd3dDev->Clear( 0, NULL, D3DCLEAR_TARGET, 
			D3DCOLOR_XRGB(GetRValue(m_colorBk), GetGValue(m_colorBk), GetBValue(m_colorBk)), 
			1.0f, 0 );
	    
		// set up the zoom transform, accounting for rectangle
		//		inflation
		CRect rectOuter;
		GetClientRect(&rectOuter);
		D3DXMATRIX mat;
		ZeroMemory(&mat, sizeof(D3DMATRIX));
		mat._11 /* (0, 0) */ = (D3DVALUE) // 0.5; // 
			1.0; // (1.0 / (rectOuter.Width()));// + 10.0));
		mat._22 /* (1, 1) */ = (D3DVALUE) // 0.5; // 
			1.0; // (1.0 / (rectOuter.Height())); // Width()));// + 10.0));
		mat._33 /* (2, 2) */ = (D3DVALUE) 1.0; // 0.5; // 1.0; 
		mat._44 /* (3, 3) */ = (D3DVALUE) 1.0;
		mat._41 = 0.0; // -rectOuter.Width()/2.0; // (D3DVALUE) -rectOuter.Width() / 2.0;
		mat._42 = 0.0; // rectOuter.Height()/2.0; // * (FLOAT) rectOuter.Height() / (FLOAT) rectOuter.Width(); // (D3DVALUE) -rectOuter.Height() / 2.0;
		mat._43 = 0.0; // -1.0;
		// D3DXMatrixOrthoRH(&mat, rectOuter.Width(), rectOuter.Height(), -0.1, -10.0); //, 1.0);
		ASSERT_HRESULT(m_pd3dDev->SetTransform(D3DTS_VIEW, 
			(D3DMATRIX*) & mat));

		D3DXMATRIX mat2;
		D3DXMatrixOrthoLH(&mat2, rectOuter.Width() / 1.0, rectOuter.Height() / 1.0, -40.0, 40.0);
		ASSERT_HRESULT(m_pd3dDev->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*) & mat2));

		D3DVIEWPORT9 vp;
		m_pd3dDev->GetViewport(&vp);
		vp.MinZ = 0.0;
		vp.MaxZ = 100.0;
		//m_pd3dDev->SetViewport(&vp);


		// identity world matrix
		D3DXMATRIX matWorld;
		D3DXMatrixIdentity(&matWorld);
		m_pd3dDev->SetTransform( D3DTS_WORLD, (D3DMATRIX*) &matWorld );

		// view matrix at z = -5, looking toward +z
		D3DXVECTOR3 vEyePt( -rectOuter.Width()/2.0, -rectOuter.Height()/2.0, 5.0f) ; // 0.0f, 0.0f, 1.0f );
		D3DXVECTOR3 vLookatPt( -rectOuter.Width()/2.0, -rectOuter.Height()/2.0, -5.0); // 0.0f, 0.0f, 0.0f );
		D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
		D3DXMATRIX matView;
		D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec);
		m_pd3dDev->SetTransform( D3DTS_VIEW, (D3DMATRIX*) &matView );

		// orthogonal projection transform
		D3DXMATRIX matProj;
		D3DXMatrixOrthoLH( &matProj, 0.5f, 0.5f, -100.5f, 100.5f); // 8.0f, 8.0f, 0.5f, 1.5f);
		// m_pd3dDev->SetTransform( D3DTS_PROJECTION, &matProj );




		D3DLIGHT9 light;
		ZeroMemory( &light, sizeof(D3DLIGHT9) );
		light.Type       = D3DLIGHT_DIRECTIONAL;
		light.Diffuse.r  = 1.0f;
		light.Diffuse.g  = 1.0f;
		light.Diffuse.b  = 1.0f;

		D3DXVECTOR3 vecDir = D3DXVECTOR3(-0.2f, -0.3f, -0.5f); 
		D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );
		light.Range       = 1000.0f;

		m_pd3dDev->SetLight( 0, &light );
		m_pd3dDev->LightEnable( 0, TRUE );
		m_pd3dDev->SetRenderState( D3DRS_LIGHTING, TRUE );

		// Finally, turn on some ambient light.
		// m_pd3dDev->SetRenderState( D3DRS_AMBIENT, 0x00000000); // 0x00202020 );
	// set up an ambient light source
	ASSERT_HRESULT(m_pd3dDev->SetRenderState(D3DRS_AMBIENT, // D3DCOLOR_RGBA(96, 96, 96, 255))); // 
		D3DCOLOR_RGBA(64, 64, 64, 255))); // (DWORD) D3DCOLOR_COLORVALUE(0.25, 0.25, 0.25, 1.0)));

	ASSERT_HRESULT(m_pd3dDev->SetRenderState(D3DRS_FOGENABLE, TRUE));

	ASSERT_HRESULT(m_pd3dDev->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR));

	ASSERT_HRESULT(m_pd3dDev->SetRenderState(D3DRS_FOGCOLOR, 
		D3DCOLOR_XRGB(GetRValue(m_colorBk), GetGValue(m_colorBk), GetBValue(m_colorBk)))); // 0x00FFFFFF)); // Highest 8 bits are not used.

	float Start = 0.0f, End = 15.0f;
        m_pd3dDev->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&Start));
        m_pd3dDev->SetRenderState(D3DRS_FOGEND,   *(DWORD *)(&End));

		// Begin the scene
		ASSERT_HRESULT(m_pd3dDev->BeginScene());

		// now populate array to hold the drawing-order for the nodeviews
#ifdef STL_COLL_SPACE_NODES
		arrNodeViewsToDraw.resize(__min(GetVisibleNodeCount() * 2, 
			GetSpace()->m_arrNodes.size())); 
		copy(GetSpace()->m_arrNodes.begin(), 
			GetSpace()->m_arrNodes.begin() + arrNodeViewsToDraw.size(),
			arrNodeViewsToDraw.begin());
#else
#ifdef STL_COLL
		arrNodeViewsToDraw.resize(__min(GetVisibleNodeCount() * 2, 
			GetSpace()->m_arrNodes.GetCount())); 
    for (int nAt = 0; nAt < arrNodeViewsToDraw.size(); nAt++)
    {
      arrNodeViewsToDraw[nAt] = GetSpace()->m_arrNodes[nAt];
    }

#else
		arrNodeViewsToDraw.SetCount(__min(GetVisibleNodeCount() * 2, 
			GetSpace()->m_arrNodes.size()));
    for (int nAt = 0; nAt < arrNodeViewsToDraw.GetCount(); nAt++)
    {
      arrNodeViewsToDraw[nAt] = GetSpace()->m_arrNodes[nAt];
    }
#endif
#endif

		// sort by activation difference comparison
#ifdef STL_COLL
		sort(arrNodeViewsToDraw.begin(), arrNodeViewsToDraw.end(),
			&CNodeView::IsActDiffGreater);
#else
		sort(&arrNodeViewsToDraw[0], &arrNodeViewsToDraw[0]+arrNodeViewsToDraw.GetCount(),
			&CNodeView::IsActDiffGreater);
#endif

		// draw the node view links
#ifdef STL_COLL
		vector<CNode*>::iterator iter;
		for (iter = arrNodeViewsToDraw.begin(); iter != arrNodeViewsToDraw.end(); iter++)
			if (!(*iter)->GetIsSubThreshold())
				((CNodeView*)(*iter)->GetView())->DrawLinks(m_pd3dDev, &m_skin);

#else
		for (int nAt = 0; nAt < arrNodeViewsToDraw.GetCount(); nAt++)
    {
      CNode *pNode = arrNodeViewsToDraw[nAt];
	  auto pNodeView = (CNodeView*) pNode->GetView();
	  if (pNodeView != NULL)
	  {
		  if (!pNode->GetIsSubThreshold())
		  {
			  pNodeView->DrawLinks(m_pd3dDev, &m_skin);
		  }
	  }
    }
#endif

		// draw in sorted order
#ifdef STL_COLL
		for (iter = arrNodeViewsToDraw.begin(); iter != arrNodeViewsToDraw.end(); iter++)
			if (!(*iter)->GetIsSubThreshold())
				((CNodeView*)(*iter)->GetView())->Draw(m_pd3dDev); 
#else
		for (int nAt = 0; nAt < arrNodeViewsToDraw.GetCount(); nAt++)
    {
      CNode *pNode = arrNodeViewsToDraw[nAt];
	  auto pNodeView = (CNodeView*)pNode->GetView();
	  if (pNodeView != NULL)
	  {
		  if (!pNode->GetIsSubThreshold())
			  pNodeView->Draw(m_pd3dDev);
	  }
    }
#endif

		// End the scene
		ASSERT_HRESULT(m_pd3dDev->EndScene());

		// Present the backbuffer contents to the display
		hr = m_pd3dDev->Present( NULL, NULL, NULL, NULL );

		// see if the device was lost
		if (D3DERR_DEVICELOST == hr)
			ResetDevice();
	}
	else if (D3DERR_DEVICENOTRESET == hr)
	{
		ResetDevice();
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

	// ONLY lay out every other
	//if (rand() < RAND_MAX / 3)
	{
		// layout the nodes and center them
		GetSpace()->GetLayoutManager()->LayoutNodes();
	}

	// update the privates
	for (int nAt = 0; nAt < GetNodeViewCount(); nAt++)
	{
		auto pNodeView = GetNodeView(nAt);
		if (pNodeView == NULL)
		{
			continue;
		}

		pNodeView->UpdateSpringPosition(GetSpace()->GetSpringConst());
		pNodeView->UpdateSpringActivation(GetSpace()->GetSpringConst());
	}

	// now center based on the new positions
	CenterNodeViews();

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

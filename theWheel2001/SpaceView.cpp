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
#include "resource.h"

// the class definition
#include "SpaceView.h"

// the optimizers
#include <ConjGradOptimizer.h>
#include <PowellOptimizer.h>

// the displayed model object
#include <Space.h>

// child node views
#include "NodeView.h"

// the new node dialog
#include "EditNodeDlg.h"

#include <ddraw.h>
#include "ddutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Constants for the CSpaceView
/////////////////////////////////////////////////////////////////////////////

// holds the current node ID
int g_nNodeID = 1100;

// constant for the tolerance of the optimization
const SPV_STATE_TYPE TOLERANCE = 
	(SPV_STATE_TYPE) 0.7;

// constant for the total activation
const SPV_STATE_TYPE TOTAL_ACTIVATION = 
	(SPV_STATE_TYPE) 0.50f;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CSpaceView::CSpaceView
// 
// constructs a new CSpaceView object 
//////////////////////////////////////////////////////////////////////
CSpaceView::CSpaceView()
: isPropagating(TRUE),
	isWaveMode(FALSE),
	m_pRecentClick1(NULL),
	m_pRecentClick2(NULL),
	m_pBuffer(NULL),
	m_pOldBitmap(NULL)
{
	// create the energy function
	m_pEnergyFunc = new CSpaceViewEnergyFunction(this);

	// create the optimizer
#ifdef USE_GRADIENT
	m_pOptimizer = new CConjGradOptimizer<SPV_STATE_DIM, SPV_STATE_TYPE>(m_pEnergyFunc);
#else
	m_pOptimizer = new CPowellOptimizer<SPV_STATE_DIM, SPV_STATE_TYPE>(m_pEnergyFunc);
#endif

	// set the tolerance
	m_pOptimizer->SetTolerance(TOLERANCE);

	// initialize the buffer
	m_dcMem.CreateCompatibleDC(NULL);
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::CSpaceView
// 
// constructs a new CSpaceView object 
//////////////////////////////////////////////////////////////////////
CSpaceView::~CSpaceView()
{
	// get rid of the node views
	nodeViews.RemoveAll();

	// delete the optimizer and energy function
	delete m_pEnergyFunc;
	delete m_pOptimizer;

	// select out the bitmap
	m_dcMem.SelectObject(m_pOldBitmap);

	// delete the bitmap buffer
	delete m_pBuffer;

	// delete the device context
	m_dcMem.DeleteDC();
}

//////////////////////////////////////////////////////////////////////
// implements the dynamic creation mechanism for the CSpaceView
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CSpaceView, CView)

//////////////////////////////////////////////////////////////////////
// CSpaceView::GetViewForNode
// 
// locates and returns the node view for the given node
//////////////////////////////////////////////////////////////////////
CNodeView *CSpaceView::GetViewForNode(CNode *pNode)
{
	return (CNodeView *)pNode->GetView();
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::AddNodeToSpace
// 
// adds a new node to the space, setting the link weights based on
//		the current node activations
//////////////////////////////////////////////////////////////////////
void CSpaceView::AddNodeToSpace(CNode *pNewNode)
{
	// compute the total weight (for determining initial link weights)
	double totalWeight = 0.0;
	int nAt;
	for (nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		CNodeView *pView = nodeViews.Get(nAt);
		totalWeight += pView->forNode->GetActivation();
	}

	// accumulate max weight, and set the parent node and initial
	//		point along the way
	float maxWeight = 0.0f;
	CNode *pParentNode = NULL;
	CPoint ptInit;

	// set the link weights based on the activation
	for (nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		// get the next node view
		CNodeView *pView = nodeViews.Get(nAt);

		// determine the appropriate link weight
		double weight = pView->forNode->GetActivation() / totalWeight;

		// establish the link
		pNewNode->LinkTo(pView->forNode.Get(), (float) weight);

		// is this a potential parent?
		if (weight > maxWeight)
		{
			maxWeight = (float) weight;
			pParentNode = pView->forNode.Get();
			ptInit = pView->GetCenter();
		}
	}

	// add the new node to the parent
	pParentNode->children.Add(pNewNode);

	// construct a new node view for this node
	CNodeView *pNewNodeView = new CNodeView(pNewNode, this);

	// set the activation for the new node
	pNewNodeView->forNode->SetActivation(maxWeight * 0.25f);

	// compute the initial rectangle for the node view
	CRect rect(ptInit.x - 100, ptInit.y - 75, 
		ptInit.x + 100, ptInit.y + 75);

	// and add to the array
	nodeViews.Add(pNewNodeView);
	ActivateNode(pNewNodeView, (float) 1.6);

	// and lay them out
	LayoutNodeViews();

	// update all other views
	GetDocument()->UpdateAllViews(this);
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::CreateNodeViews
// 
// creates the node view for the parent, as well as all child node 
//		views
//////////////////////////////////////////////////////////////////////
void CSpaceView::CreateNodeViews(CNode *pParentNode, CPoint pt)
{
	// construct a new node view for this node
	CNodeView *pNewNodeView = new CNodeView(pParentNode, this);

	// and add to the array
	nodeViews.Add(pNewNodeView);

	// TODO: should we lay out the node views here and use the new
	//		position on the children?

	// and finally, create the child node views
	int nAtNode;
	for (nAtNode = 0; nAtNode < pParentNode->children.GetSize(); nAtNode++)
	{
		// construct a new node view for this node
		CreateNodeViews((CNode *) pParentNode->children.Get(nAtNode), pt);
	}

	// activate this node view after creating children (to maximize size)
	ActivateNode(pNewNodeView, 0.5);
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
		for (int nAt = 0; nAt < nodeViews.GetSize()-1; nAt++)
		{
			CNodeView *pThisNodeView = nodeViews.Get(nAt);
			CNodeView *pNextNodeView = nodeViews.Get(nAt+1);

			if (pThisNodeView->forNode->GetActivation() < 
					pNextNodeView->forNode->GetActivation())
			{
				nodeViews.Set(nAt, pNextNodeView);
				nodeViews.Set(nAt+1, pThisNodeView);
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
	if (nodeViews.GetSize() == 0)
		return;
	
	CVector<2> vMeanCenter;

	// compute the vector sum of all node views' centers
	int nAt = 0;
	double totalScaleFactor = 0.0f;
	int nNumVizNodeViews = min(nodeViews.GetSize(), STATE_DIM / 2);
	for (nAt = 0; nAt < nNumVizNodeViews; nAt++)
	{
		CNodeView *pView = nodeViews.Get(nAt);
		double scaleFactor = 10000.0 * pView->forNode->GetActivation();

		if (pView == m_pRecentClick1)
			scaleFactor *= 4.0;
		else if (pView == m_pRecentClick2)
			scaleFactor *= 2.0;
		vMeanCenter = vMeanCenter 
			+ pView->GetCenter() * scaleFactor; 
		totalScaleFactor += scaleFactor;
	}

	// divide by number of node views
	vMeanCenter *= 1.0 / totalScaleFactor;

	// compute the vector offset for the node views
	//		window center
	CRect rectWnd;
	GetClientRect(&rectWnd);
	CVector<2> vOffset = vMeanCenter 
		- CVector<2>(rectWnd.CenterPoint()); // rectWnd.Width() / 2, rectWnd.Height() / 2);

	// offset each node view by the difference between the mean and the 
	//		window center
	for (nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		CNodeView *pView = nodeViews.Get(nAt);
		pView->SetCenter(pView->GetCenter() - vOffset);
	}
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::LayoutNodeViews
// 
// applies the optimizer to layout the node views
//////////////////////////////////////////////////////////////////////
void CSpaceView::LayoutNodeViews()
{
	// only layout if there are children
	if (nodeViews.GetSize() == 0)
		return;

	// do the second optimization
	CVector<SPV_STATE_DIM, SPV_STATE_TYPE> vCurrent = GetStateVector();

	// now optimize
	vCurrent = m_pOptimizer->Optimize(vCurrent);

	// now perform the optimization
	SetStateVector(vCurrent);

	// center the node views as part of the layout
	//		TODO: check this, see if it is really necessary
	CenterNodeViews();
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::ActivateNode
// 
// activates and propagates on a particular node view
//////////////////////////////////////////////////////////////////////
void CSpaceView::ActivateNode(CNodeView *pNodeView, float scale)
{
	// first, increase the activation of the node up to the max (from OnLMouseButton)
	double oldActivation = pNodeView->forNode->GetActivation();
	double newActivation = oldActivation 
		+ (TOTAL_ACTIVATION - oldActivation) * scale;
	pNodeView->forNode->SetActivation(newActivation);

	// now, propagate the activation
	GetDocument()->rootNode.ResetForPropagation();
	pNodeView->forNode->PropagateActivation(0.6);

	// normalize the nodes
	GetDocument()->NormalizeNodes();

	// sort the node views
	SortNodeViews();

	// compute the activation threshold, and set the thresholded activation on 
	//		all node views

	// form the number of currently visualized node views
	int nNumVizNodeViews = min(nodeViews.GetSize(), STATE_DIM / 2);
	double activationThreshold = 
		nodeViews.Get(nNumVizNodeViews - 1)->forNode->GetActivation();

	// compute the normalization factor for super-threshold node views
	double superThresholdSum = 0.0;
	for (int nAt = 0; nAt < nNumVizNodeViews; nAt++)
	{
		superThresholdSum += nodeViews.Get(nAt)->forNode->GetActivation();
	}
	double superThresholdScale = TOTAL_ACTIVATION / superThresholdSum;

	// now set the thresholdedActivation for all node views
	for (nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		CNodeView *pNodeView = nodeViews.Get(nAt);
		double activation = pNodeView->forNode->GetActivation();

		// for super-threshold nodes,
		if (activation > activationThreshold)
		{
			// scale so that super-thresholded activations are (roughly)
			//		normalized
			activation *= superThresholdScale;

			// now set the thresholded activation
			pNodeView->SetThresholdedActivation( (float)
				activation * (float) superThresholdScale);
		}
		else
		{
			// otherwise, the thresholded activation is zero
			pNodeView->SetThresholdedActivation(0.0);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSpaceView drawing

LPDIRECTDRAW		lpDD = NULL;			// DirectDraw object
LPDIRECTDRAWSURFACE	lpDDSPrimary = NULL;	// DirectDraw primary surface
LPDIRECTDRAWSURFACE	lpDDSOne = NULL;		// Offscreen surface 1
LPDIRECTDRAWSURFACE	lpDDSBitmap = NULL;		// Offscreen surface 1
LPDIRECTDRAWCLIPPER lpClipper = NULL;		// clipper for primary

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

	CBrush brush(RGB(192, 192, 192));
	pDC->FillRect(&rectClient, &brush);

	// create new node views
	int nAtNodeView;
	for (nAtNodeView = nodeViews.GetSize()-1; nAtNodeView >= 0; nAtNodeView--)
	{
		CNodeView *pNodeView = (CNodeView *)nodeViews.Get(nAtNodeView);

		CBrush greyBrush(RGB(160, 160, 160));
		CBrush *pOldBrush = pDC->SelectObject(&greyBrush);
		CPen *pOldPen = (CPen *) pDC->SelectStockObject(NULL_PEN);

		if (pNodeView->GetThresholdedActivation() > 0.0)
		{
			for (int nAtLink = 0; nAtLink < pNodeView->forNode->links.GetSize(); nAtLink++)
			{
				CNodeLink *pLink = pNodeView->forNode->links.Get(nAtLink);
				CNodeView *pLinkedView = GetViewForNode(pLink->forTarget.Get());
				if (pLinkedView->GetThresholdedActivation() > 0.0)
				{
					// draw the link
					CVector<2> vFrom = pNodeView->GetSpringCenter();
					CVector<2> vTo = pLinkedView->GetSpringCenter();

					CVector<2> vOffset = vTo - vFrom;
					CVector<2> vNormal(vOffset[1], -vOffset[0]);
					vNormal.Normalize();

					vOffset *= 0.5;

					CPoint ptPoly[6];
					ptPoly[0] = vFrom + 12.0 * vNormal;
					ptPoly[1] = vFrom + vOffset + 3.0 * vNormal;
					ptPoly[2] = vTo + 12.0 * vNormal;
					ptPoly[3] = vTo - 12.0 * vNormal;
					ptPoly[4] = vFrom + vOffset - 3.0 * vNormal;
					ptPoly[5] = vFrom - 12.0 * vNormal;

					pDC->Polygon(ptPoly, 6);
				}
			}
		}

		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);
		greyBrush.DeleteObject();
	} 

	HRESULT ddrval = lpDDSOne->ReleaseDC(pDC->m_hDC);
	ASSERT(ddrval == DD_OK);


	// create new node views
	for (nAtNodeView = nodeViews.GetSize()-1; nAtNodeView >= 0; nAtNodeView--)
	{
		CNodeView *pNodeView = (CNodeView *)nodeViews.Get(nAtNodeView);

		// get the inner rectangle for drawing the text
		CRect rectInner = pNodeView->GetInnerRect();

		// only draw if it has a substantial area
		if (rectInner.Height() > 1)
		{
			// draw the elliptangle
			// DrawElliptangle(pDC);
			int x = (int)(pNodeView->GetSpringCenter()[0] - 233.0/2.0);
			int y = (int)(pNodeView->GetSpringCenter()[1] - 176.0/2.0);
			// ddrval = lpDDSOne->BltFast(x, y, lpDDSBitmap, CRect(0, 0, 233, 176), // DDBLTFAST_NOCOLORKEY); 
			//	DDBLTFAST_SRCCOLORKEY);
			ASSERT(ddrval == DD_OK);

			// draw some stuff
			HDC hdc;
			HRESULT ddrval = lpDDSOne->GetDC(&hdc);
			ASSERT(ddrval == DD_OK);

			// draw the text
			pNodeView->DrawText(CDC::FromHandle(hdc), rectInner);

			ddrval = lpDDSOne->ReleaseDC(hdc);
			ASSERT(ddrval == DD_OK);
		}

		// pNodeView->Draw(pDC);
	} 
}

BOOL CSpaceView::InitDDraw()
{
    DDSURFACEDESC	ddsd;
    HRESULT		ddrval;

    /*
     * create the main DirectDraw object
     */
    ddrval = DirectDrawCreate( NULL, &lpDD, NULL );
    if( ddrval != DD_OK )
    {
		return FALSE;
    }
    ddrval = lpDD->SetCooperativeLevel( m_hWnd, DDSCL_NORMAL );

    // Create the primary surface
    ddsd.dwSize = sizeof( ddsd );
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    ddrval = lpDD->CreateSurface( &ddsd, &lpDDSPrimary, NULL );
    if( ddrval != DD_OK )
    {
		return FALSE;
    }

    // create a clipper for the primary surface
    ddrval = lpDD->CreateClipper( 0, &lpClipper, NULL );
    if( ddrval != DD_OK )
    {
		return FALSE;
    }
    
    ddrval = lpClipper->SetHWnd( 0, m_hWnd);
    if( ddrval != DD_OK )
    {
		return FALSE;
    }

    ddrval = lpDDSPrimary->SetClipper( lpClipper );
    if( ddrval != DD_OK )
    {
		return FALSE;
    }

/*	lpDDSBitmap = DDLoadBitmap(lpDD, MAKEINTRESOURCE(IDB_NODEBUBBLE), 0, 0);
	if (lpDDSBitmap == NULL)
	{
		return FALSE;
	}

	ddrval = DDSetColorKey(lpDDSBitmap, RGB(0, 0, 0));
    if( ddrval != DD_OK )
    {
		return FALSE;
    }
*/
    return TRUE;
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

	// delete any old node views
	nodeViews.RemoveAll();

	// create the child node views
	CRect rect;
	GetClientRect(&rect);
	CreateNodeViews(&GetDocument()->rootNode, rect.CenterPoint());

	// now snap the node views to the parameter values
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < nodeViews.GetSize(); nAtNodeView++)
	{
		CNodeView *pNodeView = (CNodeView *)nodeViews.Get(nAtNodeView);
		pNodeView->UpdateSprings(0.0);
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

//////////////////////////////////////////////////////////////////////
// CSpaceView::GetStateVector
// 
// forms the state vector for the associated CSpaceView
//////////////////////////////////////////////////////////////////////
CSpaceView::CStateVector CSpaceView::GetStateVector()
{
	// for the state vector
	CStateVector vState;
	for (int nAt = 0; nAt < STATE_DIM / 2; nAt++)
	{
		if (nAt < nodeViews.GetSize())
		{
			CNodeView *pView = nodeViews.Get(nAt);
			vState[nAt*2] = (STATE_TYPE) pView->GetCenter()[0];
			vState[nAt*2+1] = (STATE_TYPE) pView->GetCenter()[1];
		}
	}

	// return the formed state vector
	return vState;
}

//////////////////////////////////////////////////////////////////////
// CSpaceViewEnergyFunction::SetStateVector
// 
// sets the state vector for the associated CSpaceView
//////////////////////////////////////////////////////////////////////
void CSpaceView::SetStateVector(const CSpaceView::CStateVector& vState)
{
	// form the number of currently visualized node views
	int nNumVizNodeViews = min(nodeViews.GetSize(), STATE_DIM / 2);

	for (int nAt = 0; nAt < nNumVizNodeViews; nAt++)
	{
		CVector<2> vNewCenter = CVector<2>(vState[nAt*2], vState[nAt*2+1]);
		CNodeView *pView = nodeViews.Get(nAt);
		pView->SetCenter(vNewCenter);
	}
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

/////////////////////////////////////////////////////////////////////////////
// CSpaceView Message Map
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSpaceView, CView)
	//{{AFX_MSG_MAP(CSpaceView)
	ON_COMMAND(ID_VIEW_LAYOUT, OnViewLayout)
	ON_COMMAND(ID_VIEW_PROPAGATE, OnViewPropagate)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROPAGATE, OnUpdateViewPropagate)
	ON_COMMAND(ID_VIEW_WAVE, OnViewWave)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WAVE, OnUpdateViewWave)
	ON_COMMAND(ID_NEW_NODE, OnNewNode)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpaceView message handlers

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnViewLayout
// 
// menu command to lay out the node views
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnViewLayout() 
{
	// layout the node views
	LayoutNodeViews();	

	// redraw the window
	RedrawWindow();
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnViewPropagate
// 
// changes the propagate mode state
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnViewPropagate() 
{
	isPropagating.Set(!isPropagating.Get());
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnUpdateViewPropagate
// 
// sets/unsets the propagate mode toggle button
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnUpdateViewPropagate(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(isPropagating.Get() ? 1 : 0);	
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnViewWave
// 
// changes the wave mode view state
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnViewWave() 
{
	isWaveMode.Set(!isWaveMode.Get());	
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnUpdateViewWave
// 
// sets/unsets the wave mode toggle button
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnUpdateViewWave(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(isWaveMode.Get() ? 1 : 0);		
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnNewNode
// 
// launches the new node dialog box
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnNewNode() 
{
	CEditNodeDlg newDlg(this);

	if (newDlg.DoModal() == IDOK)
	{
		CNode *pNewNode = 
			new CNode(GetDocument(), newDlg.m_strName, newDlg.m_strDesc);
		AddNodeToSpace(pNewNode);
	}
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnMouseMove
// 
// changes the cursor as the user moves the mouse around
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// search throught the node views
	for (int nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		// get the current node view
		CNodeView *pNodeView = nodeViews.Get(nAt);

		// see if the mouse-click was within it
		if (pNodeView->GetShape().m_hObject != NULL
			&& pNodeView->GetShape().PtInRegion(point))
		{
			// set the hand pointer cursor
			::SetCursor(::LoadCursor(GetModuleHandle(NULL), 
				MAKEINTRESOURCE(IDC_HANDPOINT)));

			if (isWaveMode.Get())
			{
				ActivateNode(pNodeView, 0.05f);
	
				m_pRecentClick2 = m_pRecentClick1;
				m_pRecentClick1 = pNodeView;
			}

			CView::OnMouseMove(nFlags, point);

			return;
		}
	}

	// set the standard cursor
	::SetCursor(::LoadCursor(NULL, IDC_ARROW));

	if (isWaveMode.Get())
	{
		m_pRecentClick2 = m_pRecentClick1;
		m_pRecentClick1 = NULL;
	}

	// standard processing of mouse move
	CView::OnMouseMove(nFlags, point);
}


//////////////////////////////////////////////////////////////////////
// CSpaceView::OnLButtonDown
// 
// activates a node when the user left-clicks on the node view
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CNodeView *pSelectedView = NULL;
	// search throught the node views
	for (int nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		// get the current node view
		pSelectedView = nodeViews.Get(nAt);

		// see if the mouse-click was within it
		if (pSelectedView->GetShape().PtInRegion(point))
		{
			// if so, activate it
			ActivateNode(pSelectedView, 0.333f);

			// LayoutNodeViews();
			break;
		}
	}
	
	m_pRecentClick2 = m_pRecentClick1;
	m_pRecentClick1 = pSelectedView;

	// standard processing of button down
	CView::OnLButtonDown(nFlags, point);
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnRButtonDown
// 
// displays the right-click menu on the right-button down
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	ClientToScreen(&point);
	CMenu *pMenu = new CMenu();
	pMenu->LoadMenu(IDR_SPACE_POPUP);
	pMenu->GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);

	delete pMenu;

	CView::OnRButtonDown(nFlags, point);
}

static DWORD g_dwPrevTickCount = 0;
static DWORD g_dwSumTickCount = 0;
static DWORD g_dwNumTickCount = 0;

void CSpaceView::OnTimer(UINT nIDEvent) 
{
#define LOG_IDLE_TIME
#ifdef LOG_IDLE_TIME
	DWORD dwTickCount = ::GetTickCount();
	if (g_dwPrevTickCount > 0)
	{
		g_dwSumTickCount += (dwTickCount - g_dwPrevTickCount);
		g_dwNumTickCount++;

		if ((g_dwNumTickCount % 10) == 0)
			LOG_TRACE("Average time between timer events = %lf\n",
				(double) g_dwSumTickCount / (double) g_dwNumTickCount);
	}
	g_dwPrevTickCount = dwTickCount;
#endif
	
#define TIME_LAYOUT TRUE

	START_TIMER(TIME_LAYOUT)

	LayoutNodeViews();

	STOP_TIMER(TIME_LAYOUT, "Layout")

	// update the privates
	int nAt;
	for (nAt = 0; nAt < nodeViews.GetSize(); nAt++)
		nodeViews.Get(nAt)->UpdateSprings();

#define TIME_REDRAW TRUE

	START_TIMER(TIME_REDRAW)

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	STOP_TIMER(TIME_REDRAW, "Redraw")

	CView::OnTimer(nIDEvent);
}

/**********************************************************

GetOptimalDIBFormat

  Purpose:
   Retrieves the optimal DIB format for a display 
   device. The optimal DIB format is the format that 
   exactly matches the format of the target device. 
   Obtaining this is very important when dealing with 
   16bpp modes because you need to know what bitfields 
   value to use (555 or 565 for example).

   You normally use this function to get the best
   format to pass to CreateDIBSection() in order to
   maximize blt'ing performance.

  Input:
   hdc - Device to get the optimal format for.
   pbi - Pointer to a BITMAPINFO + color table
         (room for 256 colors is assumed).

  Output:
   pbi - Contains the optimal DIB format. In the 
         <= 8bpp case, the color table will contain the 
         system palette. In the >=16bpp case, the "color 
         table" will contain the correct bit fields (see 
         BI_BITFIELDS in the Platform SDK documentation 
         for more information).

  Notes:
   If you are going to use this function on a 8bpp device
   you should make sure the color table contains a identity
   palette for optimal blt'ing.

**********************************************************/ 
BOOL GetOptimalDIBFormat(HDC hdc, BITMAPINFOHEADER *pbi)
{
    HBITMAP hbm;
    BOOL bRet = TRUE;
    
    // Create a memory bitmap that is compatible with the
    // format of the target device.
    hbm = CreateCompatibleBitmap(hdc, 1, 1);
    if (!hbm)
        return FALSE;
    
    // Initialize the header.
    ZeroMemory(pbi, sizeof(BITMAPINFOHEADER));
    pbi->biSize = sizeof(BITMAPINFOHEADER);

    // First call to GetDIBits will fill in the optimal biBitCount.
    bRet = GetDIBits(hdc, hbm, 0, 1, NULL, (BITMAPINFO*)pbi, DIB_RGB_COLORS);
    
    // Second call to GetDIBits will get the optimal color table, o
    // or the optimal bitfields values.
    if (bRet)
        bRet = GetDIBits(hdc, hbm, 0, 1, NULL, (BITMAPINFO*)pbi, DIB_RGB_COLORS);
    ASSERT(pbi->biCompression == BI_BITFIELDS);

    // Clean up.
    DeleteObject(hbm);

    return bRet;
}

void CSpaceView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	if (cx == 0 || cy == 0)
		return;

	if (lpDDSOne != NULL)
	{
		// free the surface
		lpDDSOne->Release();
	}

	// create other surface
    DDSURFACEDESC	ddsd;
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsd.dwWidth = cx;
    ddsd.dwHeight = cy;

    HRESULT ddrval = lpDD->CreateSurface(&ddsd, &lpDDSOne, NULL);
	ASSERT(ddrval == DD_OK);
}

void CSpaceView::OnPaint() 
{
	if (lpDDSOne)
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
		HRESULT ddrval = lpDDSOne->GetDC(&hdc);
		ASSERT(ddrval == DD_OK);
		
		START_TIMER(TIME_DRAW)
		OnDraw(CDC::FromHandle(hdc));
		STOP_TIMER(TIME_DRAW, "Draw")

		ddrval = lpDDSOne->ReleaseDC(hdc);
		ASSERT(ddrval == DD_OK);

		// ddrval = lpDDSOne->BltFast(100, 100, lpDDSBitmap, CRect(0, 0, 233, 176), // DDBLTFAST_NOCOLORKEY); 
		//	DDBLTFAST_SRCCOLORKEY);
		// ASSERT(ddrval == DD_OK);

		START_TIMER(TIME_BITBLT)
		ddrval = lpDDSPrimary->Blt( &destRect, lpDDSOne, &rcRect, 0, NULL );
		STOP_TIMER(TIME_BITBLT, "Bitblt")

		ASSERT(ddrval == DD_OK);
	}

	// validate the client rectangle
	ValidateRect(NULL);
	
	// Do not call CView::OnPaint() for painting messages
}

int CSpaceView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// initialize the direct-draw routines
	InitDDraw();

		// create a timer
 	UINT m_nTimerID = SetTimer(7, 10, NULL);
	ASSERT(m_nTimerID != 0);

	return 0;
}

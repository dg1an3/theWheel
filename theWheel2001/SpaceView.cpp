////////////////////////////////////
// Copyright (C) 1996-2000 DG Lane
// U.S. Patent Pending
////////////////////////////////////

// SpaceView.cpp : implementation of the CSpaceView class
//

#include "stdafx.h"
#include "theWheel2001.h"

#include "Space.h"
#include "SpaceView.h"
#include "NodeView.h"

#include "NewNodeDlg.h"

#include <math.h>

#include "LookupFunction.h"

int nNodeID = 1100;
const SPV_STATE_TYPE TOLERANCE = (SPV_STATE_TYPE) 0.7;
const SPV_STATE_TYPE TOTAL_ACTIVATION = (SPV_STATE_TYPE) 0.35f;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CSpaceView

IMPLEMENT_DYNCREATE(CSpaceView, CView)

BEGIN_MESSAGE_MAP(CSpaceView, CView)
	//{{AFX_MSG_MAP(CSpaceView)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_NEW_NODE, OnNewNode)
	ON_COMMAND(ID_VIEW_LAYOUT, OnViewLayout)
	ON_COMMAND(ID_VIEW_PROPAGATE, OnViewPropagate)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROPAGATE, OnUpdateViewPropagate)
	ON_COMMAND(ID_VIEW_WAVE, OnViewWave)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WAVE, OnUpdateViewWave)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpaceView construction/destruction

CSpaceView::CSpaceView()
: isPropagating(TRUE),
	isWaveMode(FALSE)
{
	m_pEnergyFunc = new CSpaceViewEnergyFunction(this);

#ifdef USE_GRAD
	m_pOptimizer = new CConjGradOptimizer<SPV_STATE_DIM, SPV_STATE_TYPE>(m_pEnergyFunc);
#else
	m_pOptimizer = new CPowellOptimizer<SPV_STATE_DIM, SPV_STATE_TYPE>(m_pEnergyFunc);
#endif

	// set the tolerance
	m_pOptimizer->tolerance.Set(TOLERANCE);
}

CSpaceView::~CSpaceView()
{
	delete m_pOptimizer;
	delete m_pEnergyFunc;
}

BOOL CSpaceView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	// cs.style |= WS_CLIPCHILDREN;	// TODO: why does this cause
									// an exception under Win98?

	return CView::PreCreateWindow(cs);
}

void CSpaceView::AddNodeToSpace(CNode *pNewNode)
{
	// compute the total weight (for determining initial link weights)
	double totalWeight = 0.0;
	int nAt;
	for (nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		CNodeView *pView = nodeViews.Get(nAt);
		totalWeight += pView->forNode->activation.Get();
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
		double weight = pView->forNode->activation.Get() / totalWeight;

		// establish the link
		pNewNode->LinkTo(pView->forNode.Get(), (float) weight);

		// is this a potential parent?
		if (weight > maxWeight)
		{
			maxWeight = (float) weight;
			pParentNode = pView->forNode.Get();
			ptInit = pView->center.Get();
		}
	}

	// add the new node to the parent
	pParentNode->children.Add(pNewNode);

	// construct a new node view for this node
	CNodeView *pNewNodeView = new CNodeView(pNewNode);
	pNewNodeView->isWaveMode.SyncTo(&isWaveMode);

	// set the activation for the new node
	pNewNodeView->forNode->activation.Set(maxWeight * 0.25f);

	// compute the initial rectangle for the node view
	CRect rect(ptInit.x - 100, ptInit.y - 75, 
		ptInit.x + 100, ptInit.y + 75);

	// set the style for the node view
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS; 

	// create it
	if (!pNewNodeView->CreateEx(WS_EX_TRANSPARENT, NULL, NULL, dwStyle, 
			rect, this, nNodeID++))
	{
	}

	// and add to the array
	nodeViews.Add(pNewNodeView);
	ActivateNode(pNewNodeView, (float) 1.6);

	// and lay them out
	LayoutNodeViews();

	// update all other views
	GetDocument()->UpdateAllViews(this);
}

CNodeView *CSpaceView::GetViewForNode(CNode *pNode)
{
	int nAt = 0;
	for (nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		CNodeView *pView = nodeViews.Get(nAt);

		if (pView->forNode.Get() == pNode)
			return pView;
	}

	return NULL;
}

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

	CenterNodeViews();
}

void CSpaceView::ActivateNode(CNodeView *pNodeView, float scale)
{
	// first, increase the activation of the node up to the max (from OnLMouseButton)
	double oldActivation = pNodeView->forNode->activation.Get();
	double newActivation = oldActivation 
		+ (TOTAL_ACTIVATION - oldActivation) * scale;
	pNodeView->forNode->activation.Set(newActivation);

	// now, propagate the activation
	pNodeView->forNode->PropagateActivation(0.8);
	GetDocument()->rootNode.ResetForPropagation();

	// normalize the nodes
	GetDocument()->NormalizeNodes();

	// sort the node views
	SortNodeViews();

	// compute the activation threshold, and set the thresholded activation on 
	//		all node views

	// form the number of currently visualized node views
	int nNumVizNodeViews = min(nodeViews.GetSize(), STATE_DIM / 2);
	double activationThreshold = 
		nodeViews.Get(nNumVizNodeViews - 1)->forNode->activation.Get();

	// compute the normalization factor for super-threshold node views
	double superThresholdSum = 0.0;
	for (int nAt = 0; nAt < nNumVizNodeViews; nAt++)
	{
		superThresholdSum += nodeViews.Get(nAt)->forNode->activation.Get();
	}
	double superThresholdScale = TOTAL_ACTIVATION / superThresholdSum;

	// now set the thresholdedActivation for all node views
	for (nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		CNodeView *pNodeView = nodeViews.Get(nAt);
		double activation = pNodeView->forNode->activation.Get();

		// for super-threshold nodes,
		if (activation > activationThreshold)
		{
			// scale so that super-thresholded activations are (roughly)
			//		normalized
			activation *= superThresholdScale;

			// now set the thresholded activation
			pNodeView->thresholdedActivation.Set( (float)
				activation * (float) superThresholdScale);
		}
		else
		{
			// otherwise, the thresholded activation is zero
			pNodeView->thresholdedActivation.Set(0.0);
		}
	}
}

void CSpaceView::CreateNodeViews(CNode *pParentNode, CPoint pt)
{
	// construct a new node view for this node
	CNodeView *pNewNodeView = new CNodeView(pParentNode);

	// synchronize the node view's wave mode flag
	pNewNodeView->isWaveMode.SyncTo(&isWaveMode);

	// compute the initial rectangle for the node view
	CRect rect(pt.x - 10, pt.y - 5, pt.x + 10, pt.y + 5);

	// create it with the style for a node view
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS; 
	if (!pNewNodeView->CreateEx(WS_EX_TRANSPARENT, NULL, NULL, dwStyle, 
			rect, this, nNodeID++))
	{
		TRACE("Failed to create node view\n");
	}

	// and add to the array
	nodeViews.Add(pNewNodeView);

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
	for (nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		CNodeView *pView = nodeViews.Get(nAt);
		double scaleFactor = 10000.0 * pView->forNode->activation.Get();
		vMeanCenter = vMeanCenter 
			+ pView->center.Get() * scaleFactor; 
		totalScaleFactor += scaleFactor;
	}

	// divide by number of node views
	vMeanCenter *= 1.0 / totalScaleFactor;

	// compute the vector offset for the node views
	//		window center
	CRect rectWnd;
	GetWindowRect(&rectWnd);
	CVector<2> vOffset = vMeanCenter 
		- CVector<2>(rectWnd.Width() / 2, rectWnd.Height() / 2);

	// offset each node view by the difference between the mean and the 
	//		window center
	for (nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		CNodeView *pView = nodeViews.Get(nAt);
		CVector<2> vNewCenter = pView->center.Get() - vOffset;
		pView->center.Set(vNewCenter);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSpaceView drawing

void CSpaceView::OnDraw(CDC* pDC)
{
#ifdef NO_NODEVIEWS
	return;
#endif

	CSpace* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// create new node views
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < nodeViews.GetSize(); nAtNodeView++)
	{
		CWnd *pWnd = nodeViews.Get(nAtNodeView);

		CRgn rgn;
		rgn.CreateRectRgn(0, 0, 1, 1);
		pWnd->GetWindowRgn(rgn);

		CRect rect;
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		rgn.OffsetRgn(rect.TopLeft());

		pDC->SelectClipRgn(&rgn, RGN_DIFF);

		rgn.DeleteObject();
	}

	CBrush brush;
	brush.CreateSolidBrush(RGB(192, 192, 192));

	CRect rect;
	GetClientRect(&rect);
	pDC->FillRect(&rect, &brush);
}

void CSpaceView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

#if defined(_DEBUG)
	GetDocument()->Dump(afxDump);
#endif

	// create a timer
	UINT m_nTimerID = SetTimer(7, 10, NULL);
	ASSERT(m_nTimerID != 0);

	// delete any old node views
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < nodeViews.GetSize(); nAtNodeView++)
		nodeViews.Get(nAtNodeView)->DestroyWindow();
	nodeViews.RemoveAll();

	// create the child node views
	CRect rect;
	GetClientRect(&rect);
	CreateNodeViews(&GetDocument()->rootNode, rect.CenterPoint());

	// now snap the node views to the parameter values
	for (nAtNodeView = 0; nAtNodeView < nodeViews.GetSize(); nAtNodeView++)
	{
		CNodeView *pNodeView = (CNodeView *)nodeViews.Get(nAtNodeView);
		pNodeView->springActivation.Set(pNodeView->thresholdedActivation.Get());
		pNodeView->springCenter.Set(pNodeView->center.Get());
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSpaceView printing

BOOL CSpaceView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CSpaceView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

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

/////////////////////////////////////////////////////////////////////////////
// CSpaceView message handlers

void CSpaceView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	//TODO: add code to react to the user changing the view style of your window
}

void CSpaceView::OnDestroy() 
{
	CView::OnDestroy();
	
	// delete any old node views
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < nodeViews.GetSize(); nAtNodeView++)
		nodeViews.Get(nAtNodeView)->DestroyWindow();
	nodeViews.RemoveAll();
}

BOOL CSpaceView::OnEraseBkgnd(CDC* pDC) 
{
#ifdef NO_NODEVIEWS
	return CView::OnEraseBkgnd(pDC);
#endif

	// create new node views
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < nodeViews.GetSize(); nAtNodeView++)
	{
		CWnd *pWnd = nodeViews.Get(nAtNodeView);

		CRgn rgn;
		rgn.CreateRectRgn(0, 0, 1, 1);
		pWnd->GetWindowRgn(rgn);

		CRect rect;
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		rgn.OffsetRgn(rect.TopLeft());

		pDC->SelectClipRgn(&rgn, RGN_DIFF);

		rgn.DeleteObject();
	}

	CBrush brush;
	brush.CreateSolidBrush(RGB(192, 192, 192));

	CRect rect;
	GetClientRect(&rect);
	pDC->FillRect(&rect, &brush);
	
	return TRUE; // CView::OnEraseBkgnd(pDC);
}

void CSpaceView::OnViewLayout() 
{
	// layout the node views
	LayoutNodeViews();	

	// redraw the window
	RedrawWindow();
}

void CSpaceView::OnViewPropagate() 
{
	isPropagating.Set(!isPropagating.Get());
}

void CSpaceView::OnUpdateViewPropagate(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(isPropagating.Get() ? 1 : 0);	
}

void CSpaceView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	ClientToScreen(&point);
	CMenu *pMenu = new CMenu();
	pMenu->LoadMenu(IDR_SPACE_POPUP);
	pMenu->GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);

	delete pMenu;

	CView::OnRButtonDown(nFlags, point);
}

void CSpaceView::OnNewNode() 
{
	CNewNodeDlg newDlg(this);

	if (newDlg.DoModal() == IDOK)
	{
		CNode *pNewNode = 
			new CNode(newDlg.m_strName, newDlg.m_strDesc);
		AddNodeToSpace(pNewNode);
	}
}

void CSpaceView::OnViewWave() 
{
	isWaveMode.Set(!isWaveMode.Get());	
}

void CSpaceView::OnUpdateViewWave(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(isWaveMode.Get() ? 1 : 0);		
}

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

			if (pThisNodeView->forNode->activation.Get() < pNextNodeView->forNode->activation.Get())
			{
				nodeViews.Set(nAt, pNextNodeView);
				nodeViews.Set(nAt+1, pThisNodeView);
				bRearrange = TRUE;
			}
		}
	} while (bRearrange);
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
			vState[nAt*2] = (STATE_TYPE) pView->center.Get()[0];
			vState[nAt*2+1] = (STATE_TYPE) pView->center.Get()[1];
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
		CNodeView *pView = nodeViews.Get(nAt);
		pView->center.Set(CVector<2>(vState[nAt*2], vState[nAt*2+1]));
	}
}

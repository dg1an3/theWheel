// SpaceView.cpp : implementation of the CSpaceView class
//

#include "stdafx.h"
#include "theWheel2001.h"

#include "Space.h"
#include "SpaceView.h"
#include "NodeView.h"

#include "NewNodeDlg.h"

#include <math.h>

int nNodeID = 1100;
const SPV_STATE_TYPE TOLERANCE = 1.0;
const SPV_STATE_TYPE TOTAL_ACTIVATION = 0.35f;

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
	m_pEnergyFunc = new CSpaceViewEnergyFunction();
	m_pEnergyFunc->m_pView = this;

#ifdef USE_GRAD
	m_pOptimizer = new CConjGradOptimizer<SPV_STATE_DIM, double>(m_pEnergyFunc);
#else
	m_pOptimizer = new CPowellOptimizer<SPV_STATE_DIM, double>(m_pEnergyFunc);
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
	float totalWeight = 0.0f;
	int nAt;
	for (nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		CNodeView *pView = nodeViews.Get(nAt);
		totalWeight += pView->activation.Get();
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
		float weight = pView->activation.Get() / totalWeight;

		// establish the link
		pNewNode->LinkTo(pView->forNode.Get(), weight);

		// is this a potential parent?
		if (weight > maxWeight)
		{
			maxWeight = weight;
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
	pNewNodeView->activation.Set(maxWeight * 0.25f);

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

	// normalize all the node views
	NormalizeNodeViews();

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
	CVector<SPV_STATE_DIM, SPV_STATE_TYPE> vCurrent = 
		m_pEnergyFunc->GetStateVector();

	// now optimize
	vCurrent = m_pOptimizer->Optimize(vCurrent);

	// now perform the optimization
	m_pEnergyFunc->SetStateVector(vCurrent);

	CenterNodeViews();
}

void CSpaceView::NormalizeNodeViews()
{
	// only normalize if there are children
	if (nodeViews.GetSize() == 0)
		return;

	float sum = 0.0;

	int nAt = 0;
	for (nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		CNodeView *pView = nodeViews.Get(nAt);

		// only include super-threshold nodes in the normalization sum
		if (pView->activation.Get() > CNodeView::activationThreshold)
			sum += pView->activation.Get();
	}

	for (nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		CNodeView *pView = nodeViews.Get(nAt);
		float newActivation = pView->activation.Get() * (float) TOTAL_ACTIVATION / sum;
		pView->activation.Set(newActivation);
	}
}

void CSpaceView::PropagateActivation(CNodeView *pSource, float percent, float factor)
{
	if (!isPropagating.Get())
		return;

	CNodeView *pMaxDest;
	do
	{
		float maxWeight = 0.0f;
		pMaxDest = NULL;
		CNodeLink *pMaxLink = NULL;
		for (int nAt = 0; nAt < nodeViews.GetSize(); nAt++)
		{
			CNodeView *pDest = nodeViews.Get(nAt);
			if (pDest != pSource)
			{
				// determine the weight linking this
				CNodeLink *pLink = pSource->forNode->GetLink(pDest->forNode.Get());
				if ((pLink != NULL) && (!pLink->m_bPropagated) 
						&& (pLink->weight.Get() > maxWeight))
				{
					maxWeight = pLink->weight.Get();
					pMaxDest = pDest;
					pMaxLink = pLink;
				}
			}
		} 

		if (pMaxDest != NULL)
		{
			pMaxLink->m_bPropagated = TRUE;

			// compute the new activation
			float oldActivation = pMaxDest->activation.Get();
			if (oldActivation < percent * maxWeight)
			{
				float newActivation = CNodeView::ActivationCurve(oldActivation * factor,
					percent * maxWeight);
				pMaxDest->activation.Set(newActivation);
				PropagateActivation(pMaxDest, newActivation);
			}
		}

	} while (pMaxDest != NULL);
}

void CSpaceView::ResetForPropagation()
{
	for (int nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		CNodeView *pView = nodeViews.Get(nAt);
		for (int nAt = 0; nAt < pView->forNode->links.GetSize(); nAt++)
		{
			CNodeLink *pLink = pView->forNode->links.Get(nAt);
			pLink->m_bPropagated = FALSE;
		}
	}
}

void CSpaceView::CreateNodeViews(CNode *pParentNode, CPoint pt, 
								 float initActivation)
{
	if (pParentNode->children.GetSize() == 0)
		return;

	CArray<CNodeView *, CNodeView *&> arrNodeViews;

	// create the child node views
	int nAtNode;
	for (nAtNode = 0; nAtNode < pParentNode->children.GetSize(); nAtNode++)
	{
		CNode *pNode = (CNode *) pParentNode->children.Get(nAtNode);

		// construct a new node view for this node
		CNodeView *pNewNodeView = new CNodeView(pNode);
		pNewNodeView->isWaveMode.SyncTo(&isWaveMode);
		arrNodeViews.Add(pNewNodeView);

		// compute the initial rectangle for the node view
		CRect rect(pt.x - 100, pt.y - 75, pt.x + 100, pt.y + 75);

		// set the style for the node view
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS; 

		// create it
		if (!pNewNodeView->CreateEx(WS_EX_TRANSPARENT, NULL, NULL, dwStyle, 
				rect, this, nNodeID++))
		{
			TRACE("Failed to create node view\n");
		}

		// and add to the array
		nodeViews.Add(pNewNodeView);

		// compute the initial weight
		float weight = 1.0f;
		if (pParentNode)
			weight = pParentNode->GetLinkWeight(pNode) + 0.01f;

		// set the activation for the new node
		pNewNodeView->activation.Set(initActivation * weight);
	}

	// now layout the child node views
	NormalizeNodeViews();

	// layout the child node views
	LayoutNodeViews();

	// and finally, let each child create its child node views
	// now place the children
	for (nAtNode = 0; nAtNode < pParentNode->children.GetSize(); nAtNode++)
	{
		CNode *pNode = (CNode *) pParentNode->children.Get(nAtNode);

		// construct a new node view for this node
		CPoint ptCenter = arrNodeViews[nAtNode]->center.Get();

		CreateNodeViews(pNode, ptCenter, 
			arrNodeViews[nAtNode]->activation.Get() / 3.0f);
	}
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
		double scaleFactor = 10000.0 * pView->activation.Get();
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

	// delete any old node views
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < nodeViews.GetSize(); nAtNodeView++)
		nodeViews.Get(nAtNodeView)->DestroyWindow();
	nodeViews.RemoveAll();

	// create the child node views
	CRect rect;
	GetClientRect(&rect);
	CreateNodeViews(&GetDocument()->rootNode, rect.CenterPoint(), 
		(float) TOTAL_ACTIVATION 
			/ (float) (GetDocument()->rootNode.children.GetSize()));

	// now normalize all the newly created node views
	NormalizeNodeViews();

	// and finally, lay them out
	LayoutNodeViews();
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

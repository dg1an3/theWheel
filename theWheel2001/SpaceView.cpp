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

// the displayed model object
#include <Space.h>

// child node views
#include "NodeView.h"

// the new node dialog
#include "EditNodeDlg.h"


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
	(SPV_STATE_TYPE) 0.55f;

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
	isWaveMode(FALSE)
{
	// create the energy function
	m_pEnergyFunc = new CSpaceViewEnergyFunction(this);

	// create the optimizer
#ifdef USE_GRAD
	m_pOptimizer = new CConjGradOptimizer<SPV_STATE_DIM, SPV_STATE_TYPE>(m_pEnergyFunc);
#else
	m_pOptimizer = new CPowellOptimizer<SPV_STATE_DIM, SPV_STATE_TYPE>(m_pEnergyFunc);
#endif

	// set the tolerance
	m_pOptimizer->tolerance.Set(TOLERANCE);
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

//////////////////////////////////////////////////////////////////////
// CSpaceView::OnDraw
// 
// draws the individual node views, using a back-buffer
//////////////////////////////////////////////////////////////////////
void CSpaceView::OnDraw(CDC* pDC)
{
	// create a memory device context
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);

	// get the inner rectangle for drawing the text
	CRect rectClient;
	GetClientRect(&rectClient);

	// Draw the image to the back-buffer
	CBitmap bitmapBuffer;
	bitmapBuffer.CreateBitmap(rectClient.Width(), rectClient.Height(), 1, 32, NULL); 
	dcMem.SelectObject(&bitmapBuffer);

	CBrush brush;
	brush.CreateSolidBrush(RGB(192, 192, 192));
	dcMem.FillRect(&rectClient, &brush);

	// create new node views
	int nAtNodeView;
	for (nAtNodeView = nodeViews.GetSize()-1; nAtNodeView >= 0; nAtNodeView--)
	{
		CNodeView *pNodeView = (CNodeView *)nodeViews.Get(nAtNodeView);

		if (pNodeView->GetThresholdedActivation() > 0.0)
		{
			for (int nAtLink = 0; nAtLink < pNodeView->forNode->links.GetSize(); nAtLink++)
			{
				CNodeLink *pLink = pNodeView->forNode->links.Get(nAtLink);
				CNodeView *pLinkedView = GetViewForNode(pLink->forTarget.Get());
				if (pLinkedView->GetThresholdedActivation() > 0.0)
				{
					// draw the link
					dcMem.MoveTo(pNodeView->GetSpringCenter());
					dcMem.LineTo(pLinkedView->GetSpringCenter());
				}
			}
		}
	} 

	// create new node views
	for (nAtNodeView = nodeViews.GetSize()-1; nAtNodeView >= 0; nAtNodeView--)
	{
		CNodeView *pNodeView = (CNodeView *)nodeViews.Get(nAtNodeView);
		pNodeView->Draw(&dcMem);
	} 

	// Now blit the backbuffer to the screen
	pDC->BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dcMem, 0, 0, SRCCOPY);

	// clean up
	dcMem.DeleteDC();
	bitmapBuffer.DeleteObject();
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

	// create a timer
 	UINT m_nTimerID = SetTimer(7, 10, NULL);
	ASSERT(m_nTimerID != 0);

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
		CNodeView *pView = nodeViews.Get(nAt);
		pView->SetCenter(CVector<2>(vState[nAt*2], vState[nAt*2+1]));
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
			new CNode(newDlg.m_strName, newDlg.m_strDesc);
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
		if (pNodeView->GetShape().PtInRegion(point))
		{
			// set the hand pointer cursor
			::SetCursor(::LoadCursor(GetModuleHandle(NULL), 
				MAKEINTRESOURCE(IDC_HANDPOINT)));

			CView::OnMouseMove(nFlags, point);

			return;
		}
	}

	// set the standard cursor
	::SetCursor(::LoadCursor(NULL, IDC_ARROW));

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
	// search throught the node views
	for (int nAt = 0; nAt < nodeViews.GetSize(); nAt++)
	{
		// get the current node view
		CNodeView *pNodeView = nodeViews.Get(nAt);

		// see if the mouse-click was within it
		if (pNodeView->GetShape().PtInRegion(point))
		{
			// if so, activate it
			ActivateNode(pNodeView, 0.25);
			break;
		}
	}
	
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


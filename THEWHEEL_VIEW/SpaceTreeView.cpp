//////////////////////////////////////////////////////////////////////
// SpaceTreeView.cpp: implementation of the CSpaceTreeView class.
//
// Copyright (C) 1996-2002
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// resource includes
#include "THEWHEEL_VIEW_resource.h"

// class definition
#include "SpaceTreeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// constants for the window's timer
//////////////////////////////////////////////////////////////////////
const int TIMER_ELAPSED = 200;	// ms per tick
const int TIMER_ID = 7;			// luck 7


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::CSpaceTreeView
// 
// constructs a new CSpaceTreeView object 
//////////////////////////////////////////////////////////////////////
CSpaceTreeView::CSpaceTreeView()
	: m_bDragging(FALSE),
		m_pDragImageList(NULL),
		m_pPendingCurNode(NULL)
{
}	// CSpaceTreeView::CSpaceTreeView


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::~CSpaceTreeView
// 
// destroys the CSpaceView object 
//////////////////////////////////////////////////////////////////////
CSpaceTreeView::~CSpaceTreeView()
{
}	// CSpaceTreeView::~CSpaceTreeView


//////////////////////////////////////////////////////////////////////
// implements the dynamic creation mechanism for the CSpaceView
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CSpaceTreeView, CView)


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::SelectNode
// 
// selects a node as the current item in the tree
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::SelectNode(CNode *pNode)
{
	HTREEITEM hItem;
	if (m_mapItemHandles.Lookup(pNode, hItem))
	{
		m_wndTree.Select(hItem, TVGN_CARET);
		m_wndTree.SelectDropTarget(hItem);
	}

}	// CSpaceTreeView::SelectNode


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::EnablePropertyPage
// 
// enables property pages
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::EnablePropertyPage(BOOL bEnable)
{
	// set the visibility of the properties window
	m_wndSpaceProperties.ShowWindow(bEnable ? SW_SHOW : SW_HIDE);
	m_wndNodeProperties.ShowWindow(bEnable ? SW_SHOW : SW_HIDE);

	// resize to the same size, to trigger formatting of subwindows
	CRect rect;
	GetClientRect(&rect);
	OnSize(SIZE_RESTORED, rect.Width(), rect.Height());

}	// CSpaceTreeView::EnablePropertyPage


/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView drawing

//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnDraw
// 
// special drawing for the tree view
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnDraw(CDC* pDC)
{
	CSpace* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

}	// CSpaceTreeView::OnDraw


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnInitialUpdate
// 
// initialization of the tree view
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	// clear out all items to re-populate the tree
	m_wndTree.DeleteAllItems();
	m_mapItemHandles.RemoveAll();

	// re-populate the tree
	AddNodeItem(GetDocument()->GetRootNode());

}	// CSpaceTreeView::OnInitialUpdate


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnUpdate
// 
// updates the tree view when an update occurs
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (NULL == GetDocument() || !pHint)
	{
		return;
	}

	// initialize to root node, if no node is sent as hint
	CNode *pNode = (CNode *) pHint;
	ASSERT(pNode->IsKindOf(RUNTIME_CLASS(CNode)));

	// don't add the item if it has no name
	if ("" == pNode->GetName())
	{
		return;
	}

	// first check to see if an item exists for this node
	HTREEITEM hItem;
	BOOL bResult = m_mapItemHandles.Lookup(pNode, hItem);
	if (!bResult)
	{
		// add the node to the tree ctrl
		AddNodeItem(pNode);

		// and return
		return;
	}

	// the item is present, so now check the label
	CString strName = m_wndTree.GetItemText(hItem);
	if (strName != pNode->GetName())
	{
		// change text
		m_wndTree.SetItemText(hItem, pNode->GetName());
	}
	
	// update the property dialogs with the currently selected node
	HTREEITEM hSelItem = m_wndTree.GetSelectedItem();
	if (NULL != hSelItem)
	{
		CNode *pSelNode = (CNode *) m_wndTree.GetItemData(hSelItem);
		ASSERT(pSelNode->IsKindOf(RUNTIME_CLASS(CNode)));

		// check to see if the pending node is
		if (pNode == pSelNode)
		{
			// set this node as the pending current node, to be
			//		selected during the next timer tick
			m_pPendingCurNode = pNode;
		}
	}

}	// CSpaceTreeView::OnUpdate


/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView printing

//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnPreparePrinting
// 
// prepares for printing the CSpaceTreeView
//////////////////////////////////////////////////////////////////////
BOOL CSpaceTreeView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);

}	// CSpaceTreeView::OnPreparePrinting


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnBeginPrinting
// 
// begins printing the CSpaceTreeView
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing

}	// CSpaceTreeView::OnBeginPrinting


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnEndPrinting
// 
// finishes printing the CSpaceTreeView
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing

}	// CSpaceTreeView::OnEndPrinting


/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView diagnostics

#ifdef _DEBUG
void CSpaceTreeView::AssertValid() const
{
	CView::AssertValid();
}

void CSpaceTreeView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSpace* CSpaceTreeView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSpace)));
	return (CSpace*)m_pDocument;
}
#endif //_DEBUG


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::AddNodeItem
//
// adds a node and its children to the tree
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::AddNodeItem(CNode *pNode, HTREEITEM hParentItem)
{
    // only if the parent is not NULL,
    if ((hParentItem == NULL) && (pNode->GetParent() != NULL))
    {
		// look up the parent tree item's handle
		if (!m_mapItemHandles.Lookup(pNode->GetParent(), hParentItem))
		{
			hParentItem = NULL;
		}
    }

	// see if the parent is still NULL
	if (hParentItem == NULL)
	{
		// set to the space item
		hParentItem = TVI_ROOT;
	}

    // create the tree control
    HTREEITEM hTreeItem = m_wndTree.InsertItem(
		TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_PARAM,
    	pNode->GetName(), 
		hParentItem == TVI_ROOT ? m_nSpaceImageIndex : m_nNodeImageIndex,
		hParentItem == TVI_ROOT ? m_nSpaceImageIndex : m_nNodeImageIndex,
    	INDEXTOSTATEIMAGEMASK(1) | TVIS_EXPANDED,	// nState, 
    	TVIS_STATEIMAGEMASK | 0xFF,					// nStateMask, 
    	(LPARAM) pNode,								// lParam, 
    	hParentItem, 
		TVI_FIRST);

    // add the inserted item to the handle map
    m_mapItemHandles.SetAt(pNode, hTreeItem);

	// only add children if node has name
	if ("" != pNode->GetName())
	{
		// add the children to the tree
		for (int nAt = pNode->GetChildCount()-1; nAt >= 0 ; nAt--)
		{
			CNode *pChild = (CNode *)pNode->GetChildAt(nAt);
			AddNodeItem(pChild, hTreeItem);
		}
	}

}	// CSpaceTreeView::AddNodeItem


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::RemoveNodeItem
//
// removes a node and its children from the tree
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::RemoveNodeItem(CNode *pNode)
{
	HTREEITEM hItem;

	// look up the parent tree item's handle
	BOOL bResult = m_mapItemHandles.Lookup(pNode, hItem);
	if (bResult)
	{
		// remove the child nodes
		for (int nAt = 0; nAt < pNode->GetChildCount(); nAt++)
		{
			CNode *pChild = (CNode *)pNode->GetChildAt(nAt);
			RemoveNodeItem(pChild);
		}

		// delete the item
		m_wndTree.DeleteItem(hItem);

		// and remove the key from the map
		m_mapItemHandles.RemoveKey(pNode);

	}

}	// CSpaceTreeView::RemoveNodeItem


/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView Message Map
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSpaceTreeView, CView)
	//{{AFX_MSG_MAP(CSpaceTreeView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_SPACETREE, OnEndLabelEdit)
	ON_NOTIFY(TVN_BEGINDRAG, IDC_SPACETREE, OnBeginDrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_NOTIFY(TVN_SELCHANGED, IDC_SPACETREE, OnSelChanged)
	ON_NOTIFY(TCN_SELCHANGE, IDC_SPACEPROPERTIES, OnSpaceTabChanged)
	ON_NOTIFY(TCN_SELCHANGE, IDC_NODEPROPERTIES, OnNodeTabChanged)
	ON_COMMAND(ID_NEW_NODE, OnNewNode)
	ON_COMMAND(ID_DELETENODE, OnDeletenode)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView message handlers

//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnCreate
// 
// creates the space tree view and child views
//////////////////////////////////////////////////////////////////////
int CSpaceTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// create the tree control
	if (!m_wndTree.Create(WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_HASBUTTONS 
		| TVS_LINESATROOT | TVS_EDITLABELS, CRect(0, 0, 100, 100),
		this, IDC_SPACETREE))
	{
		return -1;
	}

	// initialize the tree control's menu
	m_wndTree.SetMenu(IDR_SPACE_POPUP, this);

	// create the space properties tab control
	if (!m_wndSpaceProperties.Create(WS_VISIBLE | WS_CHILD, CRect(0, 0, 100, 100),
		this, IDC_SPACEPROPERTIES))
	{
		return -1;
	}

	// insert the space property tabs
	m_wndSpaceProperties.InsertItem(0, "Space");
	m_wndSpaceProperties.InsertItem(1, "Optimize");
	m_wndSpaceProperties.InsertItem(2, "Classes");

	// create the space property pages
	if (!m_dlgSpaceProp.Create(IDD_SPACEPROP, &m_wndSpaceProperties)
		|| !m_dlgSpaceOpt.Create(IDD_SPACEOPTPROP, &m_wndSpaceProperties)
		|| !m_dlgSpaceClass.Create(IDD_SPACECLASSPROP, &m_wndSpaceProperties))
	{
		return -1;
	}

	// create the node properties tab control
	if (!m_wndNodeProperties.Create(WS_VISIBLE | WS_CHILD, CRect(0, 0, 100, 100),
		this, IDC_NODEPROPERTIES))
	{
		return -1;
	}

	// insert the node property tabs
	m_wndNodeProperties.InsertItem(0, "Node");
	m_wndNodeProperties.InsertItem(1, "Image");
	m_wndNodeProperties.InsertItem(2, "Links");

	// create the node property pages
	if (!m_dlgNodeProp.Create(IDD_NODEPROP, &m_wndNodeProperties)
		|| !m_dlgImageProp.Create(IDD_IMAGEPROP, &m_wndNodeProperties)
		|| !m_dlgLinkProp.Create(IDD_LINKPROP, &m_wndNodeProperties))
	{
		return -1;
	}

 	// set up the tree control image list
 	m_imageList.Create(16, 16, ILC_MASK, 6, 4);
 	
 	// add bitmap to the list
 	CBitmap bitmap;
	BOOL bResult = bitmap.LoadBitmap(IDB_SPACE);
	m_nSpaceImageIndex = m_imageList.Add(&bitmap, (COLORREF)0xFFFFFF);
	bitmap.DeleteObject();

 	bResult = bitmap.LoadBitmap(IDB_NODE);
 	m_nNodeImageIndex = m_imageList.Add(&bitmap, (COLORREF)0xFFFFFF);
 	bitmap.DeleteObject();
 
	// set the image list into the tree control
 	m_wndTree.SetImageList(&m_imageList, TVSIL_NORMAL);

    // intialize the drop and no-drop cursors
    m_hDropCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
    m_hNoDropCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_NO));

	return 0;

}	// CSpaceTreeView::OnCreate


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnSize
// 
// on resize, resize the property pages
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnSize(UINT nType, int cx, int cy) 
{
	if (m_wndSpaceProperties.IsWindowVisible()
		|| m_wndNodeProperties.IsWindowVisible())
	{
		m_wndTree.MoveWindow(0, 0, cx, cy - 250);
	}
	else
	{
		m_wndTree.MoveWindow(0, 0, cx, cy);
	}

	// move the space property tab control
	m_wndSpaceProperties.MoveWindow(0, cy - 250, cx, cy);

	CRect rect;
	m_wndSpaceProperties.GetClientRect(&rect);
	m_wndSpaceProperties.AdjustRect(FALSE, rect);

	// move the space property pages
	m_dlgSpaceProp.MoveWindow(rect);
	m_dlgSpaceOpt.MoveWindow(rect);
	m_dlgSpaceClass.MoveWindow(rect);

	// move the node property tab control
	m_wndNodeProperties.MoveWindow(0, cy - 250, cx, cy);

	m_wndNodeProperties.GetClientRect(&rect);
	m_wndNodeProperties.AdjustRect(FALSE, rect);

	// move the node property pagess
	m_dlgNodeProp.MoveWindow(rect);
	m_dlgImageProp.MoveWindow(rect);
	m_dlgLinkProp.MoveWindow(rect);

}	// CSpaceTreeView::OnSize


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnSelChanged
// 
// called when a new node is selected in the tree control
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	// get the selected node
	CNode *pSelNode = (CNode *)pNMTreeView->itemNew.lParam;

	// clear drop target
	m_wndTree.SelectDropTarget(NULL);

	// if it is the root node,
	if (pSelNode == GetDocument()->GetRootNode())
	{
		// show the space properties
		m_wndSpaceProperties.ShowWindow(SW_SHOW);
		m_wndNodeProperties.ShowWindow(SW_HIDE);
	}
	else if (pSelNode)
	{
		// else show the node properties
		m_wndSpaceProperties.ShowWindow(SW_HIDE);
		m_wndNodeProperties.ShowWindow(SW_SHOW);

		// and select the node
		m_dlgNodeProp.SetCurNode(pSelNode);
		m_dlgImageProp.SetCurNode(pSelNode);
		m_dlgLinkProp.SetCurNode(pSelNode);
	}

	*pResult = 0;

}	// CSpaceTreeView::OnSelChanged


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnEndLabelEdit
// 
// change to a node's title
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	// if the item text is non-NULL, then a valid edit was completed
    if (pTVDispInfo->item.pszText != NULL)
    {
		// retrieve a pointer to the CNode being edited
		CNode *pNode = 
			(CNode *)m_wndTree.GetItemData(pTVDispInfo->item.hItem);

		// set the node's name
		pNode->SetName(pTVDispInfo->item.pszText);
    }

	// return TRUE to accept the edit
	*pResult = TRUE;

}	// CSpaceTreeView::OnEndLabelEdit


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnBeginDrag
// 
// begin dragging a node to a new parent
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	// get the drag item handle
    HTREEITEM hDragItem = pNMTreeView->itemNew.hItem;
	ASSERT(hDragItem != NULL);

	// select the item to be dragged
	m_wndTree.SelectItem(hDragItem);

	// get the dragged node
	m_pNodeDrag = (CNode *)m_wndTree.GetItemData(hDragItem);
	ASSERT(m_pNodeDrag != NULL);

	// no drop target yet
    m_pNodeDrop = NULL;

	// initialize the drag image list
    ASSERT(m_pDragImageList == NULL);
    m_pDragImageList = m_wndTree.CreateDragImage(hDragItem);  // get the image list for dragging
    m_pDragImageList->DragShowNolock(TRUE);
    m_pDragImageList->SetDragCursorImage(0, CPoint(0, 0));
    m_pDragImageList->BeginDrag(0, CPoint(0, 0));

	// initial drag position
	CPoint ptAction;
	::GetCursorPos(&ptAction);
	ScreenToClient(&ptAction);
    m_pDragImageList->DragMove(ptAction);
    m_pDragImageList->DragEnter(this, ptAction);

	// capture mouse motion
    SetCapture();

    // set up the timer for auto scrolling
    m_nScrollTimerID = SetTimer(TIMER_ID, TIMER_ELAPSED, NULL);

	// set the dragging flag
    m_bDragging = TRUE;

	*pResult = 0;

}	// CSpaceTreeView::OnBeginDrag


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnMouseMove
// 
// process dragging
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnMouseMove(UINT nFlags, CPoint point) 
{
    if (m_bDragging)
    {
    	// move the dragged image
    	ASSERT(m_pDragImageList != NULL);
    	m_pDragImageList->DragMove(point);

    	// see if there is a potential drop target
    	HTREEITEM hDropItem = m_wndTree.HitTest(point);
    	if (hDropItem != NULL)
    	{
			// get the item data
			m_pNodeDrop = (CNode *)m_wndTree.GetItemData(hDropItem);
			ASSERT(m_pNodeDrop != NULL);

    		// clear the dragging images
    		m_pDragImageList->DragLeave(this);

    		// unselect the current drop target
    		m_wndTree.SelectDropTarget(NULL);

    		// droppable if the drop target is not a child of the
			//		dragged item
			CNode *pParent = m_pNodeDrop;
			while ((pParent != NULL) && (pParent != m_pNodeDrag))
			{
				pParent = pParent->GetParent();
			}

			// see if a cycle was found
			if (pParent != NULL)
			{ 
				hDropItem = NULL;
				m_pNodeDrop = NULL;
			}
		
    		// set the appropriate cursor
	   		::SetCursor((hDropItem != NULL) ? m_hDropCursor : m_hNoDropCursor);

    		// select the drop target
    		m_wndTree.SelectDropTarget(hDropItem);

    		// restore the dragging images
    		m_pDragImageList->DragEnter(this, point);
    	}
    	
    }
	
	CView::OnMouseMove(nFlags, point);

}	// CSpaceTreeView::OnMouseMove


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnLButtonUp
// 
// end dragging
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnLButtonUp(UINT nFlags, CPoint point) 
{
    // are we ending a drag?
    if (m_bDragging)
    {
    	// finish up the drag image
    	ASSERT(m_pDragImageList != NULL);
    	m_pDragImageList->DragLeave(this);
    	m_pDragImageList->EndDrag();
    	delete m_pDragImageList;
    	m_pDragImageList = NULL;

    	// check to make sure a self-drop isn't about to occur and that 
    	//		the dropped-upon item is present and willing to accept
    	if (m_pNodeDrop != NULL)
    	{
    		// do the re-parenting

			// remove the tree items for the dragged item
			RemoveNodeItem(m_pNodeDrag);

			// set the parent on the dragged item
			m_pNodeDrag->SetParent(m_pNodeDrop);

			// add the dragged item back to the tree control
			AddNodeItem(m_pNodeDrag);
    	}
    	else
    	{
    		// otherwise, beep to indicate that an invalid drop target
    		//		was selected
    		MessageBeep(0);
    	}

    	// not dragging any longer, so release the cursor and de-select
    	//		the target
    	m_bDragging = FALSE;
    	ReleaseCapture();
    	m_wndTree.SelectDropTarget(NULL);

    	// kill the scrolling timer
    	KillTimer(m_nScrollTimerID);
    }
    	
	CView::OnLButtonUp(nFlags, point);

}	// CSpaceTreeView::OnLButtonUp


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnTimer
// 
// scrolling during drag
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnTimer(UINT nIDEvent) 
{
    // check the ID to see if it is the scrolling timer
    if (nIDEvent != m_nScrollTimerID)
    {
    	CView::OnTimer(nIDEvent);
    	return;
    }

    // get the "current" mouse position
    const MSG* pMessage;
    CPoint ptMouse;
    pMessage = GetCurrentMessage();
    ASSERT ( pMessage );
    ptMouse = pMessage->pt;
    ScreenToClient(&ptMouse);

    // move the drag image based on the current mouse position
    m_pDragImageList->DragMove(ptMouse);

    // holds the new drop target after the scroll
    HTREEITEM hDropItem = NULL;

    // get the tree controls rectangle for comparison with the mouse position
    CRect rect;
    m_wndTree.GetClientRect(&rect);

    // see if the mouse is outside the tree control rectangle, indicating
    //		that scrolling is needed
    if (ptMouse.y <= 0) 
    {
    	// reset the drag cursor
    	m_pDragImageList->DragLeave(this);

    	// scroll the tree control up
    	m_wndTree.SendMessage(WM_VSCROLL, SB_LINEUP);

    	// the new drop target is already selected as the first visible item
    	hDropItem = m_wndTree.GetFirstVisibleItem();
    }
    else if (ptMouse.y >= rect.Height())
    {
    	// reset the drag cursor
    	m_pDragImageList->DragLeave(this);

    	// scroll the tree control down	
    	m_wndTree.SendMessage(WM_VSCROLL, SB_LINEDOWN);

    	// set the new drop target as the last visible item
    	hDropItem = m_wndTree.GetFirstVisibleItem();
    	for (int nAtItem = 0; 
				nAtItem < (int) m_wndTree.GetVisibleCount() - 1; nAtItem++)
    	{
    		hDropItem = m_wndTree.GetNextVisibleItem(hDropItem);
    	}
    }
    else
    {
    	CView::OnTimer(nIDEvent);
    	return;
    }

    // select the new drop target
    m_wndTree.SelectDropTarget(hDropItem);

    // find the CObjectTreeItem for the new drop target
    m_pNodeDrop = (CNode *)m_wndTree.GetItemData(hDropItem);

    // redraw the drag cursor
    m_pDragImageList->DragEnter(this, ptMouse);
    
	// check to see if the pending node is
	if (NULL != m_pPendingCurNode)
	{
		m_dlgNodeProp.SetCurNode(m_pPendingCurNode);
		m_dlgLinkProp.SetCurNode(m_pPendingCurNode);
		m_dlgImageProp.SetCurNode(m_pPendingCurNode);

		// clear the pending current node
		m_pPendingCurNode = NULL;
	}

	// base class handler
	CView::OnTimer(nIDEvent);

}	// CSpaceTreeView::OnTimer


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnSpaceTabChanged
// 
// tab change for space properties
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnSpaceTabChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int nSelection = m_wndSpaceProperties.GetCurSel();

	// determine the property page to show
	m_dlgSpaceProp.ShowWindow(nSelection == 0 ? SW_SHOW : SW_HIDE);
	m_dlgSpaceOpt.ShowWindow(nSelection == 1 ? SW_SHOW : SW_HIDE);
	m_dlgSpaceClass.ShowWindow(nSelection == 2 ? SW_SHOW : SW_HIDE);

	*pResult = 0;

}	// CSpaceTreeView::OnSpaceTabChanged


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnNodeTabChanged
// 
// tab change for node properties
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnNodeTabChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int nSelection = m_wndNodeProperties.GetCurSel();

	m_dlgNodeProp.ShowWindow(nSelection == 0 ? SW_SHOW : SW_HIDE);
	m_dlgImageProp.ShowWindow(nSelection == 1 ? SW_SHOW : SW_HIDE);
	m_dlgLinkProp.ShowWindow(nSelection == 2 ? SW_SHOW : SW_HIDE);

	*pResult = 0;

}	// CSpaceTreeView::OnNodeTabChanged


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnNewNode
// 
// menu handler for creating a new child node
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnNewNode() 
{
	// get the currently selected node
	HTREEITEM hSelItem = m_wndTree.GetSelectedItem();
	if (NULL != hSelItem)	
	{
		CNode *pParentNode = (CNode *) m_wndTree.GetItemData(hSelItem);
		ASSERT(pParentNode->IsKindOf(RUNTIME_CLASS(CNode)));

		// create the node and give it a default name
		CNode *pNewNode = new CNode();
		pNewNode->SetName("NewNode");

		// add the node to the space, with the given parent
		GetDocument()->AddNode(pNewNode, pParentNode);
	}

}	// CSpaceTreeView::OnNewNode


//////////////////////////////////////////////////////////////////////
// CSpaceTreeView::OnDeletenode
// 
// menu handler for deleting a node
//////////////////////////////////////////////////////////////////////
void CSpaceTreeView::OnDeletenode() 
{
	// get the currently selected node (the marked node for removal)
	HTREEITEM hSelItem = m_wndTree.GetSelectedItem();
	if (NULL != hSelItem)	
	{
		CNode *pMarkedNode = (CNode *) m_wndTree.GetItemData(hSelItem);
		ASSERT(pMarkedNode->IsKindOf(RUNTIME_CLASS(CNode)));

		// remove the node from the space
		GetDocument()->RemoveNode(pMarkedNode);

		// and remove from the tree control
		m_wndTree.DeleteItem(hSelItem);
	}	

}	// CSpaceTreeView::OnDeletenode


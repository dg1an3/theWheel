// SpaceTreeView.cpp : implementation of the CSpaceTreeView class
//

#include "stdafx.h"
#include "theWheel2001.h"

#include "Space.h"
#include "SpaceTreeView.h"

#include "MainFrm.h"

#include "SpaceView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView

IMPLEMENT_DYNCREATE(CSpaceTreeView, CTreeView)

BEGIN_MESSAGE_MAP(CSpaceTreeView, CTreeView)
	//{{AFX_MSG_MAP(CSpaceTreeView)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CTreeView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView construction/destruction

CSpaceTreeView::CSpaceTreeView()
	: m_bDragging(FALSE),
		m_pDragImageList(NULL)
{
	// TODO: add construction code here

}

CSpaceTreeView::~CSpaceTreeView()
{
}

BOOL CSpaceTreeView::PreCreateWindow(CREATESTRUCT& cs)
{
	// Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs; these are passed on to the tree ctrl
	cs.style |= TVS_HASLINES | TVS_HASBUTTONS 
		| TVS_LINESATROOT | TVS_EDITLABELS;

	return CTreeView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView drawing

void CSpaceTreeView::OnDraw(CDC* pDC)
{
	CSpace* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}


/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView printing

BOOL CSpaceTreeView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CSpaceTreeView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CSpaceTreeView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView diagnostics

#ifdef _DEBUG
void CSpaceTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CSpaceTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CSpace* CSpaceTreeView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSpace)));
	return (CSpace*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView message handlers

void CSpaceTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CNode *pNode = GetDocument()->GetRootNode();

	// if there is a hint, it is a CNode
	if (pHint)
	{
		pNode = (CNode *) pHint;
	}
	else
	{
		GetTreeCtrl().DeleteAllItems();
		m_mapItemHandles.RemoveAll();
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
	CString strName = GetTreeCtrl().GetItemText(hItem);
	if (strName != pNode->GetName())
	{
		// change text
		GetTreeCtrl().SetItemText(hItem, pNode->GetName());
	}
	
	// check to see if parent is correct
}


void CSpaceTreeView::AddNodeItem(CNode *pNode, HTREEITEM hParentItem)
{
    // only if the parent is not NULL,
    if ((hParentItem == NULL) && (pNode->GetParent() != NULL))
    {
		// look up the parent tree item's handle
		BOOL bResult = m_mapItemHandles.Lookup(pNode->GetParent(), hParentItem);
		if (!bResult)
			hParentItem = NULL;
    }

	// see if the parent is still NULL
	if (hParentItem == NULL)
	{
		// and set to root
		hParentItem = TVI_ROOT;
	}

    // create the tree control
    HTREEITEM hTreeItem = GetTreeCtrl().InsertItem(
		TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_PARAM,
    	pNode->GetName(), 
 		m_nNodeImageIndex,
 		m_nNodeImageIndex,
    	INDEXTOSTATEIMAGEMASK(1) | TVIS_EXPANDED,	// nState, 
    	TVIS_STATEIMAGEMASK | 0xFF,					// nStateMask, 
    	(LPARAM) pNode,								// lParam, 
    	hParentItem, 
		TVI_FIRST);

    // add the inserted item to the handle map
    m_mapItemHandles.SetAt(pNode, hTreeItem);

	// add the children to the tree
	for (int nAt = pNode->GetChildCount()-1; nAt >= 0 ; nAt--)
	{
		CNode *pChild = (CNode *)pNode->GetChildAt(nAt);
		AddNodeItem(pChild, hTreeItem);
	}

	// select the added item
	GetTreeCtrl().SelectItem(hTreeItem);
}

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
		GetTreeCtrl().DeleteItem(hItem);

		// and remove the key from the map
		m_mapItemHandles.RemoveKey(pNode);

	}
}

int CSpaceTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
 	// set up the image list
 	m_imageList.Create(16, 16, ILC_MASK, 6, 4);
 	
 	// add bitmap to the list
 	CBitmap bitmap;
 	bitmap.LoadBitmap(IDB_NODE);
 	m_nNodeImageIndex = m_imageList.Add(&bitmap, (COLORREF)0xFFFFFF);
 	bitmap.DeleteObject();
 
	// set the image list into the tree control
 	GetTreeCtrl().SetImageList(&m_imageList, TVSIL_NORMAL);

    // intialize the drop and no-drop cursors
    m_hDropCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
    m_hNoDropCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_NO));

	return 0;
}

void CSpaceTreeView::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	// if the item text is non-NULL, then a valid edit was completed
    if (pTVDispInfo->item.pszText != NULL)
    {
		TRACE1("New label = %s\n", pTVDispInfo->item.pszText);

		// retrieve a pointer to the CNode being edited
		CNode *pNode = 
			(CNode *)GetTreeCtrl().GetItemData(pTVDispInfo->item.hItem);

		// set the node's name
		pNode->SetName(pTVDispInfo->item.pszText);
    }

	// return TRUE to accept the edit
	*pResult = TRUE;
}

void CSpaceTreeView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	// get the drag item handle
    HTREEITEM hDragItem = pNMTreeView->itemNew.hItem;
	ASSERT(hDragItem != NULL);

	// select the item to be dragged
	GetTreeCtrl().SelectItem(hDragItem);

	// get the dragged node
	m_pNodeDrag = (CNode *)GetTreeCtrl().GetItemData(hDragItem);
	ASSERT(m_pNodeDrag != NULL);

	// no drop target yet
    m_pNodeDrop = NULL;

	// initialize the drag image list
    ASSERT(m_pDragImageList == NULL);
    m_pDragImageList = GetTreeCtrl().CreateDragImage(hDragItem);  // get the image list for dragging
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
    GetTreeCtrl().SetCapture();

    // set up the timer for auto scrolling
    m_nScrollTimerID = SetTimer(1, 200, NULL);

	// set the dragging flag
    m_bDragging = TRUE;

	*pResult = 0;
}

void CSpaceTreeView::OnMouseMove(UINT nFlags, CPoint point) 
{
    if (m_bDragging)
    {
    	// move the dragged image
    	ASSERT(m_pDragImageList != NULL);
    	m_pDragImageList->DragMove(point);

    	// see if there is a potential drop target
    	HTREEITEM hDropItem = GetTreeCtrl().HitTest(point);
    	if (hDropItem != NULL)
    	{
			// get the item data
			m_pNodeDrop = (CNode *)GetTreeCtrl().GetItemData(hDropItem);
			ASSERT(m_pNodeDrop != NULL);

    		// clear the dragging images
    		m_pDragImageList->DragLeave(this);

    		// unselect the current drop target
    		GetTreeCtrl().SelectDropTarget(NULL);

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
    		GetTreeCtrl().SelectDropTarget(hDropItem);

    		// restore the dragging images
    		m_pDragImageList->DragEnter(this, point);
    	}
    	
    }
	
	CTreeView::OnMouseMove(nFlags, point);
}

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
    	GetTreeCtrl().SelectDropTarget(NULL);

    	// kill the scrolling timer
    	KillTimer(m_nScrollTimerID);
    }
    	
	CTreeView::OnLButtonUp(nFlags, point);
}

void CSpaceTreeView::OnTimer(UINT nIDEvent) 
{
    // check the ID to see if it is the scrolling timer
    if (nIDEvent != m_nScrollTimerID)
    {
    	CTreeView::OnTimer(nIDEvent);
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
    GetTreeCtrl().GetClientRect(&rect);

    // see if the mouse is outside the tree control rectangle, indicating
    //		that scrolling is needed
    if (ptMouse.y <= 0) 
    {
    	// reset the drag cursor
    	m_pDragImageList->DragLeave(this);

    	// scroll the tree control up
    	GetTreeCtrl().SendMessage(WM_VSCROLL, SB_LINEUP);

    	// the new drop target is already selected as the first visible item
    	hDropItem = GetTreeCtrl().GetFirstVisibleItem();
    }
    else if (ptMouse.y >= rect.Height())
    {
    	// reset the drag cursor
    	m_pDragImageList->DragLeave(this);

    	// scroll the tree control down	
    	GetTreeCtrl().SendMessage(WM_VSCROLL, SB_LINEDOWN);

    	// set the new drop target as the last visible item
    	hDropItem = GetTreeCtrl().GetFirstVisibleItem();
    	for (int nAtItem = 0; 
				nAtItem < (int) GetTreeCtrl().GetVisibleCount() - 1; nAtItem++)
    	{
    		hDropItem = GetTreeCtrl().GetNextVisibleItem(hDropItem);
    	}
    }
    else
    {
    	CTreeView::OnTimer(nIDEvent);
    	return;
    }

    // select the new drop target
    GetTreeCtrl().SelectDropTarget(hDropItem);

    // find the CObjectTreeItem for the new drop target
    m_pNodeDrop = (CNode *)GetTreeCtrl().GetItemData(hDropItem);

    // redraw the drag cursor
    m_pDragImageList->DragEnter(this, ptMouse);
    
	CTreeView::OnTimer(nIDEvent);
}

void CSpaceTreeView::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	CMainFrame *pMainFrame = (CMainFrame *)::AfxGetMainWnd();
	CSpaceView *pSpaceView = pMainFrame->GetRightPane();
	
	CNode *pSelNode = (CNode *)pNMTreeView->itemNew.lParam;
	CNodeView *pSelNodeView = (CNodeView *)pSelNode->GetView();

	if (pSelNodeView)
	{
		pSpaceView->ActivateNodeView(pSelNodeView, 0.1f);
	}

	*pResult = 0;
}

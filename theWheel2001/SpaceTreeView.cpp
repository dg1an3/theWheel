// SpaceTreeView.cpp : implementation of the CSpaceTreeView class
//

#include "stdafx.h"
#include "theWheel2001.h"

#include "Space.h"
#include "SpaceTreeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView

IMPLEMENT_DYNCREATE(CSpaceTreeView, CView)

BEGIN_MESSAGE_MAP(CSpaceTreeView, CView)
	//{{AFX_MSG_MAP(CSpaceTreeView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView construction/destruction

CSpaceTreeView::CSpaceTreeView()
{
	// TODO: add construction code here

}

CSpaceTreeView::~CSpaceTreeView()
{
}

BOOL CSpaceTreeView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style |= TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT;

	return CView::PreCreateWindow(cs);
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

void CSpaceTreeView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

/*	GetTreeCtrl().DeleteAllItems();

	// set the space for the CView
	int nAt;
	for (nAt = 0; nAt < GetDocument()->GetNodeCount(); nAt++)
	{
		// insert each node into the tree
		GetTreeCtrl().InsertItem(GetDocument()->GetNode(nAt)->GetName(), 0, 0);
	} 
*/
}

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

/////////////////////////////////////////////////////////////////////////////
// CSpaceTreeView message handlers

void CSpaceTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (::IsWindow(GetTreeCtrl().m_hWnd))
		GetTreeCtrl().DeleteAllItems();

	CSpace *pSpace = (CSpace *)m_pDocument;
	ASSERT(m_pDocument == (CSpace*)m_pDocument);

	AddNodeItems(&GetDocument()->rootNode, NULL);

/*	// set the space for the CView
	int nAt;
	for (nAt = 0; nAt < GetDocument()->GetNodeCount(); nAt++)
	{
		// insert each node into the tree
		GetTreeCtrl().InsertItem(GetDocument()->GetNode(nAt)->GetName(), 0, 0);
	} */
}

void CSpaceTreeView::AddNodeItems(CNode *pNode, CObjectTreeItem *pParent)
{
	// set the space for the CView
	int nAt;
	for (nAt = 0; nAt < pNode->children.GetSize(); nAt++)
	{
		CNode *pChildNode = pNode->children.Get(nAt);
		CObjectTreeItem *pItem = new CObjectTreeItem();
		pItem->SetObject(pChildNode);
		pItem->SetParent(pParent);

		pItem->Create(&GetTreeCtrl());
		// insert each node into the tree
		// GetTreeCtrl().InsertItem(pItem);

		AddNodeItems(pChildNode, pItem);
	}
}

int CSpaceTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);

	if (!m_ObjectExplorer.Create(WS_VISIBLE | WS_CHILD 
		| TVS_HASLINES | TVS_LINESATROOT 
		| TVS_HASBUTTONS // | TVS_CHECKBOXES 
			| TVS_EDITLABELS, 
		rect, this, 105))
		return FALSE;
	
	return 0;
}

CObjectExplorer& CSpaceTreeView::GetTreeCtrl()
{
	return m_ObjectExplorer;
}

void CSpaceTreeView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);

	m_ObjectExplorer.MoveWindow(&rect);
}

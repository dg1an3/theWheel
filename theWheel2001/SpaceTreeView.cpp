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
	// Modify the Window class or styles here by modifying
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

void CSpaceTreeView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);

	m_ObjectExplorer.MoveWindow(&rect);
}

void CSpaceTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// delete any of the items that are present
	m_ObjectExplorer.DeleteAllItems();

	// get a pointer to the CSpace object
	CSpace *pSpace = (CSpace *)m_pDocument;
	ASSERT(m_pDocument == (CSpace*)m_pDocument);

	// get a pointer to the space's root node
	CNode *pRootNode = &pSpace->rootNode;

	// create the tree view items
	int nAt;
	for (nAt = 0; nAt < pRootNode->children.GetSize(); nAt++)
	{
		// create a new item object
		CObjectTreeItem *pItem = new CObjectTreeItem();

		// set the resource IDs for the new item object
		pItem->imageResourceID.Set(IDB_NODE);
		pItem->selectedImageResourceID.Set(IDB_NODE);

		// set the object (recursively creates children)
		pItem->forObject.Set(pRootNode->children.Get(nAt));

		// now create the Windows tree item
		pItem->Create(&m_ObjectExplorer);
	}
}


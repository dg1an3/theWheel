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

IMPLEMENT_DYNCREATE(CSpaceTreeView, CTreeView)

BEGIN_MESSAGE_MAP(CSpaceTreeView, CTreeView)
	//{{AFX_MSG_MAP(CSpaceTreeView)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CTreeView::OnFilePrintPreview)
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

void CSpaceTreeView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

/*	GetTreeCtrl().DeleteAllItems();

	// set the space for the CTreeVIew
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
	GetTreeCtrl().DeleteAllItems();

	CSpace *pSpace = (CSpace *)m_pDocument;
	ASSERT(m_pDocument == (CSpace*)m_pDocument);

	AddNodeItems(GetDocument()->rootNode.children, TVI_ROOT);

/*	// set the space for the CTreeVIew
	int nAt;
	for (nAt = 0; nAt < GetDocument()->GetNodeCount(); nAt++)
	{
		// insert each node into the tree
		GetTreeCtrl().InsertItem(GetDocument()->GetNode(nAt)->GetName(), 0, 0);
	} */
}

void CSpaceTreeView::AddNodeItems(CCollection<CNode>& arrNodes, HTREEITEM hParent)
{
	// set the space for the CTreeVIew
	int nAt;
	for (nAt = 0; nAt < arrNodes.GetSize(); nAt++)
	{
		CNode *pNode = arrNodes.Get(nAt); // (CNode *)arrNodes[nAt];

		// insert each node into the tree
		HTREEITEM hItem = 
			GetTreeCtrl().InsertItem(pNode->name.Get(), hParent, TVI_LAST);

		AddNodeItems(pNode->children, hItem);
	}
}

int CSpaceTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	return 0;
}

// theWheelView.cpp : implementation of the CtheWheelView class
//

#include "stdafx.h"
#include "theWheel.h"

#include "theWheelDoc.h"
#include "theWheelView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CtheWheelView

IMPLEMENT_DYNCREATE(CtheWheelView, CView)

BEGIN_MESSAGE_MAP(CtheWheelView, CView)
	// ON_COMMAND(ID_VIEW_DESIGNTIME, &CtheWheelView::OnViewDesigntime)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CtheWheelView construction/destruction

CtheWheelView::CtheWheelView()
{
	// TODO: add construction code here

}

CtheWheelView::~CtheWheelView()
{
}

BOOL CtheWheelView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CtheWheelView drawing

void CtheWheelView::OnDraw(CDC* /*pDC*/)
{
	CtheWheelDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


void CtheWheelView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	m_spaceView.SetSpace(&GetDocument()->m_space);
}


// CtheWheelView diagnostics

#ifdef _DEBUG
void CtheWheelView::AssertValid() const
{
	CView::AssertValid();
}

void CtheWheelView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CtheWheelDoc* CtheWheelView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CtheWheelDoc)));
	return (CtheWheelDoc*)m_pDocument;
}
#endif //_DEBUG


// CtheWheelView message handlers


void CtheWheelView::OnViewDesigntime()
{
	// TODO: Add your command handler code here
}

int CtheWheelView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rect;
	GetClientRect(&rect);
	if (!m_spaceView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, 0))
		return -1;

	return 0;
}

void CtheWheelView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	m_spaceView.MoveWindow(0, 0, cx, cy);
}

// LeftView.cpp : implementation of the CLeftView class
//

#include "stdafx.h"
#include "theWheel.h"
// #include "resource.h"

#include "theWheelDoc.h"
#include "LeftView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLeftView

IMPLEMENT_DYNCREATE(CLeftView, CView)

BEGIN_MESSAGE_MAP(CLeftView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CLeftView construction/destruction

CLeftView::CLeftView()
{
	// TODO: add construction code here
}

CLeftView::~CLeftView()
{
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

void CLeftView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_spaceTreeView.SetSpace(&GetDocument()->m_space);
}


// CLeftView diagnostics

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CView::AssertValid();
}

void CLeftView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CtheWheelDoc* CLeftView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CtheWheelDoc)));
	return (CtheWheelDoc*)m_pDocument;
}
#endif //_DEBUG


// CLeftView message handlers


int CLeftView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rect;
	GetClientRect(&rect);
	if (!m_spaceTreeView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, 0))
		return -1;

	return 0;
}

void CLeftView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	m_spaceTreeView.MoveWindow(0, 0, cx, cy);
}


void CLeftView::OnDraw(CDC* /*pDC*/)
{
	// TODO: Add your specialized code here and/or call the base class
}

// SpaceAndBrowserView.cpp : implementation file
//

#include "stdafx.h"
#include "thewheel2001.h"
#include "SpaceAndBrowserView.h"

#include "SpaceView.h"
#include "SpaceBrowser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpaceAndBrowserView

IMPLEMENT_DYNCREATE(CSpaceAndBrowserView, CView)

CSpaceAndBrowserView::CSpaceAndBrowserView()
{
}

CSpaceAndBrowserView::~CSpaceAndBrowserView()
{
}


BEGIN_MESSAGE_MAP(CSpaceAndBrowserView, CView)
	//{{AFX_MSG_MAP(CSpaceAndBrowserView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpaceAndBrowserView drawing

void CSpaceAndBrowserView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CSpaceAndBrowserView diagnostics

#ifdef _DEBUG
void CSpaceAndBrowserView::AssertValid() const
{
	CView::AssertValid();
}

void CSpaceAndBrowserView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSpaceAndBrowserView message handlers

int CSpaceAndBrowserView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//Create a splitter window with 3 panes all in one column (3 rows).
	//The 8 added to AFX_IDW_PANE_FIRST below is pretty arbitrary:
	m_wndSplitter.CreateStatic(this, 2, 1, WS_CHILD|WS_VISIBLE, AFX_IDW_PANE_FIRST+8);
	CCreateContext *pContext = (CCreateContext*) lpCreateStruct->lpCreateParams;
	CSize size(0,0);
	m_wndSplitter.CreateView(0,0, RUNTIME_CLASS(CSpaceView), size, pContext);
	m_wndSplitter.CreateView(1,0, RUNTIME_CLASS(CWnd), // CSpaceBrowser), 
		size, pContext);	
	
	CSpaceView *pSpaceView = (CSpaceView *) m_wndSplitter.GetPane(0, 0);
	// CSpaceBrowser *pSpaceBrowser= (CSpaceBrowser *) m_wndSplitter.GetPane(1, 0);

	// cross link the space view and the browser
	// pSpaceView->m_pBrowser = pSpaceBrowser;
	// pSpaceBrowser->m_pSpaceView = pSpaceView;

	return 0;
}

void CSpaceAndBrowserView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	//Increase size by 2 all around to remove hide border:
	m_wndSplitter.MoveWindow(-2, -2, cx+2, cy+2);
	
	m_wndSplitter.SetRowInfo(0, cy/2, 0); 
	m_wndSplitter.SetRowInfo(1, cy/2, 0);
	m_wndSplitter.RecalcLayout(); 
}

void CSpaceAndBrowserView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	// CHtmlView *pHtmlView = (CHtmlView *)m_wndSplitter.GetPane(1, 0);
	// CSpaceView *pSpaceView = (CSpaceView *) m_wndSplitter.GetPane(0, 0);
	// CSpace *pSpace = pSpaceView->GetDocument();
	// CNode *pRootNode = pSpace->GetRootNode();
	// if (pRootNode->GetUrl())
	//	pHtmlView->Navigate2(pRootNode->GetUrl());
}

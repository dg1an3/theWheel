// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "theWheel.h"
// #include "resource.h"

#include "MainFrm.h"
#include "LeftView.h"
#include "theWheelView.h"
#include "theWheelDoc.h"
#include "SpaceTreeView.h"

#include <SpaceView.h>
#include <DesigntimeTracker.h>
#include <RuntimeTracker.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, &CFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, &CFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, &CFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, &CFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_VIEW_DESIGNTIME, &CMainFrame::OnViewDesigntime)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DESIGNTIME, &CMainFrame::OnUpdateViewDesigntime)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
: m_bCreatedClient(false)
, m_bRuntime(false)
{
	m_bRuntime = ::AfxGetApp()->GetProfileInt(_T("Settings"), _T("RunTime"), 1) == 0;
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	// create splitter window
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
		return FALSE;

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CLeftView), CSize(100, 100), pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CtheWheelView), CSize(100, 100), pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}

	// initialize the spaceview tracker
	SetRuntime(m_bRuntime);

	m_bCreatedClient = true;

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers


CtheWheelView * CMainFrame::GetRightPane()
{
	CWnd* pWnd = m_wndSplitter.GetPane(0, 1);
	CtheWheelView *pView = DYNAMIC_DOWNCAST(CtheWheelView, pWnd);
	return pView;
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (m_bCreatedClient)
	{
		if (GetRightPane()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		{
			return TRUE;
		}
	}

	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnViewDesigntime()
{
	SetRuntime(!m_bRuntime);
	::AfxGetApp()->WriteProfileInt(_T("Settings"), _T("RunTime"), 
		m_bRuntime ? 0 : 1);
}

void CMainFrame::OnUpdateViewDesigntime(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bRuntime ? 0 : 1);	
}

// sets the runtime flag
void CMainFrame::SetRuntime(bool bRuntime)
{
	m_bRuntime = bRuntime;

	CLeftView *pLeftView = (CLeftView *) m_wndSplitter.GetPane(0, 0);
	ASSERT(pLeftView->IsKindOf(RUNTIME_CLASS(CLeftView)));
	pLeftView->m_spaceTreeView.EnablePropertyPage(!m_bRuntime);

	CSpaceView *pSpaceView = &GetRightPane()->m_spaceView;
	ASSERT(pSpaceView->IsKindOf(RUNTIME_CLASS(CSpaceView)));

	if (m_bRuntime)
	{
		// create a run time tracker
		CRuntimeTracker *pTracker = new CRuntimeTracker(pSpaceView);

		// install the tracker on the space view
		pSpaceView->SetTracker(pTracker);
	}
	else
	{
		// create a design time tracker
		CDesigntimeTracker *pTracker = new CDesigntimeTracker(pSpaceView);

		// install the tracker on the space view
		pSpaceView->SetTracker(pTracker);
	} 
}



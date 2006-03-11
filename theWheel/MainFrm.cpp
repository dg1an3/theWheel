// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "theWheel.h"

#include <SpaceTreeView.h>
#include <SpaceView.h>
#include <DesigntimeTracker.h>
#include <RuntimeTracker.h>

#include "MainFrm.h"
#include "ScaleDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_HELP_FINDER, OnHelpFinder)
	ON_COMMAND(ID_RUNTIME, OnRuntime)
	ON_UPDATE_COMMAND_UI(ID_RUNTIME, OnUpdateRuntime)
	ON_COMMAND(ID_PERTURB, OnPerturb)
	//}}AFX_MSG_MAP
	// Global help commands
	// ON_COMMAND(ID_HELP_FINDER, CFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_SPACE_SCALELINKS, OnSpaceScalelinks)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
: m_bRuntime(FALSE)
{
	m_bRuntime = ::AfxGetApp()->GetProfileInt("Settings", "RunTime", 1) == 1;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// set the space view to use the application's module state
	CSpaceView::m_pModuleState = ::AfxGetAppModuleState();

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

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		| WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// create splitter window
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
		return FALSE;

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CSpaceTreeView), CSize(200, 100), pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CSpaceView), CSize(100, 100), pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}

	// initialize the spaceview tracker
	SetRuntime(m_bRuntime);

	return TRUE;
	
	// return CFrameWnd::OnCreateClient(lpcs, pContext);
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (!GetRightPane()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	{
		return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	}

	return TRUE;
}

CSpaceView * CMainFrame::GetRightPane()
{
	CWnd* pWnd = m_wndSplitter.GetPane(0, 1);
	return DYNAMIC_DOWNCAST(CSpaceView, pWnd);
}

void CMainFrame::OnHelpFinder() 
{
	HINSTANCE hInst = ::ShellExecute(GetSafeHwnd(), "open", 
		::AfxGetApp()->m_pszHelpFilePath, "", "", SW_SHOW);
}

void CMainFrame::OnRuntime() 
{
	SetRuntime(!m_bRuntime);
	::AfxGetApp()->WriteProfileInt("Settings", "RunTime", 
		m_bRuntime ? 1 : 0);

}

void CMainFrame::OnUpdateRuntime(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bRuntime ? 1 : 0);	
}

void CMainFrame::SetRuntime(BOOL bRuntime)
{
	m_bRuntime = bRuntime;

	CSpaceTreeView *pSpaceTreeView = (CSpaceTreeView *) m_wndSplitter.GetPane(0, 0);
	ASSERT(pSpaceTreeView->IsKindOf(RUNTIME_CLASS(CSpaceTreeView)));
	pSpaceTreeView->EnablePropertyPage(!m_bRuntime);

	CSpaceView *pSpaceView = (CSpaceView *) m_wndSplitter.GetPane(0, 1);
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

void CMainFrame::OnPerturb() 
{
	// CDialog dlg(IDD_PERTURBDIALOG, this);
	if (TRUE) // dlg.DoModal() == IDOK)
	{
/*		CEdit *pEditAmt = (CEdit *) dlg.GetDlgItem(IDC_EDIT_AMOUNT);

		CString strAmount;
		pEditAmt->GetWindowText(strAmount);
*/
		double amount = 0.10;
//		sscanf(strAmount.GetBuffer(100), "%lf", &amount);

		CSpace *pSpace = (CSpace *) GetActiveDocument();
		for (int nAt = 0; nAt < pSpace->GetNodeCount(); nAt++)
		{
			CNode *pNode = pSpace->GetNodeAt(nAt);

			for (int nAtLink = 0; nAtLink < pNode->GetLinkCount(); nAtLink++)
			{
				CNodeLink *pLink = pNode->GetLinkAt(nAtLink);

				double weight = pLink->GetWeight();
				Perturb(&weight, weight * amount);	

				pNode->LinkTo(pLink->GetTarget(), weight, FALSE);
			}
		}
	}
}

void CMainFrame::OnSpaceScalelinks()
{
	CScaleDialog dlg;
	// dlg.m_pSpace = ::GetCurrent
}

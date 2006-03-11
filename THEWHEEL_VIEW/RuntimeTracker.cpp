//////////////////////////////////////////////////////////////////////
// RuntimeTracker.cpp: implementation of the CRuntimeTracker class.
//
// Copyright (C) 2003 Derek G Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "stdafx.h"

// resource includes
#include "THEWHEEL_VIEW_resource.h"

// header for this class
#include "RuntimeTracker.h"

#include "SpaceView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CRuntimeTracker::CRuntimeTracker
// 
// constructs a runtime tracker
//////////////////////////////////////////////////////////////////////
CRuntimeTracker::CRuntimeTracker(CSpaceView *pView)
: CTracker(pView),
	m_pBrowser(NULL)
{
}	// CRuntimeTracker::CRuntimeTracker


//////////////////////////////////////////////////////////////////////
// CRuntimeTracker::~CRuntimeTracker
// 
// destroys the runtime tracker
//////////////////////////////////////////////////////////////////////
CRuntimeTracker::~CRuntimeTracker()
{
	if (m_pView->m_pMaximizedView)
	{
		m_pView->m_pMaximizedView->SetMaximized(FALSE);
		m_pView->m_pMaximizedView = NULL;
	}

	if (m_pBrowser)
	{
		m_pBrowser->Release();
	}
	m_pBrowser = NULL;

}	// CRuntimeTracker::~CRuntimeTracker


//////////////////////////////////////////////////////////////////////
// CRuntimeTracker::OnMouseMove
// 
// handles mouse move events
//////////////////////////////////////////////////////////////////////
void CRuntimeTracker::OnMouseMove(UINT nFlags, CPoint point)  
{ 
	// find the node view containing the point
	CNodeView *pSelectedView = m_pView->FindNodeViewAt(point);

	// initialize the pending activation for an on-node move
	REAL deltaAct = 0.6;

	// stores the string for the URL of the pointed-to view (if any
	CString strMessage;
	strMessage.LoadString(AFX_IDS_IDLEMESSAGE);

	// set the cursor based on whether the point is in a view
	if (pSelectedView != NULL) 
	{
		// set to a hand
		::SetCursor(::LoadCursor(GetModuleHandle(NULL), 
			MAKEINTRESOURCE(IDC_HANDPOINT)));

		if ("" != pSelectedView->GetNode()->GetUrl())
		{
			strMessage = pSelectedView->GetNode()->GetUrl();
		}
	}
	else
	{
		// find the nearest node view
		pSelectedView = m_pView->FindNearestNodeView(point);

		// set to lower activation
		deltaAct *= 0.5;

		// set to an arrow
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}

	CFrameWnd *pFrameWnd = (CFrameWnd *) ::AfxGetMainWnd();
	pFrameWnd->SetMessageText(strMessage);

	// activate the node view
	if (m_pView->m_pMaximizedView == NULL && pSelectedView != NULL)
	{
		pSelectedView->AddPendingActivation(deltaAct);
	}

}	// CRuntimeTracker::OnMouseMove


//////////////////////////////////////////////////////////////////////
// CRuntimeTracker::OnTimer
// 
// handles timer events
//////////////////////////////////////////////////////////////////////
void CRuntimeTracker::OnTimer(UINT nElapsed, CPoint point)
{
	CTracker::OnTimer(nElapsed, point);

	// find the node view containing the point
	CNodeView *pSelectedView = m_pView->FindNodeViewAt(point);
	if (m_pView->m_pMaximizedView == NULL && NULL != pSelectedView)
	{
		pSelectedView->AddPendingActivation((REAL) 0.4);
	}

}	// CRuntimeTracker::OnTimer


//////////////////////////////////////////////////////////////////////
// CRuntimeTracker::OnButtonDown
// 
// maximizes the selected node
//////////////////////////////////////////////////////////////////////
void CRuntimeTracker::OnButtonDown(UINT nFlags, CPoint point) 
{ 
	// find the node view containing the point
	CNodeView *pSelectedView = m_pView->FindNodeViewAt(point);

	if (m_pView->m_pMaximizedView)
	{
		m_pView->m_pMaximizedView->SetMaximized(FALSE);
	}

	if (pSelectedView != m_pView->m_pMaximizedView)
	{
		pSelectedView->SetMaximized(TRUE);

		m_pView->m_pMaximizedView = pSelectedView;
	}
	else
	{
		m_pView->m_pMaximizedView = NULL;
	}

}	// CRuntimeTracker::OnButtonDown


//////////////////////////////////////////////////////////////////////
// CRuntimeTracker::OnButtonDblClick
// 
// opens associated URL
//////////////////////////////////////////////////////////////////////
void CRuntimeTracker::OnButtonDblClick(UINT nFlags, CPoint point) 
{ 
	// find the node view containing the point
	CNodeView *pSelectedView = m_pView->FindNodeViewAt(point);

	// was a node view found?
	if (pSelectedView != NULL)
	{
		CString strUrl = pSelectedView->GetNode()->GetUrl();
		if (strUrl.Left(7).Compare(_T("http://")) == 0)
		{
			OpenUrl(strUrl);
		}
		else if (strUrl != "")
		{
			ShellExecute(::AfxGetMainWnd()->m_hWnd, NULL, 
				strUrl, NULL, NULL, SW_SHOWNORMAL);
		}
	}

}	// CRuntimeTracker::OnButtonDblClick


//////////////////////////////////////////////////////////////////////
// CRuntimeTracker::OpenUrl
// 
// opens a URL in a browser
//////////////////////////////////////////////////////////////////////
HRESULT CRuntimeTracker::OpenUrl(const CString &strUrl)
{
	// create the browser, if its not already there
	if (!m_pBrowser)
	{
		CR(::CoCreateInstance(CLSID_InternetExplorer,
			NULL, CLSCTX_SERVER, IID_IWebBrowser2, (void **) &m_pBrowser));
	}

	// try navigation
	VARIANT vFlags = {0};
	VARIANT vTargetFrameName = {0};
	VARIANT vPostData = {0};
	VARIANT vHeaders = {0};
	HRESULT hr = m_pBrowser->Navigate(_bstr_t(strUrl), 
			&vFlags, &vTargetFrameName, &vPostData, &vHeaders);

	// has the browser window been closed?
	if (RPC_E_DISCONNECTED == hr)
	{
		// free the browser
		if (m_pBrowser)
		{
			m_pBrowser->Release();
		}
		m_pBrowser = NULL;

		// try again
		OpenUrl(strUrl);
	}
	else if (FAILED(hr))
	{
		return hr;
	}

	// set the browser window to visible
	CR(m_pBrowser->put_Visible(_variant_t(true)));

	// and send to the top
	HWND hWnd = NULL;
	CR(m_pBrowser->get_HWND((long *) &hWnd));
	::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	return S_OK;

}	// CRuntimeTracker::OpenUrl


//////////////////////////////////////////////////////////////////////
// WebSpaceView.cpp: implementation of the CWebSpaceView class
//////////////////////////////////////////////////////////////////////

// WRL, which the WebView2 SDK is built on, requires NTDDI_VERSION to be at
//		least NTDDI_VISTA.  stdafx.h still targets XP for the rest of the
//		app, so this translation unit opts out of the precompiled header and
//		raises the target for itself alone.  only header-level API gating
//		depends on these, so mixing them across the binary is safe
#define WINVER			0x0600
#define _WIN32_WINNT	0x0600
#define NTDDI_VERSION	0x06000000

#define _AFX_ALL_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <afxwin.h>
#include <afxext.h>

// supplied by stdafx.h for every other translation unit; Node.h reaches
//		WAVE.h, which needs the multimedia and DirectSound types
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <dsound.h>

#include "resource.h"
#include "WebSpaceView.h"

#include <Space.h>
#include <Node.h>

// the SDK proper lives here, and nowhere else in the app
#include <wrl/client.h>
#include <wrl/event.h>
#include <WebView2.h>
#include <shlobj.h>

using namespace Microsoft::WRL;


//////////////////////////////////////////////////////////////////////
// frame push timer -- matches CSpaceView's 20ms tick
//////////////////////////////////////////////////////////////////////
const UINT WEB_TIMER_ID = 11;
const UINT WEB_TIMER_ELAPSED = 20;


//////////////////////////////////////////////////////////////////////
// the page.  kept inline rather than shipped as a file so the view has
//		no deployment story yet; it is navigated to with
//		NavigateToString.  the drawing is deliberately plain for now --
//		the elliptangle geometry has already been ported to TypeScript
//		in the modern repo (src/geometry/ElliptangleRenderer.ts) and is
//		the intended replacement for these ellipses
//////////////////////////////////////////////////////////////////////
static const char * const SPACE_HTML =
"<!DOCTYPE html><html><head><meta charset='utf-8'><style>"
"html,body{margin:0;padding:0;overflow:hidden;background:#739ECE;height:100%}"
"canvas{display:block}"
"#hud{position:fixed;left:8px;top:6px;font:12px/1.4 Segoe UI,sans-serif;"
"color:#fff;text-shadow:0 1px 2px rgba(0,0,0,.5);pointer-events:none}"
"</style></head><body>"
"<canvas id='c'></canvas><div id='hud'>waiting for frame...</div>"
"<script>\n"
"var cv=document.getElementById('c'),cx=cv.getContext('2d'),hud=document.getElementById('hud');\n"
"var frame={nodes:[],links:[]},frames=0,t0=performance.now();\n"
"function fit(){cv.width=window.innerWidth;cv.height=window.innerHeight;}\n"
"window.addEventListener('resize',fit);fit();\n"
"window.chrome.webview.addEventListener('message',function(e){frame=e.data;frames++;});\n"
"cv.addEventListener('click',function(e){\n"
"  var best=null,bd=1e9;\n"
"  for(var i=0;i<frame.nodes.length;i++){var n=frame.nodes[i];\n"
"    var dx=(e.clientX-n.x)/Math.max(n.w/2,1),dy=(e.clientY-n.y)/Math.max(n.h/2,1);\n"
"    var d=dx*dx+dy*dy; if(d<1&&d<bd){bd=d;best=n;}}\n"
"  if(best)window.chrome.webview.postMessage({type:'activate',id:best.id});\n"
"});\n"
"function draw(){\n"
"  cx.clearRect(0,0,cv.width,cv.height);\n"
"  var i,n;\n"
"  cx.strokeStyle='rgba(255,255,255,.35)';cx.lineWidth=2;\n"
"  for(i=0;i<frame.links.length;i++){var L=frame.links[i];\n"
"    cx.beginPath();cx.moveTo(L.x1,L.y1);cx.lineTo(L.x2,L.y2);cx.stroke();}\n"
"  for(i=0;i<frame.nodes.length;i++){n=frame.nodes[i];\n"
"    cx.save();cx.translate(n.x,n.y);\n"
"    cx.beginPath();cx.ellipse(0,0,Math.max(n.w/2,2),Math.max(n.h/2,2),0,0,Math.PI*2);\n"
"    cx.fillStyle=n.col;cx.fill();\n"
"    cx.lineWidth=1.5;cx.strokeStyle='rgba(0,0,0,.35)';cx.stroke();\n"
"    if(n.h>14){cx.fillStyle='#111';cx.textAlign='center';cx.textBaseline='middle';\n"
"      cx.font=Math.max(9,Math.min(20,n.h*0.28))+'px Segoe UI,sans-serif';\n"
"      cx.fillText(n.name,0,0);}\n"
"    cx.restore();}\n"
"  var el=(performance.now()-t0)/1000;\n"
"  hud.textContent=frame.nodes.length+' nodes  '+(frames/Math.max(el,.001)).toFixed(0)+' frames/s';\n"
"  requestAnimationFrame(draw);\n"
"}\n"
"requestAnimationFrame(draw);\n"
"window.chrome.webview.postMessage({type:'ready'});\n"
"</script></body></html>";


//////////////////////////////////////////////////////////////////////
// implements dynamic creation for the CWebSpaceView
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CWebSpaceView, CWnd)

BEGIN_MESSAGE_MAP(CWebSpaceView, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::CWebSpaceView
//////////////////////////////////////////////////////////////////////
CWebSpaceView::CWebSpaceView()
	: m_pSpace(NULL)
	, m_pController(NULL)
	, m_pWebView(NULL)
	, m_bReady(FALSE)
	, m_nTimerID(0)
{
	m_tokMessage.value = 0;
	m_tokNavigationCompleted.value = 0;

}	// CWebSpaceView::CWebSpaceView


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::~CWebSpaceView
//////////////////////////////////////////////////////////////////////
CWebSpaceView::~CWebSpaceView()
{
}	// CWebSpaceView::~CWebSpaceView


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::PostNcDestroy
//
// the popup is heap-allocated and owns itself
//////////////////////////////////////////////////////////////////////
void CWebSpaceView::PostNcDestroy()
{
	CWnd::PostNcDestroy();

	delete this;

}	// CWebSpaceView::PostNcDestroy


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::SetSpace
//////////////////////////////////////////////////////////////////////
void CWebSpaceView::SetSpace(CSpace *pSpace)
{
	m_pSpace = pSpace;

}	// CWebSpaceView::SetSpace


//////////////////////////////////////////////////////////////////////
// the single popup instance, so repeated menu invocations re-show the
//		existing window rather than stacking up browsers
//////////////////////////////////////////////////////////////////////
static CWebSpaceView *g_pPopup = NULL;


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::ShowPopup
//////////////////////////////////////////////////////////////////////
CWebSpaceView *CWebSpaceView::ShowPopup(CWnd *pParent, CSpace *pSpace)
{
	// re-use the existing window if it is still alive
	if (NULL != g_pPopup && ::IsWindow(g_pPopup->m_hWnd))
	{
		g_pPopup->SetSpace(pSpace);
		g_pPopup->ShowWindow(SW_SHOW);
		g_pPopup->SetForegroundWindow();
		return g_pPopup;
	}

	g_pPopup = new CWebSpaceView();
	g_pPopup->SetSpace(pSpace);

	LPCTSTR pszClass = ::AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
		::LoadCursor(NULL, IDC_ARROW), NULL, NULL);

	if (!g_pPopup->CreateEx(0, pszClass, _T("theWheel -- WebView2 Space"),
			WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			CRect(80, 80, 1080, 780), pParent, 0))
	{
		delete g_pPopup;
		g_pPopup = NULL;
		return NULL;
	}

	g_pPopup->ShowWindow(SW_SHOW);
	g_pPopup->UpdateWindow();

	return g_pPopup;

}	// CWebSpaceView::ShowPopup


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::OnCreate
//
// starts the asynchronous creation of the browser
//////////////////////////////////////////////////////////////////////
int CWebSpaceView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	// the browser needs somewhere to keep its profile; it must be
	//		writable, so use the app's local appdata
	TCHAR szLocal[MAX_PATH] = { 0 };
	::SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szLocal);

	CString strUserData;
	strUserData.Format(_T("%s\\theWheel\\WebView2"), szLocal);

	HRESULT hr = ::CreateCoreWebView2EnvironmentWithOptions(
		NULL,							// use the installed runtime
		CT2W(strUserData),
		NULL,
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[this](HRESULT hrEnv, ICoreWebView2Environment *pEnv) -> HRESULT
			{
				return OnEnvironmentCreated(hrEnv, pEnv);
			}).Get());

	if (FAILED(hr))
	{
		// most likely the WebView2 runtime is not installed
		TRACE(_T("CreateCoreWebView2EnvironmentWithOptions failed 0x%08lX\n"), hr);
	}

	m_nTimerID = SetTimer(WEB_TIMER_ID, WEB_TIMER_ELAPSED, NULL);

	return 0;

}	// CWebSpaceView::OnCreate


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::OnEnvironmentCreated
//////////////////////////////////////////////////////////////////////
HRESULT CWebSpaceView::OnEnvironmentCreated(HRESULT hr,
											ICoreWebView2Environment *pEnv)
{
	if (FAILED(hr) || NULL == pEnv || !::IsWindow(m_hWnd))
	{
		return hr;
	}

	return pEnv->CreateCoreWebView2Controller(m_hWnd,
		Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
			[this](HRESULT hrCtl, ICoreWebView2Controller *pCtl) -> HRESULT
			{
				return OnControllerCreated(hrCtl, pCtl);
			}).Get());

}	// CWebSpaceView::OnEnvironmentCreated


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::OnControllerCreated
//
// the browser exists from here on
//////////////////////////////////////////////////////////////////////
HRESULT CWebSpaceView::OnControllerCreated(HRESULT hr,
										   ICoreWebView2Controller *pCtl)
{
	if (FAILED(hr) || NULL == pCtl || !::IsWindow(m_hWnd))
	{
		return hr;
	}

	m_pController = pCtl;
	m_pController->AddRef();

	m_pController->get_CoreWebView2(&m_pWebView);
	if (NULL == m_pWebView)
	{
		return E_FAIL;
	}

	// this is a rendering surface, not a browser: drop the chrome
	ComPtr<ICoreWebView2Settings> pSettings;
	if (SUCCEEDED(m_pWebView->get_Settings(&pSettings)) && pSettings)
	{
		pSettings->put_AreDefaultContextMenusEnabled(FALSE);
		pSettings->put_IsZoomControlEnabled(FALSE);
		pSettings->put_AreDevToolsEnabled(TRUE);
		pSettings->put_IsStatusBarEnabled(FALSE);
	}

	// messages posted by the page
	m_pWebView->add_WebMessageReceived(
		Callback<ICoreWebView2WebMessageReceivedEventHandler>(
			[this](ICoreWebView2 *, ICoreWebView2WebMessageReceivedEventArgs *pArgs) -> HRESULT
			{
				LPWSTR pszJson = NULL;
				if (SUCCEEDED(pArgs->get_WebMessageAsJson(&pszJson)) && pszJson)
				{
					OnPageMessage(CStringA(CW2A(pszJson, CP_UTF8)));
					::CoTaskMemFree(pszJson);
				}
				return S_OK;
			}).Get(),
		&m_tokMessage);

	ResizeBrowser();

	return m_pWebView->NavigateToString(CA2W(SPACE_HTML, CP_UTF8));

}	// CWebSpaceView::OnControllerCreated


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::OnPageMessage
//////////////////////////////////////////////////////////////////////
void CWebSpaceView::OnPageMessage(const CStringA& strJson)
{
	// the page announces itself once its script is running
	if (strJson.Find("\"ready\"") >= 0)
	{
		m_bReady = TRUE;
		return;
	}

	// a node was clicked: activate it, exactly as CSpaceView would
	const int nAt = strJson.Find("\"id\":");
	if (nAt >= 0 && NULL != m_pSpace)
	{
		const int nIndex = atoi((LPCSTR) strJson + nAt + 5);
		if (nIndex >= 0 && nIndex < m_pSpace->GetNodeCount())
		{
			m_pSpace->ActivateNode(m_pSpace->GetNodeAt(nIndex), (REAL) 1.4);
		}
	}

}	// CWebSpaceView::OnPageMessage


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::BuildFrameJson
//
// serializes the visible nodes, in window coordinates
//////////////////////////////////////////////////////////////////////
CStringA CWebSpaceView::BuildFrameJson()
{
	CStringA strJson("{\"nodes\":[");

	if (NULL == m_pSpace)
	{
		return strJson + "],\"links\":[]}";
	}

	const int nCount = m_pSpace->GetSuperNodeCount();

	CStringA strItem;
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		CNode *pNode = m_pSpace->GetNodeAt(nAt);
		if (NULL == pNode)
		{
			continue;
		}

		const CVectorD<3> vPos = pNode->GetPosition();
		const REAL act = pNode->GetActivation();

		// size the plaque from the activation, the way the skin does
		const REAL w = (REAL) (40.0 + 320.0 * act);
		const REAL h = (REAL) (14.0 + 120.0 * act);

		// the class colour, falling back to the skin's default title
		COLORREF color = RGB(149, 205, 208);
		m_pSpace->GetClassColorMap().Lookup(pNode->GetClass(), color);

		CStringA strName(CT2A(pNode->GetName()));
		strName.Replace("\\", "\\\\");
		strName.Replace("\"", "\\\"");

		strItem.Format("%s{\"id\":%d,\"x\":%.1f,\"y\":%.1f,"
			"\"w\":%.1f,\"h\":%.1f,\"col\":\"#%02X%02X%02X\",\"name\":\"%s\"}",
			(nAt ? "," : ""), nAt,
			(double) vPos[0], (double) vPos[1], (double) w, (double) h,
			GetRValue(color), GetGValue(color), GetBValue(color),
			(LPCSTR) strName);

		strJson += strItem;
	}

	strJson += "],\"links\":[]}";

	return strJson;

}	// CWebSpaceView::BuildFrameJson


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::PostFrame
//////////////////////////////////////////////////////////////////////
void CWebSpaceView::PostFrame()
{
	if (!m_bReady || NULL == m_pWebView)
	{
		return;
	}

	m_pWebView->PostWebMessageAsJson(CA2W(BuildFrameJson(), CP_UTF8));

}	// CWebSpaceView::PostFrame


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::OnTimer
//////////////////////////////////////////////////////////////////////
void CWebSpaceView::OnTimer(UINT_PTR nIDEvent)
{
	if (WEB_TIMER_ID == nIDEvent)
	{
		// the layout is still driven by the model; this view only shows it
		if (NULL != m_pSpace)
		{
			m_pSpace->LayoutNodes();
		}

		PostFrame();
	}

	CWnd::OnTimer(nIDEvent);

}	// CWebSpaceView::OnTimer


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::ResizeBrowser
//////////////////////////////////////////////////////////////////////
void CWebSpaceView::ResizeBrowser()
{
	if (NULL == m_pController || !::IsWindow(m_hWnd))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(&rectClient);
	m_pController->put_Bounds(rectClient);

}	// CWebSpaceView::ResizeBrowser


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::OnSize
//////////////////////////////////////////////////////////////////////
void CWebSpaceView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	ResizeBrowser();

}	// CWebSpaceView::OnSize


//////////////////////////////////////////////////////////////////////
// CWebSpaceView::OnDestroy
//////////////////////////////////////////////////////////////////////
void CWebSpaceView::OnDestroy()
{
	if (m_nTimerID != 0)
	{
		KillTimer(WEB_TIMER_ID);
		m_nTimerID = 0;
	}

	m_bReady = FALSE;

	if (NULL != m_pWebView && m_tokMessage.value != 0)
	{
		m_pWebView->remove_WebMessageReceived(m_tokMessage);
		m_tokMessage.value = 0;
	}

	if (NULL != m_pController)
	{
		m_pController->Close();
	}

	if (NULL != m_pWebView)
	{
		m_pWebView->Release();
		m_pWebView = NULL;
	}

	if (NULL != m_pController)
	{
		m_pController->Release();
		m_pController = NULL;
	}

	// the popup owns itself; drop the cached pointer so the next menu
	//		invocation creates a fresh one
	if (g_pPopup == this)
	{
		g_pPopup = NULL;
	}

	CWnd::OnDestroy();

}	// CWebSpaceView::OnDestroy

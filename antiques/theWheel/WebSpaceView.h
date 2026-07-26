//////////////////////////////////////////////////////////////////////
// WebSpaceView.h: interface for the CWebSpaceView class
//
// hosts a WebView2 browser and renders the space in it, as an
//		alternative to the GDI CSpaceView
//////////////////////////////////////////////////////////////////////

#pragma once

#include <wrl/client.h>
#include <WebView2.h>


//////////////////////////////////////////////////////////////////////
// forward declaration of the CSpace class
//////////////////////////////////////////////////////////////////////
class CSpace;


//////////////////////////////////////////////////////////////////////
// class CWebSpaceView
//
// a child window hosting a WebView2 browser.  the node geometry is
//		computed here and pushed to the page as JSON; the page draws it
//		on a canvas and posts interaction back
//////////////////////////////////////////////////////////////////////
class CWebSpaceView : public CWnd
{
	DECLARE_DYNCREATE(CWebSpaceView)

public:
	CWebSpaceView();
	virtual ~CWebSpaceView();

// Attributes
public:
	// the space being displayed
	void SetSpace(CSpace *pSpace);
	CSpace *GetSpace() { return m_pSpace; }

	// TRUE once the browser is up and the document is loaded
	BOOL IsReady() const { return m_bReady; }

// Operations
public:
	// pushes the current node set to the page
	void PostFrame();

// Implementation
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()

private:
	// creation is asynchronous: the environment arrives first, then the
	//		controller, and only then can the browser be driven
	HRESULT OnEnvironmentCreated(HRESULT hr, ICoreWebView2Environment *pEnv);
	HRESULT OnControllerCreated(HRESULT hr, ICoreWebView2Controller *pCtl);

	// a message posted by the page, as JSON
	void OnPageMessage(const CStringA& strJson);

	// serializes the visible nodes for one frame
	CStringA BuildFrameJson();

	// keeps the browser filling the client area
	void ResizeBrowser();

	// the space being displayed
	CSpace *m_pSpace;

	Microsoft::WRL::ComPtr<ICoreWebView2Controller> m_pController;
	Microsoft::WRL::ComPtr<ICoreWebView2> m_pWebView;

	// set once the document has loaded and will accept frames
	BOOL m_bReady;

	// drives the frame push
	UINT_PTR m_nTimerID;

	EventRegistrationToken m_tokMessage;
	EventRegistrationToken m_tokNavigationCompleted;

};	// class CWebSpaceView

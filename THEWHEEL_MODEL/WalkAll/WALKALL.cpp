// =========================================================================== 
// File: W A L K A L L . C P P 
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE. 
// 
// Description: 
// 
// This sample demonstrates the use of MSHTML as a UI-less HTML parser. 
// To successfully walk the HTML Scripting Object Model that the parser 
// exposes after loading the specified document, the host application must 
// wait until MSHTML has finished loading the document. To track MSHTML's  
// READYSTATE, the host implements a simple COM object that exposes  
// IPropertyNotifySink. The host application connects to MSHTML using the  
// standard connection point interfaces. As MSHTML loads the document  
// it's readiness state changes. To notify the host of these changes,  
// it executes IPropertyNotifySink::OnChanged passing along the DISPID  
// of the property that has changed (DISPID_READYSTATE). The host uses MSHTML's  
// automation interface obtained at creation time to retrieve the current value  
// of this property. When the value equals READYSTATE_COMPLETE,  
// MSHTML has finished loading the document. 
 
// If the loaded document is an HTML page which contains scripts, Java Applets and/or  
// ActiveX controls, and those scripts are coded for immediate execution, MSHTML will 
// execute them by default. To disable this feature, the host must implement two  
// additional interfaces: IOleClientSite and IDispatch. At initialization time, after 
// MSHTML is instantiated, the host should query MSHTML for its IOleObject interface, 
// pass MSHTML a reference to its IOleClientSite interface, query MSHTML for its IOleControl 
// interface, and call OnAmbientPropertyChange(DISPID_AMBIENT_DLCONTROL). MSHTML will in  
// turn query the host's IOleClientSite interface for IDispatch and then request the value 
// of this property from the host. This is the hosts opportunity to control MSHTML, disabling 
// the execution of scripts, etc. A full list of control flags are available in <mshtmdid.h>. 
 
// Requirements: 
//   * Internet Explorer version 4.0 
//   * Microsoft Visual C++ 5.0. The code uses types (_bstr_t) introduced with VC5. 
//   * Be sure that the INETSDK include and lib directories are in your INCLUDE and LIB 
//     environment variables respectively. 
 
// Instructions: 
// 
//  To use this sample: 
//   * build it using the NMAKE command. NMAKE will create WALKALL.EXE. 
//   * run WALKALL.EXE specifying the resource to download by passing an 
//     URL on the command-line. use no command-line argument to default to 
//     "http://example.microsoft.com". 
//   * The program prints all HTML tags to the console. If an IMG tag is encountered 
//     the SRC attribute is displayed. 
// 
// Modification History 
// 30 May 1997 - Matt Oshry - created 
// 12 Jun 1997 - Matt Oshry - added minimal support for IOleClientSite and IDispatch 
// Copyright 1995-1997 Microsoft Corporation.  All Rights Reserved. 
// =========================================================================== 
 
#include <windows.h> 
#include <tchar.h> 
#include <wininet.h> 
#include <urlmon.h> 
#include <mshtml.h> 
#include <iostream.h> 
#include <comdef.h> 
#include <assert.h> 
 
#include <servprov.h> 
 
#include <mshtmdid.h> 
 
#define C_USAGE _T("Usage: walkall <URL>") 
 
#define ODS(x) OutputDebugString(x) 
 
#define WM_USER_STARTWALKING ((WM_USER) + 1) 
 
const TCHAR c_szDefaultUrl[] = _T("http://example.microsoft.com/"); 
 
INTERNET_SCHEME GetScheme(LPCTSTR szURL); 
 
void PrintDISPID(DISPID dispidMember); 
 
class CApp : public IPropertyNotifySink, IOleClientSite, IDispatch 
{ 
public: 
	CApp() : m_pMSHTML(NULL), 
		m_dwRef(1),  
		m_hrConnected(CONNECT_E_CANNOTCONNECT), 
		m_dwCookie(0), 
		m_pCP(NULL), 
		m_szURL(NULL), 
		m_nScheme(INTERNET_SCHEME_UNKNOWN), 
		m_lReadyState(READYSTATE_UNINITIALIZED) 
	{
	} 

	~CApp() 
	{ 
		if (m_pMSHTML) 
		{ 
			m_pMSHTML->Release(); 
		} 
	} 
 
// IUnknown methods 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv); 
    STDMETHOD_(ULONG, AddRef)(); 
    STDMETHOD_(ULONG, Release)();  
 
// IPropertyNotifySink methods 
    STDMETHOD(OnChanged)(DISPID dispID); 
    STDMETHOD(OnRequestEdit)(DISPID dispID); 
 
// IOleClientSite methods 
	STDMETHOD(SaveObject)(void) { ODS("SaveObject\n"); return E_NOTIMPL; } 
 
	STDMETHOD(GetMoniker)(DWORD dwAssign, 
				DWORD dwWhichMoniker, 
				IMoniker** ppmk) 
		{ ODS("GetMoniker\n"); return E_NOTIMPL; } 
 
	STDMETHOD(GetContainer)(IOleContainer** ppContainer) 
		{ ODS("GetContainer\n"); return E_NOTIMPL; } 
 
	STDMETHOD(ShowObject)(void) 
		{ ODS("ShowObject\n"); return E_NOTIMPL; } 
 
	STDMETHOD(OnShowWindow)(BOOL fShow) 
		{ ODS("OnShowWindow\n"); return E_NOTIMPL; } 
 
	STDMETHOD(RequestNewObjectLayout)(void) 
		{ ODS("RequestNewObjectLayout\n"); return E_NOTIMPL; } 
 
// IDispatch method 
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) 
		{ ODS("GetTypeInfoCount\n"); return E_NOTIMPL; } 
 
	STDMETHOD(GetTypeInfo)(UINT iTInfo, 
				LCID lcid, 
				ITypeInfo** ppTInfo) 
		{ ODS("GetTypeInfo\n"); return E_NOTIMPL; } 
 
	STDMETHOD(GetIDsOfNames)(REFIID riid, 
				LPOLESTR* rgszNames, 
				UINT cNames, 
				LCID lcid, 
				DISPID* rgDispId) 
		{ ODS("GetIDsOfNames\n"); return E_NOTIMPL; } 
         
	STDMETHOD(Invoke)(DISPID dispIdMember, 
				REFIID riid, 
				LCID lcid, 
				WORD wFlags, 
				DISPPARAMS __RPC_FAR *pDispParams, 
				VARIANT __RPC_FAR *pVarResult, 
				EXCEPINFO __RPC_FAR *pExcepInfo, 
				UINT __RPC_FAR *puArgErr); 
 
// Additional class methods 
	HRESULT Init(int argc, char* argv[]); 
	HRESULT Run(); 
	HRESULT Term(); 
	HRESULT Walk(); 
 
protected: 
	// Persistence helpers 
	HRESULT LoadURLFromFile(); 
	HRESULT LoadURLFromMoniker(); 
 
	IHTMLDocument2* m_pMSHTML; 
	DWORD m_dwRef; 
	DWORD m_dwCookie; 
	LPCONNECTIONPOINT m_pCP; 
	HRESULT m_hrConnected; 
	LPTSTR m_szURL; 
	INTERNET_SCHEME m_nScheme; 
	READYSTATE m_lReadyState; 
 
	IBinding* m_pBinding; 
}; 
 
STDMETHODIMP CApp::QueryInterface(REFIID riid, LPVOID* ppv) 
{ 
	*ppv = NULL; 
 
	if (IID_IUnknown == riid || IID_IPropertyNotifySink == riid) 
	{ 
		*ppv = (LPUNKNOWN)(IPropertyNotifySink*)this; 
		AddRef(); 
		return NOERROR; 
	} 
	else if (IID_IOleClientSite == riid) 
	{ 
		*ppv = (IOleClientSite*)this; 
		AddRef(); 
		return NOERROR; 
	} 
	else if (IID_IDispatch == riid) 
	{ 
		*ppv = (IDispatch*)this; 
		AddRef(); 
		return NOERROR; 
	} 
	else 
	{ 
		OLECHAR wszBuff[39]; 
		int i = StringFromGUID2(riid, wszBuff, 39); 
		TCHAR szBuff[39]; 
		i = WideCharToMultiByte(CP_ACP, 0, wszBuff, -1, szBuff, 39, NULL, NULL); 
		ODS("QI: "); ODS(szBuff); ODS("\n"); 
		return E_NOTIMPL; 
	} 
} 
 
STDMETHODIMP_(ULONG) CApp::AddRef() 
{ 
	TCHAR szBuff[255]; 
	wsprintf(szBuff, "CApp refcount increased to %d\n", m_dwRef+1); 
	ODS(szBuff); 
	return ++m_dwRef; 
} 
 
STDMETHODIMP_(ULONG) CApp::Release() 
{ 
	TCHAR szBuff[255]; 
 
	if (--m_dwRef == 0)  
	{  
		ODS("Deleting object\n"); 
		delete this;  
		return 0;  
	} 
 
	wsprintf(szBuff, "CApp refcount reduced to %d\n", m_dwRef); 
	ODS(szBuff); 
	return m_dwRef; 
} 
 
// Fired on change of the value of a 'bindable' property 
STDMETHODIMP CApp::OnChanged(DISPID dispID) 
{ 
	HRESULT hr; 
	TCHAR szBuff[255]; 
 
	if (DISPID_READYSTATE == dispID) 
	{ 
		// check the value of the readystate property 
		assert(m_pMSHTML); 
 
		VARIANT vResult = {0}; 
		EXCEPINFO excepInfo; 
		UINT uArgErr; 
 
		DISPPARAMS dp = {NULL, NULL, 0, 0}; 
		if (SUCCEEDED(hr = m_pMSHTML->Invoke(DISPID_READYSTATE, IID_NULL, LOCALE_SYSTEM_DEFAULT,  
				DISPATCH_PROPERTYGET, &dp, &vResult, &excepInfo, &uArgErr))) 
		{ 
			TCHAR szReadyState[20]; 
			assert(VT_I4 == V_VT(&vResult)); 
			m_lReadyState = (READYSTATE)V_I4(&vResult); 
			switch (m_lReadyState) 
			{ 
			case READYSTATE_UNINITIALIZED://= 0, 
				lstrcpy(szReadyState, _T("Uninitialized")); 
				break; 
			case READYSTATE_LOADING: //= 1, 
				lstrcpy(szReadyState, _T("Loading")); 
				break; 
			case READYSTATE_LOADED:// = 2, 
				lstrcpy(szReadyState, _T("Loaded")); 
				break; 
			case READYSTATE_INTERACTIVE: //= 3, 
				lstrcpy(szReadyState, _T("Interactive")); 
				break; 
			case READYSTATE_COMPLETE: // = 4 
				lstrcpy(szReadyState, _T("Complete")); 
				BOOL fRet = PostThreadMessage(GetCurrentThreadId(), 
				WM_USER_STARTWALKING, 
				(WPARAM)0, 
				(LPARAM)0); 
				Walk();
				break; 
			} 
			wsprintf(szBuff, _T("OnChanged: readyState = %s\n"), szReadyState); 
			VariantClear(&vResult); 
		} 
		else 
		{ 
			lstrcpy(szBuff, _T("Unable to obtain readyState value\n")); 
		} 
	} 
	else 
	{ 
		wsprintf(szBuff, _T("OnChanged: %d\n"), dispID); 
	} 
 
	ODS(szBuff); 
	cout << szBuff; 
 
	return NOERROR; 
} 
 
STDMETHODIMP CApp::OnRequestEdit(DISPID dispID) 
{ 
	// Property changes are OK any time as far as this app is concerned 
	TCHAR szBuff[255]; 
	wsprintf(szBuff, _T("OnRequestEdit: %d\n"), dispID); 
	ODS(szBuff); 
 
	return NOERROR; 
} 
 
typedef CApp* LPAPP; 
 
LPAPP g_pApp; 
 
int main(int argc, char* argv[]) 
{ 
	HRESULT hr = NOERROR; 
 
	g_pApp = new CApp; 
	if (!g_pApp) 
	{ 
		return -1; 
	} 
 
	if (FAILED(hr = CoInitialize(NULL))) 
	{ 
		return -1; 
	} 
 
	if (FAILED(g_pApp->Init(argc, argv))) 
	{ 
		goto Error; 
	} 
 
	g_pApp->Run(); 
 
	g_pApp->Term(); 
 
Error: 
	if (g_pApp) g_pApp->Release(); 
 
	CoUninitialize(); 
 
	if (FAILED(hr)) 
	{ 
		return -1; 
	} 
	else 
	{ 
		return 0; 
	} 
} 
 
// Return the protocol associated with the specified URL 
INTERNET_SCHEME GetScheme(LPCTSTR szURL) 
{ 
	URL_COMPONENTS urlComponents; 
	DWORD dwFlags = 0; 
	INTERNET_SCHEME nScheme = INTERNET_SCHEME_UNKNOWN; 
 
	ZeroMemory((PVOID)&urlComponents, sizeof(URL_COMPONENTS)); 
	urlComponents.dwStructSize = sizeof(URL_COMPONENTS); 
 
	if (szURL) 
	{ 
		if (InternetCrackUrl(szURL, 0, dwFlags, &urlComponents)) 
		{ 
		nScheme = urlComponents.nScheme; 
		} 
	} 
 
	return nScheme; 
} 
 
// Initialize the app. Load MSHTML, hook up property notification sink, etc 
HRESULT CApp::Init(int argc, char* argv[]) 
{ 
	HRESULT hr; 
	LPCONNECTIONPOINTCONTAINER pCPC = NULL; 
	LPOLEOBJECT pOleObject = NULL; 
	LPOLECONTROL pOleControl = NULL; 
 
	if (argc < 2) 
	{ 
		// If no arguments were passed, load a default URL 
		m_szURL = (LPTSTR)c_szDefaultUrl; 
		m_nScheme = INTERNET_SCHEME_HTTP; 
	} 
	else 
	{ 
		// otherwise, check the scheme (protocol) 
		// to determine which persistence interface to use 
		// see the Run method for where the scheme is used 
		m_szURL = argv[1]; 
		m_nScheme = GetScheme(m_szURL); 
	} 
 
	// Create an instance of an dynamic HTML document 
	if (FAILED(hr = CoCreateInstance( CLSID_HTMLDocument, NULL,  
		CLSCTX_INPROC_SERVER, IID_IHTMLDocument2,  
		(LPVOID*)&g_pApp->m_pMSHTML ))) 
	{ 
		goto Error; 
	} 
 
	if (FAILED(hr = m_pMSHTML->QueryInterface(IID_IOleObject, (LPVOID*)&pOleObject))) 
	{ 
		goto Error; 
	} 
	hr = pOleObject->SetClientSite((IOleClientSite*)this); 
	pOleObject->Release(); 
 
	if (FAILED(hr = m_pMSHTML->QueryInterface(IID_IOleControl, (LPVOID*)&pOleControl))) 
	{ 
		goto Error; 
	} 
	hr = pOleControl->OnAmbientPropertyChange(DISPID_AMBIENT_DLCONTROL); 
	pOleControl->Release(); 
 
	// Hook up sink to catch ready state property change 
	if (FAILED(hr = m_pMSHTML->QueryInterface(IID_IConnectionPointContainer, (LPVOID*)&pCPC))) 
	{ 
		goto Error; 
	} 
 
	if (FAILED(hr = pCPC->FindConnectionPoint(IID_IPropertyNotifySink, &m_pCP))) 
	{ 
		goto Error; 
	} 
 
	m_hrConnected = m_pCP->Advise((LPUNKNOWN)(IPropertyNotifySink*)this, &m_dwCookie); 
 
Error: 
	if (pCPC) pCPC->Release(); 
 
	return hr; 
} 
 
// Clean up connection point 
HRESULT CApp::Term() 
{ 
	HRESULT hr = NOERROR; 
 
	// Disconnect from property change notifications 
	if (SUCCEEDED(m_hrConnected)) 
	{ 
	// ASSERT(m_pCP); 
	hr = m_pCP->Unadvise(m_dwCookie); 
	} 
 
	// Release the connection point 
	if (m_pCP)  
	{ 
	m_pCP->Release(); 
	} 
 
	return hr; 
} 
 
// Load the specified document and start pumping messages 
HRESULT CApp::Run() 
{ 
	HRESULT hr; 
	MSG msg; 
 
	switch(m_nScheme) 
	{ 
	case INTERNET_SCHEME_HTTP: 
	case INTERNET_SCHEME_FTP: 
	case INTERNET_SCHEME_GOPHER: 
	case INTERNET_SCHEME_HTTPS: 
	case INTERNET_SCHEME_FILE: 
		// load URL using IPersistMoniker 
		hr = LoadURLFromMoniker(); 
		break; 
	case INTERNET_SCHEME_NEWS: 
	case INTERNET_SCHEME_MAILTO: 
	case INTERNET_SCHEME_SOCKS: 
		// we don't handle these 
		return E_FAIL; 
		break; 
	//case INTERNET_SCHEME_DEFAULT: 
	//case INTERNET_SCHEME_PARTIAL = -2, 
	//case INTERNET_SCHEME_UNKNOWN = -1, 
	default: 
		// try loading URL using IPersistFile 
		hr = LoadURLFromFile(); 
		break; 
	} 
 
	if (SUCCEEDED(hr) || E_PENDING == hr) 
	{ 
 
		while (GetMessage(&msg, NULL, 0, 0)) 
		{ 
			if (WM_USER_STARTWALKING == msg.message && NULL == msg.hwnd) 
			{ 
				// g_pApp->Walk(); 
			} 
			else 
			{ 
				DispatchMessage(&msg); 
			} 
		} 
	} 
 
	return hr; 
} 
 
// Use an asynchronous Moniker to load the specified resource 
HRESULT CApp::LoadURLFromMoniker() 
{ 
	HRESULT hr; 
	OLECHAR  wszURL[MAX_PATH*sizeof(OLECHAR)]; 
	if (0 == MultiByteToWideChar(CP_ACP, 0, m_szURL, -1, wszURL, MAX_PATH*sizeof(OLECHAR))) 
	{ 
		return E_FAIL; 
	} 
 
	// Ask the system for a URL Moniker 
	LPMONIKER pMk = NULL; 
	LPBINDCTX pBCtx = NULL; 
	LPPERSISTMONIKER pPMk = NULL; 
 
	if (FAILED(hr = CreateURLMoniker(NULL, wszURL, &pMk))) 
	{ 
		return hr; 
	} 
 
	if (FAILED(hr = CreateBindCtx(0, &pBCtx))) 
	{ 
		goto Error; 
	} 
 
	// Use MSHTML moniker services to load the specified document 
	if (SUCCEEDED(hr = m_pMSHTML->QueryInterface(IID_IPersistMoniker, 
	(LPVOID*)&pPMk))) 
	{ 
		// Call Load on the IPersistMoniker 
		// This may return immediately, but the document isn't loaded until 
		// MSHTML reports READYSTATE_COMPLETE. See the implementation of  
		// IPropertyNotifySink::OnChanged above and see how the app waits 
		// for this state change before walking the document's object model. 
		TCHAR szBuff[MAX_PATH]; 
		wsprintf(szBuff, "Loading %s...\n", m_szURL); 
		ODS(szBuff); 
		cout << szBuff; 
		hr = pPMk->Load(FALSE, pMk, pBCtx, STGM_READ); 
	} 
 
Error: 
	if (pMk) pMk->Release(); 
	if (pBCtx) pBCtx->Release(); 
	return hr; 
} 
 
// A more traditional form of persistence.  
// MSHTML performs this asynchronously as well. 
HRESULT CApp::LoadURLFromFile() 
{ 
	HRESULT hr; 
	OLECHAR  wszURL[MAX_PATH*sizeof(OLECHAR)]; 
	if (0 == MultiByteToWideChar(CP_ACP, 0, m_szURL, -1, wszURL, MAX_PATH*sizeof(OLECHAR))) 
	{ 
		return E_FAIL; 
	} 
 
	LPPERSISTFILE  pPF; 
	// MSHTML supports file persistence for ordinary files. 
	if ( SUCCEEDED(hr = m_pMSHTML->QueryInterface(IID_IPersistFile, (LPVOID*) &pPF))) 
	{ 
		TCHAR szBuff[MAX_PATH]; 
		wsprintf(szBuff, "Loading %s...\n", m_szURL); 
		ODS(szBuff); 
		cout << szBuff; 
		hr = pPF->Load(wszURL, 0); 
		pPF->Release(); 
	} 
 
	return hr; 
} 
 
// Walk the object model. 
HRESULT CApp::Walk() 
{ 
	HRESULT hr; 
	IHTMLElementCollection* pColl = NULL; 
 
	assert(m_pMSHTML); 
	if (!m_pMSHTML) 
	{ 
		return E_UNEXPECTED; 
	} 
 
	if (READYSTATE_COMPLETE != m_lReadyState) 
	{ 
		ODS("Shouldn't get here 'til MSHTML changes readyState to READYSTATE_COMPLETE\n"); 
		DebugBreak(); 
		return E_UNEXPECTED; 
	} 
 
	// retrieve a reference to the ALL collection 
	if (SUCCEEDED(hr = m_pMSHTML->get_all( &pColl ))) 
	{ 
		long cElems; 
 
		assert(pColl); 
 
		// retrieve the count of elements in the collection 
		if (SUCCEEDED(hr = pColl->get_length( &cElems ))) 
		{ 
			// for each element retrieve properties such as TAGNAME and HREF 
			for ( int i=0; i<cElems; i++ ) 
			{ 
				VARIANT vIndex; 
				vIndex.vt = VT_UINT; 
				vIndex.lVal = i; 
				VARIANT var2 = { 0 }; 
				LPDISPATCH pDisp;  
 
				if (SUCCEEDED(hr = pColl->item( vIndex, var2, &pDisp ))) 
				{ 
					IHTMLElement* pElem = NULL; 
					if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLElement, (LPVOID*)&pElem ))) 
					{ 
						_bstr_t bstrTag; 
						BSTR bstr; 
 
						assert(pElem); 
 
						hr = pElem->get_tagName(&bstr); 
						if (bstr) 
						{ 
							bstrTag = bstr; 
							SysFreeString(bstr); 
						} 
 
						// if the element is an <A> get its SRC attribute 
						IHTMLAnchorElement* pAnchor = NULL; 
						if (SUCCEEDED(hr = pElem->QueryInterface( IID_IHTMLAnchorElement, (LPVOID*)&pAnchor ))) 
						{ 
							assert(pAnchor); 
							assert(bstrTag == _bstr_t("A"));
 
							pAnchor->get_href(&bstr); 
							if (bstr) 
							{ 
								DWORD dwFlags = 0; 

								URL_COMPONENTS urlComponents; 
								char pszScheme[80];
								char pszHostName[80];
								char pszUrlPath[80];

								ZeroMemory((PVOID)&urlComponents, sizeof(URL_COMPONENTS)); 
								urlComponents.dwStructSize = sizeof(URL_COMPONENTS); 
 								urlComponents.lpszScheme = pszScheme;
								urlComponents.dwSchemeLength = 80;
								urlComponents.lpszHostName = pszHostName;
								urlComponents.dwHostNameLength = 80;
								urlComponents.lpszUrlPath = pszUrlPath;
								urlComponents.dwUrlPathLength = 80;

								char pszURL[180];
								if (WideCharToMultiByte(CP_ACP, 0, bstr, -1, pszURL, 80, NULL, NULL))
									ODS("Conversion succeeded\n");

								bstrTag += " - "; 
								if (InternetCrackUrl(pszURL, 0, dwFlags, &urlComponents) 
									&& urlComponents.nScheme == INTERNET_SCHEME_HTTP) 
								{ 
									bstrTag += _bstr_t(urlComponents.lpszHostName);
								}
								else
								{
									bstrTag += "INVALID SCHEME";
								}

								// bstrTag += " - "; 
								// bstrTag += bstr; 
								SysFreeString(bstr); 
							} 
							pAnchor->Release(); 
						} 
 
						// if the element is an <IMG> get its SRC attribute 
						IHTMLImgElement* pImage = NULL; 
						if (SUCCEEDED(hr = pDisp->QueryInterface( IID_IHTMLImgElement, (LPVOID*)&pImage ))) 
						{ 
							assert(pImage); 
 
							pImage->get_src(&bstr); 
							if (bstr) 
							{ 
								bstrTag += " - "; 
								bstrTag += bstr; 
								SysFreeString(bstr); 
							} 
							pImage->Release(); 
						} 
 
						TCHAR szTag[2048]; 
						LPOLESTR wszTag = bstrTag; UINT uLen = bstrTag.length(); 
						int iBytes = WideCharToMultiByte(CP_ACP, 0, 
						wszTag, bstrTag.length(), 
						szTag, 2048, NULL, NULL); 
 
						if (iBytes < 2047) 
						{ 
							szTag[uLen] = '\0'; 
						} 
						else 
						{ 
							szTag[2047] = '\0'; 
						} 
						cout << szTag << endl; 
						ODS(szTag); 
						ODS("\n"); 
 
						pElem->Release(); 
					} // QI(IHTMLElement) 
					pDisp->Release(); 
				} // item 
			} // for 
		} // get_length 
 
		pColl->Release(); 
	} // get_all 
 
	// We're done so post ourselves a quit to terminate the message pump. 
	PostQuitMessage(0); 
 
	return hr; 
} 
 
// MSHTML Queries for the IDispatch interface of the host through the IOleClientSite 
// interface that MSHTML is passed through its implementation of IOleObject::SetClientSite() 
STDMETHODIMP CApp::Invoke(DISPID dispIdMember, 
			REFIID riid, 
			LCID lcid, 
			WORD wFlags, 
			DISPPARAMS __RPC_FAR *pDispParams, 
			VARIANT __RPC_FAR *pVarResult, 
			EXCEPINFO __RPC_FAR *pExcepInfo, 
			UINT __RPC_FAR *puArgErr) 
{ 
	if (!pVarResult) 
	{ 
		return E_POINTER; 
	} 
 
	PrintDISPID(dispIdMember); 
 
	switch(dispIdMember) 
	{ 
	case DISPID_AMBIENT_DLCONTROL:  
		// respond to this ambient to indicate that we only want to 
		// download the page, but we don't want to run scripts, 
		// Java applets, or ActiveX controls 
		V_VT(pVarResult) = VT_I4; 
		V_I4(pVarResult) =  DLCTL_DOWNLOADONLY |  
		DLCTL_NO_SCRIPTS |  
		DLCTL_NO_JAVA | 
		DLCTL_NO_DLACTIVEXCTLS | 
		DLCTL_NO_RUNACTIVEXCTLS; 
		break; 
	default: 
		return DISP_E_MEMBERNOTFOUND; 
	} 
 
	return NOERROR; 
} 
 
// Diagnostic helper to discover what ambient properties MSHTML 
// asks of the host 
void PrintDISPID(DISPID dispidMember) 
{ 
	#define ALEN(x) (sizeof(x)/(sizeof(x[0]))) 
 
	typedef struct { 
		DISPID dispid; 
		LPCTSTR szDesc; 
	} DISPIDDESC; 
 
	static DISPIDDESC aDISPIDS[] = { 
		{DISPID_AMBIENT_DLCONTROL, _T("DISPID_AMBIENT_DLCONTROL")}, // (-5512) 
		{DISPID_AMBIENT_USERAGENT, _T("DISPID_AMBIENT_USERAGENT")}, // (-5513) 
		{DISPID_AMBIENT_BACKCOLOR, _T("DISPID_AMBIENT_BACKCOLOR")},  //        (-701) 
		{DISPID_AMBIENT_DISPLAYNAME, _T("DISPID_AMBIENT_DISPLAYNAME")},  //      (-702) 
		{DISPID_AMBIENT_FONT, _T("DISPID_AMBIENT_FONT")},  //             (-703) 
		{DISPID_AMBIENT_FORECOLOR, _T("DISPID_AMBIENT_FORECOLOR")},  //        (-704) 
		{DISPID_AMBIENT_LOCALEID, _T("DISPID_AMBIENT_LOCALEID")},  //         (-705) 
		{DISPID_AMBIENT_MESSAGEREFLECT, _T("DISPID_AMBIENT_MESSAGEREFLECT")},  //   (-706) 
		{DISPID_AMBIENT_SCALEUNITS, _T("DISPID_AMBIENT_SCALEUNITS")},  //       (-707) 
		{DISPID_AMBIENT_TEXTALIGN, _T("DISPID_AMBIENT_TEXTALIGN")},  //        (-708) 
		{DISPID_AMBIENT_USERMODE, _T("DISPID_AMBIENT_USERMODE")},  //         (-709) 
		{DISPID_AMBIENT_UIDEAD, _T("DISPID_AMBIENT_UIDEAD")},  //           (-710) 
		{DISPID_AMBIENT_SHOWGRABHANDLES, _T("DISPID_AMBIENT_SHOWGRABHANDLES")},  //  (-711) 
		{DISPID_AMBIENT_SHOWHATCHING, _T("DISPID_AMBIENT_SHOWHATCHING")},  //     (-712) 
		{DISPID_AMBIENT_DISPLAYASDEFAULT, _T("DISPID_AMBIENT_DISPLAYASDEFAULT")},  // (-713) 
		{DISPID_AMBIENT_SUPPORTSMNEMONICS, _T("DISPID_AMBIENT_SUPPORTSMNEMONICS")},  // (-714) 
		{DISPID_AMBIENT_AUTOCLIP, _T("DISPID_AMBIENT_AUTOCLIP")},  //         (-715) 
		{DISPID_AMBIENT_APPEARANCE, _T("DISPID_AMBIENT_APPEARANCE")},  //       (-716) 
		{DISPID_AMBIENT_PALETTE, _T("DISPID_AMBIENT_PALETTE")},  //          (-726) 
		{DISPID_AMBIENT_TRANSFERPRIORITY, _T("DISPID_AMBIENT_TRANSFERPRIORITY")},  // (-728) 
		{DISPID_AMBIENT_RIGHTTOLEFT, _T("DISPID_AMBIENT_RIGHTTOLEFT")},  //      (-732) 
		{DISPID_AMBIENT_TOPTOBOTTOM, _T("DISPID_AMBIENT_TOPTOBOTTOM")}  //      (-733) 
	}; 
 
	TCHAR szBuff[255]; 
 
	for (int i = 0; i < ALEN(aDISPIDS); i++) 
	{ 
		if (dispidMember == aDISPIDS[i].dispid) 
		{ 
			wsprintf(szBuff, "MSHTML requesting '%s'\n", aDISPIDS[i].szDesc); 
			ODS(szBuff); 
			return; 
		} 
	} 
 
	wsprintf(szBuff, "MSHTML requesting DISPID '%d'\n", dispidMember); 
	ODS(szBuff); 
} 
 

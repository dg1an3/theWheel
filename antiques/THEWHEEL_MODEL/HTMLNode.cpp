//////////////////////////////////////////////////////////////////////
// HTMLNode.cpp: implementation of the CHTMLNode class.
//
// Copyright (C) 1999-2002 Derek Graham Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// class declaration
#include "HTMLNode.h"

// HTML schema definition
#include <mshtmdid.h> 

// helper macro
#define ODS(x) OutputDebugString(x) 

//////////////////////////////////////////////////////////////////////
// GetScheme
// 
// Return the protocol associated with the specified URL 
//////////////////////////////////////////////////////////////////////
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
 
//////////////////////////////////////////////////////////////////////
// GetScheme
// 
// Diagnostic helper to discover what ambient properties MSHTML 
// asks of the host 
//////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CHTMLNode::CHTMLNode
// 
// Constructs an HTML node
//////////////////////////////////////////////////////////////////////
CHTMLNode::CHTMLNode()
	: m_dwRef(1),  
		m_hrConnected(CONNECT_E_CANNOTCONNECT), 
		m_dwCookie(0), 
		m_pCP(NULL), 
		m_lReadyState(READYSTATE_UNINITIALIZED) 
{
}

//////////////////////////////////////////////////////////////////////
// CHTMLNode::~CHTMLNode
// 
// Destroys the HTML node
//////////////////////////////////////////////////////////////////////
CHTMLNode::~CHTMLNode()
{
}

//////////////////////////////////////////////////////////////////////
// CHTMLNode::QueryInterface
// 
// Retrieves an interface pointer for this object
//////////////////////////////////////////////////////////////////////
STDMETHODIMP CHTMLNode::QueryInterface(REFIID riid, LPVOID* ppv) 
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
 
//////////////////////////////////////////////////////////////////////
// CHTMLNode::AddRef
// 
// Increments the reference count for the node
//////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG) CHTMLNode::AddRef() 
{ 
	TCHAR szBuff[255]; 
	wsprintf(szBuff, "CHTMLNode refcount increased to %d\n", m_dwRef+1); 
	ODS(szBuff); 
	return ++m_dwRef; 
} 
 
//////////////////////////////////////////////////////////////////////
// CHTMLNode::Release
// 
// Decrements the reference count, deleting if it reaches zero
//////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG) CHTMLNode::Release() 
{ 
	TCHAR szBuff[255]; 
 
	if (--m_dwRef == 0)  
	{  
		ODS("Deleting object\n"); 
		delete this;  
		return 0;  
	} 
 
	wsprintf(szBuff, "CHTMLNode refcount reduced to %d\n", m_dwRef); 
	ODS(szBuff); 
	return m_dwRef; 
} 
 
//////////////////////////////////////////////////////////////////////
// CHTMLNode::OnChanged
// 
// Fired on change of the value of a 'bindable' property 
//////////////////////////////////////////////////////////////////////
STDMETHODIMP CHTMLNode::OnChanged(DISPID dispID) 
{ 
	HRESULT hr; 
	TCHAR szBuff[255]; 
 
	if (DISPID_READYSTATE == dispID) 
	{ 
		// check the value of the readystate property 
		ASSERT(m_pMSHTML != NULL); 
 
		VARIANT vResult = {0}; 
		EXCEPINFO excepInfo; 
		UINT uArgErr; 
 
		DISPPARAMS dp = {NULL, NULL, 0, 0}; 
		if (SUCCEEDED(hr = m_pMSHTML->Invoke(DISPID_READYSTATE, IID_NULL, LOCALE_SYSTEM_DEFAULT,  
				DISPATCH_PROPERTYGET, &dp, &vResult, &excepInfo, &uArgErr))) 
		{ 
			TCHAR szReadyState[20]; 
			ASSERT(VT_I4 == V_VT(&vResult)); 
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

				// do the walk
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
 
	return NOERROR; 
} 
 
//////////////////////////////////////////////////////////////////////
// CHTMLNode::OnRequestEdit
// 
// OK to edit the object
//////////////////////////////////////////////////////////////////////
STDMETHODIMP CHTMLNode::OnRequestEdit(DISPID dispID) 
{ 
	// Property changes are OK any time as far as this app is concerned 
	TCHAR szBuff[255]; 
	wsprintf(szBuff, _T("OnRequestEdit: %d\n"), dispID); 
	ODS(szBuff); 
 
	return NOERROR; 
} 
 
//////////////////////////////////////////////////////////////////////
// CHTMLNode::Init
// 
// Initialize the app. Load MSHTML, hook up property notification 
//		sink, etc 
//////////////////////////////////////////////////////////////////////
HRESULT CHTMLNode::Init() 
{ 
	HRESULT hr; 
	LPCONNECTIONPOINTCONTAINER pCPC = NULL; 
	LPOLEOBJECT pOleObject = NULL; 
	LPOLECONTROL pOleControl = NULL; 
 
	// Create an instance of an dynamic HTML document 
	if (FAILED(hr = CoCreateInstance( CLSID_HTMLDocument, NULL,  
		CLSCTX_INPROC_SERVER, IID_IHTMLDocument2, (LPVOID*)&m_pMSHTML ))) 
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
 
//////////////////////////////////////////////////////////////////////
// CHTMLNode::Term
// 
// Clean up connection point 
//////////////////////////////////////////////////////////////////////
HRESULT CHTMLNode::Term() 
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
 
//////////////////////////////////////////////////////////////////////
// CHTMLNode::Run
// 
// Load the specified document and start pumping messages 
//////////////////////////////////////////////////////////////////////
HRESULT CHTMLNode::Run() 
{
	HRESULT hr; 
 
	switch(::GetScheme(m_strUrl)) 
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

	return hr;
} 
 
//////////////////////////////////////////////////////////////////////
// CHTMLNode::LoadURLFromMoniker
// 
// Use an asynchronous Moniker to load the specified resource 
//////////////////////////////////////////////////////////////////////
HRESULT CHTMLNode::LoadURLFromMoniker() 
{ 
	HRESULT hr; 
	OLECHAR  wszURL[MAX_PATH*sizeof(OLECHAR)]; 
	if (0 == MultiByteToWideChar(CP_ACP, 0, m_strUrl.GetBuffer(0), -1, 
			wszURL, MAX_PATH*sizeof(OLECHAR))) 
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
		wsprintf(szBuff, "Loading %s...\n", m_strUrl); 
		ODS(szBuff); 
		hr = pPMk->Load(FALSE, pMk, pBCtx, STGM_READ); 
	} 
 
Error: 
	if (pMk) pMk->Release(); 
	if (pBCtx) pBCtx->Release(); 
	return hr; 
} 
 
//////////////////////////////////////////////////////////////////////
// CHTMLNode::LoadURLFromFile
// 
// A more traditional form of persistence.  
// MSHTML performs this asynchronously as well. 
//////////////////////////////////////////////////////////////////////
HRESULT CHTMLNode::LoadURLFromFile() 
{ 
	HRESULT hr; 
	OLECHAR  wszURL[MAX_PATH*sizeof(OLECHAR)]; 
	if (0 == MultiByteToWideChar(CP_ACP, 0, m_strUrl.GetBuffer(0), -1, 
		wszURL, MAX_PATH*sizeof(OLECHAR))) 
	{ 
		return E_FAIL; 
	}
 
	LPPERSISTFILE  pPF; 
	// MSHTML supports file persistence for ordinary files. 
	if ( SUCCEEDED(hr = m_pMSHTML->QueryInterface(IID_IPersistFile, (LPVOID*) &pPF))) 
	{ 
		TCHAR szBuff[MAX_PATH]; 
		wsprintf(szBuff, "Loading %s...\n", m_strUrl); 
		ODS(szBuff); 
		hr = pPF->Load(wszURL, 0); 
		pPF->Release(); 
	} 
 
	return hr; 
} 
 
//////////////////////////////////////////////////////////////////////
// CHTMLNode::Walk
// 
// Walk the object model. 
//////////////////////////////////////////////////////////////////////
HRESULT CHTMLNode::Walk() 
{ 
	HRESULT hr; 
	IHTMLElementCollection* pColl = NULL; 
 
	ASSERT(m_pMSHTML); 
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
 
		ASSERT(pColl); 
 
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
 
						ASSERT(pElem); 
 
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
							ASSERT(pAnchor); 
							ASSERT(bstrTag == _bstr_t("A"));
 
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
							ASSERT(pImage); 
 
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
 
	return hr; 
} 
 
//////////////////////////////////////////////////////////////////////
// CHTMLNode::Invoke
// 
// MSHTML Queries for the IDispatch interface of the host through the 
//		IOleClientSite interface that MSHTML is passed through its 
//		implementation of IOleObject::SetClientSite() 
//////////////////////////////////////////////////////////////////////
STDMETHODIMP CHTMLNode::Invoke(DISPID dispIdMember, 
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
 

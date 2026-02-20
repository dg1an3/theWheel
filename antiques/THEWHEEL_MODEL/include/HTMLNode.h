//////////////////////////////////////////////////////////////////////
// HTMLNode.h: interface for the CHTMLNode class.
//
// Copyright (C) 1999-2002 Derek Graham Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(HTMLNODE_H)
#define HTMLNODE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// base class
#include "Node.h"

//////////////////////////////////////////////////////////////////////
// class CHTMLNode
// 
// a node object that represents an HTML file on the web
//////////////////////////////////////////////////////////////////////
class CHTMLNode : public CNode,
	IPropertyNotifySink, 
	IOleClientSite, 
	IDispatch 
{
public:
	CHTMLNode();
	virtual ~CHTMLNode();

// IUnknown methods 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv); 
    STDMETHOD_(ULONG, AddRef)(); 
    STDMETHOD_(ULONG, Release)();  
 
// IPropertyNotifySink methods 
    STDMETHOD(OnChanged)(DISPID dispID); 
    STDMETHOD(OnRequestEdit)(DISPID dispID); 
 
// IOleClientSite methods 
	STDMETHOD(SaveObject)(void) 
		{ return E_NOTIMPL; } 
 
	STDMETHOD(GetMoniker)(DWORD dwAssign, 
				DWORD dwWhichMoniker, 
				IMoniker** ppmk) 
		{ return E_NOTIMPL; } 
 
	STDMETHOD(GetContainer)(IOleContainer** ppContainer) 
		{ return E_NOTIMPL; } 
 
	STDMETHOD(ShowObject)(void) 
		{ return E_NOTIMPL; } 
 
	STDMETHOD(OnShowWindow)(BOOL fShow) 
		{ return E_NOTIMPL; } 
 
	STDMETHOD(RequestNewObjectLayout)(void) 
		{ return E_NOTIMPL; } 
 
// IDispatch method 
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) 
		{ return E_NOTIMPL; } 
 
	STDMETHOD(GetTypeInfo)(UINT iTInfo, 
				LCID lcid, 
				ITypeInfo** ppTInfo) 
		{ return E_NOTIMPL; } 
 
	STDMETHOD(GetIDsOfNames)(REFIID riid, 
				LPOLESTR* rgszNames, 
				UINT cNames, 
				LCID lcid, 
				DISPID* rgDispId) 
		{ return E_NOTIMPL; } 
         
	STDMETHOD(Invoke)(DISPID dispIdMember, 
				REFIID riid, 
				LCID lcid, 
				WORD wFlags, 
				DISPPARAMS __RPC_FAR *pDispParams, 
				VARIANT __RPC_FAR *pVarResult, 
				EXCEPINFO __RPC_FAR *pExcepInfo, 
				UINT __RPC_FAR *puArgErr); 
 
// Additional class methods 
	HRESULT Init(); 
	HRESULT Run(); 
	HRESULT Term(); 
	HRESULT Walk(); 
 
protected:

	// Persistence helpers 
	HRESULT LoadURLFromFile(); 
	HRESULT LoadURLFromMoniker(); 

private:
	// the URL
	CString m_strUrl; 

	// the MSHTML document
	IHTMLDocument2* m_pMSHTML; 

	// reference counter
	DWORD m_dwRef; 

	// cookie ID 
	DWORD m_dwCookie; 

	// connection point for notification
	LPCONNECTIONPOINT m_pCP; 

	// connected state
	HRESULT m_hrConnected; 

	// ready state
	READYSTATE m_lReadyState; 
 
	// the binding
	IBinding* m_pBinding; 
};

#endif // !defined(HTMLNODE_H)

#ifndef ASYNCSOCKETCLIENT_H_INCLUDED
#define ASYNCSOCKETCLIENT_H_INCLUDED

#pragma once

#include <atlsoap.h>


//#define SOAPCLIENT_ASYNC_EXECUTION	101

class CAsyncSoapSocketClient;
typedef void (*SOAP_ASYNC_CALLBACK)(LPARAM lParam, HRESULT hRet);

struct stAsyncSoapRequest
{
	CAsyncSoapSocketClient *pClient;
	SOAP_ASYNC_CALLBACK pfnCallback;
	LPARAM lpRequestParam;
};


class CAsyncSoapWorker
{
public:
	typedef stAsyncSoapRequest* RequestType;
	
	CAsyncSoapWorker() throw()
	{
	}

	~CAsyncSoapWorker() throw()
	{
	}

	virtual BOOL Initialize(void *pvParam) throw()
	{
		// Do any initialization here
		return TRUE;
	}

	virtual void Terminate(void* pvParam) throw()
	{
		// Do any cleanup here
	}

	void Execute(stAsyncSoapRequest	*pRequestInfo, 
    void *pvParam, OVERLAPPED *pOverlapped) throw();
};

typedef CThreadPool< CAsyncSoapWorker >	CAsyncSoapThreadPool;

class CAsyncSoapSocketClient : public CSoapSocketClientT<>
{
public:
	CAsyncSoapSocketClient(LPCTSTR szUrl) 
    : CSoapSocketClientT<>(szUrl)
	{
		m_bInRequest = false;
		m_pThreadPool = NULL;
	}


	// Note : this is not thread safe, the caller should take care
	HRESULT	PrepareAsyncRequest(CAsyncSoapThreadPool	*pThreadPool, 
    SOAP_ASYNC_CALLBACK	pfnCallback, LPARAM lParam)
	{
		// return false, if already in request
		if( m_bInRequest )
			return E_PENDING;
		if( !pThreadPool )
			return E_POINTER;

		m_pThreadPool = pThreadPool;

		m_stRequestInfo.pClient = this;	
		m_stRequestInfo.lpRequestParam= lParam;
		m_stRequestInfo.pfnCallback=pfnCallback;
	
		m_bInRequest = true;

		return S_OK;
	}

	HRESULT SendRequest(LPCTSTR szAction)
	{
		HRESULT		hRet;

		if( m_bInRequest )
		{
			// launch the async request
			m_strSOAPAction = szAction;
			ATLASSERT(m_pThreadPool);
			if( m_pThreadPool->QueueRequest(&m_stRequestInfo) )
				hRet = E_PENDING;
			else
				hRet = E_FAIL;
		}
		else
		{
			// post-async request, reading the response. 
			// Nothing to do, just return hRet from the reall operation
			hRet = m_hRealRet;
		}

		return hRet;
	}

	HRESULT	InternalSendRequest()
	{
		m_hRealRet = CSoapSocketClientT<>::SendRequest( m_strSOAPAction );
		m_bInRequest = false;
		return m_hRealRet;
	}

protected:
  CString m_strSOAPAction;
  stAsyncSoapRequest m_stRequestInfo;
  bool m_bInRequest;
  HRESULT m_hRealRet;
  CAsyncSoapThreadPool *m_pThreadPool;
};


inline void CAsyncSoapWorker::Execute(stAsyncSoapRequest * pRequestInfo, 
                                      void * pvParam, OVERLAPPED * pOverlapped) throw()
{
	ATLASSERT(pRequestInfo != NULL);
	pvParam;		// unused
	pOverlapped;    // unused

	CAsyncSoapSocketClient * pClient = pRequestInfo->pClient;
	SOAP_ASYNC_CALLBACK pfnCallBack = pRequestInfo->pfnCallback;
	LPARAM lParam = pRequestInfo->lpRequestParam;
	
	ATLASSERT( pClient );
	HRESULT hRet = pClient->InternalSendRequest();

	if( pfnCallBack )
	{
		pfnCallBack( lParam, hRet);
	}
}

#endif ASYNCSOCKETCLIENT_H_INCLUDED
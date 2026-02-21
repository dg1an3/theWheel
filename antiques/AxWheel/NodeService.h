//
// sproxy.exe generated file
// do not modify this file
//
// Created: 02/24/2008@09:56:45
//

#pragma once


#if !defined(_WIN32_WINDOWS) && !defined(_WIN32_WINNT) && !defined(_WIN32_WCE)
#pragma message("warning: defining _WIN32_WINDOWS = 0x0410")
#define _WIN32_WINDOWS 0x0410
#endif

#include <atlsoap.h>

namespace NodeService
{

struct Link
{
	int ToNodeId;
	float Weight;
};

struct Node
{
	int Id;
	BSTR Name;
	BSTR Body;
	Link *Links;
	int __Links_nSizeIs;
};

template <typename TClient = CSoapSocketClientT<> >
class CNodeServiceT : 
	public TClient, 
	public CSoapRootHandler
{
protected:

	const _soapmap ** GetFunctionMap();
	const _soapmap ** GetHeaderMap();
	void * GetHeaderValue();
	const wchar_t * GetNamespaceUri();
	const char * GetServiceName();
	const char * GetNamespaceUriA();
	HRESULT CallFunction(
		void *pvParam, 
		const wchar_t *wszLocalName, int cchLocalName,
		size_t nItem);
	HRESULT GetClientReader(ISAXXMLReader **ppReader);

public:

	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv)
	{
		if (ppv == NULL)
		{
			return E_POINTER;
		}

		*ppv = NULL;

		if (InlineIsEqualGUID(riid, IID_IUnknown) ||
			InlineIsEqualGUID(riid, IID_ISAXContentHandler))
		{
			*ppv = static_cast<ISAXContentHandler *>(this);
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	ULONG __stdcall AddRef()
	{
		return 1;
	}

	ULONG __stdcall Release()
	{
		return 1;
	}

	CNodeServiceT(ISAXXMLReader *pReader = NULL)
		:TClient(_T("http://localhost:4017/NodeService.asmx"))
	{
		SetClient(true);
		SetReader(pReader);
	}
	
	~CNodeServiceT()
	{
		Uninitialize();
	}
	
	void Uninitialize()
	{
		UninitializeSOAP();
	}	

	HRESULT GetRootNode(
		Node* GetRootNodeResult
	);

	HRESULT GetNodeById(
		int Id, 
		Node* GetNodeByIdResult
	);

	HRESULT GetImageBufferForNodeId(
		int Id, 
		ATLSOAP_BLOB* GetImageBufferForNodeIdResult
	);
};

typedef CNodeServiceT<> CNodeService;

__if_not_exists(__Link_entries)
{
extern __declspec(selectany) const _soapmapentry __Link_entries[] =
{
	{ 
		0xA41DBFD6, 
		"ToNodeId", 
		L"ToNodeId", 
		sizeof("ToNodeId")-1, 
		SOAPTYPE_INT, 
		SOAPFLAG_FIELD, 
		offsetof(Link, ToNodeId),
		NULL, 
		NULL, 
		-1 
	},
	{ 
		0xD24FD168, 
		"Weight", 
		L"Weight", 
		sizeof("Weight")-1, 
		SOAPTYPE_FLOAT, 
		SOAPFLAG_FIELD, 
		offsetof(Link, Weight),
		NULL, 
		NULL, 
		-1 
	},
	{ 0x00000000 }
};

extern __declspec(selectany) const _soapmap __Link_map =
{
	0x002B7A0E,
	"Link",
	L"Link",
	sizeof("Link")-1,
	sizeof("Link")-1,
	SOAPMAP_STRUCT,
	__Link_entries,
	sizeof(Link),
	2,
	-1,
	SOAPFLAG_NONE,
	0xC2E575C3,
	"http://tempuri.org/",
	L"http://tempuri.org/",
	sizeof("http://tempuri.org/")-1
};
}

__if_not_exists(__Node_entries)
{
extern __declspec(selectany) const _soapmapentry __Node_entries[] =
{
	{ 
		0x000009CD, 
		"Id", 
		L"Id", 
		sizeof("Id")-1, 
		SOAPTYPE_INT, 
		SOAPFLAG_FIELD, 
		offsetof(Node, Id),
		NULL, 
		NULL, 
		-1 
	},
	{ 
		0x002C70A1, 
		"Name", 
		L"Name", 
		sizeof("Name")-1, 
		SOAPTYPE_STRING, 
		SOAPFLAG_FIELD | SOAPFLAG_NULLABLE, 
		offsetof(Node, Name),
		NULL, 
		NULL, 
		-1 
	},
	{ 
		0x0026168E, 
		"Body", 
		L"Body", 
		sizeof("Body")-1, 
		SOAPTYPE_STRING, 
		SOAPFLAG_FIELD | SOAPFLAG_NULLABLE, 
		offsetof(Node, Body),
		NULL, 
		NULL, 
		-1 
	},
	{ 
		0x059ABC41, 
		"Links", 
		L"Links", 
		sizeof("Links")-1, 
		SOAPTYPE_UNK, 
		SOAPFLAG_FIELD | SOAPFLAG_DYNARR | SOAPFLAG_NULLABLE, 
		offsetof(Node, Links),
		NULL, 
		&__Link_map, 
		3+1 
	},
	{ 
		0x059ABC41, 
		"Links", 
		L"Links", 
		sizeof("Links")-1, 
		SOAPTYPE_INT, 
		SOAPFLAG_FIELD | SOAPFLAG_NOMARSHAL, 
		offsetof(Node, __Links_nSizeIs),
		NULL, 
		NULL, 
		-1
	},
	{ 0x00000000 }
};

extern __declspec(selectany) const _soapmap __Node_map =
{
	0x002CAB06,
	"Node",
	L"Node",
	sizeof("Node")-1,
	sizeof("Node")-1,
	SOAPMAP_STRUCT,
	__Node_entries,
	sizeof(Node),
	4,
	-1,
	SOAPFLAG_NONE,
	0xC2E575C3,
	"http://tempuri.org/",
	L"http://tempuri.org/",
	sizeof("http://tempuri.org/")-1
};
}

struct __CNodeService_GetRootNode_struct
{
	Node GetRootNodeResult;
};

extern __declspec(selectany) const _soapmapentry __CNodeService_GetRootNode_entries[] =
{

	{
		0xC05CA429, 
		"GetRootNodeResult", 
		L"GetRootNodeResult", 
		sizeof("GetRootNodeResult")-1, 
		SOAPTYPE_UNK, 
		SOAPFLAG_NONE | SOAPFLAG_OUT | SOAPFLAG_PID | SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL,
		offsetof(__CNodeService_GetRootNode_struct, GetRootNodeResult),
		NULL,
		&__Node_map,
		-1,
	},
	{ 0x00000000 }
};

extern __declspec(selectany) const _soapmap __CNodeService_GetRootNode_map =
{
	0x49BD72B9,
	"GetRootNode",
	L"GetRootNodeResponse",
	sizeof("GetRootNode")-1,
	sizeof("GetRootNodeResponse")-1,
	SOAPMAP_FUNC,
	__CNodeService_GetRootNode_entries,
	sizeof(__CNodeService_GetRootNode_struct),
	1,
	-1,
	SOAPFLAG_NONE | SOAPFLAG_PID | SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL,
	0xC2E575C3,
	"http://tempuri.org/",
	L"http://tempuri.org/",
	sizeof("http://tempuri.org/")-1
};


struct __CNodeService_GetNodeById_struct
{
	int Id;
	Node GetNodeByIdResult;
};

extern __declspec(selectany) const _soapmapentry __CNodeService_GetNodeById_entries[] =
{

	{
		0x000009CD, 
		"Id", 
		L"Id", 
		sizeof("Id")-1, 
		SOAPTYPE_INT, 
		SOAPFLAG_NONE | SOAPFLAG_IN | SOAPFLAG_PID | SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL,
		offsetof(__CNodeService_GetNodeById_struct, Id),
		NULL,
		NULL,
		-1,
	},
	{
		0x0CEA07AD, 
		"GetNodeByIdResult", 
		L"GetNodeByIdResult", 
		sizeof("GetNodeByIdResult")-1, 
		SOAPTYPE_UNK, 
		SOAPFLAG_NONE | SOAPFLAG_OUT | SOAPFLAG_PID | SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL,
		offsetof(__CNodeService_GetNodeById_struct, GetNodeByIdResult),
		NULL,
		&__Node_map,
		-1,
	},
	{ 0x00000000 }
};

extern __declspec(selectany) const _soapmap __CNodeService_GetNodeById_map =
{
	0xEF31C73D,
	"GetNodeById",
	L"GetNodeByIdResponse",
	sizeof("GetNodeById")-1,
	sizeof("GetNodeByIdResponse")-1,
	SOAPMAP_FUNC,
	__CNodeService_GetNodeById_entries,
	sizeof(__CNodeService_GetNodeById_struct),
	1,
	-1,
	SOAPFLAG_NONE | SOAPFLAG_PID | SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL,
	0xC2E575C3,
	"http://tempuri.org/",
	L"http://tempuri.org/",
	sizeof("http://tempuri.org/")-1
};


struct __CNodeService_GetImageBufferForNodeId_struct
{
	int Id;
	ATLSOAP_BLOB GetImageBufferForNodeIdResult;
};

extern __declspec(selectany) const _soapmapentry __CNodeService_GetImageBufferForNodeId_entries[] =
{

	{
		0x000009CD, 
		"Id", 
		L"Id", 
		sizeof("Id")-1, 
		SOAPTYPE_INT, 
		SOAPFLAG_NONE | SOAPFLAG_IN | SOAPFLAG_PID | SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL,
		offsetof(__CNodeService_GetImageBufferForNodeId_struct, Id),
		NULL,
		NULL,
		-1,
	},
	{
		0xB89A44B6, 
		"GetImageBufferForNodeIdResult", 
		L"GetImageBufferForNodeIdResult", 
		sizeof("GetImageBufferForNodeIdResult")-1, 
		SOAPTYPE_BASE64BINARY, 
		SOAPFLAG_NONE | SOAPFLAG_OUT | SOAPFLAG_PID | SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL | SOAPFLAG_NULLABLE,
		offsetof(__CNodeService_GetImageBufferForNodeId_struct, GetImageBufferForNodeIdResult),
		NULL,
		NULL,
		-1,
	},
	{ 0x00000000 }
};

extern __declspec(selectany) const _soapmap __CNodeService_GetImageBufferForNodeId_map =
{
	0x47E56A86,
	"GetImageBufferForNodeId",
	L"GetImageBufferForNodeIdResponse",
	sizeof("GetImageBufferForNodeId")-1,
	sizeof("GetImageBufferForNodeIdResponse")-1,
	SOAPMAP_FUNC,
	__CNodeService_GetImageBufferForNodeId_entries,
	sizeof(__CNodeService_GetImageBufferForNodeId_struct),
	1,
	-1,
	SOAPFLAG_NONE | SOAPFLAG_PID | SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL,
	0xC2E575C3,
	"http://tempuri.org/",
	L"http://tempuri.org/",
	sizeof("http://tempuri.org/")-1
};

extern __declspec(selectany) const _soapmap * __CNodeService_funcs[] =
{
	&__CNodeService_GetRootNode_map,
	&__CNodeService_GetNodeById_map,
	&__CNodeService_GetImageBufferForNodeId_map,
	NULL
};

template <typename TClient>
inline HRESULT CNodeServiceT<TClient>::GetRootNode(
		Node* GetRootNodeResult
	)
{
    if ( GetRootNodeResult == NULL )
		return E_POINTER;

	HRESULT __atlsoap_hr = InitializeSOAP(NULL);
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_INITIALIZE_ERROR);
		return __atlsoap_hr;
	}
	
	CleanupClient();

	CComPtr<IStream> __atlsoap_spReadStream;
	__CNodeService_GetRootNode_struct __params;
	memset(&__params, 0x00, sizeof(__params));

	__atlsoap_hr = SetClientStruct(&__params, 0);
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_OUTOFMEMORY);
		goto __skip_cleanup;
	}
	
	__atlsoap_hr = GenerateResponse(GetWriteStream());
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_GENERATE_ERROR);
		goto __skip_cleanup;
	}
	
	__atlsoap_hr = SendRequest(_T("SOAPAction: \"http://tempuri.org/GetRootNode\"\r\n"));
	if (FAILED(__atlsoap_hr))
	{
		goto __skip_cleanup;
	}
	__atlsoap_hr = GetReadStream(&__atlsoap_spReadStream);
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_READ_ERROR);
		goto __skip_cleanup;
	}
	
	// cleanup any in/out-params and out-headers from previous calls
	Cleanup();
	__atlsoap_hr = BeginParse(__atlsoap_spReadStream);
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_PARSE_ERROR);
		goto __cleanup;
	}

	*GetRootNodeResult = __params.GetRootNodeResult;
	goto __skip_cleanup;
	
__cleanup:
	Cleanup();
__skip_cleanup:
	ResetClientState(true);
	memset(&__params, 0x00, sizeof(__params));
	return __atlsoap_hr;
}

template <typename TClient>
inline HRESULT CNodeServiceT<TClient>::GetNodeById(
		int Id, 
		Node* GetNodeByIdResult
	)
{
    if ( GetNodeByIdResult == NULL )
		return E_POINTER;

	HRESULT __atlsoap_hr = InitializeSOAP(NULL);
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_INITIALIZE_ERROR);
		return __atlsoap_hr;
	}
	
	CleanupClient();

	CComPtr<IStream> __atlsoap_spReadStream;
	__CNodeService_GetNodeById_struct __params;
	memset(&__params, 0x00, sizeof(__params));
	__params.Id = Id;

	__atlsoap_hr = SetClientStruct(&__params, 1);
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_OUTOFMEMORY);
		goto __skip_cleanup;
	}
	
	__atlsoap_hr = GenerateResponse(GetWriteStream());
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_GENERATE_ERROR);
		goto __skip_cleanup;
	}
	
	__atlsoap_hr = SendRequest(_T("SOAPAction: \"http://tempuri.org/GetNodeById\"\r\n"));
	if (FAILED(__atlsoap_hr))
	{
		goto __skip_cleanup;
	}
	__atlsoap_hr = GetReadStream(&__atlsoap_spReadStream);
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_READ_ERROR);
		goto __skip_cleanup;
	}
	
	// cleanup any in/out-params and out-headers from previous calls
	Cleanup();
	__atlsoap_hr = BeginParse(__atlsoap_spReadStream);
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_PARSE_ERROR);
		goto __cleanup;
	}

	*GetNodeByIdResult = __params.GetNodeByIdResult;
	goto __skip_cleanup;
	
__cleanup:
	Cleanup();
__skip_cleanup:
	ResetClientState(true);
	memset(&__params, 0x00, sizeof(__params));
	return __atlsoap_hr;
}

template <typename TClient>
inline HRESULT CNodeServiceT<TClient>::GetImageBufferForNodeId(
		int Id, 
		ATLSOAP_BLOB* GetImageBufferForNodeIdResult
	)
{
    if ( GetImageBufferForNodeIdResult == NULL )
		return E_POINTER;

	HRESULT __atlsoap_hr = InitializeSOAP(NULL);
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_INITIALIZE_ERROR);
		return __atlsoap_hr;
	}
	
	CleanupClient();

	CComPtr<IStream> __atlsoap_spReadStream;
	__CNodeService_GetImageBufferForNodeId_struct __params;
	memset(&__params, 0x00, sizeof(__params));
	__params.Id = Id;

	__atlsoap_hr = SetClientStruct(&__params, 2);
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_OUTOFMEMORY);
		goto __skip_cleanup;
	}
	
	__atlsoap_hr = GenerateResponse(GetWriteStream());
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_GENERATE_ERROR);
		goto __skip_cleanup;
	}
	
	__atlsoap_hr = SendRequest(_T("SOAPAction: \"http://tempuri.org/GetImageBufferForNodeId\"\r\n"));
	if (FAILED(__atlsoap_hr))
	{
		goto __skip_cleanup;
	}
	__atlsoap_hr = GetReadStream(&__atlsoap_spReadStream);
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_READ_ERROR);
		goto __skip_cleanup;
	}
	
	// cleanup any in/out-params and out-headers from previous calls
	Cleanup();
	__atlsoap_hr = BeginParse(__atlsoap_spReadStream);
	if (FAILED(__atlsoap_hr))
	{
		SetClientError(SOAPCLIENT_PARSE_ERROR);
		goto __cleanup;
	}

	*GetImageBufferForNodeIdResult = __params.GetImageBufferForNodeIdResult;
	goto __skip_cleanup;
	
__cleanup:
	Cleanup();
__skip_cleanup:
	ResetClientState(true);
	memset(&__params, 0x00, sizeof(__params));
	return __atlsoap_hr;
}

template <typename TClient>
ATL_NOINLINE inline const _soapmap ** CNodeServiceT<TClient>::GetFunctionMap()
{
	return __CNodeService_funcs;
}

template <typename TClient>
ATL_NOINLINE inline const _soapmap ** CNodeServiceT<TClient>::GetHeaderMap()
{
	static const _soapmapentry __CNodeService_GetRootNode_atlsoapheader_entries[] =
	{
		{ 0x00000000 }
	};

	static const _soapmap __CNodeService_GetRootNode_atlsoapheader_map = 
	{
		0x49BD72B9,
		"GetRootNode",
		L"GetRootNodeResponse",
		sizeof("GetRootNode")-1,
		sizeof("GetRootNodeResponse")-1,
		SOAPMAP_HEADER,
		__CNodeService_GetRootNode_atlsoapheader_entries,
		0,
		0,
		-1,
		SOAPFLAG_NONE | SOAPFLAG_PID | SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL,
		0xC2E575C3,
		"http://tempuri.org/",
		L"http://tempuri.org/",
		sizeof("http://tempuri.org/")-1
	};

	static const _soapmapentry __CNodeService_GetNodeById_atlsoapheader_entries[] =
	{
		{ 0x00000000 }
	};

	static const _soapmap __CNodeService_GetNodeById_atlsoapheader_map = 
	{
		0xEF31C73D,
		"GetNodeById",
		L"GetNodeByIdResponse",
		sizeof("GetNodeById")-1,
		sizeof("GetNodeByIdResponse")-1,
		SOAPMAP_HEADER,
		__CNodeService_GetNodeById_atlsoapheader_entries,
		0,
		0,
		-1,
		SOAPFLAG_NONE | SOAPFLAG_PID | SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL,
		0xC2E575C3,
		"http://tempuri.org/",
		L"http://tempuri.org/",
		sizeof("http://tempuri.org/")-1
	};

	static const _soapmapentry __CNodeService_GetImageBufferForNodeId_atlsoapheader_entries[] =
	{
		{ 0x00000000 }
	};

	static const _soapmap __CNodeService_GetImageBufferForNodeId_atlsoapheader_map = 
	{
		0x47E56A86,
		"GetImageBufferForNodeId",
		L"GetImageBufferForNodeIdResponse",
		sizeof("GetImageBufferForNodeId")-1,
		sizeof("GetImageBufferForNodeIdResponse")-1,
		SOAPMAP_HEADER,
		__CNodeService_GetImageBufferForNodeId_atlsoapheader_entries,
		0,
		0,
		-1,
		SOAPFLAG_NONE | SOAPFLAG_PID | SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL,
		0xC2E575C3,
		"http://tempuri.org/",
		L"http://tempuri.org/",
		sizeof("http://tempuri.org/")-1
	};


	static const _soapmap * __CNodeService_headers[] =
	{
		&__CNodeService_GetRootNode_atlsoapheader_map,
		&__CNodeService_GetNodeById_atlsoapheader_map,
		&__CNodeService_GetImageBufferForNodeId_atlsoapheader_map,
		NULL
	};
	
	return __CNodeService_headers;
}

template <typename TClient>
ATL_NOINLINE inline void * CNodeServiceT<TClient>::GetHeaderValue()
{
	return this;
}

template <typename TClient>
ATL_NOINLINE inline const wchar_t * CNodeServiceT<TClient>::GetNamespaceUri()
{
	return L"http://tempuri.org/";
}

template <typename TClient>
ATL_NOINLINE inline const char * CNodeServiceT<TClient>::GetServiceName()
{
	return NULL;
}

template <typename TClient>
ATL_NOINLINE inline const char * CNodeServiceT<TClient>::GetNamespaceUriA()
{
	return "http://tempuri.org/";
}

template <typename TClient>
ATL_NOINLINE inline HRESULT CNodeServiceT<TClient>::CallFunction(
	void *, 
	const wchar_t *, int,
	size_t)
{
	return E_NOTIMPL;
}

template <typename TClient>
ATL_NOINLINE inline HRESULT CNodeServiceT<TClient>::GetClientReader(ISAXXMLReader **ppReader)
{
	if (ppReader == NULL)
	{
		return E_INVALIDARG;
	}
	
	CComPtr<ISAXXMLReader> spReader = GetReader();
	if (spReader.p != NULL)
	{
		*ppReader = spReader.Detach();
		return S_OK;
	}
	return TClient::GetClientReader(ppReader);
}

} // namespace NodeService

template<>
inline HRESULT AtlCleanupValue<NodeService::Link>(NodeService::Link *pVal)
{
	pVal;
	AtlCleanupValue(&pVal->ToNodeId);
	AtlCleanupValue(&pVal->Weight);
	return S_OK;
}

template<>
inline HRESULT AtlCleanupValueEx<NodeService::Link>(NodeService::Link *pVal, IAtlMemMgr *pMemMgr)
{
	pVal;
	pMemMgr;
	
	AtlCleanupValueEx(&pVal->ToNodeId, pMemMgr);
	AtlCleanupValueEx(&pVal->Weight, pMemMgr);
	return S_OK;
}

template<>
inline HRESULT AtlCleanupValue<NodeService::Node>(NodeService::Node *pVal)
{
	pVal;
	AtlCleanupValue(&pVal->Id);
	AtlCleanupValue(&pVal->Name);
	AtlCleanupValue(&pVal->Body);
	AtlCleanupArray(pVal->Links, pVal->__Links_nSizeIs);	
	return S_OK;
}

template<>
inline HRESULT AtlCleanupValueEx<NodeService::Node>(NodeService::Node *pVal, IAtlMemMgr *pMemMgr)
{
	pVal;
	pMemMgr;
	
	AtlCleanupValueEx(&pVal->Id, pMemMgr);
	AtlCleanupValueEx(&pVal->Name, pMemMgr);
	AtlCleanupValueEx(&pVal->Body, pMemMgr);
	if (pVal->Links != NULL)
	{
		AtlCleanupArrayEx(pVal->Links, pVal->__Links_nSizeIs, pMemMgr);		
		pMemMgr->Free(pVal->Links);
	}
	return S_OK;
}

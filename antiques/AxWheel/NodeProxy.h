#pragma once

#include <Node.h>

#include "NodeService.h"
#include "AsyncSocketClient.h"

namespace NodeService 
{

//////////////////////////////////////////////////////////////////////////////
class NodeProxy : public NodeService::Node
{
public:
	NodeProxy(int initId = -1, CNode *pNode = NULL);

	// NOTE: can not be virtual, as it is passed as a callback struct
	~NodeProxy();

	CNode *GetNode()
	{
		return m_pNode;
	}

	HRESULT QueueRequest(CAsyncSoapThreadPool *pThreadPool);

	bool IsRequestQueued()
	{
		return m_bSoapRequestQueued;
	}

	bool IsRequestCompleted()
	{
		return m_bSoapRequestCompleted;
	}

	typedef CAtlMap<int, NodeService::NodeProxy*> Map;

	void CreateNodesFromNS(Map& mapNPs);

	bool NodesAreCreated()
	{
		return m_bNodesCreated;
	}

private:
	// the contained Node
	CNode *m_pNode;

	// the node service object: set up with the SOAP call prior to queueing
	NodeService::CNodeServiceT<CAsyncSoapSocketClient> m_NSProxy;

	bool m_bSoapRequestQueued;
	bool m_bSoapRequestCompleted;

	// NOTE: this should only run on the async thread
	static void SoapRequestCallbackProc(LPARAM lParam, HRESULT hRet);

	bool m_bNodesCreated;
};

}
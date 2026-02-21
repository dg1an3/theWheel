#include "StdAfx.h"

#include "NodeProxy.h"

namespace NodeService {

//////////////////////////////////////////////////////////////////////////////
NodeProxy::NodeProxy(int initId, CNode *pNode)
	: m_pNode(pNode) 
	, m_bSoapRequestQueued(false)
	, m_bSoapRequestCompleted(false)
	, m_bNodesCreated(false) 
{ 
	Id = initId;
	if (!m_pNode)
		m_pNode = new CNode();
}

//////////////////////////////////////////////////////////////////////////////
NodeProxy::~NodeProxy()
{
	delete [] Links;
}

//////////////////////////////////////////////////////////////////////////////
HRESULT NodeProxy::QueueRequest(CAsyncSoapThreadPool *pThreadPool)
{
	HRESULT hRet = m_NSProxy.PrepareAsyncRequest(pThreadPool, 
		SoapRequestCallbackProc, (LPARAM) this);
	ATLASSERT( SUCCEEDED(hRet) );

	hRet = m_NSProxy.GetNodeById(Id, this);
	ATLASSERT( E_PENDING == hRet );

	m_bSoapRequestQueued = true;

	return hRet;
}

//////////////////////////////////////////////////////////////////////////////
void NodeProxy::SoapRequestCallbackProc(LPARAM lParam, HRESULT hRet)
{
	ATLASSERT( SUCCEEDED(hRet) );

	NodeProxy	*pNP = reinterpret_cast<NodeProxy*>(lParam);
	ATLASSERT( pNP );

	hRet = pNP->m_NSProxy.GetNodeById(pNP->Id, pNP);
	ATLASSERT( SUCCEEDED(hRet) );

	pNP->m_bSoapRequestCompleted = true;
}

//////////////////////////////////////////////////////////////////////////////
void NodeProxy::CreateNodesFromNS(Map& mapNPs)
{
	GetNode()->SetName(this->Name);
	GetNode()->SetDescription(this->Body);

	for (int nAtLink = 0; nAtLink < __Links_nSizeIs; nAtLink++)
	{
		int nToNodeId = Links[nAtLink].ToNodeId;

		// lookup if the node is already set in the collection
		NodeProxy *pNP = NULL;
		if (!mapNPs.Lookup(nToNodeId, pNP))
		{
			// create new node
			pNP = new NodeProxy(nToNodeId);
			mapNPs.SetAt(nToNodeId, pNP);
		}
		m_pNode->LinkTo(pNP->m_pNode, Links[nAtLink].Weight, 
			FALSE); // not reciprocal
	}

	m_bNodesCreated = true;
}

}
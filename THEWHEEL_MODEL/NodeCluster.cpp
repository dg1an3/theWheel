// NodeCluster.cpp: implementation of the CNodeCluster class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NodeCluster.h"

#include <math.h>
#include <float.h>

#include <MathUtil.h>

#include "Space.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNodeCluster::CNodeCluster(CSpace *pSpace, int nSiblings)
: m_pSpace(pSpace),
	m_totalActivation(0.0f),
	m_error(0.0f)
{
	int nLinkDim = m_pSpace->GetSuperNodeCount() + nSiblings;

	for (int nAt = 0; nAt < nLinkDim; nAt++)
	{
		m_arrLinkVectorCurrent.Add(0.0f);
		m_arrLinkVectorFinal.Add(0.0f);
	}

	// add this to the sibling array
	m_arrSiblings.Add(this);

	// create the siblings
	for (nAt = 1; nAt < nSiblings; nAt++)
	{
		CNodeCluster *pSibling = new CNodeCluster(pSpace, 1);
		m_arrSiblings.Add(pSibling);
	}

	// populate the siblings' sibling arrays
	for (nAt = 1; nAt < nSiblings; nAt++)
	{
		CNodeCluster *pSibling = (CNodeCluster *)m_arrSiblings[nAt];
		pSibling->m_arrSiblings.Copy(m_arrSiblings);
		pSibling->m_arrLinkVectorCurrent.RemoveAll();
		pSibling->m_arrLinkVectorFinal.RemoveAll();
		for (int nAt = 0; nAt < nLinkDim; nAt++)
		{
			pSibling->m_arrLinkVectorCurrent.Add(0.0f);
			pSibling->m_arrLinkVectorFinal.Add(0.0f);
		}
	}
}

CNodeCluster::~CNodeCluster()
{
	// delete the siblings explicitly
	for (int nAt = 1; nAt < GetSiblingCount(); nAt++)
	{
		CNodeCluster *pSibling = (CNodeCluster *)m_arrSiblings[nAt];
		pSibling->m_arrSiblings.RemoveAll();
	} 

	for (nAt = 1; nAt < GetSiblingCount(); nAt++)
	{
		CNodeCluster *pSibling = (CNodeCluster *)m_arrSiblings[nAt];
		delete pSibling;
	}
	m_arrSiblings.RemoveAll();
}

// sibling management
int CNodeCluster::GetSiblingCount() const
{
	return m_arrSiblings.GetSize();
}


CNodeCluster *CNodeCluster::GetSibling(int nAt)
{
	return (CNodeCluster *) m_arrSiblings[nAt];
}


REAL CNodeCluster::GetNodeDistanceSq(CNode *pNode)
{
	int nSuperNodes = m_pSpace->GetSuperNodeCount();
	REAL dist_sq = 0.0f;
	for (int nAt = 0; nAt < nSuperNodes; nAt++)
	{
		CNode *pSuperNode = m_pSpace->GetNodeAt(nAt);
		REAL weight = pNode->GetLinkWeight(pSuperNode);
		dist_sq += (weight - GetLinkVector()[nAt]) 
			* (weight - GetLinkVector()[nAt]);
	}

	// now add the link to the siblings
	for (nAt = 0; nAt < GetSiblingCount(); nAt++)
	{
		CNodeCluster *pCluster = GetSibling(nAt);
		REAL weight = pCluster->GetLinkWeightForNode(pNode);
		dist_sq += (weight - GetLinkVector()[nAt+nSuperNodes]) 
			* (weight - GetLinkVector()[nAt+nSuperNodes]);
	}

	return dist_sq;
}


REAL CNodeCluster::GetLinkWeightForNode(CNode *pNode)
{
	REAL total_activation = 0.0f;
	REAL total_weight = 0.0f;

	// form the weight from weighted average of links from nodes in this
	for (POSITION pos = m_mapNodes.GetStartPosition(); pos != NULL; )
	{
		CNode *pOtherNode;
		m_mapNodes.GetNextAssoc(pos, pOtherNode, pOtherNode);
			
		REAL weight = pNode->GetLinkWeight(pOtherNode);
		REAL act = (REAL) (pNode->GetActivation() + pOtherNode->GetActivation());

		total_weight += weight * act;
		total_activation += act;
	}

	if (total_activation > 0.0f)
		return total_weight / total_activation;
	else
		return 0.0f;
}


REAL CNodeCluster::GetTotalActivation() 
{ 
	return m_totalActivation; 
}


REAL CNodeCluster::GetActualTotalActivation()
{
	REAL total_activation = 0.0f;

	// form the weight from weighted average of links from nodes in this
	for (POSITION pos = m_mapNodes.GetStartPosition(); pos != NULL; )
	{
		CNode *pOtherNode;
		m_mapNodes.GetNextAssoc(pos, pOtherNode, pOtherNode);
			
		total_activation += (REAL) pOtherNode->GetActivation();
	}

	return total_activation;
}


REAL CNodeCluster::GetClusterError() 
{ 
	if (m_totalActivation > 0.0f)
		return m_error / m_totalActivation; 
	else
		return 0.0f;
}


CNodeCluster * CNodeCluster::GetNearestCluster(CNode *pNode, REAL *dist_sq)
{
	REAL minDist = FLT_MAX;
	CNodeCluster *pNearestCluster = this;
	for (int nAtCluster = 0; nAtCluster < GetSiblingCount(); nAtCluster++)
	{
		CNodeCluster *pCluster = GetSibling(nAtCluster);
		REAL dist = pCluster->GetNodeDistanceSq(pNode);
		if (dist < minDist)
		{
			minDist = dist;
			pNearestCluster = pCluster;
		}
	}

	if (dist_sq)
		(*dist_sq) = minDist;

	return pNearestCluster;
}


void CNodeCluster::AddNodeToCluster(CNode *pNode, REAL dist_sq)
{
	int nSuperNodes = m_pSpace->GetSuperNodeCount();
	REAL act = (REAL) pNode->GetActivation();
	for (int nAtSuper = 0; nAtSuper < nSuperNodes; nAtSuper++)
	{
		CNode *pSuperNode = m_pSpace->GetNodeAt(nAtSuper);
		m_arrLinkVectorCurrent[nAtSuper] += 
			act * pNode->GetLinkWeight(pSuperNode);
	}

	for (int nAt = 0; nAt < GetSiblingCount(); nAt++)
	{
		CNodeCluster *pCluster = GetSibling(nAt);
		REAL weight = pCluster->GetLinkWeightForNode(pNode);
		m_arrLinkVectorCurrent[nAt+nSuperNodes] += 
			act * pCluster->GetLinkWeightForNode(pNode);
	}

	m_error += act * dist_sq;

	m_totalActivation += act;

	m_mapNodes.SetAt(pNode, pNode);
}


void CNodeCluster::UpdateClusters()
{
	if (m_pSpace->GetNodeCount() == m_pSpace->GetSuperNodeCount())
	{
		return;
	}

	// store total activation
	REAL total_activation = m_pSpace->GetTotalActivation();

	// stores the probability of including any particular node in the update
	const REAL p_update = 1.0;

	// 
	// initialize node clusters with first N subnodes
	// iterations for the k-mean computation
	for (int nIter = 0; nIter < 1; nIter++)
	{
		// finalize node clusters
		for (int nAtCluster = 0; nAtCluster < GetSiblingCount(); nAtCluster++)
		{
			CNodeCluster *pCluster = GetSibling(nAtCluster);

			// scale the total activation based on the proportion of nodes
			//		that will be updated
			pCluster->ResetTotalActivation(1.0f - p_update);

			// updates the link vector from the current weights
			pCluster->UpdateLinkVector();
		}

		// compute how many nodes will be analyzed

		int nN = __min(m_pSpace->GetNodeCount() - m_pSpace->GetSuperNodeCount(), 
			m_pSpace->GetSuperNodeCount());

		// assign nodes to the clusters based on distance
		for (int nAtNode = m_pSpace->GetSuperNodeCount(); 
			nAtNode < m_pSpace->GetSuperNodeCount() + nN; 
			nAtNode++)
		{
			CNode *pNode = m_pSpace->GetNodeAt(nAtNode);

			// if ((REAL) rand() < p_update * (REAL) RAND_MAX)
			{
				REAL dist_sq;

				// determine the nearest cluster
				CNodeCluster *pNearestCluster = GetNearestCluster(pNode, &dist_sq);
				ASSERT(pNearestCluster);

				// now add to the appropriate cluster
				pNearestCluster->AddNodeToCluster(pNode, dist_sq);
			}

			if (pNode->GetActivation() < 0.01 * GetTotalActivation())
				nAtNode = nN;
		}

		// finalize node clusters
		for (nAtCluster = 0; nAtCluster < GetSiblingCount(); nAtCluster++)
		{
			CNodeCluster *pCluster = GetSibling(nAtCluster);
			pCluster->LoadLinkWeights();
		}
	}

	// load the node's activation
	SetActivation(GetActualTotalActivation());
}


void CNodeCluster::LoadLinkWeights()
{
	RemoveAllLinks();

	int nSuperNodes = m_pSpace->GetSuperNodeCount();
	for (int nAt = 0; nAt < nSuperNodes; nAt++)
	{
		// get the super node
		CNode *pSuperNode = m_pSpace->GetNodeAt(nAt);
		ASSERT(pSuperNode);

		// compute the weight
		REAL weight = 0.0f;
		if (m_totalActivation > 0.0f)
			weight = m_arrLinkVectorCurrent[nAt] / m_totalActivation;

		// set the link weight vector element
		m_arrLinkVectorFinal[nAt] = weight;

		// set the link weight
		LinkTo(pSuperNode, weight, FALSE);
	}

	for (nAt = 0; nAt < GetSiblingCount(); nAt++)
	{
		CNodeCluster *pCluster = GetSibling(nAt);
		REAL weight = GetLinkWeightToCluster(pCluster);

		// set the link weight vector element
		m_arrLinkVectorFinal[nAt + nSuperNodes] = weight;

		// set the link weight
		LinkTo(pCluster, weight, FALSE);
	}
}


void CNodeCluster::UpdateLinkVector()
{
	int nSuperNodes = m_pSpace->GetSuperNodeCount();
	for (int nAt = 0; nAt < nSuperNodes; nAt++)
	{
		m_arrLinkVectorFinal[nAt] = 0.0f;
	}

	for (nAt = 0; nAt < nSuperNodes; nAt++)
	{
		// get the super node
		CNode *pSuperNode = m_pSpace->GetNodeAt(nAt);

		m_arrLinkVectorFinal[nAt] = GetLinkWeight(pSuperNode);
	}

	for (nAt = 0; nAt < GetSiblingCount(); nAt++)
	{
		// get the super node
		CNodeCluster *pCluster = GetSibling(nAt);

		m_arrLinkVectorFinal[nAt+nSuperNodes] = GetLinkWeight(pCluster);
	}
}


REAL * CNodeCluster::GetLinkVector()
{
	return m_arrLinkVectorFinal.GetData();
}



void CNodeCluster::ResetTotalActivation(REAL scale) 
{ 
	for (int nAt = 0; nAt < m_pSpace->GetSuperNodeCount() + GetSiblingCount(); nAt++)
	{
		m_arrLinkVectorCurrent[nAt] *= (REAL) scale;
	}
	m_error *= (REAL) scale;
	m_totalActivation *= (REAL) scale;
}


REAL CNodeCluster::GetLinkWeightToCluster(CNodeCluster *pToCluster)
{
	REAL total_activation = 0.0f;
	REAL total_weight = 0.0f;

	// form the weight from weighted average of links from nodes in this
	for (POSITION pos = m_mapNodes.GetStartPosition(); pos != NULL; )
	{
		CNode *pNode;
		m_mapNodes.GetNextAssoc(pos, pNode, pNode);		
		for (POSITION posOther = pToCluster->m_mapNodes.GetStartPosition(); posOther != NULL; )
		{
			CNode *pOtherNode;
			pToCluster->m_mapNodes.GetNextAssoc(posOther, pOtherNode, pOtherNode);
			
			REAL weight = pNode->GetLinkWeight(pOtherNode);
			REAL act = (REAL)(pNode->GetActivation() + pOtherNode->GetActivation());

			total_weight += weight * act;
			total_activation += act;
		}
	}

	if (total_activation > 0.0f)
		return total_weight / total_activation;
	else
		return 0.0f;
}


void CNodeCluster::DumpLinkVector()
{

	TRACE("Cluster link vector\n");
	for (int nAt = 0; nAt < m_pSpace->GetSuperNodeCount()+GetSiblingCount(); nAt++)
	{
		TRACE("\tLink to node %s: \t\t%lf\n",
			m_pSpace->GetNodeAt(nAt)->GetName(),
			GetLinkVector()[nAt]);
	}
}



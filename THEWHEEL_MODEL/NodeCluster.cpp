// NodeCluster.cpp: implementation of the CNodeCluster class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NodeCluster.h"

#include <math.h>
#include <float.h>

#include "Space.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNodeCluster::CNodeCluster(CSpace *pSpace, int nSiblings)
: m_pSpace(pSpace),
	m_pLinkVectorCurrent(NULL), 
	m_pLinkVectorFinal(NULL),
	m_totalActivation(0.0f),
	m_error(0.0f),
	m_vCenter(CVector<2>(300.0, 300.0))
{
	int nLinkDim = m_pSpace->GetSuperNodeCount() + nSiblings;

	m_pLinkVectorCurrent = new float[nLinkDim];
	m_pLinkVectorFinal = new float[nLinkDim];
	for (int nAt = 0; nAt < nLinkDim; nAt++)
	{
		m_pLinkVectorCurrent[nAt] = 0.0f;
		m_pLinkVectorFinal[nAt] = 0.0f;
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
		pSibling->m_pLinkVectorCurrent = new float[nLinkDim];
		pSibling->m_pLinkVectorFinal = new float[nLinkDim];
		for (int nAt = 0; nAt < nLinkDim; nAt++)
		{
			pSibling->m_pLinkVectorCurrent[nAt] = 0.0f;
			pSibling->m_pLinkVectorFinal[nAt] = 0.0f;
		}
	}
}

CNodeCluster::~CNodeCluster()
{
/*	delete [] m_pLinkVectorCurrent;
	delete [] m_pLinkVectorFinal;

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
	m_arrSiblings.RemoveAll(); */
}

float CNodeCluster::GetNodeDistanceSq(CNode *pNode)
{
	int nSuperNodes = m_pSpace->GetSuperNodeCount();
	float dist_sq = 0.0f;
	for (int nAt = 0; nAt < nSuperNodes; nAt++)
	{
		CNode *pSuperNode = m_pSpace->GetNodeAt(nAt);
		float weight = pNode->GetLinkWeight(pSuperNode);
		dist_sq += (weight - GetLinkVector()[nAt]) 
			* (weight - GetLinkVector()[nAt]);
	}

	// now add the link to the siblings
	for (nAt = 0; nAt < GetSiblingCount(); nAt++)
	{
		CNodeCluster *pCluster = GetSibling(nAt);
		float weight = pCluster->GetLinkWeightForNode(pNode);
		dist_sq += (weight - GetLinkVector()[nAt+nSuperNodes]) 
			* (weight - GetLinkVector()[nAt+nSuperNodes]);
	}

	return dist_sq;
}

float * CNodeCluster::GetLinkVector()
{
	return m_pLinkVectorFinal;
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

void CNodeCluster::AddNodeToCluster(CNode *pNode, float dist_sq)
{
	int nSuperNodes = m_pSpace->GetSuperNodeCount();
	float act = pNode->GetActivation();
	for (int nAtSuper = 0; nAtSuper < nSuperNodes; nAtSuper++)
	{
		CNode *pSuperNode = m_pSpace->GetNodeAt(nAtSuper);
		m_pLinkVectorCurrent[nAtSuper] += 
			act * pNode->GetLinkWeight(pSuperNode);
	}

	for (int nAt = 0; nAt < GetSiblingCount(); nAt++)
	{
		CNodeCluster *pCluster = GetSibling(nAt);
		float weight = pCluster->GetLinkWeightForNode(pNode);
		m_pLinkVectorCurrent[nAt+nSuperNodes] += 
			act * pCluster->GetLinkWeightForNode(pNode);
	}

	m_error += act * dist_sq;

	m_totalActivation += act;

	m_mapNodes.SetAt(pNode, pNode);
}

void CNodeCluster::ResetTotalActivation(double scale) 
{ 
	for (int nAt = 0; nAt < m_pSpace->GetSuperNodeCount() + GetSiblingCount(); nAt++)
	{
		m_pLinkVectorCurrent[nAt] *= scale;
	}
	m_error *= scale;
	m_totalActivation *= scale;
}

float CNodeCluster::GetLinkWeightToCluster(CNodeCluster *pToCluster)
{
	float total_activation = 0.0f;
	float total_weight = 0.0f;

	// form the weight from weighted average of links from nodes in this
	for (POSITION pos = m_mapNodes.GetStartPosition(); pos != NULL; )
	{
		CNode *pNode;
		m_mapNodes.GetNextAssoc(pos, pNode, pNode);		
		for (POSITION posOther = pToCluster->m_mapNodes.GetStartPosition(); posOther != NULL; )
		{
			CNode *pOtherNode;
			pToCluster->m_mapNodes.GetNextAssoc(posOther, pOtherNode, pOtherNode);
			
			float weight = pNode->GetLinkWeight(pOtherNode);
			float act = pNode->GetActivation() + pOtherNode->GetActivation();

			total_weight += weight * act;
			total_activation += act;
		}
	}

	if (total_activation > 0.0f)
		return total_weight / total_activation;
	else
		return 0.0f;
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
		float weight = 0.0f;
		if (m_totalActivation > 0.0f)
			weight = m_pLinkVectorCurrent[nAt] / m_totalActivation;

		// set the link weight vector element
		m_pLinkVectorFinal[nAt] = weight;

		// set the link weight
		LinkTo(pSuperNode, weight, FALSE);
	}

	for (nAt = 0; nAt < GetSiblingCount(); nAt++)
	{
		CNodeCluster *pCluster = GetSibling(nAt);
		float weight = GetLinkWeightToCluster(pCluster);

		// set the link weight vector element
		m_pLinkVectorFinal[nAt + nSuperNodes] = weight;

		// set the link weight
		LinkTo(pCluster, weight, FALSE);
	}
}

void CNodeCluster::InitRandom()
{
	for (int nAt = 0; nAt < m_pSpace->GetSuperNodeCount(); nAt++)
	{
		m_pLinkVectorCurrent[nAt] = 1.0f * (float) rand() / (float) RAND_MAX;
	}

	ASSERT(FALSE);

	m_totalActivation = 1.0f;
	LoadLinkWeights();
	ResetTotalActivation();
}

void CNodeCluster::UpdateLinkVector()
{
	int nSuperNodes = m_pSpace->GetSuperNodeCount();
	for (int nAt = 0; nAt < nSuperNodes; nAt++)
	{
		m_pLinkVectorFinal[nAt] = 0.0f;
	}

	for (nAt = 0; nAt < nSuperNodes; nAt++)
	{
		// get the super node
		CNode *pSuperNode = m_pSpace->GetNodeAt(nAt);

		m_pLinkVectorFinal[nAt] = GetLinkWeight(pSuperNode);
	}

	for (nAt = 0; nAt < GetSiblingCount(); nAt++)
	{
		// get the super node
		CNodeCluster *pCluster = GetSibling(nAt);

		m_pLinkVectorFinal[nAt+nSuperNodes] = GetLinkWeight(pCluster);
	}
}

float CNodeCluster::GetLinkWeightForNode(CNode *pNode)
{
	float total_activation = 0.0f;
	float total_weight = 0.0f;

	// form the weight from weighted average of links from nodes in this
	for (POSITION pos = m_mapNodes.GetStartPosition(); pos != NULL; )
	{
		CNode *pOtherNode;
		m_mapNodes.GetNextAssoc(pos, pOtherNode, pOtherNode);
			
		float weight = pNode->GetLinkWeight(pOtherNode);
		float act = pNode->GetActivation() + pOtherNode->GetActivation();

		total_weight += weight * act;
		total_activation += act;
	}

	if (total_activation > 0.0f)
		return total_weight / total_activation;
	else
		return 0.0f;
}

CNodeCluster * CNodeCluster::GetNearestCluster(CNode *pNode, float *dist_sq)
{
	float minDist = FLT_MAX;
	CNodeCluster *pNearestCluster = this;
	for (int nAtCluster = 0; nAtCluster < GetSiblingCount(); nAtCluster++)
	{
		CNodeCluster *pCluster = GetSibling(nAtCluster);
		float dist = pCluster->GetNodeDistanceSq(pNode);
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

void CNodeCluster::UpdateClusters()
{
	// store total activation
	double total_activation = m_pSpace->GetRootNode()->GetDescendantActivation();

	// stores the probability of including any particular node in the update
	const float p_update = 1.0;

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

		int nN = min(m_pSpace->GetNodeCount(), 
			m_pSpace->GetSuperNodeCount() * GetSiblingCount());

		// assign nodes to the clusters based on distance
		for (int nAtNode = m_pSpace->GetSuperNodeCount(); nAtNode < nN; nAtNode++)
		{
			CNode *pNode = m_pSpace->GetNodeAt(nAtNode);

			if ((float) rand() < p_update * (float) RAND_MAX)
			{
				float dist_sq;

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

/*			// load the links to other clusters
			for (int nAtOtherCluster = 0; nAtOtherCluster < GetClusterCount(); nAtOtherCluster++)
			{
				CNodeCluster *pOtherCluster = GetClusterAt(nAtOtherCluster);
				if (pCluster != pOtherCluster)
				{
					float weight = pCluster->GetLinkWeightToCluster(pOtherCluster);
					pCluster->LinkTo(pOtherCluster, weight, FALSE);
				}
			} */
		}
	}
}

float CNodeCluster::GetActualTotalActivation()
{
	float total_activation = 0.0f;

	// form the weight from weighted average of links from nodes in this
	for (POSITION pos = m_mapNodes.GetStartPosition(); pos != NULL; )
	{
		CNode *pOtherNode;
		m_mapNodes.GetNextAssoc(pos, pOtherNode, pOtherNode);
			
		total_activation += pOtherNode->GetActivation();
	}

	return total_activation;
}

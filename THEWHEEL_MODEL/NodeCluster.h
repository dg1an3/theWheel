// NodeCluster.h: interface for the CNodeCluster class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODECLUSTER_H__678A3037_1CBC_433C_972B_73FC63831C04__INCLUDED_)
#define AFX_NODECLUSTER_H__678A3037_1CBC_433C_972B_73FC63831C04__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Node.h"
#include <Vector.h>

class CNodeCluster : public CNode  
{
public:
	float GetActualTotalActivation();
	CNodeCluster(CSpace *pSpace = NULL, int nSiblings = 1);
	virtual ~CNodeCluster();

	// sibling management
	int GetSiblingCount() const
	{
		return m_arrSiblings.GetSize();
	}
	CNodeCluster *GetSibling(int nAt)
	{
		return (CNodeCluster *) m_arrSiblings[nAt];
	}

	// operations on individual nodes
	float GetNodeDistanceSq(CNode *pNode);

	float GetTotalActivation() { return m_totalActivation; }

	float GetClusterError() 
	{ 
		if (m_totalActivation > 0.0f)
			return m_error / m_totalActivation; 
		else
			return 0.0f;
	}

	void UpdateClusters();
	CNodeCluster * GetNearestCluster(CNode *pNode, float *dist_sq = NULL);
	float GetLinkWeightForNode(CNode *pNode);

	void DumpLinkVector();

	CVector<2> m_vCenter;

	void UpdateLinkVector();
	void InitRandom();

protected:
	friend CSpace;

	// add a node to the cluster
	void AddNodeToCluster(CNode *pNode, float dist_sq);

	// retrieves the cluster's link vector (only valid after a
	//	LoadLinkWeights call
	float * GetLinkVector();

	// reset the total activation 
	//		(prepares for another iteration of the k-means clustering,
	//			but leaves GetLinkVector and the node map intact)
	void ResetTotalActivation(double scale = 0.5);

	// helper to load the normalized link vector into the node links,
	//		including the average weights to other clusters
	void LoadLinkWeights();

	// retrieves the link weight to another cluster
	float GetLinkWeightToCluster(CNodeCluster *toCluster);

private:
	CSpace *m_pSpace;

	CObArray m_arrSiblings;

	// the link vector for this currently forming population
	float * m_pLinkVectorCurrent;
	float * m_pLinkVectorFinal;

	// total activation for all nodes added to the cluster
	float m_totalActivation;

	float m_error;

	// contains the nodes (in an easy-to-access format)
	CMap<CNode *, CNode *, CNode *, CNode *> m_mapNodes;
};

#endif // !defined(AFX_NODECLUSTER_H__678A3037_1CBC_433C_972B_73FC63831C04__INCLUDED_)

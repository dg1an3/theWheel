//////////////////////////////////////////////////////////////////////
// NodeCluster.h: interface for the CNodeCluster class.
//
// Copyright (C) 1999-2002 Derek Graham Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(NODECLUSTER_H)
#define NODECLUSTER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// base class
#include "Node.h"

//////////////////////////////////////////////////////////////////////
// class CNodeCluster
// 
// a statistical composite of many nodes
//////////////////////////////////////////////////////////////////////
class CNodeCluster : public CNode  
{
public:
	// construct
	CNodeCluster(CSpace *pSpace = NULL, int nSiblings = 1);
	virtual ~CNodeCluster();

	// sibling accessors
	int GetSiblingCount() const;
	CNodeCluster *GetSibling(int nAt);

	// returns the distance to a particular node
	REAL GetNodeDistanceSq(CNode *pNode);

	// returns the weight from the cluster to a particular node
	REAL GetLinkWeightForNode(CNode *pNode);

	// retrieves two versions of the cluster's total activation
	REAL GetTotalActivation();
	REAL GetActualTotalActivation();

	// returns the total cluster error
	REAL GetClusterError();

	// returns the nearest cluster, also returns the distance squared
	CNodeCluster * GetNearestCluster(CNode *pNode, REAL *dist_sq = NULL);

protected:
	friend CSpace;

	// add a node to the cluster
	void AddNodeToCluster(CNode *pNode, REAL dist_sq);

	// updates the clusters after a change to the node activations
	void UpdateClusters();

	// helper to load the normalized link vector into the node links,
	//		including the average weights to other clusters
	void LoadLinkWeights();

	// updates the link vector from the link weights
	void UpdateLinkVector();

	// retrieves the cluster's link vector (only valid after a
	//	LoadLinkWeights call)
	REAL * GetLinkVector();

	// reset the total activation 
	//		(prepares for another iteration of the k-means clustering,
	//			but leaves GetLinkVector and the node map intact)
	void ResetTotalActivation(REAL scale = 0.5);

	// retrieves the link weight to another cluster
	REAL GetLinkWeightToCluster(CNodeCluster *toCluster);

	// helper to dump the link vector
	void DumpLinkVector();

private:
	// the space for the clusters
	CSpace *m_pSpace;

	// the array of siblings
	CObArray m_arrSiblings;

	// the link vector for this currently forming population
	CArray<REAL, REAL> m_arrLinkVectorCurrent;

	// the 'final' link vector after the last update
	CArray<REAL, REAL> m_arrLinkVectorFinal;

	// total activation for all nodes added to the cluster
	REAL m_totalActivation;

	// error (mean squared error) for the cluster
	REAL m_error;

	// contains the nodes (in an easy-to-access format)
	CMap<CNode *, CNode *, CNode *, CNode *> m_mapNodes;
};

#endif // !defined(NODECLUSTER_H)

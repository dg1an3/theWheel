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
	CNodeCluster(CSpace *pSpace = NULL, int nSiblings = 1);
	virtual ~CNodeCluster();

	// sibling accessors
	int GetSiblingCount() const;
	CNodeCluster *GetSibling(int nAt);

	// returns the distance to a particular node
	float GetNodeDistanceSq(CNode *pNode);

	// returns the weight from the cluster to a particular node
	float GetLinkWeightForNode(CNode *pNode);

	// retrieves two versions of the cluster's total activation
	float GetTotalActivation();
	float GetActualTotalActivation();

	// returns the total cluster error
	float GetClusterError();

	// returns the nearest cluster, also returns the distance squared
	CNodeCluster * GetNearestCluster(CNode *pNode, float *dist_sq = NULL);

protected:
	friend CSpace;

	// add a node to the cluster
	void AddNodeToCluster(CNode *pNode, float dist_sq);

	// updates the clusters after a change to the node activations
	void UpdateClusters();

	// helper to load the normalized link vector into the node links,
	//		including the average weights to other clusters
	void LoadLinkWeights();

	// updates the link vector from the link weights
	void UpdateLinkVector();

	// retrieves the cluster's link vector (only valid after a
	//	LoadLinkWeights call)
	float * GetLinkVector();

	// reset the total activation 
	//		(prepares for another iteration of the k-means clustering,
	//			but leaves GetLinkVector and the node map intact)
	void ResetTotalActivation(double scale = 0.5);

	// retrieves the link weight to another cluster
	float GetLinkWeightToCluster(CNodeCluster *toCluster);

	// helper to dump the link vector
	void DumpLinkVector();

private:
	// the space for the clusters
	CSpace *m_pSpace;

	// the array of siblings
	CObArray m_arrSiblings;

	// the link vector for this currently forming population
	CArray<float, float> m_arrLinkVectorCurrent;

	// the 'final' link vector after the last update
	CArray<float, float> m_arrLinkVectorFinal;

	// total activation for all nodes added to the cluster
	float m_totalActivation;

	// error (mean squared error) for the cluster
	float m_error;

	// contains the nodes (in an easy-to-access format)
	CMap<CNode *, CNode *, CNode *, CNode *> m_mapNodes;
};

#endif // !defined(AFX_NODECLUSTER_H__678A3037_1CBC_433C_972B_73FC63831C04__INCLUDED_)

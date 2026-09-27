//////////////////////////////////////////////////////////////////////
// DisplayPolicy.h: declaration of the CDisplayPolicy class
//
// Chooses which nodes to display by minimizing expected free energy.
// See scripts/efe_policy.py for the prototype and evaluation.
//////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <vector>

class CNode;

//////////////////////////////////////////////////////////////////////
// class CDisplayPolicy
//
// active-inference model of browsing:
//
//	hidden state s:	the node the user is looking for
//	belief q(s):	activation (prior, from spreading activation) times
//			exp(click evidence), normalized
//	policy:		the set of nodes to display
//	outcome o:	which displayed node the user clicks, or no click;
//			P(o = i | s) = softmax(beta * relatedness(i, s)),
//			against a "no click" score of beta * tau
//
// the displayed set minimizes
//
//	G = -I(s; o) - gamma * P(user clicks s)
//
// greedily, one node at a time, scoring each candidate set as a whole
//////////////////////////////////////////////////////////////////////
class CDisplayPolicy
{
public:
	CDisplayPolicy();

	// model parameters
	double GetBeta() const { return m_beta; }
	void SetBeta(double beta);
	double GetTau() const { return m_tau; }
	void SetTau(double tau) { m_tau = tau; }
	double GetGamma() const { return m_gamma; }
	void SetGamma(double gamma) { m_gamma = gamma; }

	// forces relatedness to be recomputed (call when links change)
	void Invalidate();

	// strongest-path relatedness between two nodes (1.0 for the same
	//		node, 0.0 if unconnected); nodes must be in the last set
	//		passed to Select or Update
	double GetRelatedness(CNode *pFrom, CNode *pTo);

	// recomputes relatedness if the node set has changed
	void Update(const std::vector<CNode *>& arrNodes);

	// the belief q(s) over the nodes, normalized
	void GetBelief(const std::vector<CNode *>& arrNodes,
		std::vector<double>& q);

	// chooses nNumDisplayed nodes; returns their indices in arrNodes
	void Select(const std::vector<CNode *>& arrNodes, int nNumDisplayed,
		std::vector<int>& arrSelected);

	// updates each node's click evidence after the user clicks
	//		pClicked among the displayed nodes, tempered by weight
	//		(1.0 for a full click)
	void ObserveClick(const std::vector<CNode *>& arrNodes,
		const std::vector<CNode *>& arrDisplayed, CNode *pClicked,
		double weight);

private:
	// model parameters
	double m_beta;
	double m_tau;
	double m_gamma;

	// cached relatedness, indexed by m_mapIndex
	std::map<CNode *, int> m_mapIndex;
	std::vector<double> m_related;
	bool m_bValid;
};

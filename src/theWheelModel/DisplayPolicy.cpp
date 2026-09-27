//////////////////////////////////////////////////////////////////////
// DisplayPolicy.cpp: implementation of the CDisplayPolicy class
//
// Chooses which nodes to display by minimizing expected free energy.
// See scripts/efe_policy.py for the prototype and evaluation.
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

#include <algorithm>
#include <cmath>

// the node class
#include "Node.h"

// the class definition
#include "DisplayPolicy.h"

//////////////////////////////////////////////////////////////////////
// constants
//////////////////////////////////////////////////////////////////////

// how sharply the user clicks nodes related to their target
const double DEFAULT_BETA = 6.0;

// relatedness a displayed node needs to beat "no click"
const double DEFAULT_TAU = 0.3;

// preference for the user finding their target (nats)
const double DEFAULT_GAMMA = 1.0;

// fraction of click evidence forgotten per full click, so the belief
//		can follow a user whose target changes
const double EVIDENCE_DECAY = 0.05;


//////////////////////////////////////////////////////////////////////
CDisplayPolicy::CDisplayPolicy()
	: m_beta(DEFAULT_BETA)
	, m_tau(DEFAULT_TAU)
	, m_gamma(DEFAULT_GAMMA)
	, m_bValid(false)
{
}	// CDisplayPolicy::CDisplayPolicy


//////////////////////////////////////////////////////////////////////
void
	CDisplayPolicy::SetBeta(double beta)
	// sets the click sharpness
{
	m_beta = beta;

}	// CDisplayPolicy::SetBeta


//////////////////////////////////////////////////////////////////////
void
	CDisplayPolicy::Invalidate()
	// forces relatedness to be recomputed
{
	m_bValid = false;

}	// CDisplayPolicy::Invalidate


//////////////////////////////////////////////////////////////////////
void
	CDisplayPolicy::Update(const std::vector<CNode *>& arrNodes)
	// recomputes relatedness if the node set has changed
{
	const int nNodes = (int) arrNodes.size();

	// same node set?
	if (m_bValid && (int) m_mapIndex.size() == nNodes)
	{
		bool bSame = true;
		for (auto pNode : arrNodes)
		{
			if (m_mapIndex.find(pNode) == m_mapIndex.end())
			{
				bSame = false;
				break;
			}
		}
		if (bSame)
		{
			return;
		}
	}

	m_mapIndex.clear();
	for (int nAt = 0; nAt < nNodes; nAt++)
	{
		m_mapIndex[arrNodes[nAt]] = nAt;
	}

	// strongest direct link between each pair
	m_related.assign(nNodes * nNodes, 0.0);
	for (int nAt = 0; nAt < nNodes; nAt++)
	{
		m_related[nAt * nNodes + nAt] = 1.0;
		for (int nAtLink = 0; nAtLink < arrNodes[nAt]->GetLinkCount(); nAtLink++)
		{
			CNodeLink *pLink = arrNodes[nAt]->GetLinkAt(nAtLink);
			auto iter = m_mapIndex.find(pLink->GetTarget());
			if (iter == m_mapIndex.end() || iter->second == nAt)
			{
				continue;
			}
			const double weight = std::min(1.0, std::max(0.0,
				(double) pLink->GetWeight()));
			double& fwd = m_related[nAt * nNodes + iter->second];
			double& rev = m_related[iter->second * nNodes + nAt];
			fwd = rev = std::max(fwd, std::max(rev, weight));
		}
	}

	// strongest path: max over paths of the product of link weights
	for (int k = 0; k < nNodes; k++)
	{
		for (int i = 0; i < nNodes; i++)
		{
			const double r_ik = m_related[i * nNodes + k];
			if (r_ik == 0.0)
			{
				continue;
			}
			for (int j = 0; j < nNodes; j++)
			{
				const double r_ikj = r_ik * m_related[k * nNodes + j];
				if (r_ikj > m_related[i * nNodes + j])
				{
					m_related[i * nNodes + j] = r_ikj;
				}
			}
		}
	}

	m_bValid = true;

}	// CDisplayPolicy::Update


//////////////////////////////////////////////////////////////////////
double
	CDisplayPolicy::GetRelatedness(CNode *pFrom, CNode *pTo)
	// strongest-path relatedness between two nodes
{
	auto iterFrom = m_mapIndex.find(pFrom);
	auto iterTo = m_mapIndex.find(pTo);
	if (iterFrom == m_mapIndex.end() || iterTo == m_mapIndex.end())
	{
		return 0.0;
	}
	return m_related[iterFrom->second * m_mapIndex.size() + iterTo->second];

}	// CDisplayPolicy::GetRelatedness


//////////////////////////////////////////////////////////////////////
void
	CDisplayPolicy::GetBelief(const std::vector<CNode *>& arrNodes,
		std::vector<double>& q)
	// q(s) proportional to activation * exp(click evidence)
{
	q.resize(arrNodes.size());
	double sum = 0.0;
	for (size_t nAt = 0; nAt < arrNodes.size(); nAt++)
	{
		q[nAt] = std::max(0.0, (double) arrNodes[nAt]->GetActivation())
			* exp((double) arrNodes[nAt]->GetClickEvidence());
		sum += q[nAt];
	}
	for (auto& value : q)
	{
		value = (sum > 0.0) ? value / sum : 1.0 / q.size();
	}

}	// CDisplayPolicy::GetBelief


//////////////////////////////////////////////////////////////////////
void
	CDisplayPolicy::Select(const std::vector<CNode *>& arrNodes,
		int nNumDisplayed, std::vector<int>& arrSelected)
	// greedy minimization of the expected free energy of the set
{
	const int nNodes = (int) arrNodes.size();
	nNumDisplayed = std::min(nNumDisplayed, nNodes);
	arrSelected.clear();
	if (nNumDisplayed <= 0)
	{
		return;
	}

	Update(arrNodes);

	std::vector<double> q;
	GetBelief(arrNodes, q);

	// map the current order to the relatedness index
	std::vector<int> arrIndex(nNodes);
	for (int nAt = 0; nAt < nNodes; nAt++)
	{
		arrIndex[nAt] = m_mapIndex[arrNodes[nAt]];
	}

	// logits and their exponentials, [target * nNodes + node]
	std::vector<double> logit(nNodes * nNodes);
	std::vector<double> e(nNodes * nNodes);
	for (int s = 0; s < nNodes; s++)
	{
		for (int i = 0; i < nNodes; i++)
		{
			logit[s * nNodes + i] = m_beta
				* m_related[arrIndex[s] * nNodes + arrIndex[i]];
			e[s * nNodes + i] = exp(logit[s * nNodes + i]);
		}
	}
	const double logitNone = m_beta * m_tau;
	const double eNone = exp(logitNone);
	const double eSelf = exp(m_beta);

	// per-target sums over the selected set of e and e * logit
	std::vector<double> zBase(nNodes, 0.0);
	std::vector<double> elBase(nNodes, 0.0);
	std::vector<bool> bSelected(nNodes, false);
	std::vector<double> qOutcome;

	for (int nRound = 0; nRound < nNumDisplayed; nRound++)
	{
		const int nShown = (int) arrSelected.size();
		int nBest = -1;
		double bestEfe = 0.0;

		for (int c = 0; c < nNodes; c++)
		{
			if (bSelected[c])
			{
				continue;
			}

			// outcome probabilities: selected nodes, candidate, no click
			qOutcome.assign(nShown + 2, 0.0);
			double condEntropy = 0.0;
			double pHit = 0.0;

			for (int s = 0; s < nNodes; s++)
			{
				if (q[s] == 0.0)
				{
					continue;
				}
				const double eCand = e[s * nNodes + c];
				const double z = zBase[s] + eCand + eNone;
				const double invZ = 1.0 / z;
				const double qInvZ = q[s] * invZ;

				// H[P(o | s)] = ln Z - sum(e * logit) / Z
				condEntropy += q[s] * (log(z) - (elBase[s]
					+ eCand * logit[s * nNodes + c] + eNone * logitNone) * invZ);

				for (int j = 0; j < nShown; j++)
				{
					qOutcome[j] += qInvZ * e[s * nNodes + arrSelected[j]];
				}
				qOutcome[nShown] += qInvZ * eCand;
				qOutcome[nShown + 1] += qInvZ * eNone;

				// the user clicks their target if it is shown
				if (bSelected[s] || s == c)
				{
					pHit += qInvZ * eSelf;
				}
			}

			double outcomeEntropy = 0.0;
			for (auto p : qOutcome)
			{
				if (p > 0.0)
				{
					outcomeEntropy -= p * log(p);
				}
			}

			const double efe = -(outcomeEntropy - condEntropy) - m_gamma * pHit;
			if (nBest < 0 || efe < bestEfe)
			{
				nBest = c;
				bestEfe = efe;
			}
		}

		// add the best candidate to the set
		arrSelected.push_back(nBest);
		bSelected[nBest] = true;
		for (int s = 0; s < nNodes; s++)
		{
			zBase[s] += e[s * nNodes + nBest];
			elBase[s] += e[s * nNodes + nBest] * logit[s * nNodes + nBest];
		}
	}

}	// CDisplayPolicy::Select


//////////////////////////////////////////////////////////////////////
void
	CDisplayPolicy::ObserveClick(const std::vector<CNode *>& arrNodes,
		const std::vector<CNode *>& arrDisplayed, CNode *pClicked,
		double weight)
	// bayesian update of the click evidence: each node s gains
	//		weight * ln P(click on pClicked | target s, displayed set)
{
	if (weight <= 0.0)
	{
		return;
	}
	weight = std::min(weight, 1.0);

	// forget some of the old evidence
	for (auto pNode : arrNodes)
	{
		pNode->SetClickEvidence((REAL) (pNode->GetClickEvidence()
			* (1.0 - EVIDENCE_DECAY * weight)));
	}

	// a click on a node that is not displayed (e.g. chosen from a
	//		list) has no likelihood under the model
	if (std::find(arrDisplayed.begin(), arrDisplayed.end(), pClicked)
		== arrDisplayed.end())
	{
		return;
	}

	Update(arrNodes);

	std::vector<double> arrEvidence(arrNodes.size());
	double maxEvidence = -1e300;
	for (size_t nAt = 0; nAt < arrNodes.size(); nAt++)
	{
		CNode *pTarget = arrNodes[nAt];
		double z = exp(m_beta * m_tau);
		for (auto pShown : arrDisplayed)
		{
			z += exp(m_beta * GetRelatedness(pShown, pTarget));
		}
		const double logLikelihood = m_beta * GetRelatedness(pClicked, pTarget) 
			- log(z);
		arrEvidence[nAt] = pTarget->GetClickEvidence() + weight * logLikelihood;
		maxEvidence = std::max(maxEvidence, arrEvidence[nAt]);
	}

	// only relative evidence matters; keep the largest at zero
	for (size_t nAt = 0; nAt < arrNodes.size(); nAt++)
	{
		arrNodes[nAt]->SetClickEvidence((REAL) (arrEvidence[nAt] - maxEvidence));
	}

}	// CDisplayPolicy::ObserveClick

////////////////////////////////////
// Copyright (C) 1996-2000 DG Lane
// U.S. Patent Pending
////////////////////////////////////

#include "stdafx.h"

#include <ScalarFunction.h>
#include <LookupFunction.h>

#include "SpaceViewEnergyFunction.h"
#include "SpaceView.h"

SPV_STATE_TYPE SPACER_AMPL = 35.0f;
SPV_STATE_TYPE ATTRACT_AMPL = 140.0f; 

SPV_STATE_TYPE CENTER_AMPL = 0.03f;

SPV_STATE_TYPE attract_func(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return Gauss2D<SPV_STATE_TYPE>(x, y , 1.0f, 1.0f);
}

CLookupFunction<SPV_STATE_TYPE> attractFunc(&attract_func, 
		-4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, "ATTRFUNC.TMP");

SPV_STATE_TYPE spacer_func(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return // 2.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f / 2.0f, 1.0f / 2.0f)
		+ 4.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f / 4.0f, 1.0f / 4.0f)
		+ 8.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f / 8.0f, 1.0f / 8.0f);
}

CLookupFunction<SPV_STATE_TYPE> spacerFunc(&spacer_func, 
		-4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, "SPACERFUNC.TMP");

SPV_STATE_TYPE MinSize(SPV_STATE_TYPE x, SPV_STATE_TYPE xmin)
{
	SPV_STATE_TYPE frac = x / (x + xmin);

	return (1.0f - frac) * xmin + frac * x;
}

SPV_STATE_TYPE CenterField(SPV_STATE_TYPE x, SPV_STATE_TYPE y, SPV_STATE_TYPE width, SPV_STATE_TYPE height, SPV_STATE_TYPE cs)
{
	SPV_STATE_TYPE energy = 0.0f;

	// compute distance to nearest side
	SPV_STATE_TYPE distToLeft = x;
	if (distToLeft < 0.0)
		energy += -distToLeft;

	SPV_STATE_TYPE distToRight = width - x;
	if (distToRight < 0.0)
		energy += -distToRight;

	SPV_STATE_TYPE distToTop = y;
	if (distToTop < 0.0)
		energy += -distToTop;

	SPV_STATE_TYPE distToBottom = height - y;
	if (distToBottom < 0.0)
		energy += -distToBottom;

	SPV_STATE_TYPE distHorz = min(distToLeft, distToRight);
	SPV_STATE_TYPE distVert = min(distToTop, distToBottom);

	SPV_STATE_TYPE distMin = min(distHorz, distVert);

	return energy + 24.0f * Gauss(distMin + cs, cs);
}

CVector<SPV_STATE_DIM, SPV_STATE_TYPE> CSpaceViewEnergyFunction::GetStateVector()
{
	// compute the threshold
	GetThreshold();

	// set up the mapping array
//	m_arrElementMap.SetSize(m_pView->nodeViews.GetSize());

	CVector<SPV_STATE_DIM, SPV_STATE_TYPE> vState;
	for (int nAt = 0; nAt < SPV_STATE_DIM / 2; nAt++)
	{
		if (nAt < m_pView->nodeViews.GetSize())
		{
			CNodeView *pView = m_pView->nodeViews.Get(nAt);
			vState[nAt*2] = (SPV_STATE_TYPE) pView->center.Get()[0];
			vState[nAt*2+1] = (SPV_STATE_TYPE) pView->center.Get()[1];
		}
		else
		{
			vState[nAt*2] = (SPV_STATE_TYPE) 0.0;
			vState[nAt*2+1] = (SPV_STATE_TYPE) 0.0;
		}
	}

/*	int nAtVectorElement = 0;
	for (int nAt = 0; nAt < m_pView->nodeViews.GetSize() && nAtVectorElement < SPV_STATE_DIM; nAt++)
	{
		CNodeView *pView = m_pView->nodeViews.Get(nAt);
		if (pView->activation.Get() >= CNodeView::activationThreshold)
		{
			vState[nAtVectorElement] = (SPV_STATE_TYPE) pView->center.Get()[0];
			vState[nAtVectorElement+1] = (SPV_STATE_TYPE) pView->center.Get()[1];

			m_arrElementMap[nAt] = nAtVectorElement;

			nAtVectorElement += 2;
		}
		else
			// flag to indicate the node view is not represented in the state vector
			m_arrElementMap[nAt] = -1;
	}
*/
	return vState;
}

void CSpaceViewEnergyFunction::SetStateVector(const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vState)
{
	int nNumVizNodeViews = min(m_pView->nodeViews.GetSize(), SPV_STATE_DIM / 2);
	for (int nAt = 0; nAt < nNumVizNodeViews; nAt++)
	{
		CNodeView *pView = m_pView->nodeViews.Get(nAt);
		pView->center.Set(CVector<2>(vState[nAt*2], vState[nAt*2+1]));
	}

/*	for (int nAt = 0; nAt < m_pView->nodeViews.GetSize(); nAt++)
	{
		if (m_arrElementMap[nAt] != -1)
		{
			CNodeView *pView = m_pView->nodeViews.Get(nAt);
			pView->center.Set(CVector<2>(vState[m_arrElementMap[nAt]], 
				vState[m_arrElementMap[nAt]+1]));
		}
	} */
}


SPV_STATE_TYPE CSpaceViewEnergyFunction::GetThreshold()
{
/*	if (m_pView->nodeViews.GetSize() <= SPV_STATE_DIM / 2)
	{
		CNodeView::activationThreshold = 0.0000001f;
		return 0.0000001f;
	}
*/
	int nNumVizNodeViews = min(m_pView->nodeViews.GetSize(), SPV_STATE_DIM / 2);
	CNodeView::activationThreshold = 
		m_pView->nodeViews.Get(nNumVizNodeViews - 1)->activation.Get();
	return CNodeView::activationThreshold;

	// set initial threshold greater than the max activation
	SPV_STATE_TYPE threshold = 200.0;
	SPV_STATE_TYPE prevThreshold;

	// set initial count to zero
	int nSuperThresholdViewCount = 0;
	int nPrevSuperThresholdViewCount;

	while (nSuperThresholdViewCount < SPV_STATE_DIM / 2)
	{
		nPrevSuperThresholdViewCount = nSuperThresholdViewCount;
		prevThreshold = threshold;

		threshold = 0.0;
		for (int nAt = 0; nAt < m_pView->nodeViews.GetSize(); nAt++)
		{
			SPV_STATE_TYPE currActivation = m_pView->nodeViews.Get(nAt)->activation.Get();
			if (currActivation < prevThreshold)
			{
				if (currActivation > threshold)
				{
					// set the threshold to the current activation
					threshold = currActivation;

					// reset count (to disregard previous activations, which
					//		must have been lower then the new threshold
					nSuperThresholdViewCount = nPrevSuperThresholdViewCount;
				}
				
				// if we are equal, increment the count
				if (currActivation == threshold)
					nSuperThresholdViewCount++;
			}
		}
	}

	ASSERT(nPrevSuperThresholdViewCount < SPV_STATE_DIM / 2);

	CNodeView::activationThreshold = (float) threshold; // prevThreshold;
	return threshold; // prevThreshold;
}

/*
const CArray<int, int>& CSpaceViewEnergyFunction::GetMap()
{
	return m_arrElementMap;
}
*/

SPV_STATE_TYPE CSpaceViewEnergyFunction::operator()(const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput)
{
	// count how many nodeviews have an activation >= the threshold
	TRACE0("Super-threshold NodeViews: \n");
	int nAtSuperthreshCount = 0;
	for (int nAt = 0; nAt < m_pView->nodeViews.GetSize(); nAt++)
	{
		float activation = m_pView->nodeViews.Get(nAt)->activation.Get();
		if (activation >= CNodeView::activationThreshold)
		{
			TRACE1(">>>> Node %s\n", m_pView->nodeViews.Get(nAt)->forNode->name.Get());
		}
	}

	// reset the energy
	m_energy = 0.0;

	// store this input
	m_vInput = vInput;

	// retrieve the parent's rectangle
	CRect rectSpaceView;
	m_pView->GetWindowRect(&rectSpaceView);

	// iterate over all child node views
	int nNumVizNodeViews = min(m_pView->nodeViews.GetSize(), SPV_STATE_DIM / 2);
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < nNumVizNodeViews; // m_pView->nodeViews.GetSize(); 
		nAtNodeView++)
	{
		// if the node is not in the state vector, skip it
		// if (GetMap()[nAtNodeView] != -1)
		{
			// get convenience pointers for the current node view and node
			CNodeView *pAtNodeView = m_pView->nodeViews.Get(nAtNodeView);
			CNode *pAtNode = pAtNodeView->forNode.Get();

			// get the rectangle for the current node view
			CRect rectNodeView;
			pAtNodeView->GetWindowRect(&rectNodeView);

			// temporary variables to hold the derivative of the energy
			SPV_STATE_TYPE dEnergyDx = 0.0;
			SPV_STATE_TYPE dEnergyDy = 0.0;

			// stores the x and y coordinates for various computations;
			SPV_STATE_TYPE x;
			SPV_STATE_TYPE y;
			SPV_STATE_TYPE dx = (SPV_STATE_TYPE) rectNodeView.Width() / 2.0;
			SPV_STATE_TYPE dy = (SPV_STATE_TYPE) rectNodeView.Height() / 2.0;

			// iterate over the potential linked views
			int nAtLinkedView;
			for (nAtLinkedView = 0; nAtLinkedView < nNumVizNodeViews; // m_pView->nodeViews.GetSize(); 
					nAtLinkedView++)
			{
				// only processs the linked view if it is in the vector
				if (nAtLinkedView != nAtNodeView) 
						// && GetMap()[nAtLinkedView] != -1)
				{
					// get convenience pointers for the linked node view and node
					CNodeView *pAtLinkedView = m_pView->nodeViews.Get(nAtLinkedView);
					CNode *pAtLinkedNode = pAtLinkedView->forNode.Get();

					// get the rectangle of the current linked view
					CRect rectLinked;
					pAtLinkedView->GetWindowRect(&rectLinked);

					// get the weight of the link
//					CNodeLink *pLink = pAtLinkedNode->GetLink(pAtNode);

					// retrieve the link weight
					SPV_STATE_TYPE weight1 = 
						pAtLinkedNode->GetLinkWeightBoltz(pAtNode, // sqrt
							(pAtLinkedView->activation.Get() + 0.1f)); 
						//pAtLinkedNode->GetLinkWeight(pAtNode); 
						// 0.0;
//					if (pLink != NULL)
//						weight1 = pLink->weight.Get();

//					pLink = pAtNode->GetLink(pAtLinkedNode);
					SPV_STATE_TYPE weight2 = 
						pAtNode->GetLinkWeightBoltz(pAtLinkedNode, // sqrt
						(pAtNodeView->activation.Get() + 0.1f));
						// pAtNode->GetLinkWeight(pAtLinkedNode);
						// 0.0;

//					if (pLink != NULL)
//						weight2 = pLink->weight.Get();

					SPV_STATE_TYPE weight = (weight1 + weight2) / 2.0f;

					SPV_STATE_TYPE ssx = MinSize((SPV_STATE_TYPE) rectLinked.Width(), 
						(SPV_STATE_TYPE) rectNodeView.Width() / 8.0f);
					SPV_STATE_TYPE ssy = MinSize((SPV_STATE_TYPE) rectLinked.Height(), 
						(SPV_STATE_TYPE) rectNodeView.Height() / 8.0f);

					x = vInput[nAtNodeView*2 + 0] // GetMap()[nAtNodeView] + 0] 
						- vInput[nAtLinkedView*2 + 0];
					y = vInput[nAtNodeView*2 + 1] 
						- vInput[nAtLinkedView*2 + 1];

					// compute the energy due to this interation
					for (int nX = -1; nX <= 1; nX++)
						for (int nY = -1; nY <= 1; nY++)
						{
							m_energy += 1.0 / 10.0
								* spacerFunc((x + dx * (SPV_STATE_TYPE) nX) / ssx, 
									(y + dy * (SPV_STATE_TYPE) nY) / ssy);
						}

					// add general repulsion
					m_energy += 2.0
						* attractFunc(x / (ssx * 1.0), y / (ssy * 1.0));

					m_energy -= weight * 75.0
						* attractFunc(x / (ssx * 6.0), y / (ssy * 6.0));

				}
			}

			SPV_STATE_TYPE width = (SPV_STATE_TYPE) rectSpaceView.Width();
			SPV_STATE_TYPE height = (SPV_STATE_TYPE) rectSpaceView.Height();
			SPV_STATE_TYPE sigma = width + height / 32.0;

			SPV_STATE_TYPE nodeViewWidth = (SPV_STATE_TYPE) rectNodeView.Width();
			SPV_STATE_TYPE nodeViewHeight = (SPV_STATE_TYPE) rectNodeView.Height();

			// set the x and y coordinates for the centering calculation
			x = vInput[nAtNodeView*2 + 0]; // GetMap()[nAtNodeView] + 0];
			y = vInput[nAtNodeView*2 + 1];

			m_energy += CenterField(x - nodeViewWidth / 2.0,  y, width, height, sigma);
			m_energy += CenterField(x,                        y, width, height, sigma);
			m_energy += CenterField(x + nodeViewWidth / 2.0,  y, width, height, sigma);

			m_energy += CenterField(x, y - nodeViewHeight / 2.0, width, height, sigma);
//			m_energy += CenterField(x, y,                        width, height, sigma);
			m_energy += CenterField(x, y + nodeViewHeight / 2.0, width, height, sigma);

		}
	}

	return m_energy;
}

CVector<SPV_STATE_DIM, SPV_STATE_TYPE> 
	CSpaceViewEnergyFunction::Grad(const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput)
{
	// if the input vector is not equal to the cached value
	// if (m_vInput != vInput)
		// compute the energy (computes gradient as side effect)
		(*this)(vInput);

	return m_vGrad;
}



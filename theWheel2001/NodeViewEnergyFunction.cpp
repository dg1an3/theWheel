//////////////////////////////////////////////////////////////////////
// NodeViewEnergyFunction.cpp: implementation of the 
//		CNodeViewEnergyFunction objective function.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// defines the gaussian function
#include <ScalarFunction.h>

// header files for the class
#include "NodeViewEnergyFunction.h"
#include "SpaceView.h"

#include <PowellOptimizer.h>

#define SIGMA_POINT 1.60

const STATE_TYPE k = 
	(0.4f - (float) exp(-SIGMA_POINT * SIGMA_POINT / 2.0f) / (float) (sqrt(2.0f * PI)));


//////////////////////////////////////////////////////////////////////
// CNodeViewEnergyFunction::CNodeViewEnergyFunction
// 
// constructs an energy function object for the associated space 
//		view
//////////////////////////////////////////////////////////////////////
CNodeViewEnergyFunction::CNodeViewEnergyFunction(CSpaceView *pView)
	: CObjectiveFunction<2, STATE_TYPE>(FALSE),
		m_pView(pView),

		m_vInput(CVector<2, STATE_TYPE>()),
		m_energy(0.0)

{
}

//////////////////////////////////////////////////////////////////////
// CNodeViewEnergyFunction::LoadSizesLinks()
// 
// loads the sizes and links for quick access
//////////////////////////////////////////////////////////////////////
void CNodeViewEnergyFunction::LoadSizesLinks()
{
	// iterate over all current visualized node views
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < m_pView->GetVisibleNodeCount(); nAtNodeView++)
	{
		// get convenience pointers for the current node view and node
		CNodeView *pAtNodeView = m_pView->GetNodeView(nAtNodeView);
		CNode *pAtNode = pAtNodeView->GetNode();

		// compute the x- and y-scales for the fields (from the linked rectangle)
		CRect rect = pAtNodeView->GetOuterRect(); // GetRectNoSpring(); // 
		STATE_TYPE ssx = 0.85f * (STATE_TYPE) rect.Width();
		STATE_TYPE ssy = (STATE_TYPE) rect.Height();

		// store the size
		m_vSize[nAtNodeView][0] = ssx + 10.0f;
		m_vSize[nAtNodeView][1] = ssy + 10.0f;
		m_act[nAtNodeView] = pAtNodeView->GetSpringActivation();

		// get convenience pointers for the linked node view and node
		CNode *pAtLinkedNode = m_pNodeView->GetNode();

		// retrieve the link weight for layout
		STATE_TYPE weight = (STATE_TYPE) 0.5 *
			(pAtNode->GetLinkWeight(pAtLinkedNode)
			+ pAtLinkedNode->GetLinkWeight(pAtNode));
		weight += 0.0001f;

		// store the link weight
		m_mLinks[nAtNodeView] = weight;
	}

	CSpace *pSpace = m_pView->GetDocument();
	CRect rectClient;
	m_pView->GetClientRect(&rectClient);
	int nClientArea = rectClient.Width() * rectClient.Height();

	// now load the clusters
	int nAtCluster;
	for (nAtCluster = 0; nAtCluster < pSpace->GetClusterCount(); nAtCluster++)
	{
		// get convenience pointers for the current node view and node
		CNodeCluster *pCluster = pSpace->GetClusterAt(nAtCluster);

		// compute the x- and y-scales for the fields (from the linked rectangle)
		// CRect rect = pAtNodeView->GetOuterRect(); // GetRectNoSpring(); // 
		// compute the area interpreting springActivation as the fraction of the 
		//		parent's total area
		float area = pCluster->GetTotalActivation() * nClientArea; 

		// compute the desired aspect ratio (maintain the current aspect ratio)
		float aspectRatio = 0.75f - 0.375f * (float) exp(-8.0f * pCluster->GetTotalActivation());

		// compute the new width and height from the desired area and the desired
		//		aspect ratio
		int nWidth = (int) sqrt(area / aspectRatio);
		int nHeight = (int) sqrt(area * aspectRatio);

		// set the width and height of the window, keeping the center constant
		CRect rect;
		rect.left =		- nWidth / 2;
		rect.right =	+ nWidth / 2;
		rect.top =		- nHeight / 2;
		rect.bottom =	+ nHeight / 2;

		STATE_TYPE ssx = 0.85f * (STATE_TYPE) rect.Width();
		STATE_TYPE ssy = (STATE_TYPE) rect.Height();

		// store the size
		m_vSize[m_pView->GetVisibleNodeCount()+nAtCluster][0] = ssx + 10.0f;
		m_vSize[m_pView->GetVisibleNodeCount()+nAtCluster][1] = ssy + 10.0f;
		m_act[m_pView->GetVisibleNodeCount()+nAtCluster] = pCluster->GetTotalActivation();

		// get convenience pointers for the linked node view and node
		CNode *pAtLinkedNode = m_pNodeView->GetNode();

		// retrieve the link weight for layout
		STATE_TYPE weight = (STATE_TYPE) 0.5 *
			pCluster->GetLinkWeight(pAtLinkedNode);
		weight += 0.0001f;

		// store the link weight
		m_mLinks[m_pView->GetVisibleNodeCount()+nAtCluster] = weight;
	}
}

//////////////////////////////////////////////////////////////////////
// CNodeViewEnergyFunction::operator()
// 
// evaluates the energy function at the given point
//////////////////////////////////////////////////////////////////////
STATE_TYPE CNodeViewEnergyFunction::operator()(
		const CVector<2, STATE_TYPE>& vInput)
{
	m_nEvaluations++;

	// reset the energy
	m_energy = 0.0f;
	STATE_TYPE dEnergyDx = 0.0f;
	STATE_TYPE dEnergyDy = 0.0f;

	// store this input
	m_vInput = vInput;

	// retrieve the parent's rectangle
	CRect rectSpaceView;
	m_pView->GetWindowRect(&rectSpaceView);

	// form the number of currently visualized node views
	int nRealNodeCount = m_pView->GetVisibleNodeCount(); 
	int nVizNodeCount = m_pView->GetVisibleNodeCount() + m_pView->GetDocument()->GetClusterCount();

	// iterate over all current visualized node views
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < nVizNodeCount; nAtNodeView++)
	{
		STATE_TYPE x;
		STATE_TYPE y;

		if (nAtNodeView < nRealNodeCount)
		{
			CNodeView *pNodeView = m_pView->GetNodeView(nAtNodeView);

			// compute the x- and y-offset between the views

			x = vInput[0] - pNodeView->GetNode()->GetPosition()[0];
			y = vInput[1] - pNodeView->GetNode()->GetPosition()[1];
		}
		else
		{
			CNodeCluster *pCluster = m_pView->GetDocument()->GetClusterAt(nAtNodeView - nRealNodeCount);
			x = vInput[0] - pCluster->m_vCenter[0];
			y = vInput[0] - pCluster->m_vCenter[1];
		}

		// compute the x- and y-scales for the fields (from the linked rectangle)

		STATE_TYPE ssx = m_vSize[nAtNodeView][0]; 
		STATE_TYPE ssy = m_vSize[nAtNodeView][1];

		// compute the energy due to this interation

		STATE_TYPE dx_ratio = x / (ssx * ssx);
		STATE_TYPE x_ratio = x * dx_ratio; 
		STATE_TYPE dy_ratio = y / (ssy * ssy);
		STATE_TYPE y_ratio = y * dy_ratio;

		// add the repulsion field

		if (nAtNodeView < nRealNodeCount)
		{
			STATE_TYPE inv_sq = 1.0f / (x_ratio + y_ratio + 0.1f);
			m_energy += 12.0f * inv_sq;
		}

		STATE_TYPE weight = m_mLinks[nAtNodeView] 
			* (STATE_TYPE) 400.0;
		m_energy -= weight * (0.4f - 
			2.0 * (m_act[nAtNodeView] + m_pNodeView->GetNode()->GetActivation()) * k // 16.0f 
				* (x_ratio + y_ratio));
	}

	// now include a positional energy term
	STATE_TYPE x = vInput[0] - rectSpaceView.Width() / 2.0f;
	STATE_TYPE y = vInput[1] - rectSpaceView.Height() / 2.0f;

	STATE_TYPE ssx = rectSpaceView.Width() / 4.0f + 10.0f;
	STATE_TYPE ssy = rectSpaceView.Height() / 4.0f + 10.0f;
	STATE_TYPE ssz = rectSpaceView.Width() / 2.0f + 10.0f;

	m_energy +=  // 1000.0 * act * act 
		0.001f * m_pNodeView->GetOuterRect().Width() // * m_vSize[nAtNodeView][1] 
			* (x * x / (ssx * ssx) + y * y / (ssy * ssy));

	return m_energy;
}

//////////////////////////////////////////////////////////////////////
// CNodeViewEnergyFunction::Grad
// 
// evaluates the gradient of the energy function at the given point
//////////////////////////////////////////////////////////////////////
CVector<2, STATE_TYPE> CNodeViewEnergyFunction::Grad(
		const CVector<2, STATE_TYPE>& vInput)
{
	// if the input vector is not equal to the cached value
	// if (m_vInput != vInput)
		// compute the energy (computes gradient as side effect)
		(*this)(vInput);

	return m_vGrad;
}


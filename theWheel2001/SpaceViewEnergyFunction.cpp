//////////////////////////////////////////////////////////////////////
// SpaceViewEnergyFunction.cpp: implementation of the 
//		CSpaceViewEnergyFunction objective function.
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
#include "SpaceViewEnergyFunction.h"
#include "SpaceView.h"

// #define USE_NODES3D

#define SIGMA_POINT 1.60

const STATE_TYPE k = 
	(0.4f - (float) exp(-SIGMA_POINT * SIGMA_POINT / 2.0f) / (float) (sqrt(2.0f * PI)));


//////////////////////////////////////////////////////////////////////
// attract_func
// 
// function which evaluates a gaussian attraction field at a given 
//		point
//////////////////////////////////////////////////////////////////////
STATE_TYPE attract_func(STATE_TYPE x, STATE_TYPE y)
{
	return 0.4f - k * (x * x + y * y);
}

//////////////////////////////////////////////////////////////////////
// dattract_func_dx
// 
// function which evaluates a derivative of gaussian attraction 
//		field at a given point
//////////////////////////////////////////////////////////////////////
STATE_TYPE dattract_func_dx(STATE_TYPE x, STATE_TYPE y)
{
	return 2 * k * x;
}

//////////////////////////////////////////////////////////////////////
// dattract_func_dy
// 
// function which evaluates a derivative of gaussian attraction 
//		field at a given point
//////////////////////////////////////////////////////////////////////
STATE_TYPE dattract_func_dy(STATE_TYPE x, STATE_TYPE y)
{
	return 2 * k * y;
}

//////////////////////////////////////////////////////////////////////
// spacer_func
// 
// function which evaluates a gaussian spacer field at a given 
//		point
//////////////////////////////////////////////////////////////////////
STATE_TYPE spacer_func(STATE_TYPE x, STATE_TYPE y)
{
	return 1.0f / (x * x + y * y + 0.1f);
}

//////////////////////////////////////////////////////////////////////
// dspacer_func_dx
// 
// function which evaluates a gaussian spacer field at a given 
//		point
//////////////////////////////////////////////////////////////////////
STATE_TYPE dspacer_func_dx(STATE_TYPE x, STATE_TYPE y)
{
	return 2.0f * x * spacer_func(x, y);
}

//////////////////////////////////////////////////////////////////////
// dspacer_func_dy
// 
// function which evaluates a gaussian spacer field at a given 
//		point
//////////////////////////////////////////////////////////////////////
STATE_TYPE dspacer_func_dy(STATE_TYPE x, STATE_TYPE y)
{
	return 2.0f * y * spacer_func(x, y);
}

//////////////////////////////////////////////////////////////////////
// CSpaceViewEnergyFunction::CSpaceViewEnergyFunction
// 
// constructs an energy function object for the associated space 
//		view
//////////////////////////////////////////////////////////////////////
CSpaceViewEnergyFunction::CSpaceViewEnergyFunction(CSpaceView *pView)
	: CObjectiveFunction<STATE_DIM, STATE_TYPE>(FALSE),
		m_pView(pView),

		m_vInput(CVector<STATE_DIM, STATE_TYPE>()),
		m_energy(0.0)

{
}

//////////////////////////////////////////////////////////////////////
// CSpaceViewEnergyFunction::LoadSizesLinks()
// 
// loads the sizes and links for quick access
//////////////////////////////////////////////////////////////////////
void CSpaceViewEnergyFunction::LoadSizesLinks()
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

		// iterate over the potential linked views
		int nAtLinkedView;
		for (nAtLinkedView = 0; nAtLinkedView < m_pView->GetVisibleNodeCount(); nAtLinkedView++)
		{
			// only process the linked view if it is in the vector
			if (nAtLinkedView != nAtNodeView) 
			{
				// get convenience pointers for the linked node view and node
				CNodeView *pAtLinkedView = m_pView->GetNodeView(nAtLinkedView);
				CNode *pAtLinkedNode = pAtLinkedView->GetNode();

				// retrieve the link weight for layout
				STATE_TYPE weight = (STATE_TYPE) 0.5 *
					(pAtNode->GetLinkWeight(pAtLinkedNode)
					+ pAtLinkedNode->GetLinkWeight(pAtNode));
				weight += 0.0001f;

				// store the link weight
				m_mLinks[nAtNodeView][nAtLinkedView] = weight;
			}
		}
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
		float activation = 0.5 * pCluster->GetActualTotalActivation();
		float area = activation * nClientArea; 

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
		m_act[m_pView->GetVisibleNodeCount()+nAtCluster] = activation;

		// iterate over the potential linked views
		int nAtLinkedView;
		for (nAtLinkedView = 0; nAtLinkedView < m_pView->GetVisibleNodeCount(); nAtLinkedView++)
		{
			// get convenience pointers for the linked node view and node
			CNodeView *pAtLinkedView = m_pView->GetNodeView(nAtLinkedView);
			CNode *pAtLinkedNode = pAtLinkedView->GetNode();

			// retrieve the link weight for layout
			STATE_TYPE weight = (STATE_TYPE) pCluster->GetLinkWeight(pAtLinkedNode);
			weight += 0.0001f;

			// store the link weight
			m_mLinks[m_pView->GetVisibleNodeCount()+nAtCluster][nAtLinkedView] = weight;
			m_mLinks[nAtLinkedView][m_pView->GetVisibleNodeCount()+nAtCluster] = weight;
		}

		// load cluster-cluster links
		int nAtOtherCluster;
		for (nAtOtherCluster = 0; nAtOtherCluster < pSpace->GetClusterCount(); nAtOtherCluster++)
		{
			// get convenience pointers for the current node view and node
			CNodeCluster *pOtherCluster = pSpace->GetClusterAt(nAtOtherCluster);

			// retrieve the link weight for layout
			STATE_TYPE weight = (STATE_TYPE) 0.5 *
				(pCluster->GetLinkWeight(pOtherCluster) +
				pOtherCluster->GetLinkWeight(pCluster));
			weight += 0.0001f;

			// store the link weight
			m_mLinks[m_pView->GetVisibleNodeCount()+nAtCluster][m_pView->GetVisibleNodeCount()+nAtOtherCluster] = weight;
			m_mLinks[m_pView->GetVisibleNodeCount()+nAtOtherCluster][m_pView->GetVisibleNodeCount()+nAtCluster] = weight;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSpaceView::GetStateVector
// 
// forms the state vector for the associated CSpaceView
//////////////////////////////////////////////////////////////////////
CSpaceViewEnergyFunction::CStateVector 
	CSpaceViewEnergyFunction::GetStateVector()
{
	// for the state vector
	CStateVector vState;

	int nClusters = m_pView->GetDocument()->GetClusterCount();

#ifdef USE_NODES3D
	for (int nAt = 0; nAt < STATE_DIM / 3; nAt++)
#else
	for (int nAt = 0; nAt < STATE_DIM / 2 - nClusters; nAt++)
#endif
	{
		if (nAt < m_pView->GetNodeViewCount())
		{
			CNodeView *pView = m_pView->GetNodeView(nAt);

#ifdef USE_NODES3D
			vState[nAt*3] = (STATE_TYPE) pView->GetNode()->GetPosition()[0];
			vState[nAt*3+1] = (STATE_TYPE) pView->GetNode()->GetPosition()[1];
			vState[nAt*3+2] = (STATE_TYPE) pView->GetNode()->GetPosition()[2];
#else
			vState[nAt*2] = (STATE_TYPE) pView->GetNode()->GetPosition()[0];
			vState[nAt*2+1] = (STATE_TYPE) pView->GetNode()->GetPosition()[1];
#endif
		}
	}

	for (int nAtCluster = 0; nAtCluster < nClusters; nAtCluster++)
	{
		CNodeCluster *pCluster = m_pView->GetDocument()->GetClusterAt(nAtCluster);
		vState[(STATE_DIM / 2 - nClusters + nAtCluster)*2] = pCluster->GetPosition()[0];
		vState[(STATE_DIM / 2 - nClusters + nAtCluster)*2+1] = pCluster->GetPosition()[1];
	}

	// return the formed state vector
	return vState;
}

//////////////////////////////////////////////////////////////////////
// CSpaceViewEnergyFunction::SetStateVector
// 
// sets the state vector for the associated CSpaceView
//////////////////////////////////////////////////////////////////////
void CSpaceViewEnergyFunction::SetStateVector(
		const CSpaceViewEnergyFunction::CStateVector& vState)
{
	// form the number of currently visualized node views
	for (int nAt = 0; nAt < m_pView->GetVisibleNodeCount(); nAt++)
	{
#ifdef USE_NODES3D
		CVector<3> vNewCenter = CVector<3>(vState[nAt*3], vState[nAt*3+1], vState[nAt*3+2]);
#else
		CVector<3> vNewCenter = CVector<3>(vState[nAt*2], vState[nAt*2+1], 0.0);
#endif
		CNodeView *pView = m_pView->GetNodeView(nAt);
		pView->GetNode()->SetPosition(vNewCenter);
	}

	int nClusters = m_pView->GetDocument()->GetClusterCount();

	for (int nAtCluster = 0; nAtCluster < nClusters; nAtCluster++)
	{
		CNodeCluster *pCluster = m_pView->GetDocument()->GetClusterAt(nAtCluster);

		CVector<3> vPosition(
			vState[(STATE_DIM / 2 - nClusters + nAtCluster)*2],
			vState[(STATE_DIM / 2 - nClusters + nAtCluster)*2+1],
			0.0);
		pCluster->SetPosition(vPosition);
	}
}

//////////////////////////////////////////////////////////////////////
// CSpaceViewEnergyFunction::operator()
// 
// evaluates the energy function at the given point
//////////////////////////////////////////////////////////////////////
STATE_TYPE CSpaceViewEnergyFunction::operator()(
		const CVector<STATE_DIM, STATE_TYPE>& vInput)
{
	m_nEvaluations++;

	// reset the energy
	m_energy = 0.0f;
	STATE_TYPE dEnergyDx = 0.0f;
	STATE_TYPE dEnergyDy = 0.0f;

	// initialize the gradient vector to zeros
	for (int nAt = 0; nAt < STATE_DIM; nAt++)
		m_vGrad[nAt] = 0.0f;

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
		// iterate over the potential linked views
		int nAtLinkedView;
		for (nAtLinkedView = 0; nAtLinkedView < nVizNodeCount; nAtLinkedView++)
		{
			// only processs the linked view if it is in the vector
			if (nAtLinkedView != nAtNodeView) 
			{
				// compute the x- and y-offset between the views
#ifdef USE_NODES3D
				STATE_TYPE x = vInput[nAtNodeView*3 + 0] - vInput[nAtLinkedView*3 + 0];
				STATE_TYPE y = vInput[nAtNodeView*3 + 1] - vInput[nAtLinkedView*3 + 1];
				STATE_TYPE z = vInput[nAtNodeView*3 + 2] - vInput[nAtLinkedView*3 + 2];
#else
				STATE_TYPE x = vInput[nAtNodeView*2 + 0] - vInput[nAtLinkedView*2 + 0];
				STATE_TYPE y = vInput[nAtNodeView*2 + 1] - vInput[nAtLinkedView*2 + 1];
#endif

				// compute the x- and y-scales for the fields (from the linked rectangle)
				// CRect rectLinked = pAtLinkedView->GetOuterRect();
				STATE_TYPE ssx = m_vSize[nAtLinkedView][0]; 
				STATE_TYPE ssy = m_vSize[nAtLinkedView][1];
#ifdef USE_NODES3D
				STATE_TYPE ssz = m_vSize[nAtLinkedView][1] * 128.0;
#endif
				// compute the energy due to this interation
				STATE_TYPE dx_ratio = x / (ssx * ssx);
				STATE_TYPE x_ratio = x * dx_ratio; // (x * x) / (ssx * ssx);
				STATE_TYPE dy_ratio = y / (ssy * ssy);
				STATE_TYPE y_ratio = y * dy_ratio; // (y * y) / (ssy * ssy);
#ifdef USE_NODES3D
				STATE_TYPE dz_ratio = z / (ssz * ssz);
				STATE_TYPE z_ratio = z * dz_ratio; // (y * y) / (ssy * ssy);
#endif

				// add the repulsion field

#ifdef USE_NODES3D
				STATE_TYPE inv_sq = 1.0f / (x_ratio + y_ratio + z_ratio + 0.1f);
				m_energy += 8.0f * inv_sq;
#else
				if (nAtLinkedView < nRealNodeCount)
				{
					STATE_TYPE inv_sq = 1.0f / (x_ratio + y_ratio + 0.1f);
					m_energy += 16.0f * inv_sq;
				}
#endif

				// compute the partial derivatives
/*				dEnergyDx = 3.5f * inv_sq * inv_sq * -2.0f * 2.0f * dx_ratio;
				dEnergyDy = 3.5f * inv_sq * inv_sq * -2.0f * 2.0f * dy_ratio;
				m_vGrad[nAtNodeView * 2 + 0] += dEnergyDx;
				m_vGrad[nAtNodeView * 2 + 1] += dEnergyDy;
				m_vGrad[nAtLinkedView * 2 + 0] -= dEnergyDx;
				m_vGrad[nAtLinkedView * 2 + 1] -= dEnergyDy;
*/
				// add attraction * weight
#ifdef USE_NODES3D
				STATE_TYPE weight = m_mLinks[nAtNodeView][nAtLinkedView] 
					* (STATE_TYPE) 800.0;
				m_energy -= weight * (0.4f - 
					k / 16.0f * (x_ratio + y_ratio + z_ratio));
#else
				STATE_TYPE weight = m_mLinks[nAtNodeView][nAtLinkedView] 
					* (STATE_TYPE) 400.0;
				m_energy -= weight * (0.4f - 
					2.0 * (m_act[nAtNodeView] + m_act[nAtLinkedView]) * k // 16.0f 
						* (x_ratio + y_ratio));
#endif

				// compute the partial derivatives
/*				dEnergyDx = weight * k / 16.0f * 2.0f * dx_ratio;
				dEnergyDy = weight * k / 16.0f * 2.0f * dy_ratio;
				m_vGrad[nAtNodeView * 2 + 0] -= dEnergyDx;
				m_vGrad[nAtNodeView * 2 + 1] -= dEnergyDy;
				m_vGrad[nAtLinkedView * 2 + 0] += dEnergyDx;
				m_vGrad[nAtLinkedView * 2 + 1] += dEnergyDy;
*/
			}
		}

		// now include a positional energy term
		// get convenience pointers for the current node view and node
/*		CNodeView *pAtNodeView = m_pView->nodeViews.Get(nAtNodeView);
		CNode *pAtNode = pAtNodeView->forNode.Get();
		STATE_TYPE act = pAtNodeView->GetThresholdedActivation();
*/
#ifdef USE_NODES3D
		STATE_TYPE x = vInput[nAtNodeView*3 + 0] - rectSpaceView.Width() / 2.0f;
		STATE_TYPE y = vInput[nAtNodeView*3 + 1] - rectSpaceView.Height() / 2.0f;
		STATE_TYPE z = vInput[nAtNodeView*3 + 2];
#else
		STATE_TYPE x = vInput[nAtNodeView*2 + 0] - rectSpaceView.Width() / 2.0f;
		STATE_TYPE y = vInput[nAtNodeView*2 + 1] - rectSpaceView.Height() / 2.0f;
#endif
		STATE_TYPE ssx = rectSpaceView.Width() / 4.0f + 10.0f;
		STATE_TYPE ssy = rectSpaceView.Height() / 4.0f + 10.0f;
		STATE_TYPE ssz = rectSpaceView.Width() / 2.0f + 10.0f;

#ifdef USE_NODES3D
		m_energy += // 1000.0 * act * act * (x * x / (ssx * ssx) + y * y / (ssy * ssy)) 
			+ z * z / (ssz * ssz);
			// + 1000.0 * act * act * z * z / (ssz * ssz) + z / 4.0
			;
#else
/*		m_energy +=  // 1000.0 * act * act 
			0.001f * m_vSize[nAtNodeView][0] // * m_vSize[nAtNodeView][1] 
				* (x * x / (ssx * ssx) + y * y / (ssy * ssy)); */
#endif

	}

	return m_energy;
}

//////////////////////////////////////////////////////////////////////
// CSpaceViewEnergyFunction::Grad
// 
// evaluates the gradient of the energy function at the given point
//////////////////////////////////////////////////////////////////////
CVector<STATE_DIM, STATE_TYPE> CSpaceViewEnergyFunction::Grad(
		const CVector<STATE_DIM, STATE_TYPE>& vInput)
{
	// if the input vector is not equal to the cached value
	// if (m_vInput != vInput)
		// compute the energy (computes gradient as side effect)
		(*this)(vInput);

	return m_vGrad;
}


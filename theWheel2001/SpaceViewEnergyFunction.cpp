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

const SPV_STATE_TYPE k = 
	(0.4f - exp(-SIGMA_POINT * SIGMA_POINT / 2.0f) / (sqrt(2.0f * PI)));


//////////////////////////////////////////////////////////////////////
// attract_func
// 
// function which evaluates a gaussian attraction field at a given 
//		point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE attract_func(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return 0.4f - k * (x * x + y * y);
}

//////////////////////////////////////////////////////////////////////
// dattract_func_dx
// 
// function which evaluates a derivative of gaussian attraction 
//		field at a given point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE dattract_func_dx(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return 2 * k * x;
}

//////////////////////////////////////////////////////////////////////
// dattract_func_dy
// 
// function which evaluates a derivative of gaussian attraction 
//		field at a given point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE dattract_func_dy(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return 2 * k * y;
}

//////////////////////////////////////////////////////////////////////
// spacer_func
// 
// function which evaluates a gaussian spacer field at a given 
//		point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE spacer_func(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return 1.0f / (x * x + y * y + 0.1f);
}

//////////////////////////////////////////////////////////////////////
// dspacer_func_dx
// 
// function which evaluates a gaussian spacer field at a given 
//		point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE dspacer_func_dx(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return 2.0f * x * spacer_func(x, y);
}

//////////////////////////////////////////////////////////////////////
// dspacer_func_dy
// 
// function which evaluates a gaussian spacer field at a given 
//		point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE dspacer_func_dy(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
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
	: CObjectiveFunction<SPV_STATE_DIM, SPV_STATE_TYPE>(FALSE),
		m_pView(pView),

		m_vInput(CVector<SPV_STATE_DIM, SPV_STATE_TYPE>()),
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
	// form the number of currently visualized node views
	int nNumVizNodeViews = min(m_pView->nodeViews.GetSize(), 
		SPV_STATE_DIM / 2);

	// iterate over all current visualized node views
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < nNumVizNodeViews; nAtNodeView++)
	{
		// get convenience pointers for the current node view and node
		CNodeView *pAtNodeView = m_pView->nodeViews.Get(nAtNodeView);
		CNode *pAtNode = pAtNodeView->forNode.Get();

		// compute the x- and y-scales for the fields (from the linked rectangle)
		CRect rect = pAtNodeView->GetOuterRect(); // GetRectNoSpring(); // 
		SPV_STATE_TYPE ssx = 0.85f * (SPV_STATE_TYPE) rect.Width();
		SPV_STATE_TYPE ssy = (SPV_STATE_TYPE) rect.Height();

		// store the size
		m_vSize[nAtNodeView][0] = ssx + 10.0f;
		m_vSize[nAtNodeView][1] = ssy + 10.0f;

		// iterate over the potential linked views
		int nAtLinkedView;
		for (nAtLinkedView = 0; nAtLinkedView < nNumVizNodeViews; nAtLinkedView++)
		{
			// only process the linked view if it is in the vector
			if (nAtLinkedView != nAtNodeView) 
			{
				// get convenience pointers for the linked node view and node
				CNodeView *pAtLinkedView = m_pView->nodeViews.Get(nAtLinkedView);
				CNode *pAtLinkedNode = pAtLinkedView->forNode.Get();

				// retrieve the link weight for layout
				SPV_STATE_TYPE weight = (SPV_STATE_TYPE) 0.5 *
					(pAtNode->GetLinkWeight(pAtLinkedNode)
					+ pAtLinkedNode->GetLinkWeight(pAtNode));
				weight += 0.0001f;

				// store the link weight
				m_mLinks[nAtNodeView][nAtLinkedView] = weight;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CSpaceViewEnergyFunction::operator()
// 
// evaluates the energy function at the given point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE CSpaceViewEnergyFunction::operator()(
		const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput)
{
	m_nEvaluations++;

	// reset the energy
	m_energy = 0.0f;
	SPV_STATE_TYPE dEnergyDx = 0.0f;
	SPV_STATE_TYPE dEnergyDy = 0.0f;

	// initialize the gradient vector to zeros
	for (int nAt = 0; nAt < SPV_STATE_DIM; nAt++)
		m_vGrad[nAt] = 0.0f;

	// store this input
	m_vInput = vInput;

	// retrieve the parent's rectangle
	CRect rectSpaceView;
	m_pView->GetWindowRect(&rectSpaceView);

	// form the number of currently visualized node views
#ifdef USE_NODES3D
	int nNumVizNodeViews = min(m_pView->nodeViews.GetSize(), SPV_STATE_DIM / 3);
#else
	int nNumVizNodeViews = min(m_pView->nodeViews.GetSize(), SPV_STATE_DIM / 2);
#endif

	// iterate over all current visualized node views
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < nNumVizNodeViews; nAtNodeView++)
	{
		// iterate over the potential linked views
		int nAtLinkedView;
		for (nAtLinkedView = 0; nAtLinkedView < nNumVizNodeViews; nAtLinkedView++)
		{
			// only processs the linked view if it is in the vector
			if (nAtLinkedView != nAtNodeView) 
			{
				// compute the x- and y-offset between the views
#ifdef USE_NODES3D
				SPV_STATE_TYPE x = vInput[nAtNodeView*3 + 0] - vInput[nAtLinkedView*3 + 0];
				SPV_STATE_TYPE y = vInput[nAtNodeView*3 + 1] - vInput[nAtLinkedView*3 + 1];
				SPV_STATE_TYPE z = vInput[nAtNodeView*3 + 2] - vInput[nAtLinkedView*3 + 2];
#else
				SPV_STATE_TYPE x = vInput[nAtNodeView*2 + 0] - vInput[nAtLinkedView*2 + 0];
				SPV_STATE_TYPE y = vInput[nAtNodeView*2 + 1] - vInput[nAtLinkedView*2 + 1];
#endif

				// compute the x- and y-scales for the fields (from the linked rectangle)
				// CRect rectLinked = pAtLinkedView->GetOuterRect();
				SPV_STATE_TYPE ssx = m_vSize[nAtLinkedView][0]; 
				SPV_STATE_TYPE ssy = m_vSize[nAtLinkedView][1];
#ifdef USE_NODES3D
				SPV_STATE_TYPE ssz = m_vSize[nAtLinkedView][1] * 128.0;
#endif
				// compute the energy due to this interation
				SPV_STATE_TYPE dx_ratio = x / (ssx * ssx);
				SPV_STATE_TYPE x_ratio = x * dx_ratio; // (x * x) / (ssx * ssx);
				SPV_STATE_TYPE dy_ratio = y / (ssy * ssy);
				SPV_STATE_TYPE y_ratio = y * dy_ratio; // (y * y) / (ssy * ssy);
#ifdef USE_NODES3D
				SPV_STATE_TYPE dz_ratio = z / (ssz * ssz);
				SPV_STATE_TYPE z_ratio = z * dz_ratio; // (y * y) / (ssy * ssy);
#endif

				// add the repulsion field

#ifdef USE_NODES3D
				SPV_STATE_TYPE inv_sq = 1.0f / (x_ratio + y_ratio + z_ratio + 0.1f);
				m_energy += 8.0f * inv_sq;
#else
				SPV_STATE_TYPE inv_sq = 1.0f / (x_ratio + y_ratio + 0.1f);
				m_energy += 3.8f * inv_sq;
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
				SPV_STATE_TYPE weight = m_mLinks[nAtNodeView][nAtLinkedView] 
					* (SPV_STATE_TYPE) 800.0;
				m_energy -= weight * (0.4f - 
					k / 16.0f * (x_ratio + y_ratio + z_ratio));
#else
				SPV_STATE_TYPE weight = m_mLinks[nAtNodeView][nAtLinkedView] 
					* (SPV_STATE_TYPE) 400.0;
				m_energy -= weight * (0.4f - 
					k / 16.0f * (x_ratio + y_ratio));
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
		CNodeView *pAtNodeView = m_pView->nodeViews.Get(nAtNodeView);
		CNode *pAtNode = pAtNodeView->forNode.Get();
		SPV_STATE_TYPE act = pAtNodeView->GetThresholdedActivation();

		CRect rect;
		m_pView->GetClientRect(&rect);

#ifdef USE_NODES3D
		SPV_STATE_TYPE x = vInput[nAtNodeView*3 + 0] - rect.Width() / 2.0;
		SPV_STATE_TYPE y = vInput[nAtNodeView*3 + 1] - rect.Height() / 2.0;
		SPV_STATE_TYPE z = vInput[nAtNodeView*3 + 2];
#else
		// SPV_STATE_TYPE x = vInput[nAtNodeView*2 + 0] - rect.Width() / 2.0;
		// SPV_STATE_TYPE y = vInput[nAtNodeView*2 + 1] - rect.Height() / 2.0;
#endif
		SPV_STATE_TYPE ssx = rect.Width() / 8.0;
		SPV_STATE_TYPE ssy = rect.Height() / 8.0;
		SPV_STATE_TYPE ssz = rect.Width() / 2.0;

#ifdef USE_NODES3D
		m_energy += // 1000.0 * act * act * (x * x / (ssx * ssx) + y * y / (ssy * ssy)) 
			+ z * z / (ssz * ssz);
			// + 1000.0 * act * act * z * z / (ssz * ssz) + z / 4.0
			;
#else
		// m_energy +=  1000.0 * act * act * (x * x / (ssx * ssx) + y * y / (ssy * ssy));
#endif

	}

	return m_energy;
}

//////////////////////////////////////////////////////////////////////
// CSpaceViewEnergyFunction::Grad
// 
// evaluates the gradient of the energy function at the given point
//////////////////////////////////////////////////////////////////////
CVector<SPV_STATE_DIM, SPV_STATE_TYPE> CSpaceViewEnergyFunction::Grad(
		const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput)
{
	// if the input vector is not equal to the cached value
	// if (m_vInput != vInput)
		// compute the energy (computes gradient as side effect)
		(*this)(vInput);

	return m_vGrad;
}


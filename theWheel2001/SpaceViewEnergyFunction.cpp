////////////////////////////////////
// Copyright (C) 1996-2000 DG Lane
// U.S. Patent Pending
////////////////////////////////////

#include "stdafx.h"

#include <ScalarFunction.h>

#include "SpaceViewEnergyFunction.h"
#include "SpaceView.h"

//////////////////////////////////////////////////////////////////////
// attract_func
// 
// function which evaluates a gaussian attraction field at a given 
//		point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE attract_func(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return Gauss2D<SPV_STATE_TYPE>(x, y , 1.0f, 1.0f);
}

//////////////////////////////////////////////////////////////////////
// spacer_func
// 
// function which evaluates a gaussian spacer field at a given 
//		point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE spacer_func(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return
		+ 1.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f, 1.0f) 
		+ 2.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f / 2.0f, 1.0f / 2.0f)
		+ 4.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f / 4.0f, 1.0f / 4.0f)
		+ 8.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f / 8.0f, 1.0f / 8.0f);
}

//////////////////////////////////////////////////////////////////////
// CSpaceViewEnergyFunction::CSpaceViewEnergyFunction
// 
// constructs an energy function object for the associated space 
//		view
//////////////////////////////////////////////////////////////////////
CSpaceViewEnergyFunction::CSpaceViewEnergyFunction(CSpaceView *pView)
	: m_pView(pView),
		m_attractFunc(&attract_func, -4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, 
			"ATTRFUNC.TMP"),
		m_spacerFunc(&spacer_func, -4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, 
			"SPACERFUNC.TMP")
{
}

//////////////////////////////////////////////////////////////////////
// CSpaceViewEnergyFunction::operator()
// 
// evaluates the energy function at the given point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE CSpaceViewEnergyFunction::operator()(
		const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput)
{
	// reset the energy
	m_energy = 0.0;

	// store this input
	m_vInput = vInput;

	// retrieve the parent's rectangle
	CRect rectSpaceView;
	m_pView->GetWindowRect(&rectSpaceView);

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

		// get the rectangle for the current node view
		CRect rectNodeView;
		pAtNodeView->GetWindowRect(&rectNodeView);

		// iterate over the potential linked views
		int nAtLinkedView;
		for (nAtLinkedView = 0; nAtLinkedView < nNumVizNodeViews; nAtLinkedView++)
		{
			// only processs the linked view if it is in the vector
			if (nAtLinkedView != nAtNodeView) 
			{
				// get convenience pointers for the linked node view and node
				CNodeView *pAtLinkedView = m_pView->nodeViews.Get(nAtLinkedView);
				CNode *pAtLinkedNode = pAtLinkedView->forNode.Get();

				// get the rectangle of the current linked view
				CRect rectLinked;
				pAtLinkedView->GetWindowRect(&rectLinked);

				// retrieve the link weight for layout
				SPV_STATE_TYPE weight = (SPV_STATE_TYPE) 0.5 *
					(pAtNode->GetLinkWeight(pAtLinkedNode)
					+ pAtLinkedNode->GetLinkWeight(pAtNode));

				SPV_STATE_TYPE ssx = (SPV_STATE_TYPE) rectLinked.Width();
				SPV_STATE_TYPE ssy = (SPV_STATE_TYPE) rectLinked.Height();

				SPV_STATE_TYPE x = vInput[nAtNodeView*2 + 0] - vInput[nAtLinkedView*2 + 0];
				SPV_STATE_TYPE y = vInput[nAtNodeView*2 + 1] - vInput[nAtLinkedView*2 + 1];
				
				// compute the energy due to this interation

				// add the repulsion fiel
				m_energy += (SPV_STATE_TYPE) 5.0
					* m_spacerFunc(
						x / ssx, 
						y / ssy);

				// add attraction * weight
				m_energy -= weight * (SPV_STATE_TYPE) 275.0
					* m_attractFunc(
						x / (ssx * (SPV_STATE_TYPE) 3.0), 
						y / (ssy * (SPV_STATE_TYPE) 3.0));
			}
		}
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


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
	return // 1.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f, 1.0f) 
		+ 2.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f / 2.0f, 1.0f / 2.0f)
		+ 4.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f / 4.0f, 1.0f / 4.0f)
		+ 8.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f / 8.0f, 1.0f / 8.0f);
}

//////////////////////////////////////////////////////////////////////
// center_func
// 
// function which evaluates a centering field at a given point, with
//		the given parameters
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE center_func(SPV_STATE_TYPE x, SPV_STATE_TYPE y, 
						   SPV_STATE_TYPE width, SPV_STATE_TYPE height, 
						   SPV_STATE_TYPE cs)
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

		// stores the x and y coordinates for various computations;
		SPV_STATE_TYPE x;
		SPV_STATE_TYPE y;
		SPV_STATE_TYPE dx = (SPV_STATE_TYPE) rectNodeView.Width() 
			/ (SPV_STATE_TYPE) 4.0;
		SPV_STATE_TYPE dy = (SPV_STATE_TYPE) rectNodeView.Height() 
			/ (SPV_STATE_TYPE) 4.0;

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

				x = vInput[nAtNodeView*2 + 0] - vInput[nAtLinkedView*2 + 0];
				y = vInput[nAtNodeView*2 + 1] - vInput[nAtLinkedView*2 + 1];

				// compute the energy due to this interation
				for (int nX = -2; nX <= 2; nX++)
					for (int nY = -2; nY <= 2; nY++)
					{
						m_energy += (SPV_STATE_TYPE) (1.0 / 20.0)
							* m_spacerFunc((x + dx * (SPV_STATE_TYPE) nX) / ssx, 
								(y + dy * (SPV_STATE_TYPE) nY) / ssy);
					}

				// add general repulsion
				// m_energy += 0.5
				//	* m_attractFunc(x / (ssx * 1.0), y / (ssy * 1.0));

				// add attraction * weight
				m_energy -= weight * (SPV_STATE_TYPE) 125.0
					* m_attractFunc(
						x / (ssx * (SPV_STATE_TYPE) 7.0), 
						y / (ssy * (SPV_STATE_TYPE) 7.0));
			}
		}

		SPV_STATE_TYPE width = (SPV_STATE_TYPE) rectSpaceView.Width();
		SPV_STATE_TYPE height = (SPV_STATE_TYPE) rectSpaceView.Height();
		SPV_STATE_TYPE sigma = width + height / (SPV_STATE_TYPE) 32.0;

		SPV_STATE_TYPE nodeViewWidth = (SPV_STATE_TYPE) rectNodeView.Width();
		SPV_STATE_TYPE nodeViewHeight = (SPV_STATE_TYPE) rectNodeView.Height();

		// set the x and y coordinates for the centering calculation
		x = vInput[nAtNodeView*2 + 0];
		y = vInput[nAtNodeView*2 + 1];

		m_energy += center_func(x - nodeViewWidth / (SPV_STATE_TYPE) 2.0,  y, 
			width, height, sigma);
		m_energy += center_func(x, y, 
			width, height, sigma);
		m_energy += center_func(x + nodeViewWidth / (SPV_STATE_TYPE) 2.0,  
			y, width, height, sigma);

		m_energy += center_func(x, y - nodeViewHeight / (SPV_STATE_TYPE) 2.0, 
			width, height, sigma);
//			m_energy += center_func(x, y,                        width, height, sigma);
		m_energy += center_func(x, y + nodeViewHeight / (SPV_STATE_TYPE) 2.0, 
			width, height, sigma);
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


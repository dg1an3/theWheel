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

#define SIGMA_POINT 1.60

//////////////////////////////////////////////////////////////////////
// attract_func
// 
// function which evaluates a gaussian attraction field at a given 
//		point
//////////////////////////////////////////////////////////////////////

const SPV_STATE_TYPE k = (0.3989f - exp(-SIGMA_POINT * SIGMA_POINT / 2.0f) / (sqrt(2.0f * PI)));

SPV_STATE_TYPE attract_func(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return // Gauss2D<SPV_STATE_TYPE>(x, y , 1.0f, 1.0f);
		0.3989f - k * (x * x + y * y);
}

SPV_STATE_TYPE dSq_attract_func(SPV_STATE_TYPE dSq, SPV_STATE_TYPE)
{
	return // Gauss2D<SPV_STATE_TYPE>(x, y , 1.0f, 1.0f);
		0.3989f - k * dSq;
}

//////////////////////////////////////////////////////////////////////
// dattract_func_dx
// 
// function which evaluates a derivative of gaussian attraction 
//		field at a given point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE dattract_func_dx(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return dGauss2D_dx<SPV_STATE_TYPE>(x, y , 1.0f, 1.0f);
}

//////////////////////////////////////////////////////////////////////
// dattract_func_dy
// 
// function which evaluates a derivative of gaussian attraction 
//		field at a given point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE dattract_func_dy(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return dGauss2D_dy<SPV_STATE_TYPE>(x, y , 1.0f, 1.0f);
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
		1.0f / // sqrt
			(x * x + y * y + 0.1);
	/*
		+ 1.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 8.0f, 8.0f) 
		+ 1.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 4.0f, 4.0f) 
		+ 1.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 2.0f, 2.0f) 
		+ 1.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f, 1.0f) 
		+ 2.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f / 2.0f, 1.0f / 2.0f)
		+ 4.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f / 4.0f, 1.0f / 4.0f)
		+ 8.0f * Gauss2D<SPV_STATE_TYPE>(x, y, 1.0f / 8.0f, 1.0f / 8.0f);
	*/
}

SPV_STATE_TYPE dSq_spacer_func(SPV_STATE_TYPE dSq, SPV_STATE_TYPE)
{
	return 1.0f / sqrt(dSq);
}

//////////////////////////////////////////////////////////////////////
// dspacer_func_dx
// 
// function which evaluates a gaussian spacer field at a given 
//		point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE dspacer_func_dx(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return
		+ 1.0f * dGauss2D_dx<SPV_STATE_TYPE>(x, y, 1.0f, 1.0f) 
				/ ((SPV_STATE_TYPE) 2.0 * 1.0f * 1.0f)
		+ 2.0f * dGauss2D_dx<SPV_STATE_TYPE>(x, y, 1.0f / 2.0f, 1.0f / 2.0f)
				/ ((SPV_STATE_TYPE) 2.0 * 1.0f / 2.0f * 1.0f / 2.0f)
		+ 4.0f * dGauss2D_dx<SPV_STATE_TYPE>(x, y, 1.0f / 4.0f, 1.0f / 4.0f)
				/ ((SPV_STATE_TYPE) 2.0 * 1.0f / 4.0f * 1.0f / 4.0f)
		+ 8.0f * dGauss2D_dx<SPV_STATE_TYPE>(x, y, 1.0f / 8.0f, 1.0f / 8.0f)
				/ ((SPV_STATE_TYPE) 2.0 * 1.0f / 8.0f * 1.0f / 8.0f);
}

//////////////////////////////////////////////////////////////////////
// dspacer_func_dy
// 
// function which evaluates a gaussian spacer field at a given 
//		point
//////////////////////////////////////////////////////////////////////
SPV_STATE_TYPE dspacer_func_dy(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return
		+ 1.0f * dGauss2D_dy<SPV_STATE_TYPE>(x, y, 1.0f, 1.0f) 
				/ ((SPV_STATE_TYPE) 2.0 * 1.0f * 1.0f)
		+ 2.0f * dGauss2D_dy<SPV_STATE_TYPE>(x, y, 1.0f / 2.0f, 1.0f / 2.0f)
				/ ((SPV_STATE_TYPE) 2.0 * 1.0f / 2.0f * 1.0f / 2.0f)
		+ 4.0f * dGauss2D_dy<SPV_STATE_TYPE>(x, y, 1.0f / 4.0f, 1.0f / 4.0f)
				/ ((SPV_STATE_TYPE) 2.0 * 1.0f / 4.0f * 1.0f / 4.0f)
		+ 8.0f * dGauss2D_dy<SPV_STATE_TYPE>(x, y, 1.0f / 8.0f, 1.0f / 8.0f)
				/ ((SPV_STATE_TYPE) 2.0 * 1.0f / 8.0f * 1.0f / 8.0f);
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

		/*
		m_attractFunc(&attract_func, -4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, 
			"ATTRFUNC.TMP", __TIMESTAMP__),
		m_dattractFunc_dx(&dattract_func_dx, -4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, 
			"ATTRFUNC_DX.TMP", __TIMESTAMP__),
		m_dattractFunc_dy(&dattract_func_dy, -4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, 
			"ATTRFUNC_DY.TMP", __TIMESTAMP__),
		m_spacerFunc(&spacer_func, -4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, 
			"SPACERFUNC.TMP", __TIMESTAMP__),
		m_dspacerFunc_dx(&dspacer_func_dx, -4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, 
			"SPACERFUNC_DX.TMP", __TIMESTAMP__),
		m_dspacerFunc_dy(&dspacer_func_dy, -4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, 
			"SPACERFUNC_DY.TMP", __TIMESTAMP__) 
		*/

//		m_attractFunc(&attract_func, 0.0f, 0.0f, 1, 0.0f, 0.0f, 1, 
//			"ATTRFUNC.TMP", __TIMESTAMP__),
//		m_attractFunc(&attract_func, -32.0f, 32.0f, 4096, -32.0f, 32.0f, 4096, 
//			"ATTRFUNC.TMP", __TIMESTAMP__),
//		m_dattractFunc_dx(&dattract_func_dx, -32.0f, 32.0f, 4096, -32.0f, 32.0f, 4096, 
//			"ATTRFUNC_DX.TMP", __TIMESTAMP__),
//		m_dattractFunc_dy(&dattract_func_dy, -32.0f, 32.0f, 4096, -32.0f, 32.0f, 4096, 
//			"ATTRFUNC_DY.TMP", __TIMESTAMP__),

//		m_spacerFunc(&spacer_func, 0.0f, 0.0f, 1, 0.0f, 0.0f, 1, 
//			"SPACERFUNC.TMP", __TIMESTAMP__)
//		m_spacerFunc(&spacer_func, -32.0f, 32.0f, 4096, -32.0f, 32.0f, 4096, 
//			"SPACERFUNC.TMP", __TIMESTAMP__),
//		m_dspacerFunc_dx(&dspacer_func_dx, -32.0f, 32.0f, 4096, -32.0f, 32.0f, 4096, 
//			"SPACERFUNC_DX.TMP", __TIMESTAMP__),
//		m_dspacerFunc_dy(&dspacer_func_dy, -32.0f, 32.0f, 4096, -32.0f, 32.0f, 4096, 
//			"SPACERFUNC_DY.TMP", __TIMESTAMP__)

		// m_dSq_attractFunc(&dSq_attract_func, -1024.0f, 1024.0f, 4096, -0.1f, 0.0f, 1,
		//	"DSQSPACERFUNC.TMP", __TIMESTAMP__),
//		m_dSq_spacerFunc(&dSq_spacer_func, -1024.0f, 1024.0f, 4096, -0.1f, 0.0f, 1,
//			"DSQATTRFUNC.TMP", __TIMESTAMP__)
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
		m_vSize[nAtNodeView][0] = ssx + 10.0;
		m_vSize[nAtNodeView][1] = ssy + 10.0;

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
				weight += 0.0001;

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
	m_energy = 0.0;

	// initialize the gradient vector to zeros
	m_vGrad = CVector<SPV_STATE_DIM, SPV_STATE_TYPE>();

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
		// iterate over the potential linked views
		int nAtLinkedView;
		for (nAtLinkedView = 0; nAtLinkedView < nNumVizNodeViews; nAtLinkedView++)
		{
			// only processs the linked view if it is in the vector
			if (nAtLinkedView != nAtNodeView) 
			{
				// compute the x- and y-offset between the views
				SPV_STATE_TYPE x = vInput[nAtNodeView*2 + 0] - vInput[nAtLinkedView*2 + 0];
				SPV_STATE_TYPE y = vInput[nAtNodeView*2 + 1] - vInput[nAtLinkedView*2 + 1];

				// compute the x- and y-scales for the fields (from the linked rectangle)
				// CRect rectLinked = pAtLinkedView->GetOuterRect();
				SPV_STATE_TYPE ssx = m_vSize[nAtLinkedView][0]; 
				SPV_STATE_TYPE ssy = m_vSize[nAtLinkedView][1];

				// compute the energy due to this interation

				// add the repulsion fiel
				m_energy += (SPV_STATE_TYPE) 3.5
				//	/ (x * x / (ssx) + y * y / (ssy) + 0.1);
					/ (x * x / (ssx * ssx) + y * y / (ssy * ssy) + 0.1);
				// spacer_func( // m_spacerFunc(
				//		x / ssx, 
				//		y / ssy);

				// add attraction * weight
				// SPV_STATE_TYPE attract_term = 
				//	m_attractFunc(
				//		x / (ssx * (SPV_STATE_TYPE) 4.0), 
				//		y / (ssy * (SPV_STATE_TYPE) 4.0));
				SPV_STATE_TYPE attract_term_comp = 
					attract_func(
						x / (ssx * (SPV_STATE_TYPE) 4.0), 
						y / (ssy * (SPV_STATE_TYPE) 4.0));
				// ASSERT(fabs(attract_term - attract_term_comp) < 1.0);
				// SPV_STATE_TYPE dSq_attract_term = 
				//	dSq_attract_func(
				//		x * x / (ssx * ssx * 16.0f) + y * y / (ssy * ssy * 16.0f), 0);
				m_energy -= m_mLinks[nAtNodeView][nAtLinkedView] * (SPV_STATE_TYPE) 400.0
					* attract_term_comp;

				// m_vInput[0] = dSq_attract_term;
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


////////////////////////////////////
// Copyright (C) 1996-2000 DG Lane
// U.S. Patent Pending
////////////////////////////////////

#include "stdafx.h"

#include "SpaceViewEnergyFunction.h"
#include "SpaceView.h"

#include "LookupFunction.h"

SPV_STATE_TYPE SPACER_AMPL = 35.0f;
SPV_STATE_TYPE ATTRACT_AMPL = 140.0f; 

SPV_STATE_TYPE CENTER_AMPL = 0.03f;

SPV_STATE_TYPE Gauss2D(SPV_STATE_TYPE x, SPV_STATE_TYPE y, SPV_STATE_TYPE sx, SPV_STATE_TYPE sy)
{
	SPV_STATE_TYPE d = (x * x) / (2.0f * sx * sx) 
		+ (y * y) / (2.0f * sy * sy);

	return (SPV_STATE_TYPE) exp(-d)
		/ (SPV_STATE_TYPE) (sqrt(2.0f * PI * sx * sy));
}

SPV_STATE_TYPE Gauss(SPV_STATE_TYPE x, SPV_STATE_TYPE s)
{
	SPV_STATE_TYPE d = (x * x) / (2.0f * s * s); 

	return (SPV_STATE_TYPE) exp(-d)
		/ (SPV_STATE_TYPE) (sqrt(2.0f * PI * s));
}

SPV_STATE_TYPE attract_func(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return Gauss2D(x, y, 1.0f, 1.0f);
}

CLookupFunction<SPV_STATE_TYPE> attractFunc(&attract_func, 
		-4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, "ATTRFUNC.TMP");

SPV_STATE_TYPE dattract_func_dx(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return -x * Gauss2D(x, y, 1.0f, 1.0f);
}

CLookupFunction<SPV_STATE_TYPE> dAttractFuncDx(&attract_func, 
		-4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, "ATTRFUNCDX.TMP");

SPV_STATE_TYPE dattract_func_dy(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return -y * Gauss2D(x, y, 1.0f, 1.0f);
}

CLookupFunction<SPV_STATE_TYPE> dAttractFuncDy(&attract_func, 
		-4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, "ATTRFUNCDY.TMP");

SPV_STATE_TYPE spacer_func(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return Gauss2D(x, y, 1.0f, 1.0f)
		+ 2.0f * Gauss2D(x, y, 1.0f / 2.0f, 1.0f / 2.0f)
		+ 4.0f * Gauss2D(x, y, 1.0f / 4.0f, 1.0f / 4.0f);
}

CLookupFunction<SPV_STATE_TYPE> spacerFunc(&spacer_func, 
		-4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, "SPACERFUNC.TMP");

SPV_STATE_TYPE dspacer_func_dx(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return -x * Gauss2D(x, y, 1.0f, 1.0f)
		+ 2.0f * -x * 4.0f * Gauss2D(x, y, 1.0f / 2.0f, 1.0f / 2.0f)
		+ 4.0f * -x * 16.0f * Gauss2D(x, y, 1.0f / 4.0f, 1.0f / 4.0f);
}

CLookupFunction<SPV_STATE_TYPE> dSpacerFuncDx(&spacer_func, 
		-4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, "SPACERFUNCDX.TMP");

SPV_STATE_TYPE dspacer_func_dy(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return -y * Gauss2D(x, y, 1.0f, 1.0f)
		+ 2.0f * -y * 4.0f * Gauss2D(x, y, 1.0f / 2.0f, 1.0f / 2.0f)
		+ 4.0f * -y * 16.0f * Gauss2D(x, y, 1.0f / 4.0f, 1.0f / 4.0f);
}

CLookupFunction<SPV_STATE_TYPE> dSpacerFuncDy(&spacer_func, 
		-4.0f, 4.0f, 1024, -4.0f, 4.0f, 1024, "SPACERFUNCDY.TMP");

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

SPV_STATE_TYPE dCenterFieldDx(SPV_STATE_TYPE x, SPV_STATE_TYPE y, SPV_STATE_TYPE width, SPV_STATE_TYPE height, SPV_STATE_TYPE cs)
{
	SPV_STATE_TYPE denergy = 0.0f;

	// compute distance to nearest side
	SPV_STATE_TYPE distToLeft = x;
	if (distToLeft < 0.0)
		denergy = -1.0;

	SPV_STATE_TYPE distToRight = width - x;
	if (distToRight < 0.0)
		denergy = 1.0;

	SPV_STATE_TYPE distMin = min(distToLeft, distToRight);

	return denergy + 24.0f * -(distMin + cs) / (cs * cs) * Gauss(distMin + cs, cs);
}

SPV_STATE_TYPE dCenterFieldDy(SPV_STATE_TYPE x, SPV_STATE_TYPE y, SPV_STATE_TYPE width, SPV_STATE_TYPE height, SPV_STATE_TYPE cs)
{
	SPV_STATE_TYPE denergy = 0.0f;

	SPV_STATE_TYPE distToTop = y;
	if (distToTop < 0.0)
		denergy = -1.0;

	SPV_STATE_TYPE distToBottom = height - y;
	if (distToBottom < 0.0)
		denergy = 1.0;

	SPV_STATE_TYPE distMin = min(distToTop, distToBottom);

	return denergy + 24.0f * -(distMin + cs) / (cs * cs) * Gauss(distMin + cs, cs);
}

CVector<SPV_STATE_DIM, SPV_STATE_TYPE> CSpaceViewEnergyFunction::GetStateVector()
{
	// compute the threshold
	GetThreshold();

	// set up the mapping array
	m_arrElementMap.SetSize(m_pView->nodeViews.GetSize());

	CVector<SPV_STATE_DIM, SPV_STATE_TYPE> vState;
	int nAtVectorElement = 0;
	for (int nAt = 0; nAt < m_pView->nodeViews.GetSize() && nAtVectorElement < SPV_STATE_DIM; nAt++)
	{
		CNodeView *pView = m_pView->nodeViews.Get(nAt);
		if (pView->activation.Get() > CNodeView::activationThreshold)
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
	return vState;
}

void CSpaceViewEnergyFunction::SetStateVector(const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vState)
{
	for (int nAt = 0; nAt < m_pView->nodeViews.GetSize(); nAt++)
	{
		if (m_arrElementMap[nAt] != -1)
		{
			CNodeView *pView = m_pView->nodeViews.Get(nAt);
			pView->center.Set(CVector<2>(vState[m_arrElementMap[nAt]], 
				vState[m_arrElementMap[nAt]+1]));
		}
	}
}


SPV_STATE_TYPE CSpaceViewEnergyFunction::GetThreshold()
{
	if (m_pView->nodeViews.GetSize() <= SPV_STATE_DIM / 2)
	{
		CNodeView::activationThreshold = 0.0000001f;
		return 0.0000001f;
	}

	// set initial threshold greater than the max activation
	SPV_STATE_TYPE threshold = 2.0;
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

	CNodeView::activationThreshold = (float) prevThreshold;
	return prevThreshold;
}

const CArray<int, int>& CSpaceViewEnergyFunction::GetMap()
{
	return m_arrElementMap;
}

SPV_STATE_TYPE CSpaceViewEnergyFunction::operator()(const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput)
{
	// reset the energy
	m_energy = 0.0;

	// store this input
	m_vInput = vInput;

	// retrieve the parent's rectangle
	CRect rectSpaceView;
	m_pView->GetWindowRect(&rectSpaceView;);

	// iterate over all child node views
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < m_pView->nodeViews.GetSize(); nAtNodeView++)
	{
		// if the node is not in the state vector, skip it
		if (GetMap()[nAtNodeView] != -1)
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

			// iterate over the potential linked views
			int nAtLinkedView;
			for (nAtLinkedView = 0; nAtLinkedView < m_pView->nodeViews.GetSize(); 
					nAtLinkedView++)
			{
				// only processs the linked view if it is in the vector
				if (nAtLinkedView != nAtNodeView 
						&& GetMap()[nAtLinkedView] != -1)
				{
					// get convenience pointers for the linked node view and node
					CNodeView *pAtLinkedView = m_pView->nodeViews.Get(nAtLinkedView);
					CNode *pAtLinkedNode = pAtLinkedView->forNode.Get();

					// get the rectangle of the current linked view
					CRect rectLinked;
					pAtLinkedView->GetWindowRect(&rectLinked);

					// get the weight of the link
					CNodeLink *pLink = pAtLinkedNode->GetLink(pAtNode);

					// retrieve the link weight
					SPV_STATE_TYPE weight1 = 0.0;
					if (pLink != NULL)
						weight1 = pLink->weight.Get();

					pLink = pAtNode->GetLink(pAtLinkedNode);
					SPV_STATE_TYPE weight2 = 0.0;
					if (pLink != NULL)
						weight2 = pLink->weight.Get();

					SPV_STATE_TYPE weight = (weight1 + weight2) / 2.0f;

					SPV_STATE_TYPE ssx = MinSize((SPV_STATE_TYPE) rectLinked.Width(), 
						(SPV_STATE_TYPE) rectWnd.Width() / 8.0f);
					SPV_STATE_TYPE ssy = MinSize((SPV_STATE_TYPE) rectLinked.Height(), 
						(SPV_STATE_TYPE) rectWnd.Height() / 8.0f);

					SPV_STATE_TYPE x = vInput[GetMap()[nAtNodeView] + 0] 
						- vInput[GetMap()[nAtLinkedView] + 0];
					SPV_STATE_TYPE y = vInput[GetMap()[nAtNodeView] + 1] 
						- vInput[GetMap()[nAtLinkedView] + 1];
					SPV_STATE_TYPE dx = (SPV_STATE_TYPE) rectNodeView.Width() / 2.0;
					SPV_STATE_TYPE dy = (SPV_STATE_TYPE) rectNodeView.Height() / 2.0;

					// compute the energy due to this interation
					for (int nX = -1; nX <= 1; nX++)
						for (int nY = -1; nY <= 1; nY++)
						{
							m_energy += 1.0 / 20.0
								* spacerFunc((x + dx * (SPV_STATE_TYPE) nX) / ssx, 
									(y + dy * (SPV_STATE_TYPE) nY) / ssy);

#ifdef USE_GRAD
							dEnergyDx +=  1.0 / 20.0
								* dSpacerFuncDx((x + dx * (SPV_STATE_TYPE) nX) / ssx, 
									(y + dy * (SPV_STATE_TYPE) nY) / ssy)
									/ ssx;

							dEnergyDy += 1.0 / 20.0
								* dSpacerFuncDy((x + dx * (SPV_STATE_TYPE) nX) / ssx, 
									(y + dy * (SPV_STATE_TYPE) nY) / ssy)
									/ ssy;
#endif
						}

					m_energy -= weight * 4.5
						* attractFunc(x / (ssx * 2.0), y / (ssy * 2.0));

#ifdef USE_GRAD
					dEnergyDx -= weight * 4.5
						* dAttractFuncDx(x / (ssx * 2.0), y / (ssy * 2.0))
							/ (ssx * 2.0);

					dEnergyDy -= weight * 4.5
						* dAttractFuncDy(x / (ssx * 2.0), y / (ssy * 2.0))
							/ (ssy * 2.0);
#endif
				}
			}

			SPV_STATE_TYPE width = (SPV_STATE_TYPE) rectWnd.Width();
			SPV_STATE_TYPE height = (SPV_STATE_TYPE) rectWnd.Height();
			SPV_STATE_TYPE sigma = width + height / 32.0;

			m_energy += CenterField(vInput[GetMap()[nAtNodeView]] - rectNodeView.Width() / 2.0, 
				vInput[GetMap()[nAtNodeView]+1],
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);

			m_energy += CenterField(vInput[GetMap()[nAtNodeView]], vInput[GetMap()[nAtNodeView]+1],
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);

			m_energy += CenterField(vInput[GetMap()[nAtNodeView]] + rectNodeView.Width() / 2.0, 
				vInput[GetMap()[nAtNodeView]+1],
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);

			m_energy += CenterField(vInput[GetMap()[nAtNodeView]], 
				vInput[GetMap()[nAtNodeView]+1] - rectNodeView.Height() / 2.0,
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);

			m_energy += CenterField(vInput[GetMap()[nAtNodeView]], vInput[GetMap()[nAtNodeView]+1],
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);

			m_energy += CenterField(vInput[GetMap()[nAtNodeView]], 
				vInput[GetMap()[nAtNodeView]+1] + rectNodeView.Height() / 2.0,
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);

#ifdef USE_GRAD

			dEnergyDx += dCenterFieldDx(vInput[GetMap()[nAtNodeView]], 
				vInput[GetMap()[nAtNodeView]+1],
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);
			dEnergyDx += dCenterFieldDx(vInput[GetMap()[nAtNodeView]] - rectNodeView.Width() / 2.0, 
				vInput[GetMap()[nAtNodeView]+1],
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);
			dEnergyDx += dCenterFieldDx(vInput[GetMap()[nAtNodeView]] + rectNodeView.Width() / 2.0, 
				vInput[GetMap()[nAtNodeView]+1],
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);
			dEnergyDx += dCenterFieldDx(vInput[GetMap()[nAtNodeView]], 
				vInput[GetMap()[nAtNodeView]+1] - rectNodeView.Height() / 2.0,
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);
			dEnergyDx += dCenterFieldDx(vInput[GetMap()[nAtNodeView]], 
				vInput[GetMap()[nAtNodeView]+1] + rectNodeView.Height() / 2.0,
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);

			dEnergyDy += dCenterFieldDy(vInput[GetMap()[nAtNodeView]], 
				vInput[GetMap()[nAtNodeView]+1],
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);
			dEnergyDy += dCenterFieldDy(vInput[GetMap()[nAtNodeView]] - rectNodeView.Width() / 2.0, 
				vInput[GetMap()[nAtNodeView]+1],
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);
			dEnergyDy += dCenterFieldDy(vInput[GetMap()[nAtNodeView]] + rectNodeView.Width() / 2.0, 
				vInput[GetMap()[nAtNodeView]+1],
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);
			dEnergyDy += dCenterFieldDy(vInput[GetMap()[nAtNodeView]], 
				vInput[GetMap()[nAtNodeView]+1] - rectNodeView.Height() / 2.0,
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);
			dEnergyDy += dCenterFieldDy(vInput[GetMap()[nAtNodeView]], 
				vInput[GetMap()[nAtNodeView]+1] + rectNodeView.Height() / 2.0,
				(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);

			// now store the two-dimensional gradient components;
			m_vGrad[GetMap()[nAtNodeView]] = dEnergyDx;
			m_vGrad[GetMap()[nAtNodeView]+1] = dEnergyDy;
#endif

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



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

SPV_STATE_TYPE spacer_func(SPV_STATE_TYPE x, SPV_STATE_TYPE y)
{
	return Gauss2D(x, y, 1.0f, 1.0f)
		+ 2.0f * Gauss2D(x, y, 1.0f / 2.0f, 1.0f / 2.0f)
		+ 4.0f * Gauss2D(x, y, 1.0f / 4.0f, 1.0f / 4.0f);
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
	CVector<SPV_STATE_DIM, SPV_STATE_TYPE> vState;
	for (int nAt = 0; nAt < min(SPV_STATE_DIM / 2, m_pView->nodeViews.GetSize()); nAt++)
	{
		CNodeView *pView = m_pView->nodeViews.Get(nAt);
		vState[nAt * 2] = (SPV_STATE_TYPE) pView->center.Get()[0];
		vState[nAt * 2 + 1] = (SPV_STATE_TYPE) pView->center.Get()[1];
	}
	return vState;
}

void CSpaceViewEnergyFunction::SetStateVector(const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vState)
{
	for (int nAt = 0; nAt < min(SPV_STATE_DIM / 2, m_pView->nodeViews.GetSize()); nAt++)
	{
		CNodeView *pView = m_pView->nodeViews.Get(nAt);
		pView->center.Set(CVector<2>(vState[nAt * 2], vState[nAt * 2 + 1]));
	}
}


SPV_STATE_TYPE CSpaceViewEnergyFunction::operator()(const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput)
{
	SPV_STATE_TYPE energy = 0.0;

	CRect rectWnd;
	m_pView->GetWindowRect(&rectWnd);

	CCollection<CNodeView>& nodeViews = m_pView->nodeViews;

	// iterate over all child node views
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < nodeViews.GetSize(); 
			nAtNodeView++)
	{
		/* if (nAtNodeView->activation.Get() < m_threshold)
			break; */

		// get convenience pointers for the current node view and node
		CNodeView *pAtNodeView = nodeViews.Get(nAtNodeView);
		CNode *pAtNode = pAtNodeView->forNode.Get();

		CRect rectNodeView;
		pAtNodeView->GetWindowRect(&rectNodeView);

		// iterate over the potential linked views
		int nAtLinkedView;
		for (nAtLinkedView = 0; nAtLinkedView < nodeViews.GetSize(); 
				nAtLinkedView++)
		{
			if (nAtLinkedView != nAtNodeView)
			{

				// get convenience pointers for the linked node view and node
				CNodeView *pAtLinkedView = nodeViews.Get(nAtLinkedView);
				CNode *pAtLinkedNode = pAtLinkedView->forNode.Get();

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

				CRect rectLinked;
				pAtLinkedView->GetWindowRect(&rectLinked);

				SPV_STATE_TYPE ssx = MinSize((SPV_STATE_TYPE) rectLinked.Width(), 
					(SPV_STATE_TYPE) rectWnd.Width() / 8.0f);
				SPV_STATE_TYPE ssy = MinSize((SPV_STATE_TYPE) rectLinked.Height(), 
					(SPV_STATE_TYPE) rectWnd.Height() / 8.0f);

				SPV_STATE_TYPE x = vInput[nAtNodeView*2 + 0] - vInput[nAtLinkedView*2 + 0];
				SPV_STATE_TYPE y = vInput[nAtNodeView*2 + 1] - vInput[nAtLinkedView*2 + 1];
				SPV_STATE_TYPE dx = (SPV_STATE_TYPE) rectNodeView.Width() / 2.0;
				SPV_STATE_TYPE dy = (SPV_STATE_TYPE) rectNodeView.Height() / 2.0;

				// compute the energy due to this interation
				for (int nX = -1; nX <= 1; nX++)
					for (int nY = -1; nY <= 1; nY++)
						energy += 1.0 / 20.0
							* spacerFunc((x + dx * (SPV_STATE_TYPE) nX) / ssx, 
								(y + dy * (SPV_STATE_TYPE) nY) / ssy);

				energy -= weight * 4.5
					* attractFunc(x / (ssx * 2.0), y / (ssy * 2.0));
			}
		}

		SPV_STATE_TYPE sigma = (SPV_STATE_TYPE) rectWnd.Width() + (SPV_STATE_TYPE) rectWnd.Height() / 32.0;

		energy += CenterField(vInput[nAtNodeView*2], 
			vInput[nAtNodeView*2+1],
			(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);
		energy += CenterField(vInput[nAtNodeView*2] - rectNodeView.Width() / 2.0, 
			vInput[nAtNodeView*2+1],
			(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);
		energy += CenterField(vInput[nAtNodeView*2] + rectNodeView.Width() / 2.0, 
			vInput[nAtNodeView*2+1],
			(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);
		energy += CenterField(vInput[nAtNodeView*2], 
			vInput[nAtNodeView*2+1] - rectNodeView.Height() / 2.0,
			(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);
		energy += CenterField(vInput[nAtNodeView*2], 
			vInput[nAtNodeView*2+1] + rectNodeView.Height() / 2.0,
			(SPV_STATE_TYPE) rectWnd.Width(), (SPV_STATE_TYPE) rectWnd.Height(), sigma);
	}

	return energy;
}


#include "StdAfx.h"

#include <CastVectorD.h>
#include "NodeViewSkin2.h"
#include "NodeView.h"
#include "Elliptangle.h"

namespace theWheel2007 {

// default border radius for plaque
const REAL BORDER_RADIUS = 2.0f; // 0.04f;

// constants (should be in configuration)
const REAL RECT_SIZE_MAX = 13.0f / 16.0f;
const REAL RECT_SIZE_SCALE = 6.0f / 16.0f;

/////////////////////////////////////////////////////////////////////////////
REAL ComputeEllipticalness(REAL activation)
{
	// compute the r, which represents the amount of "elliptical-ness"
	REAL scale = (1.0f - 1.0f / sqrt(2.0f));
	return 1.0f - scale * exp(-1.5f * activation + 0.01f);

}	// ComputeEllipticalness


//////////////////////////////////////////////////////////////////////
void
	NodeViewSkin::CalcRectForActivation(REAL activation, CExtent<2, REAL>& extent)
	// returns the size of the node (width & height)
{
	// compute the desired aspect ratio (maintain the current aspect ratio)
	REAL aspectRatio = RECT_SIZE_MAX - RECT_SIZE_SCALE * exp(-2.0f * activation);

	// compute the new width and height from the desired area and the desired
	//		aspect ratio
	CVectorD<2, REAL> sz;
	sz[0] = sqrt(activation / aspectRatio) / 1.0f;
	sz[1] = sqrt(activation * aspectRatio) / 1.0f;

	// CExtent<3, REAL> rect;
	extent.Deflate(-sz[0], -sz[1], -sz[0], -sz[1]); // Inflate(sz);  /// TODO: add Inflate to Extent

	// return the computed size
	// return rect;

}	// CalcRectForActivation


//////////////////////////////////////////////////////////////////////
int
	NodeViewSkin::GetPlaqueIndex(REAL activation)
{
	// form the index
	int nIndex = Round<int>(activation * 200.0f); // 1000.0f);
	nIndex = __max(nIndex, 1);

	// see if we have the molding yet
	if (nIndex >= m_arrPlaques.size())
	{
		// make room for the new plaque
		m_arrPlaques.resize(nIndex+1);
	}

	// is the plaque already existing?
	if (!m_arrPlaques[nIndex])
	{
		REAL indexActivation = (REAL) nIndex / 100.0f; // 1000.0f;

		// set the width and height of the window, keeping the center constant
		CExtent<2, REAL> rectInner;
		CalcRectForActivation(indexActivation, rectInner);

		/// set up elliptangle
		Elliptangle *pElliptangle = 
			new Elliptangle(rectInner, ComputeEllipticalness(indexActivation));

		// create a previous molding, to connect to next
		REAL borderRadius = BORDER_RADIUS;
		if (rectInner.GetSize(1) < borderRadius * 4.0f)
		{
			borderRadius = rectInner.GetSize(1) / 4.0f;
		}

		// and create it
		m_arrPlaques[nIndex] = new Plaque(m_pDevice, pElliptangle, borderRadius);
	}

	// and return the index
	return nIndex;

}	// NodeViewSkin::GetMolding


//////////////////////////////////////////////////////////////////////
void 
	NodeViewSkin::Render(CNodeView *pNV)
	// renders the node view skin at the given height
{
	REAL activation = pNV->GetNode()->GetActivation();

	// get nearest plaque for the activation
	int nIndex = GetPlaqueIndex(activation);

	// compute scaling to requested activation size
	CExtent<2,REAL> extent;
	CalcRectForActivation(activation, extent);
	pNV->m_extInner.SetMin(CCastVectorD<3,REAL>(extent.GetMin()));
	pNV->m_extInner.SetMax(CCastVectorD<3,REAL>(extent.GetMax()));

	REAL sizeUp = 100.0 + 1200.0 * (activation) * (activation);
	REAL scaleX = sizeUp * pNV->m_extInner.GetSize(0) /
		m_arrPlaques[nIndex]->GetShape()->InnerRect.GetSize(0);
	REAL scaleY = sizeUp * pNV->m_extInner.GetSize(1) /
		m_arrPlaques[nIndex]->GetShape()->InnerRect.GetSize(1);
	//REAL scaleX = m_arrPlaques[nIndex]->GetShape()->InnerRect.Width / pNV->InnerRect.Width;
	//REAL scaleY = m_arrPlaques[nIndex]->GetShape()->InnerRect.Height / pNV->InnerRect.Height;

	// and set the world transform for the scale factos
	D3DXMATRIX oldWorld;
	ASSERT_HRESULT(m_pDevice->GetTransform( D3DTS_WORLD, (D3DMATRIX*) & oldWorld));

	D3DXMATRIX scaleWorld;
	D3DXMatrixScaling(&scaleWorld, scaleX, scaleY, (scaleX + scaleY) / 2.0f);

	D3DXMATRIX newWorld = scaleWorld * oldWorld;
	// D3DXMatrixMultiply(&newWorld, &scaleWorld, &oldWorld);
	
	ASSERT_HRESULT(m_pDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*) & newWorld));

	// render
	m_arrPlaques[nIndex]->Render();

	ASSERT_HRESULT(m_pDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&oldWorld ));

}	// NodeViewSkin::Render

}	// namespace 

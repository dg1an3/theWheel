// Molding.cpp: implementation of the CMolding class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Molding.h"

#include "DDUTIL.H"

#include <MatrixD.h>

D3DVERTEX& FromVectors(CVectorD<3> vVert, CVectorD<3> vNorm)
{
	static D3DVERTEX vert;
	ZeroMemory(&vert, sizeof(D3DVERTEX));

	vert.x = vVert[0];
	vert.y = vVert[1];
	vert.z = vVert[2];

	vert.nx = vNorm[0];
	vert.ny = vNorm[1];
	vert.nz = vNorm[2];

	return vert;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CMolding::CMolding(double theta, CVectorD<3> vOffset, double radius)
: m_borderRadius(radius)
{
	// create the rotation matrix for this molding fragment
	CMatrixD<4> mRotate = 
		CMatrixD<4>(CreateRotate(theta, CVectorD<3>(0.0, 0.0, 1.0)));

	// steps through the border
	const double step = (PI / 2) / (NUM_STEPS - 1);

	// step from angle = 0 - PI/2
	double angle = 0;
	for (int nAt = 0; nAt < NUM_STEPS; nAt++)
	{
		// create the vertex point
		m_arrVertices[nAt] = radius *
			FromHomogeneous<3, double>(mRotate * 
				CVectorD<4>(cos(angle), 0.0, sin(angle), 1.0));

		// offset into position
		m_arrVertices[nAt] += vOffset;

		// create the normal
		m_arrNormals[nAt] = 
			FromHomogeneous<3, double>(mRotate *
				CVectorD<4>(cos(angle), 0.0, sin(angle), 1.0));

		// next angle step
		angle += step;
	}
}


CMolding *CMolding::RenderMoldingSection(LPDIRECT3DDEVICE2 lpD3DDev, 
		double angle_start, double angle_stop,
		CVectorD<3> (*Eval)(double, double*), double *p, 
		CMolding *pPrevMolding)
{
	// step size for the molding section
	double step = (angle_stop - angle_start) / NUM_STEPS;

	// step through the angles
	for (double theta = angle_start; theta <= angle_stop; theta += step)
	{
		// create the molding from the template
		CVectorD<3> vShapePt = Eval(theta, p);

		// create a new molding object
		CMolding *pNewMolding = new CMolding(theta, vShapePt, 
			pPrevMolding->m_borderRadius);

		for (int nAt = 0; nAt < NUM_STEPS-1; nAt++)
		{
			ASSERT_HRESULT(lpD3DDev->Begin(D3DPT_TRIANGLESTRIP, 
				D3DVT_VERTEX, D3DDP_DONOTCLIP));

			lpD3DDev->Vertex((LPVOID) 
				&FromVectors(pPrevMolding->m_arrVertices[nAt],
					pPrevMolding->m_arrNormals[nAt]));

			lpD3DDev->Vertex((LPVOID) 
				&FromVectors(pPrevMolding->m_arrVertices[nAt+1],
					pPrevMolding->m_arrNormals[nAt+1]));

			lpD3DDev->Vertex((LPVOID) 
				&FromVectors(pNewMolding->m_arrVertices[nAt],
					pNewMolding->m_arrNormals[nAt]));

			lpD3DDev->Vertex((LPVOID) 
				&FromVectors(pNewMolding->m_arrVertices[nAt+1],
					pNewMolding->m_arrNormals[nAt+1]));

			ASSERT_HRESULT(lpD3DDev->End(0));
		}

		// draw the "pie-wedge" to fill the interior
		ASSERT_HRESULT(lpD3DDev->Begin(D3DPT_TRIANGLELIST, 
			D3DVT_VERTEX, D3DDP_DONOTCLIP));

		lpD3DDev->Vertex((LPVOID) 
			&FromVectors(pPrevMolding->m_arrVertices[NUM_STEPS-1],
				pPrevMolding->m_arrNormals[NUM_STEPS-1]));

		lpD3DDev->Vertex((LPVOID) 
			&FromVectors(pNewMolding->m_arrVertices[NUM_STEPS-1],
				pNewMolding->m_arrNormals[NUM_STEPS-1]));

		lpD3DDev->Vertex((LPVOID) 
			&FromVectors(CVectorD<3>(0.0, 0.0, 0.0),
				pNewMolding->m_arrNormals[NUM_STEPS-1]));

		ASSERT_HRESULT(lpD3DDev->End(0));

		delete pPrevMolding;
		pPrevMolding = pNewMolding;
	}

	return pPrevMolding;
}

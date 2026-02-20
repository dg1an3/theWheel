//////////////////////////////////////////////////////////////////////
// Molding.cpp: implementation of the CMolding class.
//
// Copyright (C) 1996-2003 Derek G Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// header file
#include "Molding.h"

// DirectDraw utilities
#include "DDUTIL.H"

// matrix stuff
#include <MatrixD.h>
// #include <MatrixNxM.inl>


//////////////////////////////////////////////////////////////////////
// FromVectors
// 
// helper function to construct a D3DVERTEX from CVectors
//////////////////////////////////////////////////////////////////////
D3DVERTEX& FromVectors(CVectorD<3, double> vVert, CVectorD<3, double> vNorm)
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

}	// FromVectors


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// CMolding::CMolding
// 
// constructs a new CMolding object
//////////////////////////////////////////////////////////////////////
CMolding::CMolding(double theta, CVectorD<3, double> vOffset, double radius)
	: m_borderRadius(radius)
{
	// create the rotation matrix for this molding fragment
	// CMatrixD<4, double> mRotate = 
	//	CMatrixD<4, double>(CreateRotate<double>(theta, CVectorD<3, double>(0.0, 0.0, 1.0)));
	CMatrixD<3, double> mRotate_sub = CreateRotate<double>(theta, CVectorD<3, double>(0.0, 0.0, 1.0));
	CMatrixD<4, double> mRotate;
	mRotate[0][0] = mRotate_sub[0][0];
	mRotate[0][1] = mRotate_sub[0][1];
	mRotate[0][2] = mRotate_sub[0][2];
	mRotate[1][0] = mRotate_sub[1][0];
	mRotate[1][1] = mRotate_sub[1][1];
	mRotate[1][2] = mRotate_sub[1][2];
	mRotate[2][0] = mRotate_sub[2][0];
	mRotate[2][1] = mRotate_sub[2][1];
	mRotate[2][2] = mRotate_sub[2][2];


	// steps through the border
	const double step = (PI / 2) / (NUM_STEPS - 1);

	// step from angle = 0 - PI/2
	double angle = 0;
	for (int nAt = 0; nAt < NUM_STEPS; nAt++)
	{
		// create the vertex point
		m_arrVertices[nAt] = radius *
			FromHomogeneous<3, double>(mRotate * 
				CVectorD<4, double>(cos(angle), 0.0, sin(angle), 1.0));

		// offset into position
		m_arrVertices[nAt] += vOffset;

		// create the normal
		m_arrNormals[nAt] = 
			FromHomogeneous<3, double>(mRotate *
				CVectorD<4, double>(cos(angle), 0.0, sin(angle), 1.0));

		// next angle step
		angle += step;
	}

}	// CMolding::CMolding


//////////////////////////////////////////////////////////////////////
// CMolding::RenderMoldingSection
// 
// renders the molding segment on a D3D device
//////////////////////////////////////////////////////////////////////
CMolding *CMolding::RenderMoldingSection(LPDIRECT3DDEVICE2 lpD3DDev, 
		double angle_start, 
		double angle_stop,
		CVectorD<3, double> (*Eval)(double, double*),		// evaluation function
		double *p,											// parameters for eval
		CMolding *pPrevMolding)
{
	// step size for the molding section
	double step = (angle_stop - angle_start) / NUM_STEPS;

	// step through the angles
	for (double theta = angle_start; theta <= angle_stop; theta += step)
	{
		// create the molding from the template
		CVectorD<3, double> vShapePt = Eval(theta, p);

		// create a new molding object
		CMolding *pNewMolding = new CMolding(theta, vShapePt, 
			pPrevMolding->m_borderRadius);

		// render facets on molding
		for (int nAt = 0; nAt < NUM_STEPS-1; nAt++)
		{
			// start rendering
			ASSERT_HRESULT(lpD3DDev->Begin(D3DPT_TRIANGLESTRIP, 
				D3DVT_VERTEX, D3DDP_DONOTCLIP));

			// render triangles
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

			// end rendering
			ASSERT_HRESULT(lpD3DDev->End(0));
		}

		// draw the "pie-wedge" to fill the interior

		// start rendering
		ASSERT_HRESULT(lpD3DDev->Begin(D3DPT_TRIANGLELIST, 
			D3DVT_VERTEX, D3DDP_DONOTCLIP));

		// render triangles
		lpD3DDev->Vertex((LPVOID) 
			&FromVectors(pPrevMolding->m_arrVertices[NUM_STEPS-1],
				pPrevMolding->m_arrNormals[NUM_STEPS-1]));

		lpD3DDev->Vertex((LPVOID) 
			&FromVectors(pNewMolding->m_arrVertices[NUM_STEPS-1],
				pNewMolding->m_arrNormals[NUM_STEPS-1]));

		lpD3DDev->Vertex((LPVOID) 
			&FromVectors(CVectorD<3, double>(0.0, 0.0, 0.0),
				pNewMolding->m_arrNormals[NUM_STEPS-1]));

		// end rendering
		ASSERT_HRESULT(lpD3DDev->End(0));

		// delete the previous molding
		delete pPrevMolding;

		// store the next molding section
		pPrevMolding = pNewMolding;
	}

	// return the next molding section
	return pPrevMolding;

}	// CMolding::RenderMoldingSection


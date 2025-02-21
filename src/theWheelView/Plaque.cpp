/// <remarks>
/// Plaque.cpp
/// Implementation of the Plaque class.
///
/// Copyright (C) 1996-2007 Derek G Lane
/// </remarks>

#include "StdAfx.h"

// header file
#include "Plaque.h"

namespace theWheel2007 {

//////////////////////////////////////////////////////////////////////////////
Plaque::Plaque(LPDIRECT3DDEVICE9 pDevice, RadialShape *pShape, REAL radius)
	: m_pDevice(pDevice)
	, m_borderRadius(radius)
	, m_nNumSteps(8)
	, m_nVertPerSection(0)
	, m_nFacesPerSection(0)
	, m_pTriStripVB(NULL)
	, m_pTriStripIB(NULL)
	, m_pTriFanVB(NULL)
{
	m_pShape = pShape;

}	// Plaque::Plaque


//////////////////////////////////////////////////////////////////////////////
void
	Plaque::Render()
		// renders to the device
{
	ASSERT_HRESULT(m_pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL));

	// make sure both buffers are formed
	GetVertexBuffer();
	GetIndexBuffer();

	// set up a material
	D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
	mtrl.Diffuse.r = mtrl.Ambient.r = 
		0.8f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 
		0.8f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 
		0.9f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	m_pDevice->SetMaterial( &mtrl );

	// set up stream source
	ASSERT_HRESULT(m_pDevice->SetStreamSource(0, m_pTriStripVB, 0, sizeof(NormalVertex)));
	ASSERT_HRESULT(m_pDevice->SetIndices(m_pTriStripIB));

	for (auto nAt = 0U; nAt < m_arrSectionStartVB.size()-1; nAt++)
	{
		//// and draw to the device
		ASSERT_HRESULT(m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 
			m_arrSectionStartVB[nAt],	// base vertex index
			0,							// min vertex index
			2 * m_nVertPerSection,		// num vertices used
			0,							// first index
			m_nFacesPerSection			// number of primitives
			));
	}  

	GetVertexBufferFan();

	// D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
	mtrl.Diffuse.r = mtrl.Ambient.r = 0.7f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 0.4f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 0.4f;
	m_pDevice->SetMaterial( &mtrl );

	// set up stream source
	ASSERT_HRESULT(m_pDevice->SetStreamSource(0, m_pTriFanVB, 0, sizeof(NormalVertex)));
	ASSERT_HRESULT(m_pDevice->SetIndices(NULL));

	//// and draw to the device
	ASSERT_HRESULT(m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 
		0,							// start vertex 
		m_nNumFan					// number of primitives
		));

}	// Plaque::RenderMolding


//////////////////////////////////////////////////////////////////////
LPDIRECT3DVERTEXBUFFER9	
	Plaque::GetVertexBuffer()
	// creates the vertex buffer from a temp array
{
	if (!m_pTriStripVB)
	{
		// calculate the vertices for the section
		std::vector<NormalVertex> arrVerts;
		CalcSections(arrVerts);

		// create a D3D vertex buffer
		ASSERT_HRESULT(m_pDevice->CreateVertexBuffer(sizeof(NormalVertex) * arrVerts.size(), 
			D3DUSAGE_WRITEONLY, 
			D3DFVF_XYZ | D3DFVF_NORMAL, 
			D3DPOOL_DEFAULT,
			&m_pTriStripVB, NULL));

		// write vertices to the VB using graphics stream object
		NormalVertex *pVecs;
		ASSERT_HRESULT(m_pTriStripVB->Lock(0, 0, (void**)&pVecs, 0));
		CopyMemory(pVecs, &arrVerts[0], sizeof(NormalVertex) * arrVerts.size());
		m_pTriStripVB->Unlock(); 
	}

	return m_pTriStripVB;

}	// Plaque::GetVertexBuffer


//////////////////////////////////////////////////////////////////////
void
	Plaque::CalcSections(std::vector<NormalVertex>& arrVerts)
{
	// track the shape discontinuities
	std::vector<REAL>& arrDiscont = m_pShape->Discontinuities();
	auto nNextDiscont = 0U;

	// step size for the four quadrants of the molding section
	const REAL step = 2.0f * PIf / REAL(m_nNumSteps * 4.0f);

	// step through the angles
	REAL theta = 0.0f; 
	for (auto nSlice = 0U; nSlice < (size_t)m_nNumSteps * 4; nSlice++)
	{
		// are we at the last step?
		if (nSlice + 1 == m_nNumSteps * 4)
		{
			// make sure we close where we started
			theta = 0.0f;
		}

		// evaluate the next shape point
		CVectorD<2, REAL> vPt = m_pShape->Eval(theta);

		// add index of next section
		m_arrSectionStartVB.push_back(arrVerts.size());

		// calculate vertices for new section
		CalcVertsForSection(theta, D3DXVECTOR3(vPt[0], vPt[1], 0.0), arrVerts); 

		// next angle
		theta += step;

		// have we passed the next discontinuity?
		if (nNextDiscont < arrDiscont.size()
			&& theta > arrDiscont[nNextDiscont])
		{
			theta = arrDiscont[nNextDiscont++];

			// add an extra slice for the discont
			nSlice--;
		}
	}

}	// Plaque::CalcSections


//////////////////////////////////////////////////////////////////////
void
	Plaque::CalcVertsForSection(REAL theta, const D3DXVECTOR3& vOffset, 
		std::vector<NormalVertex>& arrVerts)
		// calculates the new section of vertices
		//		returns index to first in new section
{
	// store the current starting index, to return
	int nOrigLength = arrVerts.size();

	// create the rotation matrix for this molding fragment
	D3DXMATRIX mRotate;
	D3DXMatrixRotationZ(&mRotate, theta);

	// steps through the border
	const REAL step = (PIf / 2.0f) / REAL(m_nNumSteps - 1.0f);

	// holds the new vertex
	NormalVertex vert;

	// step from angle = 0 - PI/2
	REAL angle = 0;
	for (int nAt = 0; nAt < m_nNumSteps; nAt++)
	{
		// create the vertex point
		vert.Position = D3DXVECTOR3(cos(angle), 0.0, sin(angle));

		// set up the normal
		vert.Normal = vert.Position;
		D3DXVec3TransformNormal(&vert.Normal, &vert.Normal, &mRotate);

		// set up the position
		D3DXVec3TransformCoord(&vert.Position, &vert.Position, &mRotate);
		vert.Position *= m_borderRadius;

		// offset into position (on the edge of the RadialShape)
		vert.Position += vOffset;

		// store the new vertex
		arrVerts.push_back(vert);

		// next angle step
		angle += step;
	}

	// stores pie-wedge tip
	vert.Position = D3DXVECTOR3(0.0, 0.0, m_borderRadius);
	arrVerts.push_back(vert);

	// store the vertices calculated per section (should be the same for all sections)
	m_nVertPerSection = arrVerts.size() - nOrigLength;

}	// Plaque::CalcVertsForSection


//////////////////////////////////////////////////////////////////////
LPDIRECT3DINDEXBUFFER9
	Plaque::GetIndexBuffer()
{
	if (!m_pTriStripIB)
	{
		// array of vertices arranged as triangle strips
		std::vector<short> arrTriVertIndices;
		
		// populate faces on molding
		for (int nAt = 0; nAt < m_nNumSteps-1; nAt++)
		{
			arrTriVertIndices.push_back(nAt); 
			arrTriVertIndices.push_back(m_nVertPerSection + nAt); 
			arrTriVertIndices.push_back(nAt+1);
			arrTriVertIndices.push_back(m_nVertPerSection + nAt+1);
		}

		// draw the "pie-wedge" to fill the interior
		arrTriVertIndices.push_back(m_nNumSteps-1); 
		arrTriVertIndices.push_back(m_nVertPerSection + m_nNumSteps-1); 

		// always is location of origin vertex (for tip of pie-wedge)
		arrTriVertIndices.push_back(m_nVertPerSection + m_nNumSteps); 

		// store the number of faces per section 
		//    (for triangle strip, 2 less than vertex count)
		m_nFacesPerSection = arrTriVertIndices.size() - 2;

		// now set up the index buffer
		ASSERT_HRESULT(m_pDevice->CreateIndexBuffer(sizeof(short) * arrTriVertIndices.size(), 
			D3DUSAGE_WRITEONLY, 
			D3DFMT_INDEX16, 
			D3DPOOL_DEFAULT,
			&m_pTriStripIB, NULL));

		// write the index values to the index buffer using graphics stream
		short *pIndex;
		ASSERT_HRESULT(m_pTriStripIB->Lock(0, 0, (void**)&pIndex, 0));
		CopyMemory(pIndex, &arrTriVertIndices[0], sizeof(short) * arrTriVertIndices.size());
		m_pTriStripIB->Unlock();
	}

	return m_pTriStripIB;

}	// Plaque::GetIndexBuffer

LPDIRECT3DVERTEXBUFFER9 Plaque::GetVertexBufferFan()
{
	if (!m_pTriFanVB)
	{
		std::vector<REAL>& arrDiscont = m_pShape->Discontinuities();
		auto nNextDiscont = 0U;
	
		// calculate the vertices for the section
		std::vector<NormalVertex> arrVerts;

		REAL Y = // 0.05; // 
			m_pShape->InnerRect.GetMin(1) + (m_pShape->InnerRect.GetSize(1)) * 0.75f; //  + 0.5f; // .Height / 8.0f;
		REAL angleStart = 0.0f, angleEnd = PIf;
		m_pShape->InvEvalY(Y, angleStart, angleEnd);

		// holds the new vertex
		NormalVertex vert; // TODO: make sure this is initialized
		vert.Position = D3DXVECTOR3(0.0, Y, m_borderRadius + 0.1f);
		vert.Normal = D3DXVECTOR3(0.0, 0.0, 1.0);
		arrVerts.push_back(vert); 
/*
		vert.Position = D3DXVECTOR3(-1.0, Y, m_borderRadius - 0.1);
		arrVerts.push_back(vert); 
		vert.Position = D3DXVECTOR3(-1.0, Y*4.0, m_borderRadius - 0.1);
		arrVerts.push_back(vert); 
		vert.Position = D3DXVECTOR3(1.0, Y*4.0, m_borderRadius - 0.1);
		arrVerts.push_back(vert); 
		vert.Position = D3DXVECTOR3(1.0, Y, m_borderRadius - 0.1);
		arrVerts.push_back(vert); */

		REAL angleDiff = angleEnd - angleStart;
		for (int nN = 0; nN <= m_nNumSteps * 4; nN++)
		{
			// evaluate the next shape point
			CVectorD<2, REAL> vPt = m_pShape->Eval(angleStart);
			vert.Position = D3DXVECTOR3(vPt[0], vPt[1], m_borderRadius - 0.1f);
			arrVerts.push_back(vert);

			angleStart += angleDiff / (REAL) (m_nNumSteps * 4);

			if (nNextDiscont < arrDiscont.size()
				&& angleStart > arrDiscont[nNextDiscont])
			{
				CVectorD<2, REAL> vPt = m_pShape->Eval(arrDiscont[nNextDiscont++]);
				vert.Position = D3DXVECTOR3(vPt[0], vPt[1], m_borderRadius - 0.1f);
				arrVerts.push_back(vert);
			}
		}

		// create a D3D vertex buffer
		ASSERT_HRESULT(m_pDevice->CreateVertexBuffer(sizeof(NormalVertex) * arrVerts.size(), 
			D3DUSAGE_WRITEONLY, 
			D3DFVF_XYZ | D3DFVF_NORMAL, 
			D3DPOOL_DEFAULT,
			&m_pTriFanVB, NULL));

		// write vertices to the VB using graphics stream object
		NormalVertex *pVecs;
		ASSERT_HRESULT(m_pTriFanVB->Lock(0, 0, (void**)&pVecs, 0));
		CopyMemory(pVecs, &arrVerts[0], sizeof(NormalVertex) * arrVerts.size());
		m_pTriFanVB->Unlock(); 

		m_nNumFan = arrVerts.size() - 2;
	}

	return m_pTriFanVB;
}

}	// namespace theWheel2007

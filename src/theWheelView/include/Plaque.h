/// <remarks>
/// Plaque.h
/// Interface of the Plaque class.
///
/// Copyright (C) 1996-2007 Derek G Lane
/// </remarks>

#pragma once

// stores the radial shape for the plaque
#include "RadialShape.h"

namespace theWheel2007 {

// using namespace System::Collections;
// using namespace System::Collections::Generic;
// using namespace Microsoft::DirectX;
// using namespace Microsoft::DirectX::Direct3D;

/// <summary>
/// helper class to render 3D Plaque shapes based on a RadialShape
/// </summary>
class Plaque 
{
public:	
	/// <summary>
	/// constructs a new plaque for the given shape
	/// </summary>
	/// <param name="d3dDev">the device to use for rendering</param>
	/// <param name="shape">the shape to render</param>
	/// <param name="radius">the bevel radius</param>
	Plaque(LPDIRECT3DDEVICE9 pDevice, RadialShape *pShape, REAL radius);

	/// <summary>
	/// the associated radial shape for the plaque
	/// </summary>
	RadialShape *GetShape()
	{
		return m_pShape;
	}

	/// <summary>
	// renders to the device
	/// </summary>
	void Render();

	/// destroys the plaque
	virtual ~Plaque()
	{
		if (m_pShape)
			delete m_pShape;

		if (m_pTriStripVB != NULL)
			m_pTriStripVB->Release();

		if (m_pTriStripIB != NULL)
			m_pTriStripIB->Release();

		if (m_pTriFanVB != NULL)
			m_pTriFanVB->Release();
	}

protected:
	/// creates the vertex buffer
	LPDIRECT3DVERTEXBUFFER9 GetVertexBuffer();

	// structure for vertices
	struct NormalVertex
	{
		D3DXVECTOR3 Position;
		D3DXVECTOR3 Normal;
	};

	// helper to calculate the molding sections
	void CalcSections(std::vector<NormalVertex>& arrVerts);

	// calculates the new section of vertices; returns index to first in new section
	void CalcVertsForSection(REAL theta, const D3DXVECTOR3& vOffset, 
		std::vector<NormalVertex>& arrVerts);

	// creates the section index (only one is needed for all sections
	LPDIRECT3DINDEXBUFFER9 GetIndexBuffer();

	LPDIRECT3DVERTEXBUFFER9 GetVertexBufferFan();

private:
	// stores the device pointer
	LPDIRECT3DDEVICE9 m_pDevice;

	// the shape for the plaque
	RadialShape *m_pShape;

	// molding radius
	float m_borderRadius;

	// number of steps for forming the mesh
	int m_nNumSteps;

	// index of start of each section in the vertex buffer
	std::vector<int> m_arrSectionStartVB;

	// count vertices per section
	int m_nVertPerSection;

	// count faces per section
	int m_nFacesPerSection;

	// the D3D vertex and index buffers for the plaque
	LPDIRECT3DVERTEXBUFFER9 m_pTriStripVB;
	LPDIRECT3DINDEXBUFFER9 m_pTriStripIB;

	// vertex buffer for fan
	LPDIRECT3DVERTEXBUFFER9 m_pTriFanVB;
	int m_nNumFan;

};	// class Plaque

}	// namespace 

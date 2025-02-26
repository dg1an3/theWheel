### NodeViewSkin
has collection of plaques, based on different sizes for the node views.

```c++
class NodeViewSkin
{
public:
	/// <summary> constructs a new skin for the device </summary>
	NodeViewSkin(LPDIRECT3DDEVICE9 pDevice)
		: m_pDevice(pDevice) { }

	/// <summary> renders the node view skin at the given height </summary>
	void Render(CNodeView *pNV);

	/// <summary> calculates the size for a given activation </summary>
	void CalcRectForActivation(REAL activation, CExtent<2, REAL>& rect);

	virtual ~NodeViewSkin(void)
	{
		for (auto plaque : m_arrPlaques)
			if (plaque)
				delete plaque;
	}

protected:
	/// <summary> retrieves (and possibly creates) the index of nearest plaque, given the activation </summary>
	int GetPlaqueIndex(REAL activation);

private:
	// stores the device
	LPDIRECT3DDEVICE9 m_pDevice;

	// array of moldings for the meshes
	std::vector<Plaque*> m_arrPlaques;

};	// class NodeViewSkin
```

```c++
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
void NodeViewSkin::CalcRectForActivation(REAL activation, CExtent<2, REAL>& extent)
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
int NodeViewSkin::GetPlaqueIndex(REAL activation)
{
	// form the index
	auto nIndex = Round<size_t>(activation * 200.0f); // 1000.0f);
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
void NodeViewSkin::Render(CNodeView *pNV)
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

	REAL sizeUp = 100.0f + 1200.0f * (activation) * (activation);
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
```

### Plaque
a plaque is a 3D shape that is based on a radial shape.  It is used to render the node view skin.

```c++
/// <summary> helper class to render 3D Plaque shapes based on a RadialShape </summary>
class Plaque 
{
public:	
	/// <summary> constructs a new plaque for the given shape </summary>
	/// <param name="d3dDev">the device to use for rendering</param>
	/// <param name="shape">the shape to render</param>
	/// <param name="radius">the bevel radius</param>
	Plaque(LPDIRECT3DDEVICE9 pDevice, RadialShape *pShape, REAL radius);

	/// <summary>
	/// the associated radial shape for the plaque
	/// </summary>
	RadialShape *GetShape() { return m_pShape; }

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
```

```c++

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
LPDIRECT3DINDEXBUFFER9 Plaque::GetIndexBuffer()
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
```

### Elliptangle
an elliptangle is a radial shape that is a combination of two ellipses.
```c++
/// <summary> represents an "elliptangle" radial shape </summary>
class Elliptangle : public RadialShape
{
public:
	/// <summary> construct a new elliptangle with given extent and elliptangle-ness </summary>
	/// <param name="extOuter">outer rectangle for the elliptangle</param>
	/// <param name="ellipt">elliptangle-ness</param>
	Elliptangle(CExtent<2, REAL> rectInner, REAL ellipt)
	{
		SetExtentEllipt(rectInner, ellipt);
	}

	// destructor
	virtual ~Elliptangle(void) { }

	/// <summary> sets the extents (both inner and outer) </summary>
	/// <param name="extOuter">outer rectangle for the elliptangle</param>
	/// <param name="ellipt">elliptangle-ness</param>
	void SetExtentEllipt(CExtent<2, REAL> rectInner, REAL ellipt);

	/// <summary cref="RadialShape::Eval"> evaluates the radial shape at the given point </summary>
	/// <param name="angle">angle at which to evaluate the radial shape</param>
	/// <returns>point on the elliptangle at the angle</returns>
	virtual CVectorD<2, REAL> Eval(REAL angle) override;

	/// <summary cref="RadialShape::InvEvalY"> evaluates the radial shape at the given point </summary>
	/// <param name="angle">angle at which to evaluate the radial shape</param>
	/// <returns>point on the elliptangle at the angle</returns>
	virtual void InvEvalY(REAL Y, REAL& angle1, REAL& angle2);

protected:

	// the "fat" ellipse extent
	CExtent<2, REAL> m_extFatEllipse;

	// the "tall" ellipse extent
	CExtent<2, REAL> m_extTallEllipse;

};	// class Elliptangle
```

```c++
/////////////////////////////////////////////////////////////////////////////
void Elliptangle::SetExtentEllipt(CExtent<2, REAL> rectInner, REAL ellipt)
		// sets the extents (both inner and outer)
{
	// set inner extent
	InnerRect = rectInner;

	// calculate outer extent
	rectInner.Deflate(
		-(1.0f - ellipt) * rectInner.GetSize(0) / 2.0f, 
		-(1.0f - ellipt) * rectInner.GetSize(1) / 2.0f,
		-(1.0f - ellipt) * rectInner.GetSize(0) / 2.0f, 
		-(1.0f - ellipt) * rectInner.GetSize(1) / 2.0f);
	OuterRect = rectInner;

	// now calculate the supporting extents

	// calculate the fat ellipse extent

	// compute the fat ellipse's b
	REAL ry = InnerRect.GetSize(1) / OuterRect.GetSize(1); 
	REAL by = sqrt(InnerRect.GetSize(0) * InnerRect.GetSize(0) * 0.25f
						/ (1.0f - ry * ry));

	// compute the fat ellipse's rectangle
	m_extFatEllipse = OuterRect;
	m_extFatEllipse.Deflate(m_extFatEllipse.GetSize(0) / 2.0f, 0, m_extFatEllipse.GetSize(0) / 2.0f, 0);
	m_extFatEllipse.Deflate(-by, 0, -by, 0);


	// calculate the tall ellipse extent

	// compute the tall ellipse's b
	REAL rx = InnerRect.GetSize(0) / OuterRect.GetSize(0); 
	REAL bx = sqrt(InnerRect.GetSize(1) * InnerRect.GetSize(1) * 0.25f
						/ (1.0f - rx * rx));

	// compute the tall ellipse's rectangle
	m_extTallEllipse = OuterRect;
	m_extTallEllipse.Deflate(0, m_extTallEllipse.GetSize(1) / 2.0f, 0, m_extTallEllipse.GetSize(1) / 2.0f);
	m_extTallEllipse.Deflate(0, -bx, 0, -bx);


	// now set up discont list

	// compute the angle dividing the sections of the elliptangle
	REAL sectionAngle = atan2(OuterRect.GetSize(1), OuterRect.GetSize(0));

	// determine whether angle is in fat or tall ellipse
	m_arrDiscont.resize(4); // = gcnew array<REAL>(4);
	m_arrDiscont[0] = sectionAngle;
	m_arrDiscont[1] = PIf - sectionAngle;
	m_arrDiscont[2] = PIf + sectionAngle;
	m_arrDiscont[3] = 2.0f * PIf - sectionAngle;

}	// Elliptangle::SetExtentEllipt


//////////////////////////////////////////////////////////////////////////
CVectorD<2, REAL> Elliptangle::Eval(REAL theta)
		// helper function to evaluate an elliptangle
{
	CVectorD<2, REAL> vPoint;

	// check theta range
	if (theta < 0.0f)
	{
		vPoint = Eval(theta + 2.0f * PIf);
	}
	else if (2.0f * PIf < theta)
	{
		vPoint = Eval(theta - 2.0f * PIf);
	}
	else
	{
		// compare tan(theta) to slope, to see which section we are in 
		REAL slope = OuterRect.GetSize(1) / OuterRect.GetSize(0);
		if (fabs(tan(theta)) >= fabs(slope))
		{
			// tangent greater than slope, so we are in fat ellipse domain
			vPoint[0] = m_extFatEllipse.GetSize(0) / 2.0f;
			vPoint[1] = m_extFatEllipse.GetSize(1) / 2.0f;
		}
		else
		{
			// tangent less than slope, so we are in tall ellipse domain
			vPoint[0] = m_extTallEllipse.GetSize(0) / 2.0f;
			vPoint[1] = m_extTallEllipse.GetSize(1) / 2.0f;
		}

		// compute theta_prime for the ellipse
		REAL sign = cos(theta) > 0 ? 1.0f : -1.0f;
		REAL theta_prime = atan2(sign * vPoint[0] * tan(theta), sign * vPoint[1]);

		vPoint[0] *= cos(theta_prime);
		vPoint[1] *= sin(theta_prime);
	}

	return vPoint;

}	// Elliptangle::Eval


//////////////////////////////////////////////////////////////////////////
void Elliptangle::InvEvalY(REAL Y, REAL& angle1, REAL& angle2)
{
	// compute point on tall ellipse
	REAL a = m_extTallEllipse.GetSize(0) / 2.0f;
	REAL b = m_extTallEllipse.GetSize(1) / 2.0f;
	REAL X = a * sqrt(1.0f - Y * Y / (b * b));

	// see if the point is greater than tall slope
	REAL slope = OuterRect.GetSize(1) / OuterRect.GetSize(0);
	if (fabs(Y / X) >= fabs(slope))
	{
		// yes, so compute for fat ellipse
		a = m_extFatEllipse.GetSize(0)  / 2.0f;
		b = m_extFatEllipse.GetSize(1) / 2.0f;
		X = a * sqrt(1.0f - Y * Y / (b * b));
	}

	// assign the values
	angle1 = atan2(Y, X);
	angle2 = atan2(Y, -X);
}

```

### RadialShape

```c++
/// <summary> base class for radial shapes (shapes which are functions of angle) </summary>
class RadialShape
{
public:
	/// <summary> constructs a new RadialShape </summer>
	RadialShape(void) { }

	// default destructor
	virtual ~RadialShape(void) { }

	// evaluates the radial shape at the given point
	virtual CVectorD<2, REAL> Eval(REAL angle) = 0;

	// inverse evaluation, given Y
	virtual void InvEvalY(REAL Y, REAL& angle1, REAL& angle2) = 0;

	/// <summary> collection of discontinuities for the shape </summer>
	std::vector<REAL>& Discontinuities() { return m_arrDiscont; }

	/// <summary> the outer rectangle for the shape </summary>
	CExtent<2, REAL> OuterRect;

	/// <summary> the inner rectangle for the shape </summer>
	CExtent<2, REAL> InnerRect;

protected:
	// stores the array of discontinuities
	std::vector<REAL> m_arrDiscont;

};	// class RadialShape
```

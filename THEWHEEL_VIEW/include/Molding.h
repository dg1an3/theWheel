// Molding.h: interface for the CMolding class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOLDING_H__439417B4_2D02_44BD_97DE_6AC3EDF482DE__INCLUDED_)
#define AFX_MOLDING_H__439417B4_2D02_44BD_97DE_6AC3EDF482DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// vector manipulation
#include <VectorD.h>

// Direct3D for rendering
#include <d3d.h>

const int NUM_STEPS = 8;

class CMolding  
{
public:	
	CMolding(double theta, CVectorD<3> vOffset, double radius);

	// renders a sequence of moldings along a constant angle
	//		path (a section
	static CMolding *RenderMoldingSection(LPDIRECT3DDEVICE2 lpD3DDev, 
		double angle_start, double angle_stop, 
		CVectorD<3> (*Eval)(double, double *), double *p,
		CMolding *pPrev);

private:
	// molding radius
	double m_borderRadius;

	// molding vertices and normals
	CVectorD<3> m_arrVertices[NUM_STEPS];
	CVectorD<3> m_arrNormals[NUM_STEPS];
};


#endif // !defined(AFX_MOLDING_H__439417B4_2D02_44BD_97DE_6AC3EDF482DE__INCLUDED_)

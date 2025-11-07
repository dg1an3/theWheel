//////////////////////////////////////////////////////////////////////
// Molding.h: interface for the CMolding class.
//
// Copyright (C) 1996-2003
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_MOLDING_H__439417B4_2D02_44BD_97DE_6AC3EDF482DE__INCLUDED_)
#define AFX_MOLDING_H__439417B4_2D02_44BD_97DE_6AC3EDF482DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// vector manipulation
#include <VectorD.h>

// Direct3D for rendering
#ifdef SKIN_RENDER_3D
#include <d3d.h>
#endif

// constant for number of steps from front plane to edge
const int NUM_STEPS = 8;

//////////////////////////////////////////////////////////////////////
// class CMolding
//
// helper class to render molding for nodes
//////////////////////////////////////////////////////////////////////
#ifdef SKIN_RENDER_3D
class CMolding  
{
public:	
	// constructor
	CMolding(double theta, CVectorD<3, double> vOffset, double radius);

	// renders a sequence of moldings along a constant angle
	//		path (a section
	static CMolding *RenderMoldingSection(LPDIRECT3DDEVICE2 lpD3DDev, 
		double angle_start, double angle_stop, 
		CVectorD<3, double> (*Eval)(double, double *), double *p,
		CMolding *pPrev);

private:
	// molding radius
	double m_borderRadius;

	// molding vertices and normals
	CVectorD<3, double> m_arrVertices[NUM_STEPS];
	CVectorD<3, double> m_arrNormals[NUM_STEPS];

};	// class CMolding
#endif // SKIN_RENDER_3D


#endif // !defined(AFX_MOLDING_H__439417B4_2D02_44BD_97DE_6AC3EDF482DE__INCLUDED_)

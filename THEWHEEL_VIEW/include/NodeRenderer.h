// NodeRenderer.h: interface for the CNodeRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODERENDERER_H__35348172_09C8_401B_A1D6_955CE7CE536D__INCLUDED_)
#define AFX_NODERENDERER_H__35348172_09C8_401B_A1D6_955CE7CE536D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// #include <Value.h>
// #include <Association.h>

#include <Dib.h>

#include <Rendererable.h>

class CMolding;

class CNodeRenderer : public COpenGLRenderer  
{
public:
	// constructor/destructor
	CNodeRenderer(COpenGLView *pView);
	virtual ~CNodeRenderer();

	// association to the node to be rendered
	CDib m_Image;

	// handler for activation change
	void SetActivation(double);

protected:
	// evaluates the elliptangle at a point
	CVector<3> EvalEllipseMolding(double theta, double a, double b);

	// assembles sections of the molding
	CMolding *RenderMoldingSection(double angle_start, double angle_stop, 
		double a, double b, CMolding *pOld = NULL);

	// draws the text and image for the node
	void DrawText(const CString& strText, CRect rect);
	// void DrawImage(const CRect& rect);

	// renders the whole node
	virtual void OnRenderScene();

private:
	// stores the current position of the node's center
	CVector<2> m_position;

	// stores the node's current activation value
	double m_activation;

	// stores the node's aspect ratio
	double m_aspectRatio;

	// stores the radius of the border
	double m_borderRadius;

	// computes the node's x-size (ellipse a)
	double m_width;

	// computes the node's rectangularity (from sqrt(2.0e) to 1.0))
	double m_rectangularity;
};

#endif // !defined(AFX_NODERENDERER_H__35348172_09C8_401B_A1D6_955CE7CE536D__INCLUDED_)

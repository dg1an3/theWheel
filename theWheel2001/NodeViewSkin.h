// NodeViewSkin.h: interface for the CNodeViewSkin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODEVIEWSKIN_H__D4502BC9_C346_4A52_BDC6_60153C909045__INCLUDED_)
#define AFX_NODEVIEWSKIN_H__D4502BC9_C346_4A52_BDC6_60153C909045__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Vector.h>

class CNodeView;

class CNodeViewSkin  
{
public:
	CNodeViewSkin();
	virtual ~CNodeViewSkin();

	// sets the client area for this skin
	void SetClientArea(int nWidth, int nHeight);

	// computes the inner and outer rectangles for a node view, 
	//		given an activation
	CRect CalcOuterRect(CNodeView *pNodeView);
	CRect CalcInnerRect(CNodeView *pNodeView);

	// computes the shape of a node view, given an activation
	void CalcShape(CNodeView *pNodeView, CRgn *pRgn, int nErode = 0);

// Operations
public:
	// draw the skin
	void DrawSkin(CDC *pDC, CNodeView *pNodeView);

	// draw a link
	void DrawLink(CDC *pDC, CVector<3>& vFrom, float actFrom,
		CVector<3>& vTo, float actTo);

// Implementation
protected:
	// accessors for the left-right and top-bottom ellipse rectangles
	CRect CalcTopBottomEllipseRect(CNodeView *pNodeView);
	CRect CalcLeftRightEllipseRect(CNodeView *pNodeView);

private:
	// stores the current client area
	int m_nWidth;
	int m_nHeight;
};

#endif // !defined(AFX_NODEVIEWSKIN_H__D4502BC9_C346_4A52_BDC6_60153C909045__INCLUDED_)

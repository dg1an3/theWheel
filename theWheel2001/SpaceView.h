//////////////////////////////////////////////////////////////////////
// SpaceView.h: interface for the CSpaceView class.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPACEVIEW_H__0C8AA65C_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_SPACEVIEW_H__0C8AA65C_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Space.h"
#include "NodeView.h"

#include "SpaceViewEnergyFunction.h"

#ifdef USE_GRAD
#include "ConjGradOptimizer.h"
#else
#include "PowellOptimizer.h"
#endif

//////////////////////////////////////////////////////////////////////
// class CSpaceView
//
// manages the dynamic layout of the space; contains a node view
//		for each node in the spcce
//////////////////////////////////////////////////////////////////////
class CSpaceView : public CView
{
protected: // create from serialization only
	CSpaceView();
	virtual ~CSpaceView();

	DECLARE_DYNCREATE(CSpaceView)

// Attributes
public:
	// returns the CSpace that is being displayed
	CSpace* GetDocument();

	// boolean variable to indicate that propagation is to occur
	CValue<BOOL> isPropagating;

	// flag to indicate we are in wave mode
	CValue<BOOL> isWaveMode;

	// the child node views
	CCollection<CNodeView> nodeViews;

	// finds the node view for a particular node
	CNodeView *GetViewForNode(CNode *pNode);

// Operations
public:
	// adds a new node to the space, creating and initializing the node view
	//		along the way
	void AddNodeToSpace(CNode *pNewNode);

	// creates the node views for the children of the passed node
	void CreateNodeViews(CNode *pParentNode, CPoint pt);

	// sort the node views
	void SortNodeViews();

	// centering all child views based on center-of-gravity
	void CenterNodeViews();

	// layout for all child views
	void LayoutNodeViews();

	// activates a particular node by a particular scale factor
	void ActivateNode(CNodeView *pNodeView, float scale);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpaceView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	// the type for the state vector
	typedef SPV_STATE_TYPE STATE_TYPE;

	// the number of dimensions in the state vector
	enum { STATE_DIM = SPV_STATE_DIM };

	// defines the type for the state vector
	typedef CVector<STATE_DIM, STATE_TYPE> CStateVector;

	// gets and sets the state vector for this CSpaceView
	CStateVector GetStateVector();
	void SetStateVector(const CStateVector& vState);

	// pointers to recently selected node views
	CNodeView *m_pRecentClick1;
	CNodeView *m_pRecentClick2;

	// back buffer for drawing
	CDC m_dcMem;
	CBitmap m_bitmapBuffer;
	CBitmap *m_pOldBitmap;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CSpaceView)
	afx_msg void OnViewLayout();
	afx_msg void OnViewPropagate();
	afx_msg void OnUpdateViewPropagate(CCmdUI* pCmdUI);
	afx_msg void OnViewWave();
	afx_msg void OnUpdateViewWave(CCmdUI* pCmdUI);
	afx_msg void OnNewNode();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// the optimizer used to lay out the child views
	COptimizer<SPV_STATE_DIM, SPV_STATE_TYPE> *m_pOptimizer;

	// the energy function that the optimizer uses as an objective function
	CSpaceViewEnergyFunction *m_pEnergyFunc;
};

#ifndef _DEBUG  // debug version in SpaceView.cpp
inline CSpace* CSpaceView::GetDocument()
   { return (CSpace*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPACEVIEW_H__0C8AA65C_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)

// SpaceView.h : interface of the CSpaceView class
//
/////////////////////////////////////////////////////////////////////////////

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

class CSpaceView : public CView
{
protected: // create from serialization only
	CSpaceView();
	DECLARE_DYNCREATE(CSpaceView)

// Attributes
public:
	CSpace* GetDocument();

	// boolean variable to indicate that propagation is to occur
	CValue<BOOL> isPropagating;

	// flag to indicate we are in wave mode
	CValue<BOOL> isWaveMode;

	// the child node views
	CCollection<CNodeView> nodeViews;
	CNodeView *GetViewForNode(CNode *pNode);

// Operations
public:
	// adds a new node to the space, creating and initializing the node view
	//		along the way
	void AddNodeToSpace(CNode *pNewNode);

	// layout for all child views
	void LayoutNodeViews();

	// normalization
	void NormalizeNodeViews();

	// propagates the activation for a particular node view
	void PropagateActivation(CNodeView *pFrom, float percent, float factor = 1.3f);

	// resets the propagation flag after a propagation
	void ResetForPropagation();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpaceView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSpaceView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// creates the node views for the children of the passed node
	void CreateNodeViews(CNode *pParentNode, CPoint pt, float initActivation);

	// centering all child views based on center-of-gravity
	void CenterNodeViews();

// Generated message map functions
protected:
	//{{AFX_MSG(CSpaceView)
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnNewNode();
	afx_msg void OnViewLayout();
	afx_msg void OnViewPropagate();
	afx_msg void OnUpdateViewPropagate(CCmdUI* pCmdUI);
	afx_msg void OnViewWave();
	afx_msg void OnUpdateViewWave(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	DECLARE_MESSAGE_MAP()

private:
	// the optimizer used to lay out the child views
	COptimizer<SPV_STATE_DIM, double> *m_pOptimizer;

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

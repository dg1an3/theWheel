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

#include <ddraw.h>
#include "ddutil.h"

#include "Space.h"
#include "NodeView.h"
#include "NodeViewSkin.h"


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

	// accessors for the child node views
	int GetNodeViewCount();
	CNodeView *GetNodeView(int nAt);

	// returns the number of currently visible nodes
	int GetVisibleNodeCount();

	// locates the topmost node view containing the specified point
	CNodeView *FindNodeViewAt(CPoint pt);

	// pointer to the connected browser (for navigating to a link);
	// CHtmlView *m_pBrowser;

// Operations
public:
	// activates a particular node by a particular scale factor
	void ActivateNodeView(CNodeView *pNodeView, REAL scale);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpaceView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	double GetSpringConst();
	void SetSpringConst(double springConst);
	// creates the node views for the children of the passed node
	void CreateNodeViews(CNode *pParentNode, CPoint pt);

	// sort the node views
	void SortNodeViews();

	// centering all child views based on center-of-gravity
	void CenterNodeViews();

	// initializes the direct draw surfaces
	BOOL InitDDraw();

	// processes a mouse wave activation
	void WaveActivate(CPoint pt);

	double GetSaccadeFactor(int nIndex);
	CVector<2> GetVelocity(int nIndex);
	CVector<2> GetAvgVelocity(int nIndex);
	CVector<2> GetAccel(int nIndex);
	CVector<2> GetAvgAccel(int nIndex);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CSpaceView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnNewNode();
	afx_msg void OnNewEevorg();
	afx_msg void OnEditOptions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// the child node views
	CObArray m_arrNodeViews;

	// spring constant for node views
	double m_springConst;

	// the skin for the node views
	CNodeViewSkin m_skin;

	// back buffer for drawing
	LPDIRECTDRAW		m_lpDD;			// DirectDraw object
	LPDIRECTDRAWSURFACE	m_lpDDSPrimary;	// DirectDraw primary surface
	LPDIRECTDRAWSURFACE	m_lpDDSOne;		// Offscreen surface 1
	LPDIRECTDRAWSURFACE	m_lpDDSBitmap;	// Offscreen surface 1
	LPDIRECTDRAWCLIPPER m_lpClipper;	// clipper for primary

	// holds the timer ID
	UINT m_nTimerID;

	// sync object for thread synchronization
	CCriticalSection m_sync;

	// flag to indicate we are in wave mode
	BOOL m_bWaveMode;

	// pointers to recently selected node views
	CNodeView *m_pRecentClick[2];

	// array of previous mouse movement points
	CPoint m_ptPrev;

	// arrays for saccade processing
	CArray<DWORD, DWORD> m_arrTimeStamps;
	CArray<CPoint, CPoint&> m_arrPoints;
};

#ifndef _DEBUG  // debug version in SpaceView.cpp
inline CSpace* CSpaceView::GetDocument()
   { return (CSpace*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPACEVIEW_H__0C8AA65C_F7A7_11D4_9E3E_00B0D0609AB0__INCLUDED_)

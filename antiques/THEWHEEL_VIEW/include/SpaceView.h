//////////////////////////////////////////////////////////////////////
// SpaceView.h: interface for the CSpaceView class.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(SPACEVIEW_H)
#define SPACEVIEW_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// the displayed model CSpace object
#include <Space.h>

// DirectDraw and utilities
#include <ddraw.h>
#include "ddutil.h"

// subordinate classes
#include "NodeView.h"
#include "NodeViewSkin.h"
#include "NodeLayoutManager.h"
#include "Tracker.h"


//////////////////////////////////////////////////////////////////////
// class CSpaceView
//
// manages the dynamic layout of the space; contains a node view
//		for each node in the spcce
//////////////////////////////////////////////////////////////////////
class CSpaceView : public CWnd
{
public:
	CSpaceView();
	virtual ~CSpaceView();

	// CSpaceView must be dynamically created
	DECLARE_DYNCREATE(CSpaceView)

// Attributes
public:
	// returns the CSpace that is being displayed
	CSpace* GetSpace();

	// accessors for the child node views
	int GetNodeViewCount();
	CNodeView *GetNodeView(int nAt);

	// returns the number of currently visible nodes
	int GetVisibleNodeCount();

	// locates the topmost node view containing the specified point
	CNodeView *FindNodeViewAt(CPoint pt);

	// locates the topmost node view containing the specified point
	CNodeView *FindNearestNodeView(CPoint pt);

	// finds a link, given a line segment that crosses the link
	BOOL FindLink(CPoint ptFrom, CPoint ptTo, 
		CNodeView **pLinkingView, CNodeView**pLinkedView);

	// set the tracker
	void SetTracker(CTracker *pTracker);

	// sets the background color for the space view
	void SetBkColor(COLORREF color);

	// initialize if a different module state is needed for context switch
	static AFX_MODULE_STATE *m_pModuleState;

// Operations
public:
	// activates a particular node by a particular scale factor
	void ActivateNodeView(CNodeView *pNodeView, REAL scale);

	CNodeView *m_pMaximizedView;

	// called when a new node is added to the space
	void OnNodeAdded(CObservableEvent * pEvent, void * pParam);

	// called when a new node is added to the space
	void OnNodeRemoved(CObservableEvent * pEvent, void * pParam);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpaceView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave();
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	afx_msg void OnGoClicked();
	virtual void SetSpace(CSpace *pSpace); // called first time after construct
	//}}AFX_VIRTUAL

// Implementation
public:
	// initializes the direct draw surfaces
	BOOL InitDDraw();

	// creates the node views for the children of the passed node
	void CreateNodeViews(CNode *pParentNode, CPoint pt);

	// sort the node views
	void SortNodeViews();

	// centering all child views based on center-of-gravity
	void CenterNodeViews();

	// activates pending
	void ActivatePending();

	// "Go" button
	CBitmapButton m_btnGo;

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
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	friend CNodeView;

	// TODO: make this pointed to by each CNodeView
	// the skin for the node views
	CNodeViewSkin m_skin;

	// stores reference to the top-level node layout manager
	CNodeLayoutManager *m_pNLM;

private:
	// space to be viewed
	CSpace *m_pSpace;

	// the child node views
	CObArray m_arrNodeViews;

	// the global UID for direct draw
	LPGUID m_lpGuid;

	// back buffer for drawing
	LPDIRECTDRAW		m_lpDD;			// DirectDraw object
	LPDIRECTDRAWSURFACE	m_lpDDSPrimary;	// DirectDraw primary surface
	LPDIRECTDRAWSURFACE	m_lpDDSOne;		// Offscreen surface 1
	LPDIRECTDRAWCLIPPER m_lpClipper;	// clipper for primary

	// the background color
	COLORREF m_colorBk;

	// holds the timer ID
	UINT m_nTimerID;

	// pointers to recently activated node views
	CNodeView *m_pRecentActivated[2];

	// the tracker object
	CTracker *m_pTracker;

	// flag to indicate mouse drag is occurring
	BOOL m_bDragging;

	// for drag-drop
	COleDropTarget m_dropTarget;

};	// class CSpaceView

inline CSpace* CSpaceView::GetSpace()
   { return m_pSpace; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(SPACEVIEW_H)

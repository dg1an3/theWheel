//////////////////////////////////////////////////////////////////////
// DesigntimeTracker.h: interface for the CDesigntimeTracker class.
//
// Copyright (C) 2002
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////


#if !defined(DesigntimeTracker_H)
#define DesigntimeTracker_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// base class
#include "Tracker.h"

// the node view for tracking links
#include "NodeView.h"

//////////////////////////////////////////////////////////////////////
// class CDesigntimeTracker
//
// responds to mouse events on the view
//////////////////////////////////////////////////////////////////////
class CDesigntimeTracker : public CTracker
{
public:
	// constructor/destructor
	CDesigntimeTracker(CSpaceView *pView);
	virtual ~CDesigntimeTracker();

	// handlers for button click events
	virtual void OnButtonDown(UINT nFlags, CPoint point);	
	virtual void OnButtonUp(UINT nFlags, CPoint point);

	// handlers for mouse move events
	virtual void OnMouseMove(UINT nFlags, CPoint point);
	virtual void OnMouseDrag(UINT nFlags, CPoint point);

	// drawing function for special annotations
	virtual void OnDraw(CDC *pDC);

protected:
	// pointer to the view being linked in a linking view
	CNodeView *m_pLinkingView;

	// flag to indicate cutting
	BOOL m_bCutting;

	// cut start point
	CPoint m_ptStart;
};

#endif // !defined(DesigntimeTracker_H)
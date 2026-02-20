//////////////////////////////////////////////////////////////////////
// Tracker.h: interface for the CTracker class.
//
// Copyright (C) 2003  Derek G Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////


#if !defined(TrackerSPV_H)
#define TrackerSPV_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// forward declaration of the view class
class CSpaceView;

//////////////////////////////////////////////////////////////////////
// class CTracker
//
// responds to mouse events on the view
//////////////////////////////////////////////////////////////////////
class CTracker : public CObject
{
public:
	// constructor/destructor
	CTracker(CSpaceView *pView);
	virtual ~CTracker();

	// handlers for button click events
	virtual void OnButtonDown(UINT nFlags, CPoint point);	
	virtual void OnButtonUp(UINT nFlags, CPoint point);
	virtual void OnButtonDblClk(UINT nFlags, CPoint point) { }

	// handlers for mouse move events
	virtual void OnMouseMove(UINT nFlags, CPoint point);
	virtual void OnMouseDrag(UINT nFlags, CPoint point);

	virtual void OnKeyDown(UINT nChar, UINT nFlags);

	// drawing function for special annotations
	virtual void OnDraw(CDC *pDC);

	// timer message
	virtual void OnTimer(UINT nElapsed, CPoint point);

protected:
	// pointer to the view
	CSpaceView *m_pView;

};	// class CTracker


#endif // !defined(TrackerSPV_H)
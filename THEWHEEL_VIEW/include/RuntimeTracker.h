//////////////////////////////////////////////////////////////////////
// RuntimeTracker.h: interface for the CRuntimeTracker class.
//
// Copyright (C) 2002
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////


#if !defined(RuntimeTracker_H)
#define RuntimeTracker_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// base class
#include "Tracker.h"

//////////////////////////////////////////////////////////////////////
// class CRuntimeTracker
//
// responds to mouse events on the view
//////////////////////////////////////////////////////////////////////
class CRuntimeTracker : public CTracker
{
public:
	// constructor/destructor
	CRuntimeTracker(CSpaceView *pView);
	virtual ~CRuntimeTracker();

	// handlers for button click events
	virtual void OnButtonDown(UINT nFlags, CPoint point);	

	// handlers for mouse move events
	virtual void OnMouseMove(UINT nFlags, CPoint point);

	virtual void OnKeyDown(UINT nChar, UINT nFlags);

	// timer message
	virtual void OnTimer(UINT nElapsed, CPoint point);
};

#endif // !defined(RuntimeTracker_H)
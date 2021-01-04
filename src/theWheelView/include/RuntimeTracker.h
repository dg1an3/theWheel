//////////////////////////////////////////////////////////////////////
// RuntimeTracker.h: interface for the CRuntimeTracker class.
//
// Copyright (C) 2002
// $Id: RuntimeTracker.h,v 1.3 2003/10/17 04:12:29 default Exp $
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
	virtual void OnButtonDblClick(UINT nFlags, CPoint point);	

	// handlers for mouse move events
	virtual void OnMouseMove(UINT nFlags, CPoint point);

	// timer message
	virtual void OnTimer(UINT nElapsed, CPoint point);

protected:
	// helper to browse to a particular page
	HRESULT OpenUrl(const CString& strUrl);

	// pointer to the browser 
	IWebBrowser2 * m_pBrowser;

};	// class CRuntimeTracker


#endif // !defined(RuntimeTracker_H)
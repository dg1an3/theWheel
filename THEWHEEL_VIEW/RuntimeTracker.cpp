// RuntimeTracker.cpp: implementation of the CRuntimeTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

// resource includes
#include "THEWHEEL_VIEW_resource.h"

#include "RuntimeTracker.h"

#include "SpaceView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRuntimeTracker::CRuntimeTracker(CSpaceView *pView)
: CTracker(pView)
{
}

CRuntimeTracker::~CRuntimeTracker()
{

}

void CRuntimeTracker::OnMouseMove(UINT nFlags, CPoint point)  
{ 
	// find the node view containing the point
	CNodeView *pSelectedView = m_pView->FindNodeViewAt(point);

	// initialize the pending activation for an on-node move
	double deltaAct = 0.10;

	// set the cursor based on whether the point is in a view
	if (pSelectedView != NULL) 
	{
		// set to a hand
		::SetCursor(::LoadCursor(GetModuleHandle(NULL), 
			MAKEINTRESOURCE(IDC_HANDPOINT)));
	}
	else
	{
		// find the nearest node view
		pSelectedView = m_pView->FindNearestNodeView(point);

		// set to lower activation
		deltaAct /= 2.0;

		// set to an arrow
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}

	// activate the node view
	if (pSelectedView != NULL)
	{
		pSelectedView->AddPendingActivation(deltaAct);
	}
}

void CRuntimeTracker::OnTimer(UINT nElapsed, CPoint point)
{
	CTracker::OnTimer(nElapsed, point);

	// find the node view containing the point
	CNodeView *pSelectedView = m_pView->FindNodeViewAt(point);
	if (NULL != pSelectedView)
	{
		pSelectedView->AddPendingActivation((REAL) 0.02);
	}
}
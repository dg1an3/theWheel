//////////////////////////////////////////////////////////////////////
// DesigntimeTracker.cpp: implementation of the CDesigntimeTracker class.
//
// Copyright (C) 2002
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// resource includes
#include "THEWHEEL_VIEW_resource.h"

// class declaration
#include "DesigntimeTracker.h"

// the space view
#include "SpaceView.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CDesigntimeTracker::CDesigntimeTracker
// 
// constructs a new CDesigntimeTracker object 
//////////////////////////////////////////////////////////////////////
CDesigntimeTracker::CDesigntimeTracker(CSpaceView *pView)
: CTracker(pView),
	m_pLinkingView(NULL),
	m_bCutting(FALSE)
{
}	// CDesigntimeTracker::CDesigntimeTracker


//////////////////////////////////////////////////////////////////////
// CDesigntimeTracker::~CDesigntimeTracker
// 
// destroys the CDesigntimeTracker object 
//////////////////////////////////////////////////////////////////////
CDesigntimeTracker::~CDesigntimeTracker()
{
}	// CDesigntimeTracker::~CDesigntimeTracker


//////////////////////////////////////////////////////////////////////
// CDesigntimeTracker::OnButtonDown
// 
// processes the design-time button down
//////////////////////////////////////////////////////////////////////
void CDesigntimeTracker::OnButtonDown(UINT nFlags, CPoint point) 
{ 
	// find the node view containing the point
	CNodeView *pSelectedView = m_pView->FindNodeViewAt(point);

	// was a node view found?
	if (pSelectedView != NULL)
	{
		// start a link
		m_pLinkingView = pSelectedView;
		m_ptStart = point 
			- (CPoint) (m_pLinkingView->GetSpringCenter()); // GetOuterRect().CenterPoint();
	}
	else
	{
		// start a cut
		m_bCutting = TRUE;
		m_ptStart = point;
	}

}	// CDesigntimeTracker::OnButtonDown


//////////////////////////////////////////////////////////////////////
// CDesigntimeTracker::OnButtonUp
// 
// processes the design-time button down
//////////////////////////////////////////////////////////////////////
void CDesigntimeTracker::OnButtonUp(UINT nFlags, CPoint point)   
{ 
	// stores one end of the link
	CNodeView *pLinkedView = NULL;

	// was a node view found?
	if (m_pLinkingView != NULL)
	{
		// find the node view containing ending point
		pLinkedView = m_pView->FindNodeViewAt(point);
		if (pLinkedView != NULL)
		{
			// see if a self-link is attempted
			if (pLinkedView == m_pLinkingView)
			{
				// if so, activate it
				m_pLinkingView->AddPendingActivation((REAL) 0.45);

				// set the new pending node
				CNode *pNode = m_pLinkingView->GetNode();
				pNode->GetSpace()->SetCurrentNode(pNode);
			}
			else
			{
				// increase the link weight
				double weight1 = 
					m_pLinkingView->GetNode()->GetLinkWeight(pLinkedView->GetNode());
				double weight2 = 
					pLinkedView->GetNode()->GetLinkWeight(m_pLinkingView->GetNode());

				// compute the new weight
				double newWeight = 1.2 * (weight1 + weight2) * 0.5 + 0.1;

				// set the new weight
				m_pLinkingView->GetNode()->LinkTo(
					pLinkedView->GetNode(), newWeight, TRUE);
			}
		}

		// restore cursor
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}
	else if (m_bCutting)
	{
		// try to find the two nodes being linked
		BOOL bFound = m_pView->FindLink(m_ptStart, point,
			&m_pLinkingView, &pLinkedView);

		if (bFound)
		{
			// increase the link weight
			double weight1 = 
				m_pLinkingView->GetNode()->GetLinkWeight(pLinkedView->GetNode());
			double weight2 = 
				pLinkedView->GetNode()->GetLinkWeight(m_pLinkingView->GetNode());

			// set the new weight
			double newWeight = 0.8 * (weight1 + weight2) * 0.5;
			m_pLinkingView->GetNode()->LinkTo(pLinkedView->GetNode(), newWeight, TRUE);
			pLinkedView->GetNode()->LinkTo(m_pLinkingView->GetNode(), newWeight, TRUE);
		}

		// stop cut
		m_bCutting = FALSE;

		// restore cursor
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}

	// if a linking view was found
	if (m_pLinkingView)
	{
		// notify views of the change in the node
		m_pView->GetDocument()->UpdateAllViews(m_pView, 0L, 
			m_pLinkingView->GetNode());

		// clear the linking view
		m_pLinkingView = NULL;
	}

	// if a linked view was found
	if (pLinkedView)
	{
		// notify views of the change in the node
		m_pView->GetDocument()->UpdateAllViews(m_pView, 0L, 
			pLinkedView->GetNode());
	}

}	// CDesigntimeTracker::OnButtonUp
	

//////////////////////////////////////////////////////////////////////
// CDesigntimeTracker::OnMouseMove
// 
// processes mouse move by changing the cursor
//////////////////////////////////////////////////////////////////////
void CDesigntimeTracker::OnMouseMove(UINT nFlags, CPoint point)  
{ 
	if (m_pView->FindNodeViewAt(point) != NULL)
	{
		::SetCursor(::LoadCursor(GetModuleHandle(NULL), 
			MAKEINTRESOURCE(IDC_HANDPOINT)));
	}
	else
	{
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}

}	// CDesigntimeTracker::OnMouseMove


//////////////////////////////////////////////////////////////////////
// CDesigntimeTracker::OnMouseDrag
// 
// processes mouse drag by changing the cursor
//////////////////////////////////////////////////////////////////////
void CDesigntimeTracker::OnMouseDrag(UINT nFlags, CPoint point)  
{ 
	if (NULL != m_pLinkingView)
	{
		::SetCursor(::LoadCursor(GetModuleHandle(NULL), 
			MAKEINTRESOURCE(IDC_LINK)));
	}
	else if (m_bCutting)
	{
		::SetCursor(::LoadCursor(GetModuleHandle(NULL), 
			MAKEINTRESOURCE(IDC_SAW)));
	}

}	// CDesigntimeTracker::OnMouseDrag


//////////////////////////////////////////////////////////////////////
// CDesigntimeTracker::OnDraw
// 
// processes mouse drag by changing the cursor
//////////////////////////////////////////////////////////////////////
void CDesigntimeTracker::OnDraw(CDC *pDC)
{
	if (m_pLinkingView != NULL || m_bCutting)
	{
		CPen pen(PS_DOT, 1, RGB(128, 96, 96));
		CPen *pOldPen = pDC->SelectObject(&pen);

		if (NULL != m_pLinkingView)
		{
			CRect rectOuter = (CRect) m_pLinkingView->m_extOuter;
			pDC->MoveTo(m_ptStart + //  m_pLinkingView->GetOuterRect()
				rectOuter.CenterPoint());
		}
		else
		{
			pDC->MoveTo(m_ptStart);
		}

		CPoint ptCurrent;
		::GetCursorPos(&ptCurrent);
		m_pView->ScreenToClient(&ptCurrent);
		pDC->LineTo(ptCurrent);

		pDC->SelectObject(pOldPen);
	}

}	// CDesigntimeTracker::OnDraw

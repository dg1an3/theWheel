// TreeMenuCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "THEWHEEL_TREEVIEW_resource.h"
#include "TreeMenuCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeMenuCtrl

CTreeMenuCtrl::CTreeMenuCtrl()
: m_pCmdTarget(NULL)
{
}

CTreeMenuCtrl::~CTreeMenuCtrl()
{
}


BEGIN_MESSAGE_MAP(CTreeMenuCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CTreeMenuCtrl)
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTreeMenuCtrl message handlers

void CTreeMenuCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if (NULL == m_pCmdTarget)
	{
		return;
	}

    // select the drop target
	HTREEITEM hTarget = HitTest(point);
	if (NULL != hTarget)
	{
		Select(hTarget, TVGN_CARET);
		SelectDropTarget(hTarget);

		ClientToScreen(&point);
		CMenu *pMenu = new CMenu();
		pMenu->LoadMenu(m_nMenuResourceID);
		pMenu->GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN, 
			point.x, point.y, m_pCmdTarget);

		delete pMenu;
	}
	
	CTreeCtrl::OnRButtonDown(nFlags, point);
}

void CTreeMenuCtrl::SetMenu(UINT nResourceID, CWnd *pCmdTarget)
{
	m_nMenuResourceID = nResourceID;
	m_pCmdTarget = pCmdTarget;
}

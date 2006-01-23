// ActivationControlBar.cpp : implementation file
//

#include "stdafx.h"
#include "TestNodeRenderer.h"
#include "ActivationControlBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CActivationControlBar dialog


CActivationControlBar::CActivationControlBar(CWnd* pParent /*=NULL*/)
: m_pNode(NULL)
//	: CDialog(CActivationControlBar::IDD, pParent)
{
	//{{AFX_DATA_INIT(CActivationControlBar)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CActivationControlBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CActivationControlBar)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CActivationControlBar, CDialogBar)
	//{{AFX_MSG_MAP(CActivationControlBar)
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CActivationControlBar message handlers

void CActivationControlBar::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (nSBCode == TB_THUMBTRACK)
	{
		TRACE1("Slider position = %i\n", nPos);
		m_activation = (double) nPos / 200.0;
		m_pNode->SetActivation(m_activation);
	}
		
	CDialogBar::OnVScroll(nSBCode, nPos, pScrollBar);
}

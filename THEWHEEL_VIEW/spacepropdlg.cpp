// SpacePropDlg.cpp : implementation file
//

#include "stdafx.h"

// resource includes
#include "THEWHEEL_VIEW_resource.h"

#include "SpacePropDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpacePropDlg dialog


CSpacePropDlg::CSpacePropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpacePropDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpacePropDlg)
	m_spring = 0.0;
	m_primSec = 0.0;
	m_learnRate = 0.0;
	//}}AFX_DATA_INIT
}


void CSpacePropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpacePropDlg)
	DDX_Text(pDX, IDC_SPRING, m_spring);
	DDV_MinMaxDouble(pDX, m_spring, 0.1, 0.9);
	DDX_Text(pDX, IDC_PRIMSEC, m_primSec);
	DDV_MinMaxDouble(pDX, m_primSec, 0.1, 0.9);
	DDX_Text(pDX, IDC_LEARNRATE, m_learnRate);
	DDV_MinMaxDouble(pDX, m_learnRate, 1., 2.);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpacePropDlg, CDialog)
	//{{AFX_MSG_MAP(CSpacePropDlg)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_SETINITIALSTATE, OnSetinitialstate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpacePropDlg message handlers

void CSpacePropDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
		
	// holds the rectangles for the controls
	CRect rectCtrl;

	CWnd *pEditSpring = GetDlgItem(IDC_SPRING);
	if (pEditSpring)
	{
		pEditSpring->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx - 5;
		pEditSpring->MoveWindow(&rectCtrl);
	}

	CWnd *pEditPrimSec = GetDlgItem(IDC_PRIMSEC);
	if (pEditPrimSec)
	{
		pEditPrimSec->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx - 5;
		pEditPrimSec->MoveWindow(&rectCtrl);
	}	

	CWnd *pEditLearnRate = GetDlgItem(IDC_LEARNRATE);
	if (pEditLearnRate)
	{
		pEditLearnRate->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx - 5;
		pEditLearnRate->MoveWindow(&rectCtrl);
	}	

	CWnd *pBtnSetInitialState = GetDlgItem(IDC_SETINITIALSTATE);
	if (pBtnSetInitialState)
	{
		pBtnSetInitialState->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.OffsetRect(cx / 2 - rectCtrl.CenterPoint().x, 0);
		pBtnSetInitialState->MoveWindow(&rectCtrl);
	}
}

void CSpacePropDlg::OnSetinitialstate() 
{
	// TODO: Add your control notification handler code here
	
}

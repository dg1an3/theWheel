// SpaceOptDlg.cpp : implementation file
//

#include "stdafx.h"

// resource includes
#include "THEWHEEL_VIEW_resource.h"

#include "SpaceOptDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpaceOptDlg dialog


CSpaceOptDlg::CSpaceOptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpaceOptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpaceOptDlg)
	m_nSuperNodeCount = 0;
	m_tolerance = 0.0;
	m_kpos = 0.0;
	m_krep = 0.0;
	//}}AFX_DATA_INIT
}


void CSpaceOptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpaceOptDlg)
	DDX_Text(pDX, IDC_SUPERNODECOUNT, m_nSuperNodeCount);
	DDV_MinMaxUInt(pDX, m_nSuperNodeCount, 10, 255);
	DDX_Text(pDX, IDC_TOLERANCE, m_tolerance);
	DDV_MinMaxDouble(pDX, m_tolerance, 0.1, 10.);
	DDX_Text(pDX, IDC_K_POS, m_kpos);
	DDV_MinMaxDouble(pDX, m_kpos, 100., 1000.);
	DDX_Text(pDX, IDC_K_REP, m_krep);
	DDV_MinMaxDouble(pDX, m_krep, 100., 1000.);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpaceOptDlg, CDialog)
	//{{AFX_MSG_MAP(CSpaceOptDlg)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpaceOptDlg message handlers

void CSpaceOptDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
		
	// holds the rectangles for the controls
	CRect rectCtrl;

	CWnd *pEditSuperNodeCount = GetDlgItem(IDC_SUPERNODECOUNT);
	if (pEditSuperNodeCount)
	{
		pEditSuperNodeCount->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx - 5;
		pEditSuperNodeCount->MoveWindow(&rectCtrl);
	}

	CWnd *pEditTolerance = GetDlgItem(IDC_TOLERANCE);
	if (pEditTolerance)
	{
		pEditTolerance->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx - 5;
		pEditTolerance->MoveWindow(&rectCtrl);
	}	

	CWnd *pEditKpos = GetDlgItem(IDC_K_POS);
	if (pEditKpos)
	{
		pEditKpos->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx / 2 - 2;
		pEditKpos->MoveWindow(&rectCtrl);
	}
	
	CWnd *pEditKrep = GetDlgItem(IDC_K_REP);
	if (pEditKrep)
	{
		pEditKrep->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.left = cx / 2 + 2;
		rectCtrl.right = cx - 5;
		pEditKrep->MoveWindow(&rectCtrl);
	}

	CWnd *pStaticKrep = GetDlgItem(IDC_STATIC_K_REP);
	if (pStaticKrep)
	{
		pStaticKrep->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.left = cx / 2 + 2;
		pStaticKrep->MoveWindow(&rectCtrl);
	}
}

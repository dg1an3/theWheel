// SpaceOptDlg.cpp : implementation file
//

#include "stdafx.h"

// resource includes
#include "THEWHEEL_TREEVIEW_resource.h"

#include <Space.h>

#include "SpaceOptDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpaceOptDlg dialog


CSpaceOptDlg::CSpaceOptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpaceOptDlg::IDD, pParent),
		m_pSpace(NULL)
{
	//{{AFX_DATA_INIT(CSpaceOptDlg)
	m_nSuperNodeCount = 20;
	m_tolerance = 0.1;
	m_kpos = 100.0;
	m_krep = 100.0;
	//}}AFX_DATA_INIT
}


void CSpaceOptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	CSpaceLayoutManager *pSLM = NULL;
	if (m_pSpace != NULL)
	{
		pSLM = m_pSpace->GetLayoutManager();
	}

	if (!pDX->m_bSaveAndValidate && pSLM != NULL)
	{
		m_nSuperNodeCount = pSLM->GetStateDim() / 2;
		m_tolerance = pSLM->GetTolerance();
		m_kpos = pSLM->GetKPos();
		m_krep = pSLM->GetKRep();
	}

	//{{AFX_DATA_MAP(CSpaceOptDlg)
	DDX_Text(pDX, IDC_SUPERNODECOUNT, m_nSuperNodeCount);
	DDV_MinMaxUInt(pDX, m_nSuperNodeCount, 10, 255);
	DDX_Text(pDX, IDC_TOLERANCE, m_tolerance);
	DDV_MinMaxDouble(pDX, m_tolerance, 0.001, 10.);
	DDX_Text(pDX, IDC_K_POS, m_kpos);
	DDV_MinMaxDouble(pDX, m_kpos, 100., 100000.);
	DDX_Text(pDX, IDC_K_REP, m_krep);
	DDV_MinMaxDouble(pDX, m_krep, 100., 100000.);
	//}}AFX_DATA_MAP
	
	if (pDX->m_bSaveAndValidate && pSLM != NULL)
	{
		m_pSpace->SetMaxSuperNodeCount(m_nSuperNodeCount);
		pSLM->SetTolerance(m_tolerance);
		pSLM->SetKPos(m_kpos);
		pSLM->SetKRep(m_krep);	
	}
}


BEGIN_MESSAGE_MAP(CSpaceOptDlg, CDialog)
	//{{AFX_MSG_MAP(CSpaceOptDlg)
	ON_WM_SIZE()
	ON_EN_KILLFOCUS(IDC_SUPERNODECOUNT, OnKillfocusAny)
	ON_EN_KILLFOCUS(IDC_TOLERANCE, OnKillfocusAny)
	ON_EN_KILLFOCUS(IDC_K_POS, OnKillfocusAny)
	ON_EN_KILLFOCUS(IDC_K_REP, OnKillfocusAny)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpaceOptDlg message handlers

void CSpaceOptDlg::OnKillfocusAny() 
{
	UpdateData();
}

// SpacePropDlg.cpp : implementation file
//

#include "stdafx.h"

// resource includes
#include "resource.h"

#include <Space.h>

#include "SpacePropDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpacePropDlg dialog


CSpacePropDlg::CSpacePropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpacePropDlg::IDD, pParent),
		m_pSpace(NULL)
{
	//{{AFX_DATA_INIT(CSpacePropDlg)
	m_spring = 0.1;
	m_primSec = 0.1;
	//}}AFX_DATA_INIT
}


void CSpacePropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	if (!pDX->m_bSaveAndValidate && m_pSpace != NULL)
	{
		m_spring = m_pSpace->GetSpringConst();
		m_primSec = m_pSpace->GetPrimSecRatio();
	}

	//{{AFX_DATA_MAP(CSpacePropDlg)
	DDX_Text(pDX, IDC_SPRING, m_spring);
	DDV_MinMaxDouble(pDX, m_spring, 0.1, 0.99);
	DDX_Text(pDX, IDC_PRIMSEC, m_primSec);
	DDV_MinMaxDouble(pDX, m_primSec, 0.1, 10.);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate && m_pSpace != NULL)
	{
		m_pSpace->SetSpringConst(m_spring);
		m_pSpace->SetPrimSecRatio(m_primSec);
	}
}


BEGIN_MESSAGE_MAP(CSpacePropDlg, CDialog)
	//{{AFX_MSG_MAP(CSpacePropDlg)
	ON_EN_KILLFOCUS(IDC_PRIMSEC, OnKillfocusAny)
	ON_EN_KILLFOCUS(IDC_SPRING, OnKillfocusAny)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpacePropDlg message handlers


void CSpacePropDlg::OnKillfocusAny() 
{
	UpdateData();
}

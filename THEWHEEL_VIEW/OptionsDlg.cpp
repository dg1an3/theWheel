// OptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "THEWHEEL_VIEW_resource.h"
#include "OptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog


COptionsDlg::COptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionsDlg)
	m_k_pos = 0.0;
	m_k_rep = 0.0;
	m_nSuperNodeCount = 0;
	m_tolerance = 0.0;
	m_springConst = 0.0;
	//}}AFX_DATA_INIT
}


void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsDlg)
	DDX_Text(pDX, IDC_K_POS, m_k_pos);
	DDX_Text(pDX, IDC_K_REP, m_k_rep);
	DDX_Text(pDX, IDC_SUPERNODECOUNT, m_nSuperNodeCount);
	DDX_Text(pDX, IDC_TOLERANCE, m_tolerance);
	DDX_Text(pDX, IDC_SPRING, m_springConst);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
	//{{AFX_MSG_MAP(COptionsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg message handlers

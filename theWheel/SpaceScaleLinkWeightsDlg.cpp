// SpaceScaleLinkWeightsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "theWheel.h"
#include "SpaceScaleLinkWeightsDlg.h"
#include "theWheelDoc.h"

// CSpaceScaleLinkWeightsDlg dialog

IMPLEMENT_DYNAMIC(CSpaceScaleLinkWeightsDlg, CDialog)

CSpaceScaleLinkWeightsDlg::CSpaceScaleLinkWeightsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpaceScaleLinkWeightsDlg::IDD, pParent)
	, m_currMaxLinkWgt(0)
	, m_newMaxLinkWgt(0)
	, m_pSpace(NULL)
{

}

CSpaceScaleLinkWeightsDlg::~CSpaceScaleLinkWeightsDlg()
{
}

void CSpaceScaleLinkWeightsDlg::DoDataExchange(CDataExchange* pDX)
{
	if (!pDX->m_bSaveAndValidate)
	{
		m_currMaxLinkWgt = Round<int>(m_pSpace->GetRootNode()->GetMaxLinkWeight() * 100.0);
		m_newMaxLinkWgt = Round<int>(m_pSpace->GetRootNode()->GetMaxLinkWeight() * 100.0);
	}

	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CURR_MAXLINKWGT, m_currMaxLinkWgt);
	DDV_MinMaxInt(pDX, m_currMaxLinkWgt, 0, 100);
	DDX_Text(pDX, IDC_EDIT_NEW_MAXLINKWGT, m_newMaxLinkWgt);
	DDV_MinMaxInt(pDX, m_newMaxLinkWgt, 0, 100);
}


BEGIN_MESSAGE_MAP(CSpaceScaleLinkWeightsDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CSpaceScaleLinkWeightsDlg::OnBnClickedOk)
END_MESSAGE_MAP()


void CSpaceScaleLinkWeightsDlg::OnBnClickedOk()
{
	// now dismiss
	OnOK();

	m_pSpace->GetRootNode()->ScaleLinkWeights((REAL) m_newMaxLinkWgt
			/ (REAL) m_currMaxLinkWgt);
}

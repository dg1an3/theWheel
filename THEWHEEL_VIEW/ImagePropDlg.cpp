// ImagePropDlg.cpp : implementation file
//

#include "stdafx.h"

// resource includes
#include "THEWHEEL_VIEW_resource.h"

#include "ImagePropDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImagePropDlg dialog


CImagePropDlg::CImagePropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImagePropDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImagePropDlg)
	m_strImageFilename = _T("");
	m_strUrl = _T("");
	m_strClass = _T("");
	//}}AFX_DATA_INIT
}

void CImagePropDlg::SetCurNode(CNode *pNode)
{
	m_pCurNode = pNode;

	// set the member variables
	m_strClass = pNode->GetClass();
	m_strImageFilename = pNode->GetImageFilename();
	m_strUrl = pNode->GetUrl();

	// load the fields
	UpdateData(FALSE);
}

void CImagePropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImagePropDlg)
	DDX_Text(pDX, IDC_EDITIMAGEFILENAME, m_strImageFilename);
	DDX_Text(pDX, IDC_EDITURL, m_strUrl);
	DDX_CBString(pDX, IDC_COMBOCLASS, m_strClass);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImagePropDlg, CDialog)
	//{{AFX_MSG_MAP(CImagePropDlg)
	ON_WM_SIZE()
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImagePropDlg message handlers


void CImagePropDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// holds the rectangles for the controls
	CRect rectCtrl;

	CWnd *pComboClass = GetDlgItem(IDC_COMBOCLASS);
	if (pComboClass)
	{
		pComboClass->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx - 5;
		pComboClass->MoveWindow(&rectCtrl);
	}

	CWnd *pBtnTitleColor = GetDlgItem(IDC_NODETITLECOLOR);
	if (pBtnTitleColor)
	{
		pBtnTitleColor->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.OffsetRect(cx / 2 - rectCtrl.CenterPoint().x, 0);
		pBtnTitleColor->MoveWindow(&rectCtrl);
	}

	CWnd *pEditImageFilename = GetDlgItem(IDC_EDITIMAGEFILENAME);
	if (pEditImageFilename)
	{
		pEditImageFilename->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx - 5;
		pEditImageFilename->MoveWindow(&rectCtrl);
	}

	CWnd *pEditUrl = GetDlgItem(IDC_EDITURL);
	if (pEditUrl)
	{
		pEditUrl->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx - 5;
		pEditUrl->MoveWindow(&rectCtrl);
	}	
}

void CImagePropDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CButton *pBtnTitleColor = (CButton *) GetDlgItem(IDC_NODETITLECOLOR);
	if (pBtnTitleColor)
	{
	//	pBtnTitleColor->DrawItem(lpDrawItemStruct);
	}

	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

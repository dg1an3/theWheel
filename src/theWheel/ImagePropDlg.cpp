// ImagePropDlg.cpp : implementation file
//

#include "stdafx.h"

// resource includes
#include "resource.h"

#include <Space.h>

#include "ImagePropDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImagePropDlg dialog


CImagePropDlg::CImagePropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImagePropDlg::IDD, pParent),
		m_pCurNode(NULL)
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
	m_strUrl = ""; // pNode->GetUrl();

	// populate the class combo box
	CComboBox *pComboBox = (CComboBox *) GetDlgItem(IDC_COMBOCLASS);
	pComboBox->ResetContent();
	pComboBox->AddString(_T("<none>"));

	auto mapClasses = pNode->GetSpace()->GetClassColorMap();
	auto pos = mapClasses.begin();
	while (pos != mapClasses.end())
	{
		auto pszClassName = pos->first;
		auto color = pos->second;		
		pComboBox->AddString(pszClassName);
	}

	// load the fields
	UpdateData(FALSE);
}

void CImagePropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	if (!pDX->m_bSaveAndValidate && m_pCurNode != NULL)
	{
		m_strClass = m_pCurNode->GetClass();
		if (m_strClass == "")
		{
			m_strClass = "<none>";
		}
		m_strImageFilename = m_pCurNode->GetImageFilename();
		m_strUrl = ""; // m_pCurNode->GetUrl();
	}

	//{{AFX_DATA_MAP(CImagePropDlg)
	DDX_Text(pDX, IDC_EDITIMAGEFILENAME, m_strImageFilename);
	DDX_Text(pDX, IDC_EDITURL, m_strUrl);
	DDX_CBString(pDX, IDC_COMBOCLASS, m_strClass);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate && m_pCurNode != NULL)
	{
		if (m_strClass == "<none>")
		{
			m_strClass = "";
		}
		m_pCurNode->SetClass(m_strClass);

		m_pCurNode->SetImageFilename(m_strImageFilename);
		// m_pCurNode->SetUrl(m_strUrl);
	}
}


BEGIN_MESSAGE_MAP(CImagePropDlg, CDialog)
	//{{AFX_MSG_MAP(CImagePropDlg)
	ON_WM_SIZE()
	ON_EN_KILLFOCUS(IDC_EDITIMAGEFILENAME, OnKillfocusAny)
	ON_EN_KILLFOCUS(IDC_EDITURL, OnKillfocusAny)
	ON_CBN_SELCHANGE(IDC_COMBOCLASS, OnSelchangeComboclass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImagePropDlg message handlers


void CImagePropDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// holds the rectangles for the controls
	CRect rectCtrl;

	CComboBox *pComboClass = (CComboBox *) GetDlgItem(IDC_COMBOCLASS);
	if (pComboClass)
	{
		pComboClass->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx - 5;
		pComboClass->MoveWindow(&rectCtrl);
		pComboClass->SetItemHeight(0, 300);
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

void CImagePropDlg::OnKillfocusAny() 
{
	UpdateData(TRUE);
}

void CImagePropDlg::OnSelchangeComboclass() 
{
	UpdateData(TRUE);	
}

// NodePropDlg.cpp : implementation file
//

#include "stdafx.h"

// resource includes
#include "THEWHEEL_TREEVIEW_resource.h"

#include "NodePropDlg.h"

#include <Space.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNodePropDlg dialog


CNodePropDlg::CNodePropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNodePropDlg::IDD, pParent),
		m_pCurNode(NULL)
{
	//{{AFX_DATA_INIT(CNodePropDlg)
	m_strName = _T("");
	m_strDesc = _T("");
	//}}AFX_DATA_INIT
}

void CNodePropDlg::SetCurNode(CNode *pNode)
{
	m_pCurNode = pNode;

	// load the fields
	UpdateData(FALSE);
}

void CNodePropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	if (!pDX->m_bSaveAndValidate && m_pCurNode != NULL)
	{
		// set the member variables
		m_strName = m_pCurNode->GetName();
		m_strDesc = m_pCurNode->GetDescription();
	}

	//{{AFX_DATA_MAP(CNodePropDlg)
	DDX_Text(pDX, IDC_EDITNAME, m_strName);
	DDX_Text(pDX, IDC_EDITDESC, m_strDesc);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate && m_pCurNode != NULL)
	{
		// set the member variables
		m_pCurNode->SetName(m_strName);
		m_pCurNode->SetDescription(m_strDesc);
	}
}


BEGIN_MESSAGE_MAP(CNodePropDlg, CDialog)
	//{{AFX_MSG_MAP(CNodePropDlg)
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_EDITNAME, OnChangeEditname)
	ON_EN_CHANGE(IDC_EDITDESC, OnChangeEditdesc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNodePropDlg message handlers


void CNodePropDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// holds the rectangles for the controls
	CRect rectCtrl;

	CWnd *pEditName = GetDlgItem(IDC_EDITNAME);
	if (pEditName)
	{
		pEditName->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx - 5;
		pEditName->MoveWindow(&rectCtrl);
	}

	CWnd *pEditDesc = GetDlgItem(IDC_EDITDESC);
	if (pEditDesc)
	{
		pEditDesc->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx - 5;
		pEditDesc->MoveWindow(&rectCtrl);
	}
}

void CNodePropDlg::OnChangeEditname() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	UpdateData(TRUE);
	
	// m_pCurNode->SetName(m_strName);
	// if (m_pCurNode->GetSpace() != NULL)
	{
	//	m_pCurNode->GetSpace()->UpdateAllViews(NULL, 0L, m_pCurNode);
	}
}

void CNodePropDlg::OnChangeEditdesc() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	UpdateData(TRUE);
	
	// m_pCurNode->SetDescription(m_strDesc);
}

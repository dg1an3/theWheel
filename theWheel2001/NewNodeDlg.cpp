// NewNodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "theWheel2001.h"
#include "NewNodeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewNodeDlg dialog


CNewNodeDlg::CNewNodeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewNodeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewNodeDlg)
	m_strName = _T("");
	m_strDesc = _T("");
	m_strImageFilename = _T("");
	//}}AFX_DATA_INIT
}


void CNewNodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewNodeDlg)
	DDX_Control(pDX, IDC_LINKLIST, m_LinkList);
	DDX_Text(pDX, IDC_EDITNAME, m_strName);
	DDX_Text(pDX, IDC_EDITDESC, m_strDesc);
	DDX_Text(pDX, IDC_EDITIMAGEFILENAME, m_strImageFilename);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewNodeDlg, CDialog)
	//{{AFX_MSG_MAP(CNewNodeDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewNodeDlg message handlers

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
// CEditLabelListCtrl 




/////////////////////////////////////////////////////////////////////////////
// CNewNodeDlg dialog


CNewNodeDlg::CNewNodeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewNodeDlg::IDD, pParent),
		m_pNode(NULL)
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
    ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LINKLIST, OnEndLabelEdit) 
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewNodeDlg message handlers

BOOL CNewNodeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CRect rect;
	m_LinkList.GetWindowRect(&rect); 
    m_LinkList.InsertColumn(0, "Weight", LVCFMT_LEFT, rect.Width() * 1/4, 0); 
    m_LinkList.InsertColumn(1, "ToNode", LVCFMT_LEFT, rect.Width() * 2/3, 1); 

	// now add entries into the list of links
	for (int nAt = 0; nAt < m_pNode->links.GetSize(); nAt++)
	{
		CNodeLink *pLink = m_pNode->links.Get(nAt);

		// set up the item structure
		LVITEM lvitem; 
		lvitem.mask = LVIF_TEXT | LVIF_PARAM; 
		lvitem.iItem = nAt; 
		lvitem.iSubItem = 0;	// for main item
		lvitem.lParam = (DWORD) pLink;

		// format a string with the link weight
		CString strWeight;
		strWeight.Format("%f", pLink->weight.Get());
		lvitem.pszText = strWeight.GetBuffer(100);
		lvitem.cchTextMax = 100;
		lvitem.iItem = m_LinkList.InsertItem(&lvitem); 

		// now insert the subitem for the link weight
		lvitem.mask = LVIF_TEXT; 
		lvitem.iSubItem = 1;
		lvitem.pszText = pLink->forTarget->name.Get().GetBuffer(100);
		BOOL bResult = m_LinkList.SetItem(&lvitem); 
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewNodeDlg::OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult)
{
	// get the information about the edited item
	LV_DISPINFO  *plvDispInfo = (LV_DISPINFO *)pnmhdr;
	LVITEM       *plvItem = &plvDispInfo->item;

	if (plvItem->pszText != NULL)
	{
		// now retrieve the value from the text
		float weight;
		sscanf(plvItem->pszText, "%f", &weight);

		// set the link weight
		CNodeLink *pLink = 
			(CNodeLink *)m_LinkList.GetItemData(plvItem->iItem);
		pLink->weight.Set(weight);

	}

	// make sure we maintain the edit
    *pLResult = TRUE;
}

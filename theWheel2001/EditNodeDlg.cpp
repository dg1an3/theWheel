//////////////////////////////////////////////////////////////////////
// EditNodeDlg.cpp: implementation of the CEditNodeDlg class.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// resource includes
#include "resource.h"

// class definition for node dialog
#include "EditNodeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// CEditNodeDlg::CEditNodeDlg
// 
// constructs a new CEditNodeDlg dialog object
//////////////////////////////////////////////////////////////////////
CEditNodeDlg::CEditNodeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditNodeDlg::IDD, pParent),
		m_pNode(NULL)
{
	//{{AFX_DATA_INIT(CEditNodeDlg)
	m_strName = _T("");
	m_strDesc = _T("");
	m_strImageFilename = _T("");
	m_strUrl = _T("");
	//}}AFX_DATA_INIT
}

//////////////////////////////////////////////////////////////////////
// CEditNodeDlg::DoDataExchange
// 
// performs the data exchange before/after something changes
//////////////////////////////////////////////////////////////////////
void CEditNodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	if (!pDX->m_bSaveAndValidate)
	{
		// populate fields from the node
		m_strName = m_pNode->GetName();
		m_strDesc = m_pNode->GetDescription();
		m_strImageFilename = m_pNode->GetImageFilename();
		m_strUrl = m_pNode->GetUrl();
	}

	//{{AFX_DATA_MAP(CEditNodeDlg)
	DDX_Control(pDX, IDC_LINKLIST, m_LinkList);
	DDX_Text(pDX, IDC_EDITNAME, m_strName);
	DDX_Text(pDX, IDC_EDITDESC, m_strDesc);
	DDX_Text(pDX, IDC_EDITIMAGEFILENAME, m_strImageFilename);
	DDX_Text(pDX, IDC_EDITURL, m_strUrl);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CEditNodeDlg Message Map
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CEditNodeDlg, CDialog)
	//{{AFX_MSG_MAP(CEditNodeDlg)
    ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LINKLIST, OnEndLabelEdit) 
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditNodeDlg message handlers


BOOL CEditNodeDlg::IsLinkPresent(CNode *pToNode)
{
	// check the array to see if this link's target is already present
	BOOL bPresent = FALSE;
	for (int nAtNode = 0; nAtNode < m_arrNodes.GetSize(); nAtNode++)
	{
		if (pToNode == m_arrNodes[nAtNode])
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CEditNodeDlg::AddLinksFromOtherNode(CNode *pOtherNode, int nLevels)
{
	// array containing list of nodes that were just added
	CArray<CNode *, CNode *> arrNewNodes;

	// now add entries into the list of links
	for (int nAtLink = 0; nAtLink < pOtherNode->GetLinkCount(); nAtLink++)
	{
		CNodeLink *pLink = pOtherNode->GetLinkAt(nAtLink);
		CNode *pLinkedNode = pLink->GetTarget();
		if ((pLinkedNode != m_pNode)
				&& !IsLinkPresent(pLinkedNode))
		{
			// set up the item structure
			LVITEM lvitem; 
			lvitem.mask = LVIF_TEXT | LVIF_PARAM; 
			lvitem.iItem = m_nItemCount++; 
			lvitem.iSubItem = 0;	// for main item
			lvitem.lParam = (DWORD) pLinkedNode;	// to indicate no link present

			// format a string with the link weight
			CString strWeight;
			strWeight.Format("%f", 0.0);
			lvitem.pszText = strWeight.GetBuffer(100);
			lvitem.cchTextMax = 100;
			lvitem.iItem = m_LinkList.InsertItem(&lvitem); 

			// format a string with the link name
			lvitem.mask = LVIF_TEXT; 
			lvitem.iSubItem = 1;
			CString strName(pLinkedNode->GetName());
			lvitem.pszText = strName.GetBuffer(100);

			// now insert the subitem for the link weight
			BOOL bResult = m_LinkList.SetItem(&lvitem); 

			// add to the list of nodes
			m_arrNodes.Add(pLink->GetTarget());

			// also add to the local list, so that it can be traversed later
			arrNewNodes.Add(pLink->GetTarget());
		}
	}

	// now call recursively
	if (nLevels > 0)
	{
		// now add entries into the list of links
		for (int nAtNode = 0; nAtNode < arrNewNodes.GetSize(); nAtNode++)
		{
			AddLinksFromOtherNode(arrNewNodes[nAtNode], nLevels-1);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// CEditNodeDlg::OnInitDialog
// 
// initializes the dialog
//////////////////////////////////////////////////////////////////////
BOOL CEditNodeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// initialize the link list control
	CRect rect;
	m_LinkList.GetWindowRect(&rect); 
    m_LinkList.InsertColumn(0, "Weight", LVCFMT_LEFT, rect.Width() * 1/4, 0); 
    m_LinkList.InsertColumn(1, "ToNode", LVCFMT_LEFT, rect.Width() * 2/3, 1); 

	// initialize the items count
	m_nItemCount = 0;

	// now add entries into the list of links
	for (int nAt = 0; nAt < m_pNode->GetLinkCount(); nAt++)
	{
		CNodeLink *pLink = m_pNode->GetLinkAt(nAt);

		// set up the item structure
		LVITEM lvitem; 
		lvitem.mask = LVIF_TEXT | LVIF_PARAM; 
		lvitem.iItem = m_nItemCount++; 
		lvitem.iSubItem = 0;	// for main item
		lvitem.lParam = (DWORD) pLink->GetTarget();

		// format a string with the link weight
		CString strWeight;
		strWeight.Format("%f", pLink->GetWeight());
		lvitem.pszText = strWeight.GetBuffer(100);
		lvitem.cchTextMax = 100;
		lvitem.iItem = m_LinkList.InsertItem(&lvitem); 

		// format a string with the target name
		lvitem.mask = LVIF_TEXT; 
		lvitem.iSubItem = 1;
		CString strTargetName(pLink->GetTarget()->GetName());
		lvitem.pszText = strTargetName.GetBuffer(100);

		// now insert the subitem for the link weight
		BOOL bResult = m_LinkList.SetItem(&lvitem);

		m_arrNodes.Add(pLink->GetTarget());
	}

	// now add entries for links that don't exist
	for (int nAtNode = 0; nAtNode < m_arrNodes.GetSize(); nAtNode++)
	{
		AddLinksFromOtherNode(m_arrNodes[nAtNode]);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////
// CEditNodeDlg::OnEndLabelEdit
// 
// initializes the dialog
//////////////////////////////////////////////////////////////////////
void CEditNodeDlg::OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult)
{
	// get the information about the edited item
	LV_DISPINFO  *plvDispInfo = (LV_DISPINFO *)pnmhdr;
	LVITEM       *plvItem = &plvDispInfo->item;

	if (plvItem->pszText != NULL)
	{
		// now retrieve the value from the text
		float weight;
		sscanf(plvItem->pszText, "%f", &weight);

		// get the target
		CNode *pOtherNode = 
			(CNode *)m_LinkList.GetItemData(plvItem->iItem);

		// and establish the weight
		m_pNode->LinkTo(pOtherNode, weight);

	}

	// make sure we maintain the edit
    *pLResult = TRUE;
}

void CEditNodeDlg::OnOK() 
{
	CDialog::OnOK();

	// set the values for the node
	m_pNode->SetName(m_strName);
	m_pNode->SetDescription(m_strDesc);
	m_pNode->SetImageFilename(m_strImageFilename);
	m_pNode->SetUrl(m_strUrl);
}

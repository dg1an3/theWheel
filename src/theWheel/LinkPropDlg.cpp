// LinkPropDlg.cpp : implementation file
//

#include "stdafx.h"

// resource includes
#include "resource.h"

#include "LinkPropDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int CALLBACK CompareLinkWeights(LPARAM lParam1, LPARAM lParam2, 
   LPARAM lParamSort)
{
	CNode *pNode1 = (CNode *) lParam1;
	ASSERT(pNode1->IsKindOf(RUNTIME_CLASS(CNode)));
	CNode *pNode2 = (CNode *) lParam2;
	ASSERT(pNode2->IsKindOf(RUNTIME_CLASS(CNode)));
	CNode *pCurNode = (CNode *) lParamSort;
	ASSERT(pCurNode->IsKindOf(RUNTIME_CLASS(CNode)));

	REAL weight1 = pCurNode->GetLinkWeight(pNode1);
	REAL weight2 = pCurNode->GetLinkWeight(pNode2);

	return (weight1 > weight2 ? -1 : (weight1 == weight2 ? 0 : 1));
}


/////////////////////////////////////////////////////////////////////////////
// CLinkPropDlg dialog


CLinkPropDlg::CLinkPropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLinkPropDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLinkPropDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CLinkPropDlg::SetCurNode(CNode *pNode)
{
	if (m_pCurNode != pNode)
	{
		m_pCurNode = pNode;

		// clear items from the list
		m_LinkList.DeleteAllItems();
	}

	// load the fields
	UpdateData(FALSE);

	if (NULL != m_pCurNode)
	{
		// reset sort flag
		m_bSort = FALSE;

		// update the list of links
		UpdateLinkList(m_pCurNode);

		if (m_bSort)
		{
			// sort the nodes
			m_LinkList.SortItems(CompareLinkWeights, (DWORD_PTR) m_pCurNode);
		}
	}
}

void CLinkPropDlg::UpdateLinkList(CNode *pOtherNode, int nLevels)
{
	// array containing list of nodes that were just added
	CArray<CNode *, CNode *> arrNewNodes;

	// now add entries into the list of links
	for (int nAtLink = 0; nAtLink < pOtherNode->GetLinkCount(); nAtLink++)
	{
		CNodeLink *pLink = pOtherNode->GetLinkAt(nAtLink);
		CNode *pLinkedNode = pLink->GetTarget();
		if (pLinkedNode == m_pCurNode)
		{
			continue;
		}

		// try to find the list item for this link
		LVFINDINFO findInfo;
		findInfo.flags = LVFI_PARAM;
		findInfo.lParam = (LPARAM) pLinkedNode;
		int nItem = m_LinkList.FindItem(&findInfo);

		if (nItem != -1)
		{
			// update the item's weight, if necessary
			CString strWeight;
			strWeight.Format(_T("%5.2f"), 
				m_pCurNode->GetLinkWeight(pLinkedNode) * 100.0);
			if (strWeight != m_LinkList.GetItemText(nItem, 0))
			{
				m_LinkList.SetItemText(nItem, 0, strWeight);

				// flag for re-sorting
				m_bSort = TRUE;
			}
		}
		else
		{
			// set up the item structure
			LVITEM lvitem; 
			lvitem.mask = LVIF_TEXT | LVIF_PARAM; 
			lvitem.iItem = m_LinkList.GetItemCount(); 
			lvitem.iSubItem = 0;	// for main item
			lvitem.lParam = (LPARAM) pLinkedNode;	// to indicate no link present

			// format a string with the link weight
			CString strWeight;
			strWeight.Format(_T("%5.2f"), 
				m_pCurNode->GetLinkWeight(pLinkedNode) * 100.0); 
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

			// now insert the subitem for the stabilizer flag
			lvitem.mask = LVIF_TEXT;
			lvitem.iSubItem = 2;
			lvitem.pszText = pLink->GetIsStabilizer() ? _T("X") : _T(" ");
			bResult = m_LinkList.SetItem(&lvitem); 

			// flag for re-sorting
			m_bSort = TRUE;

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
			UpdateLinkList(arrNewNodes[nAtNode], nLevels-1);
		}
	}
}

void CLinkPropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLinkPropDlg)
	DDX_Control(pDX, IDC_LINKLIST, m_LinkList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLinkPropDlg, CDialog)
	//{{AFX_MSG_MAP(CLinkPropDlg)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDC_LINKLIST, OnClickLinklist)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LINKLIST, OnEndlabeleditLinklist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLinkPropDlg message handlers

void CLinkPropDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// holds the rectangles for the controls
	CRect rectCtrl;

	if (::IsWindow(m_LinkList.m_hWnd))
	{
		m_LinkList.GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx - 5;
		m_LinkList.MoveWindow(&rectCtrl);

		m_LinkList.GetWindowRect(&rectCtrl);
		m_LinkList.SetColumnWidth(0, rectCtrl.Width() / 4);
		m_LinkList.SetColumnWidth(1, rectCtrl.Width() / 2);
		m_LinkList.SetColumnWidth(2, rectCtrl.Width() / 4);
	}
}

BOOL CLinkPropDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
		
	// initialize the link list control
	CRect rect;
	m_LinkList.GetWindowRect(&rect); 
    m_LinkList.InsertColumn(0, _T("Weight"), LVCFMT_LEFT, rect.Width() / 4, 0); 
    m_LinkList.InsertColumn(1, _T("To Node"), LVCFMT_LEFT, rect.Width() / 2, 1); 
    m_LinkList.InsertColumn(2, _T("S"), LVCFMT_LEFT, rect.Width() / 4, 2); 
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLinkPropDlg::OnEndlabeleditLinklist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// get the information about the edited item
	LV_DISPINFO  *plvDispInfo = (LV_DISPINFO *)pNMHDR;
	LVITEM       *plvItem = &plvDispInfo->item;

	if (plvItem->pszText != NULL)
	{
		// now retrieve the value from the text
		float weight;
		_stscanf_s(plvItem->pszText, _T("%f"), &weight, 10);

		// get the target
		CNode *pOtherNode = 
			(CNode *)m_LinkList.GetItemData(plvItem->iItem);

		// and establish the weight
		m_pCurNode->LinkTo(pOtherNode, weight / 100.0);

		// update the list of links
		UpdateLinkList(m_pCurNode);

		// sort the nodes
		m_LinkList.SortItems(CompareLinkWeights, (DWORD_PTR) m_pCurNode);

		// ensure the just-edited item is visible
		POSITION pos = m_LinkList.GetFirstSelectedItemPosition();
		m_LinkList.EnsureVisible(m_LinkList.GetNextSelectedItem(pos), FALSE);

		*pResult = TRUE; 
	}
	
	*pResult = FALSE;
}

void CLinkPropDlg::OnClickLinklist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMLISTVIEW pNMLISTVIEW = (LPNMLISTVIEW) pNMHDR;

	LVHITTESTINFO htinfo;
	memset(&htinfo, 0, sizeof(htinfo));
	htinfo.pt = pNMLISTVIEW->ptAction;

	// test for a click on the stabilizer field of one of the list items
	if (m_LinkList.SubItemHitTest(&htinfo) != -1
		&& htinfo.iSubItem == 2)
	{
		CNode *pLinkedNode = (CNode *)m_LinkList.GetItemData(htinfo.iItem);
		if (pLinkedNode)
		{
			CNodeLink *pLink = m_pCurNode->GetLinkTo(pLinkedNode);
			if (pLink != NULL)
			{
				// flip-flop flag
				pLink->SetIsStabilizer(!pLink->GetIsStabilizer());
				
				// set item text
				m_LinkList.SetItemText(htinfo.iItem, 2, 
					pLink->GetIsStabilizer() ? _T("X") : _T(" "));

				// flip-flop reverse link
				CNodeLink *pRevLink = pLinkedNode->GetLinkTo(m_pCurNode);
				if (pRevLink)
				{
					pRevLink->SetIsStabilizer(pLink->GetIsStabilizer());
				}
			}
		}
	}

	*pResult = 0;
}

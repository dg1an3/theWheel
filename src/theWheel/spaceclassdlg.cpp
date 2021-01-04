// SpaceClassDlg.cpp : implementation file
//

#include "stdafx.h"

// resource includes
#include "resource.h"

#include <Space.h>

#include "SpaceClassDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpaceClassDlg dialog


CSpaceClassDlg::CSpaceClassDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpaceClassDlg::IDD, pParent),
		m_pSpace(NULL)
{
	//{{AFX_DATA_INIT(CSpaceClassDlg)
	m_strNewClassName = _T("");
	//}}AFX_DATA_INIT
}


void CSpaceClassDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpaceClassDlg)
	DDX_Control(pDX, IDC_CLASSLIST, m_ClassList);
	DDX_Text(pDX, IDC_NEWCLASS, m_strNewClassName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpaceClassDlg, CDialog)
	//{{AFX_MSG_MAP(CSpaceClassDlg)
	ON_WM_SIZE()
	ON_WM_DRAWITEM()
	ON_NOTIFY(NM_CLICK, IDC_CLASSLIST, OnClickClasslist)
	ON_BN_CLICKED(IDC_BTNNEWCLASS, OnBtnnewclass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpaceClassDlg message handlers

void CSpaceClassDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// holds the rectangles for the controls
	CRect rectCtrl;

	if (::IsWindow(m_ClassList.m_hWnd))
	{
		m_ClassList.GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);
		rectCtrl.right = cx - 5;
		m_ClassList.MoveWindow(&rectCtrl);

		m_ClassList.GetWindowRect(&rectCtrl);
		m_ClassList.SetColumnWidth(0, rectCtrl.Width() / 2);
		m_ClassList.SetColumnWidth(1, rectCtrl.Width() / 2);
	}
}

void CSpaceClassDlg::PopulateClassList()
{
	m_ClassList.DeleteAllItems();

	if (m_pSpace == NULL)
		return;
	
	int nIndex = 0;
	POSITION pos = m_pSpace->GetClassColorMap().GetStartPosition();
	while (NULL != pos)
	{
		CString strClass;
		COLORREF color;
		m_pSpace->GetClassColorMap().GetNextAssoc(pos, strClass, color);

		// set up the item structure
		LVITEM lvitem; 
		lvitem.mask = LVIF_TEXT | LVIF_PARAM; 
		lvitem.iItem = m_ClassList.GetItemCount()+1; 
		lvitem.lParam = (DWORD) color;
		lvitem.iSubItem = 0;	// for main item

		// format a string with the link weight
		lvitem.pszText = strClass.GetBuffer(100);
		lvitem.cchTextMax = 100;
		lvitem.iItem = m_ClassList.InsertItem(&lvitem); 
	}
}


BOOL CSpaceClassDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// initialize the link list control
	CRect rect;
	m_ClassList.GetClientRect(&rect); 
    m_ClassList.InsertColumn(0, _T("Class"), LVCFMT_LEFT, rect.Width() / 2 - 10, 0); 
    m_ClassList.InsertColumn(1, _T("Color"), LVCFMT_LEFT, rect.Width() / 2 - 10, 1); 
	
	// m_mapClassColors.SetAt("Genre", (void *)RGB(255,0,0));
	// m_mapClassColors.SetAt("Artist", (void *)RGB(0,255,0));
	// m_mapClassColors.SetAt("Title", (void *)RGB(0,0,255));

	// populate the list 
	PopulateClassList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSpaceClassDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	ASSERT(nIDCtl == IDC_CLASSLIST);

	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);

	// get the string
	CString strClass = m_ClassList.GetItemText(lpDrawItemStruct->itemID, 0);

	// get the rectangle for drawing the string
	CRect rectClass;
	m_ClassList.GetSubItemRect(lpDrawItemStruct->itemID, 0, LVIR_BOUNDS, rectClass);

	// now draw a rectangle, if the item is selected
	DWORD dwStyle = ETO_CLIPPED;
	if ((ODS_SELECTED & lpDrawItemStruct->itemState) != 0)
	{
		dwStyle |= ETO_OPAQUE;
	}

	rectClass.DeflateRect(3, 1, 1, 1);

	// output the class name
	dc.ExtTextOut(rectClass.left, rectClass.top, 
		dwStyle, rectClass, strClass, NULL);

	// get the color
	COLORREF color = (COLORREF) m_ClassList.GetItemData(lpDrawItemStruct->itemID);

	// get the rectangle for drawing the string
	CRect rectColor;
	m_ClassList.GetSubItemRect(lpDrawItemStruct->itemID, 1, LVIR_BOUNDS, rectColor);
	rectColor.DeflateRect(2, 2, 6, 2);

	CBrush brush(color);
	CBrush* pOldBrush = dc.SelectObject(&brush);
	dc.Rectangle(rectColor);
	dc.SelectObject(pOldBrush);

	dc.Detach();
}

void CSpaceClassDlg::OnClickClasslist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if (pNMListView->iItem != -1)
	{
		// see if the color field was clicked
		if (pNMListView->iSubItem == 1)
		{
			CColorDialog dlgColor;
			dlgColor.m_cc.Flags |= CC_RGBINIT;
			dlgColor.m_cc.rgbResult = 
				(COLORREF) m_ClassList.GetItemData(pNMListView->iItem);
			if (dlgColor.DoModal() == IDOK)
			{
				m_ClassList.SetItemData(pNMListView->iItem, dlgColor.GetColor());
				if (m_pSpace)
				{
					CString strClass = m_ClassList.GetItemText(pNMListView->iItem, 0);
					m_pSpace->GetClassColorMap().SetAt(strClass, dlgColor.GetColor());
				}
				Invalidate(TRUE);
			}
		}
	}

	*pResult = 0;
}

void CSpaceClassDlg::OnBtnnewclass() 
{
	// get name from edit control
	UpdateData();

	if (m_strNewClassName != "")
	{
		// add a new class item to the space
		m_pSpace->GetClassColorMap().SetAt(m_strNewClassName, RGB(0, 0, 0));

		PopulateClassList();
	}

	m_strNewClassName = "";

	UpdateData(FALSE);
}

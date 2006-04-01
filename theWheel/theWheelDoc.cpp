// theWheelDoc.cpp : implementation of the CtheWheelDoc class
//

#include "stdafx.h"
#include "theWheel.h"

#include "theWheelDoc.h"

#include "SpacePropagationPropPage.h"
#include "SpaceLayoutPropPage.h"

#include "SpaceScaleLinkWeightsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CtheWheelDoc

IMPLEMENT_DYNCREATE(CtheWheelDoc, CDocument)

BEGIN_MESSAGE_MAP(CtheWheelDoc, CDocument)
	ON_COMMAND(ID_SPACE_SCALELINKWEIGHTS, &CtheWheelDoc::OnSpaceScalelinkweights)
	ON_COMMAND(ID_SPACE_PERTURBLINKWEIGHTS, &CtheWheelDoc::OnSpacePerturblinkweights)
	ON_COMMAND(ID_SPACE_PROPERTIES, &CtheWheelDoc::OnSpaceProperties)
END_MESSAGE_MAP()


// CtheWheelDoc construction/destruction

CtheWheelDoc::CtheWheelDoc()
: m_pSpacePropSheet(NULL)
{
	m_space.NodeAddedEvent.AddObserver(this,
		(ListenerFunction) &CtheWheelDoc::OnSpaceChanged);
	m_space.NodeRemovedEvent.AddObserver(this,
		(ListenerFunction) &CtheWheelDoc::OnSpaceChanged);
	m_space.NodeAttributeChangedEvent.AddObserver(this,
		(ListenerFunction) &CtheWheelDoc::OnSpaceChanged);
}

CtheWheelDoc::~CtheWheelDoc()
{
	delete m_pSpacePropSheet;
}

BOOL CtheWheelDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	if (!m_space.CreateSimpleSpace())
		return FALSE;

	return TRUE;
}




// CtheWheelDoc serialization

void CtheWheelDoc::Serialize(CArchive& ar)
{
	CDocument::Serialize(ar);

	m_space.Serialize(ar);
}


// CtheWheelDoc diagnostics

#ifdef _DEBUG
void CtheWheelDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CtheWheelDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CtheWheelDoc commands


void CtheWheelDoc::OnSpaceProperties()
{
	if (!m_pSpacePropSheet)
	{
		m_pSpacePropSheet = new CPropertySheet(_T("Space Properties"));
		m_pSpacePropSheet->AddPage(new CSpaceLayoutPropPage());
		m_pSpacePropSheet->AddPage(new CSpacePropagationPropPage());
	}

	if (!::IsWindow(m_pSpacePropSheet->m_hWnd))
	{
		m_pSpacePropSheet->Create(::AfxGetMainWnd(), 
			WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, WS_EX_TOOLWINDOW);
	}
}

void CtheWheelDoc::OnSpaceScalelinkweights()
{
	static CSpaceScaleLinkWeightsDlg dlg;
	dlg.m_pSpace = &m_space;
	if (!::IsWindow(dlg.m_hWnd))
	{
		dlg.Create(CSpaceScaleLinkWeightsDlg::IDD, ::AfxGetMainWnd());
	}
	else
	{
		// make sure data is up-to-date
		dlg.UpdateData(FALSE);
	}
	dlg.ShowWindow(SW_SHOW);
}

void CtheWheelDoc::OnSpacePerturblinkweights()
{
	// TODO: Add your command handler code here
}

// called when a node is removed from the space
void CtheWheelDoc::OnSpaceChanged(CObservableEvent * pEvent, void * pParam)
{
	SetModifiedFlag(TRUE);
}

void CtheWheelDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
	CDocument::SetPathName(lpszPathName, bAddToMRU);
	m_space.SetPathName(lpszPathName);
}

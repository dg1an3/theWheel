// SpaceBrowser.cpp : implementation file
//

#include "stdafx.h"
#include "thewheel2001.h"
#include "SpaceBrowser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpaceBrowser

IMPLEMENT_DYNCREATE(CSpaceBrowser, CHtmlView)

CSpaceBrowser::CSpaceBrowser()
: m_pSpaceView(NULL)
{
	//{{AFX_DATA_INIT(CSpaceBrowser)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CSpaceBrowser::~CSpaceBrowser()
{
}

void CSpaceBrowser::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpaceBrowser)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpaceBrowser, CHtmlView)
	//{{AFX_MSG_MAP(CSpaceBrowser)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpaceBrowser diagnostics

#ifdef _DEBUG
void CSpaceBrowser::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CSpaceBrowser::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSpaceBrowser message handlers

void CSpaceBrowser::OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel) 
{
	// store the URL being navigated to
	m_strPrevUrl = GetLocationURL();
	m_strNavigatingToUrl = lpszURL;
	
	CHtmlView::OnBeforeNavigate2(lpszURL, nFlags,	lpszTargetFrameName, baPostedData, lpszHeaders, pbCancel);
}

void CSpaceBrowser::OnNavigateComplete2(LPCTSTR strURL) 
{
	// completed the navigation, so now see if we should create a node in the space view
	CSpace *pSpace = m_pSpaceView->GetDocument();
/*	CNode *pNode = pSpace->GetNodeForUrl(m_strNavigatingToUrl);
	if (pNode)
	{
		pSpace->ActivateNode(pNode);
	}
	else
	{
		// otherwise, create the node
		pNode = new CNode(pSpace);
		
		// set the parent to the previous node
	}
*/
	CHtmlView::OnNavigateComplete2(strURL);
}

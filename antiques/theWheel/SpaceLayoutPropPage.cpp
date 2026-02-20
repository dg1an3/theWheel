// SpaceLayoutPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "theWheel.h"
#include "SpaceLayoutPropPage.h"


// CSpaceLayoutPropPage dialog

IMPLEMENT_DYNAMIC(CSpaceLayoutPropPage, CPropertyPage)

CSpaceLayoutPropPage::CSpaceLayoutPropPage()
	: CPropertyPage(CSpaceLayoutPropPage::IDD)
{

}

CSpaceLayoutPropPage::~CSpaceLayoutPropPage()
{
}

void CSpaceLayoutPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSpaceLayoutPropPage, CPropertyPage)
END_MESSAGE_MAP()


// CSpaceLayoutPropPage message handlers

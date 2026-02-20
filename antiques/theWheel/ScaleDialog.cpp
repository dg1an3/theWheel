// ScaleDialog.cpp : implementation file
//

#include "stdafx.h"
#include "theWheel.h"
#include "ScaleDialog.h"


// CScaleDialog dialog

IMPLEMENT_DYNAMIC(CScaleDialog, CDialog)
CScaleDialog::CScaleDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CScaleDialog::IDD, pParent)
{
}

CScaleDialog::~CScaleDialog()
{
}

void CScaleDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CScaleDialog, CDialog)
END_MESSAGE_MAP()


// CScaleDialog message handlers

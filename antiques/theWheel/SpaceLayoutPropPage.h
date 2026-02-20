#pragma once


// CSpaceLayoutPropPage dialog

class CSpaceLayoutPropPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSpaceLayoutPropPage)

public:
	CSpaceLayoutPropPage();
	virtual ~CSpaceLayoutPropPage();

// Dialog Data
	enum { IDD = IDD_SPACE_LAYOUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

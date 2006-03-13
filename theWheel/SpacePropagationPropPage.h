#pragma once


// CSpacePropagationPropPage dialog

class CSpacePropagationPropPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSpacePropagationPropPage)

public:
	CSpacePropagationPropPage();
	virtual ~CSpacePropagationPropPage();

// Dialog Data
	enum { IDD = IDD_SPACE_PROP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

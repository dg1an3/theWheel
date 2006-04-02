#pragma once

class CSpace;

// CSpaceScaleLinkWeightsDlg dialog

class CSpaceScaleLinkWeightsDlg : public CDialog
{
	DECLARE_DYNAMIC(CSpaceScaleLinkWeightsDlg)

public:
	CSpaceScaleLinkWeightsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSpaceScaleLinkWeightsDlg();

	// pointer to the space
	CSpace *m_pSpace;

public:
// Dialog Data
	enum { IDD = IDD_SPACE_SCALELINKWEIGHTS };

	// stores the current max link weight
	int m_currMaxLinkWgt;

	// stores new value for max link
	int m_newMaxLinkWgt;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
};

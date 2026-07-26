#pragma once


//////////////////////////////////////////////////////////////////////
// forward declaration of the CSpace class
//////////////////////////////////////////////////////////////////////
class CSpace;


// CSpaceLayoutPropPage dialog

class CSpaceLayoutPropPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSpaceLayoutPropPage)

public:
	CSpaceLayoutPropPage();
	virtual ~CSpaceLayoutPropPage();

// Dialog Data
	enum { IDD = IDD_SPACE_LAYOUT };

	// the space whose layout parameters this page edits; set by
	//		CtheWheelDoc before the sheet is created
	CSpace *m_pSpace;

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnChangeKPos();
	afx_msg void OnChangeKRep();
	afx_msg void OnChangeSuperNodes();
	afx_msg void OnChangeSpring();
	afx_msg void OnChangeTolerance();
	afx_msg void OnChangeGainCenter();
	afx_msg void OnChangeGainSteep();
	afx_msg void OnBnClickedRevert();

	DECLARE_MESSAGE_MAP()

private:
	// pushes the space's current parameters out to every control
	void LoadFromSpace();

	// applies the given edit box to the space, and moves its slider to match
	void ApplyEdit(UINT nEditID);

	// refreshes the energy readout
	void UpdateEnergy();

	// the sliders, which shadow the edit boxes beside them
	CSliderCtrl m_sliderKPos;
	CSliderCtrl m_sliderKRep;
	CSliderCtrl m_sliderSuperNodes;
	CSliderCtrl m_sliderSpring;
	CSliderCtrl m_sliderGainCenter;
	CSliderCtrl m_sliderGainSteep;

	// parameters as they stood when the page was first shown, for Revert;
	//		held as double so this header need not pull in the REAL typedef
	double m_kPosOrig;
	double m_kRepOrig;
	double m_springOrig;
	double m_toleranceOrig;
	double m_gainCenterOrig;
	double m_gainSteepOrig;
	int m_superNodesOrig;

	// set once the controls exist, so the EN_CHANGE handlers know the
	//		space is safe to write to
	BOOL m_bInitialized;

	// guards the EN_CHANGE handlers while we are writing the edit boxes
	//		ourselves, which would otherwise re-enter and fight the user
	BOOL m_bUpdating;
};

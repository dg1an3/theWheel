#pragma once

#include <Extent.h>

class CNodeView;

// CNodeLayoutManager

class CNodeLayoutManager : public CObject
{
public:
	CNodeLayoutManager();
	virtual ~CNodeLayoutManager();

public:
	// computes / returns inner rect extent
	CExtent<REAL> GetInnerRect(REAL activation);

	// returns next smaller scale layout manager
	CNodeLayoutManager * GetNextLayoutManager(void);
	
	// returns max act for this layout
	REAL GetMinLayoutSelection(void);

	// returns the title extent for given activation
	void CalcExtent(const CExtent<REAL>& rectInner,
		CNodeView *pView,
		REAL layoutSelection,
		CExtent<REAL>& rectTitle,
		CExtent<REAL>& rectImage,
		CExtent<REAL>& rectDesc);

	REAL GetTitleHeight(const CExtent<REAL>& rectInner);

	void CalcImageExtent(
		const CExtent<REAL>& rectInner,
		REAL titleHeight,
		CNodeView *pView,
		CExtent<REAL>& rectImage);

	void CalcTitleExtent(
		const CExtent<REAL>& rectInner,
		REAL titleHeight,
		const CExtent<REAL>& rectImage,
		CExtent<REAL>& rectTitle);

	void CalcDescExtent(
		const CExtent<REAL>& rectInner,
		const CExtent<REAL>& rectTitle,
		const CExtent<REAL>& rectImage,
		CExtent<REAL>& rectDesc);

// private:
	bool m_bShowDescription;

	REAL m_titleLogWeight;
	REAL m_maxTitleHeight;

	bool m_bImageBesideTitle;

	REAL m_maxImageWidth;
	REAL m_imageLogWeight;

	REAL m_maxDescWidth;

	REAL m_spacing;
public:
	// stores max layout selection value for this manager
	REAL m_minLayoutSelection;

	// next smaller layout manager
	CNodeLayoutManager *m_pNextNLM;
};
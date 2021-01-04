#pragma once

#include <VectorD.h>
#include <Extent.h>

class CNodeView;

// CNodeLayoutManager
//////////////////////////////////////////////////////////////////////////////
class CNodeLayoutManager : public CObject
{
public:
	CNodeLayoutManager();
	virtual ~CNodeLayoutManager();

	// computes / returns inner rect extent
	void GetInnerRect(REAL activation, CExtent<3,REAL>& extInner);

	// returns next smaller scale layout manager
	CNodeLayoutManager * GetNextLayoutManager(void);
	
	// when to use this layout?
	DECLARE_ATTRIBUTE(MinLayoutSelection, REAL);

	// title layout
	DECLARE_ATTRIBUTE(MaxTitleHeight, REAL);
	DECLARE_ATTRIBUTE(TitleLogWeight, REAL);

	// image layout
	DECLARE_ATTRIBUTE(ImageBesideTitle, bool);
	DECLARE_ATTRIBUTE(MaxImageWidth, REAL);
	DECLARE_ATTRIBUTE(ImageLogWeight, REAL);

	// description layout
	DECLARE_ATTRIBUTE(ShowDescription, bool);
	DECLARE_ATTRIBUTE(MaxDescWidth, REAL);

	// returns the title extent for given activation
	void CalcExtent(const CExtent<3,REAL>& rectInner,
		CNodeView *pView,
		REAL layoutSelection,
		CExtent<3,REAL>& rectTitle,
		CExtent<3,REAL>& rectImage,
		CExtent<3,REAL>& rectDesc);

	REAL GetTitleHeight(const CExtent<3,REAL>& rectInner);

	void CalcImageExtent(
		const CExtent<3,REAL>& rectInner,
		REAL titleHeight,
		CNodeView *pView,
		CExtent<3,REAL>& rectImage);

	void CalcTitleExtent(
		const CExtent<3,REAL>& rectInner,
		REAL titleHeight,
		const CExtent<3,REAL>& rectImage,
		CExtent<3,REAL>& rectTitle);

	void CalcDescExtent(
		const CExtent<3,REAL>& rectInner,
		const CExtent<3,REAL>& rectTitle,
		const CExtent<3,REAL>& rectImage,
		CExtent<3,REAL>& rectDesc);

private:
	// next smaller layout manager
	CNodeLayoutManager *m_pNextNLM;

	// spacing for elements
	REAL m_spacing;
};

// NodeLayoutManager.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>

#include <Dib.h>

#include <NodeLayoutManager.h>
#include <NodeView.h>


// CNodeLayoutManager

CNodeLayoutManager::CNodeLayoutManager()
: m_pNextNLM(NULL),
	m_titleLogWeight(0.0),
	m_imageLogWeight(0.0),
	m_spacing(5)

{
}

CNodeLayoutManager::~CNodeLayoutManager()
{
}

// returns max act for this layout
REAL CNodeLayoutManager::GetMinLayoutSelection(void)
{
	return m_minLayoutSelection;
}

// returns next smaller scale layout manager
CNodeLayoutManager * CNodeLayoutManager::GetNextLayoutManager(void)
{
	if (m_pNextNLM == NULL)
	{
		// init as duplicate, except max will be 0.0
		m_pNextNLM = new CNodeLayoutManager();
		m_pNextNLM->m_minLayoutSelection = 0.0;
		m_pNextNLM->m_bImageBesideTitle = m_bImageBesideTitle;
		m_pNextNLM->m_bShowDescription = m_bShowDescription;
		m_pNextNLM->m_maxTitleHeight = m_maxTitleHeight;
		m_pNextNLM->m_titleLogWeight = m_titleLogWeight;
		m_pNextNLM->m_maxImageWidth = m_maxImageWidth;
		m_pNextNLM->m_imageLogWeight = m_imageLogWeight;
		m_pNextNLM->m_maxDescWidth = m_maxDescWidth;  
	}

	return m_pNextNLM;
}

// returns the title extent for given activation
void CNodeLayoutManager::CalcExtent(const CExtent<REAL>& rectInner,
		CNodeView *pView,
		REAL layoutSelection,
		CExtent<REAL>& rectTitle,
		CExtent<REAL>& rectImage,
		CExtent<REAL>& rectDesc)
{
	if (layoutSelection > GetNextLayoutManager()->GetMinLayoutSelection())
	{
		// calculate my extents
		REAL titleHeight = GetTitleHeight(rectInner);
		CalcImageExtent(rectInner, titleHeight, pView, rectImage);
		CalcTitleExtent(rectInner, titleHeight, rectImage, rectTitle);
		CalcDescExtent(rectInner, rectTitle, rectImage, rectDesc);

		if (layoutSelection < GetMinLayoutSelection())
		{
			CExtent<REAL> rectTitleSub;
			CExtent<REAL> rectImageSub;
			CExtent<REAL> rectDescSub;
			GetNextLayoutManager()->CalcExtent(rectInner, pView, layoutSelection, 
				rectTitleSub, rectImageSub, rectDescSub);

			// now interpolate based on layoutSelection value
			REAL frac = (layoutSelection 
					- GetNextLayoutManager()->GetMinLayoutSelection())
				/ (GetMinLayoutSelection() 
					- GetNextLayoutManager()->GetMinLayoutSelection());

			rectTitle.Interpolate(rectTitleSub, frac);
			rectImage.Interpolate(rectImageSub, frac);
			rectDesc.Interpolate(rectDescSub, frac);
		}
	}
	else
	{
		// otherwise just call sub to return values
		GetNextLayoutManager()->CalcExtent(rectInner, pView, layoutSelection, 
			rectTitle, rectImage, rectDesc);
	}
}

REAL CNodeLayoutManager::GetTitleHeight(const CExtent<REAL>& rectInner)
{
	REAL titleHeight = __max(rectInner.Height(), 0.0);
	if (m_bShowDescription)
	{
		titleHeight = m_titleLogWeight * log(titleHeight + 1.0);
	}
	titleHeight = __min(titleHeight, m_maxTitleHeight);

	return titleHeight;
}

// returns the title extent for given activation
void CNodeLayoutManager::CalcImageExtent(
		const CExtent<REAL>& rectInner,
		REAL titleHeight,
		CNodeView *pView,
		CExtent<REAL>& rectImage)
{
	rectImage = rectInner;

	// add a log value, if requested
	rectImage.SetTop(rectImage.Top() 
		+ m_imageLogWeight * log(__max(rectInner.Height(), 0.0)));

	// position top
	if (!m_bImageBesideTitle)
	{
		rectImage.SetTop(rectImage.Top() + titleHeight + m_spacing);
	}

	// determine max height from max width
	CSize sz(0, 0);
	REAL widthToHeight = 1.0;
	CDib *pDib = pView->GetNode()->GetDib();
	if (pDib)
	{
		sz = pView->GetNode()->GetDib()->GetBitmapSize();

		// compute ratio
		widthToHeight = (REAL) sz.cx / (REAL) sz.cy;

		// ensure width <= height
		widthToHeight = __max(widthToHeight, 1.0);
	}
	else
	{
		rectImage.SetWidth(0.0);
		return;
	}

	// cutoff at max width
	REAL imageWidth = __min(rectImage.Height() * widthToHeight, m_maxImageWidth);
	imageWidth = __min(imageWidth, rectInner.Width() / 3.0);
	rectImage.SetWidth(imageWidth);

	// make proportional, if it is not on title bar
	if (!m_bImageBesideTitle)
	{
		widthToHeight = (REAL) sz.cx / (REAL) sz.cy;
		rectImage.SetHeight(imageWidth / widthToHeight);
	}
}

// returns the title extent for given activation
void CNodeLayoutManager::CalcTitleExtent(
		const CExtent<REAL>& rectInner,
		REAL titleHeight,
		const CExtent<REAL>& rectImage,
		CExtent<REAL>& rectTitle)
{
	rectTitle = rectInner;

	// reposition if image beside title 
	if (m_bImageBesideTitle 
		&& rectImage.Width() > 0.0)
	{
		rectTitle.SetLeft(rectImage.Right() + m_spacing);
	}
	else if (rectImage.Width() > 0.0)
	{
		// aligns to half-way over image, to make more room
		rectTitle.SetLeft(rectImage.Right() + 
			(rectTitle.Left() - rectImage.Right()) / 2.0);
	};

	rectTitle.SetHeight(titleHeight);
}

void CNodeLayoutManager::CalcDescExtent(
		const CExtent<REAL>& rectInner,
		const CExtent<REAL>& rectTitle,
		const CExtent<REAL>& rectImage,
		CExtent<REAL>& rectDesc)
{
	rectDesc = rectInner;

	rectDesc.SetTop(__min(rectTitle.Bottom() + m_spacing, rectDesc.Bottom()));
	if (rectImage.Width() > 0.0)
	{
		rectDesc.SetLeft(rectImage.Right() + m_spacing);
	}

	REAL descWidth = rectDesc.Width(); //  - (rectImage.Width() + m_spacing);
	descWidth = __min(descWidth, m_maxDescWidth);
	rectDesc.SetWidth(descWidth);
}

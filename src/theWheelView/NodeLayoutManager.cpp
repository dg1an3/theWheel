// NodeLayoutManager.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>

#include <Dib.h>

#include <NodeLayoutManager.h>
#include <NodeView.h>


// CNodeLayoutManager

//////////////////////////////////////////////////////////////////////////////
CNodeLayoutManager::CNodeLayoutManager()
	: m_pNextNLM(NULL)
	, m_TitleLogWeight(0.0)
	, m_ImageLogWeight(0.0)
	, m_spacing(5)
{
}	// CNodeLayoutManager::CNodeLayoutManager

//////////////////////////////////////////////////////////////////////////////
CNodeLayoutManager::~CNodeLayoutManager()
{
}	// CNodeLayoutManager::~CNodeLayoutManager

//////////////////////////////////////////////////////////////////////////////
CNodeLayoutManager * 
	CNodeLayoutManager::GetNextLayoutManager(void)
	// returns next smaller scale layout manager
{
	if (m_pNextNLM == NULL)
	{
		// init as duplicate, except max will be 0.0
		m_pNextNLM = new CNodeLayoutManager();
		m_pNextNLM->SetMinLayoutSelection(0.0);
		m_pNextNLM->SetImageBesideTitle(GetImageBesideTitle());
		m_pNextNLM->SetShowDescription(GetShowDescription());
		m_pNextNLM->SetMaxTitleHeight(GetMaxTitleHeight());
		m_pNextNLM->SetTitleLogWeight(GetTitleLogWeight());
		m_pNextNLM->SetMaxImageWidth(GetMaxImageWidth());
		m_pNextNLM->SetImageLogWeight(GetImageLogWeight());
		m_pNextNLM->SetMaxDescWidth(GetMaxDescWidth());  
	}

	return m_pNextNLM;

}	// CNodeLayoutManager::GetNextLayoutManager

//////////////////////////////////////////////////////////////////////////////
void 
	CNodeLayoutManager::CalcExtent(const CExtent<3,REAL>& rectInner,
		CNodeView *pView,
		REAL layoutSelection,
		CExtent<3,REAL>& rectTitle,
		CExtent<3,REAL>& rectImage,
		CExtent<3,REAL>& rectDesc)
	// returns the extents for given activation
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
			CExtent<3,REAL> rectTitleSub;
			CExtent<3,REAL> rectImageSub;
			CExtent<3,REAL> rectDescSub;
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

}	// CNodeLayoutManager::CalcExtent

//////////////////////////////////////////////////////////////////////////////
REAL 
	CNodeLayoutManager::GetTitleHeight(const CExtent<3,REAL>& rectInner)
{
	// calculate title height
	REAL titleHeight = __max(rectInner.GetSize(1), 0.0f);

	// is description enabled?
	if (GetShowDescription())
	{
		// adjust for log title height
		titleHeight = GetTitleLogWeight() * logf(titleHeight + 1.0f);
	}
	// TRACE("titleHeight = %f\n", titleHeight);

	// adjust for max title height
	titleHeight = __min(titleHeight, GetMaxTitleHeight());
	// titleHeight = __max(4.0, titleHeight);

	return titleHeight;

}	// CNodeLayoutManager::GetTitleHeight

//////////////////////////////////////////////////////////////////////////////
void 
	CNodeLayoutManager::CalcImageExtent(
		const CExtent<3,REAL>& rectInner,
		REAL titleHeight,
		CNodeView *pView,
		CExtent<3,REAL>& rectImage)
	// returns the image extent for given activation
{
	// initialize to inner rectangle
	rectImage = rectInner;

	// add a log value, if one set
	rectImage.SetMin(1, rectImage.GetMin(1) 
		+ GetImageLogWeight() * logf(__max(rectInner.GetSize(1), 0.0f)));

	// position top, based on whether image is below or beside title
	if (!GetImageBesideTitle())
	{
		rectImage.SetMin(1, rectImage.GetMin(1) + titleHeight + m_spacing);
	}

	// default ratio = 1/1
	REAL widthToHeight = 1.0f;

	// determine max height from max width
	CDib *pDib = pView->GetNode()->GetDib();
	if (pDib)
	{
		// get default size of image
		CSize sz = pDib->GetBitmapSize();

		// compute ratio
		widthToHeight = (REAL) sz.cx / (REAL) sz.cy;

		// ensure width <= height
		widthToHeight = __max(widthToHeight, 1.0f);
	}
	else
	{
		// no image present, set size to zero
		rectImage.SetSize(0, 0.0);
		return;
	}

	// calc image width as proportional to height; cutoff at max width
	REAL imageWidth = __min(rectImage.GetSize(1) * widthToHeight, GetMaxImageWidth());

	// adjust for max (based on inner rectangle width)
	imageWidth = __min(imageWidth, rectInner.GetSize(0) / 3.0f);

	// and set the rectangle size
	rectImage.SetSize(0, imageWidth, FIX_MIN);

	// make proportional, if it is not on title bar
	if (!GetImageBesideTitle())
	{
		rectImage.SetSize(1, imageWidth / widthToHeight, FIX_MIN);
	}

}	// CNodeLayoutManager::CalcImageExtent


//////////////////////////////////////////////////////////////////////////////
void 
	CNodeLayoutManager::CalcTitleExtent(
		const CExtent<3,REAL>& rectInner,
		REAL titleHeight,
		const CExtent<3,REAL>& rectImage,
		CExtent<3,REAL>& rectTitle)
	// returns the title extent for given activation
{
	// initialize to inner rectangle
	rectTitle = rectInner;

	// reposition if image beside title 
	if (rectImage.GetSize(0) > 0.0)
	{
		if (GetImageBesideTitle())
		{
			// set right edge to left edge of image + spacing
			rectTitle.SetMin(0, rectImage.GetMax(0) + m_spacing);
		}
		else 
		{
			// aligns to half-way over image, to make more room
			rectTitle.SetMin(0, rectImage.GetMax(0) + 
				(rectTitle.GetMin(0) - rectImage.GetMax(0)) / 2.0f);
		}
	}
	
	// now set title height
	rectTitle.SetSize(1, titleHeight, FIX_MIN);

}	// CNodeLayoutManager::CalcTitleExtent

//////////////////////////////////////////////////////////////////////////////
void 
	CNodeLayoutManager::CalcDescExtent(
		const CExtent<3,REAL>& rectInner,
		const CExtent<3,REAL>& rectTitle,
		const CExtent<3,REAL>& rectImage,
		CExtent<3,REAL>& rectDesc)
{
	// description is initialized to inner rect
	rectDesc = rectInner;

	// adjust for title height
	rectDesc.SetMin(1, __min(rectTitle.GetMax(1) + m_spacing, rectDesc.GetMax(1)));

	// if image rect is there,
	if (rectImage.GetSize(0) > 0.0)
	{
		// adjust right border
		rectDesc.SetMin(0, rectImage.GetMax(0) + m_spacing);
	}

	// calculate width of description, accounting for max allowed
	REAL descWidth = __min(rectDesc.GetSize(0), GetMaxDescWidth());

	// set max (not size) because we want to preserve the min border
	rectDesc.SetMax(0, rectDesc.GetMin(0) + descWidth);

}	// CNodeLayoutManager::CalcDescExtent


///////////////////////////////////////////////////////////////////////////////
// Eevorg.cpp: implementation of the CEevorg class.
//
// Copyright (C) 1996-2002 Derek G. Lane
// $Id$
// U.S. Patent Pending
///////////////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// class declaration
#include "Eevorg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// helper macro for random numbers
///////////////////////////////////////////////////////////////////////////////
#define random(max) (int)((long)rand()*((long)(max))/(long)RAND_MAX)

///////////////////////////////////////////////////////////////////////////////
// construction/destruction
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CEevorg::CEevorg
// 
// constructs a default CEevorg
///////////////////////////////////////////////////////////////////////////////
CEevorg::CEevorg(CSpace *pSpace)
	: CNode(pSpace),
		m_nMaxRule(0),
		m_nMaxValue(0)
{
	// set the parent to none
	SetParent((CEevorg *) NULL); 

	// set up the rules and generations
	InitRulesGen();

}	// CEevorg::CEevorg


///////////////////////////////////////////////////////////////////////////////
// CEevorg::CEevorg
// 
// constructs a CEevorg object with the given parent, mutating if requested
///////////////////////////////////////////////////////////////////////////////
CEevorg::CEevorg(CSpace *pSpace, const CNode *pNodeParent, BOOL bMutate)
	: CNode(pSpace),
		m_nMaxRule(0),
		m_nMaxValue(0)
{	
	// set the parent 
	SetParent((CNode *) pNodeParent);
	
	// set up the rules and generations
	InitRulesGen();

	// inherit m_arrRuless from parent, if parent is CEevorg
	if (pNodeParent->IsKindOf(RUNTIME_CLASS(CEevorg)))
	{
		CEevorg *pParent = (CEevorg *) pNodeParent;

		ASSERT(pParent->m_nMaxRule >= 0);

		// copy the parents rules
		m_arrRules.Copy(pParent->m_arrRules);
		m_nMaxRule = pParent->m_nMaxRule;
		m_nMaxValue = pParent->m_nMaxValue;

		// mutate a rule
		if (bMutate)
		{	
			Mutate();
		}	
	}
	
}	// CEevorg::CEevorg


///////////////////////////////////////////////////////////////////////////////
// CEevorg::~CEevorg
// 
// destroys the Eevorg
///////////////////////////////////////////////////////////////////////////////
CEevorg::~CEevorg()
{
	// delete the generations
	for (int nAt = 0; nAt < m_arrGen.GetSize(); nAt++)
	{
		delete m_arrGen[nAt];
	}

}	// CEevorg::~CEevorg


///////////////////////////////////////////////////////////////////////////////
// Serialization Support
///////////////////////////////////////////////////////////////////////////////
IMPLEMENT_SERIAL(CEevorg, CNode, 0)

///////////////////////////////////////////////////////////////////////////////
// CEevorg::GetGenCount
// 
// returns the number of computed generations
///////////////////////////////////////////////////////////////////////////////
int CEevorg::GetGenCount()  
{ 
	return m_arrGen.GetSize(); 

}	// CEevorg::GetGenCount


///////////////////////////////////////////////////////////////////////////////
// CEevorg::GetGenAt
// 
// retrieves the given generation, generating it if necessary
///////////////////////////////////////////////////////////////////////////////
CByteArray *CEevorg::GetGenAt(int nAt) 
{ 
	// make sure enough generations are computed
	while (nAt >= GetGenCount())
	{
		NextGen();
	}

	return (CByteArray *) m_arrGen.GetAt(nAt); 

}	// CEevorg::GetGenAt


///////////////////////////////////////////////////////////////////////////////
// CEevorg::DrawAt
// 
// draws the Eevorg on the given DC
///////////////////////////////////////////////////////////////////////////////
void CEevorg::DrawAt(CDC* pDC, int nAtX, int nAtY, int nHeight)
{   
	if (nHeight < 2)
	{
		return;
	}

	// compute bitmap size
	const int nZoom = 1;
	int nPixelHeight = nHeight / nZoom;
	int nPixelWidth = nPixelHeight;

	// create the bitmap
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, nPixelWidth, nPixelHeight);
	bitmap.SetBitmapBits(nPixelWidth * nPixelHeight * 4, 
		GetBitmapPixels(nPixelHeight)->GetData());

	// create the memory DC
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap *pOldBitmap = memDC.SelectObject(&bitmap);

	// stretchblt
	pDC->SetStretchBltMode(HALFTONE);
	pDC->StretchBlt(nAtX - nHeight / 2, nAtY, nHeight, nHeight, 
		&memDC, 0, 0, nPixelWidth, nPixelHeight, SRCCOPY);

	// clean up DC
	memDC.SelectObject(pOldBitmap);
	memDC.DeleteDC();

	// clean up bitmap
	bitmap.DeleteObject();

}	// CEevorg::DrawAt


///////////////////////////////////////////////////////////////////////////////
// CEevorg::Serialize
// 
// input/output of the Eevorg
///////////////////////////////////////////////////////////////////////////////
void CEevorg::Serialize(CArchive& ar)
{
	CNode::Serialize(ar);

	if (ar.IsStoring())
	{	
		// write member variables to archive
		ar << (WORD) m_nMaxValue;
		ar << (WORD) m_nMaxRule;

		m_arrRules.Serialize(ar);
	} 
	else
	{	
		// read member variables from archive
		ar >> (WORD&) m_nMaxValue;
		ar >> (WORD&) m_nMaxRule;

		m_arrRules.Serialize(ar);
	}

}	// CEevorg::Serialize


///////////////////////////////////////////////////////////////////////////////
// CEevorg diagnostics

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////////
// CEevorg::AssertValid
// 
// asserts the validity of the object
///////////////////////////////////////////////////////////////////////////////
void CEevorg::AssertValid() const
{   
	ASSERT((m_nMaxRule > 0) && (m_nMaxRule < m_arrRules.GetSize()));
	CObject::AssertValid();

}   // CEevorg::AssertValid


///////////////////////////////////////////////////////////////////////////////
// CEevorg::Dump
// 
// dumps the object to a dump context
///////////////////////////////////////////////////////////////////////////////
void CEevorg::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

}	// CEevorg::Dump

#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////
// CEevorg commands

///////////////////////////////////////////////////////////////////////////////
// CEevorg::InitRulesGen
// 
// intializes the rules and generations for the Eevorg
///////////////////////////////////////////////////////////////////////////////
void CEevorg::InitRulesGen()
{
	// set up the rules
	m_arrRules.Add(0); 
	m_arrRules.Add(2); 
	m_arrRules.Add(0); 
	m_arrRules.Add(1); 
	m_arrRules.Add(1);
	m_arrRules.Add(3); 
	m_arrRules.Add(2); 
	m_arrRules.Add(1); 
	m_arrRules.Add(1); 
	m_arrRules.Add(0);
	m_nMaxRule = m_arrRules.GetSize()-1;
	m_nMaxValue = 3;		
	
	// set up the first generation
	CByteArray* arrFirstGen = new CByteArray;
	arrFirstGen->Add(1);
	m_arrGen.Add(arrFirstGen);

}	// CEevorg::InitRulesGen


///////////////////////////////////////////////////////////////////////////////
// CEevorg::Mutate
// 
// introduces a point mutation in the Eevorg
///////////////////////////////////////////////////////////////////////////////
void CEevorg::Mutate()
{
	// determine the rule to change
	int nRuleNum = random(m_nMaxRule + 1);

	// retrieve the rule value
	int nRuleVal;
	if (nRuleNum > m_nMaxRule)
	{
		nRuleVal = 0;
		m_nMaxRule = nRuleNum;
	} 
	else
	{
		ASSERT(nRuleNum < m_arrRules.GetSize());
		nRuleVal = m_arrRules.GetAt(nRuleNum);
	}
		
	// compute the new rule value
	nRuleVal++;
	nRuleVal = (nRuleVal > 0) ? nRuleVal : 0;

	// add rules if we are over the top
	while (nRuleVal > m_nMaxValue)
	{
		// create three new m_arrRuless
		m_arrRules.Add(0);
		m_arrRules.Add(0);
		m_arrRules.Add(0);
		m_nMaxValue++;
	}

	// set the new rule
	m_arrRules.SetAt(nRuleNum, nRuleVal);

}	// CEevorg::Mutate


///////////////////////////////////////////////////////////////////////////////
// CEevorg::NextGen
// 
// computes the next generation
///////////////////////////////////////////////////////////////////////////////
int CEevorg::NextGen()
{	
	// compute next generation and return
	CByteArray *arrNext = new CByteArray; 
	int nNextNdx = m_arrGen.Add(arrNext);
	CByteArray *arrPrev = 
		(CByteArray *)m_arrGen.GetAt(nNextNdx-1);
	
	if (arrPrev->GetSize() == 1)
	{
		arrNext->Add(m_arrRules.GetAt(arrPrev->GetAt(0)));
		arrNext->Add(m_arrRules.GetAt(arrPrev->GetAt(0)));
	} 
	else
	{
		arrNext->Add(m_arrRules.GetAt(
			arrPrev->GetAt(0)
			+ 2 * arrPrev->GetAt(1)));

		for (int n = 1; n < arrPrev->GetSize()-1; n++)
		{
			arrNext->Add(m_arrRules.GetAt(
				arrPrev->GetAt(n-1)
				+ arrPrev->GetAt(n)
				+ arrPrev->GetAt(n + 1)));
		}

		arrNext->Add(m_arrRules.GetAt(
			arrPrev->GetAt(n-1)
			+ arrPrev->GetAt(n)));
		arrNext->Add(m_arrRules.GetAt(arrPrev->GetAt(n)));
	}
	
	return nNextNdx;

}	// CEevorg::NextGen


///////////////////////////////////////////////////////////////////////////////
// CEevorg::GetBitmapPixels
// 
// either computes or retrieves a cached version of pixels for the given
//		bitmap height
///////////////////////////////////////////////////////////////////////////////
CArray<COLORREF, COLORREF> *CEevorg::GetBitmapPixels(int nBitmapHeight)
{
	if (m_arrBitmapPixels.GetSize()-1 < nBitmapHeight
		|| NULL == m_arrBitmapPixels[nBitmapHeight])
	{
		// compute the bitmap width
		int nBitmapWidth = nBitmapHeight;

		// create the pixel array
		CArray<COLORREF, COLORREF> *pArrPixels = 
			new CArray<COLORREF, COLORREF>;
		pArrPixels->SetSize(nBitmapHeight * nBitmapWidth);

		// initialize with background color
		for (int nAt = 0; nAt < pArrPixels->GetSize(); nAt++)
		{
			(*pArrPixels)[nAt] = RGB(232, 232, 232);
		}

		// iterate over generations
		for (int nAtGen = 0; nAtGen < nBitmapHeight; nAtGen++)
		{ 
			// get the current generation
			CByteArray *pGen = GetGenAt(nAtGen);

			// iterate over cells in generation
			for (int nAtCell = pGen->GetSize()-1; nAtCell > 0; nAtCell--)
			{   
				// compute the x position for the current cell
				int nAtX = (double) nAtCell 
					* (1.0 - (double)(nAtGen * nAtGen * nAtGen)
						/ (double)(nBitmapHeight * nBitmapHeight * nBitmapHeight));
				
				// compute the color based on the current cell value
				int nColorBase = pGen->GetAt(nAtCell / 2) * 128 / m_nMaxValue;
				COLORREF color = RGB(nColorBase * 4, nColorBase * 2, nColorBase);

				// assign cell value, using symmetry
				(*pArrPixels)[nBitmapWidth / 2 + nAtX + nAtGen * nBitmapWidth] = color;
				(*pArrPixels)[nBitmapWidth / 2 - nAtX + nAtGen * nBitmapWidth] = color;
			}
		}

		// add to the uber-array
		m_arrBitmapPixels.SetAtGrow(nBitmapHeight, pArrPixels);
	}

	// return the array of pixels
	return (CArray<COLORREF, COLORREF> *) m_arrBitmapPixels[nBitmapHeight];

}	// CEevorg::GetBitmapPixels



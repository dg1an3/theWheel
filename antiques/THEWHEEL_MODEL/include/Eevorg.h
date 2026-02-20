//////////////////////////////////////////////////////////////////////
// Eevorg.h: interface for the CEevorg class.
//
// Copyright (C) 1996-2002 Derek G. Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#ifndef EEVORG_H
#define EEVORG_H

// base class
#include "Node.h"

// forward declare the CSpace object
class CSpace;

//////////////////////////////////////////////////////////////////////
// class CEevorg
// 
// an Eevorg is a genetic cellular automaton.  Eevorg can be spawned
//		from parents, and their rules can be mutated
//////////////////////////////////////////////////////////////////////
class CEevorg : public CNode
{
public:
	// constructor/destructor
	CEevorg(CSpace *pSpace = NULL);
	CEevorg(CSpace *pSpace, const CNode *fromParent, BOOL bMutate);
	virtual ~CEevorg();

	// serialization support
	DECLARE_SERIAL(CEevorg)
	
	// accessors for generations
	int GetGenCount();  
    CByteArray *GetGenAt(int nIndex);

	// drawing function
	void DrawAt(CDC *pDC, int atX, int atY, int hgt);

	// overridden for serialization support
	virtual void Serialize(CArchive& ar);	
	
#ifdef _DEBUG
	virtual	void AssertValid() const;
	virtual	void Dump(CDumpContext& dc) const;
#endif

protected:
	// initializes the rules and generations
	void InitRulesGen();

	// mutates one rule
	void Mutate();

	// compute next generation 
    int NextGen();	

	// returns the pixels for the given bitmap height
	CArray<COLORREF, COLORREF> *GetBitmapPixels(int nHeight);

private:
	// holds the rules for the eevorg
	CByteArray m_arrRules;

	// maximum rule number
	int m_nMaxRule;

	// successive generations
	CObArray m_arrGen;

	// maximum value
	int m_nMaxValue;

	// bitmap pixels
	CObArray m_arrBitmapPixels;
};

#endif
/////////////////////////////////////////////////////////////////////////////

// SSELayoutManager.h: interface for the CSSELayoutManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSELAYOUTMANAGER_H__B9E239F3_FF26_410E_8DAC_0F59DCACB005__INCLUDED_)
#define AFX_SSELAYOUTMANAGER_H__B9E239F3_FF26_410E_8DAC_0F59DCACB005__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SpaceLayoutManager.h"

class CSSELayoutManager : public CSpaceLayoutManager  
{
public:
	CSSELayoutManager(CSpace *pSpace);
	virtual ~CSSELayoutManager();

	// loads the links and sizes for quick access
	virtual void LoadSizesLinks();

	// evaluates the energy function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL);


};

#endif // !defined(AFX_SSELAYOUTMANAGER_H__B9E239F3_FF26_410E_8DAC_0F59DCACB005__INCLUDED_)

//////////////////////////////////////////////////////////////////////
// LineFunction.cpp: implementation of the LineFunction
//
// Copyright (C) 1996-2001
// $Id: LineFunction.cpp,v 1.5 2005/03/19 20:02:49 HP_Administrator Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "LineFunction.h"

//////////////////////////////////////////////////////////////////////
// LineFunction::LineFunction
// 
// construct a new line function object, given an DIM-dimensional
//		objective function
//////////////////////////////////////////////////////////////////////
LineFunction::LineFunction(ObjectiveFunction *pProjFunc)
	: ObjectiveFunction(pProjFunc->HasGradientInfo()),
		m_pProjFunc(pProjFunc)
{
}

//////////////////////////////////////////////////////////////////////
// LineFunction::GetPoint
// 
// returns the point on the line
//////////////////////////////////////////////////////////////////////
const VectorN<>& LineFunction::GetPoint() const
{
	return m_vPoint;
}

//////////////////////////////////////////////////////////////////////
// LineFunction::GetDirection
// 
// returns the direction of the line
//////////////////////////////////////////////////////////////////////
const VectorN<>& LineFunction::GetDirection() const
{
	return m_vDirection;
}

//////////////////////////////////////////////////////////////////////
// LineFunction::SetLine
// 
// sets the line parameters
//////////////////////////////////////////////////////////////////////
void LineFunction::SetLine(const VectorN<>& vPoint, 
							const VectorN<>& vDir)
{
	ASSERT(vPoint.GetDim() == vDir.GetDim());

	m_vPoint.SetDim(vPoint.GetDim());
	m_vPoint = vPoint;
	m_vDirection.SetDim(vDir.GetDim());
	m_vDirection = vDir;
}

//////////////////////////////////////////////////////////////////////
// LineFunction::operator()
// 
// evaluates the line function
//////////////////////////////////////////////////////////////////////
REAL LineFunction::operator()(const VectorN<>& vInput, 
							   VectorN<> *pGrad) const
{
	// ensure this is a one-dimensional input vector
	ASSERT(vInput.GetDim() == 1);

	if (pGrad)
	{
		// set dimension of intermediate gradient vector
		m_vGrad.SetDim(GetPoint().GetDim());
	}

	// form the point at which evaluation is to occur
	// m_vEvalPoint = GetPoint() + vInput[0] * GetDirection();
	m_vEvalPoint.SetDim(m_vDirection.GetDim());
	m_vEvalPoint = GetDirection();
	m_vEvalPoint *= vInput[0];
	m_vEvalPoint += GetPoint();

	// evaluate the function and gradient, if needed
	REAL value = (*m_pProjFunc)(m_vEvalPoint, pGrad ? &m_vGrad : NULL);

	// assign
	if (pGrad)
	{
		// set the dimension
		pGrad->SetDim(1);

		// and assign (dot product of gradient and direction)
		(*pGrad)[0] = m_vGrad * GetDirection();
	}

	return value;
}

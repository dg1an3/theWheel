//////////////////////////////////////////////////////////////////////
// LineFunction.h: interface for the LineFunction
//
// Copyright (C) 1996-2001
// $Id: LineFunction.h,v 1.3 2004/02/16 15:56:09 default Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(LINEFUNCTION_H)
#define LINEFUNCTION_H

// base class
#include "ObjectiveFunction.h"

//////////////////////////////////////////////////////////////////////
// class LineFunction
// 
// defines a line function given another objective function and
//		a line in the vector-space domain of that objective function
//////////////////////////////////////////////////////////////////////
class LineFunction : public ObjectiveFunction
{
public:
	// construct a new line function object, given an DIM-dimensional
	//		objective function
	LineFunction(ObjectiveFunction *pProjFunc);

	// returns the point on the line
	const VectorN<>& GetPoint() const;

	// returns the direction of the line
	const VectorN<>& GetDirection() const;

	// sets the line parameters
	void SetLine(const VectorN<>& vPoint, const VectorN<>& vDir);

	// evaluates the line function
	virtual REAL operator()(const VectorN<>& vInput,
		VectorN<> *pGrad = NULL) const;

private:
	// pointer to the objective function upon which this line function
	//		is based
	ObjectiveFunction *m_pProjFunc;

	// stores a point on the line
	VectorN<> m_vPoint;

	// stores the direction of the line
	VectorN<> m_vDirection;

	// temporary store of evaluation point
	mutable VectorN<> m_vEvalPoint;

	// stores the gradient at the last evaluated point
	mutable VectorN<> m_vGrad;
};

// Backward compatibility
using CLineFunction = LineFunction;

#endif

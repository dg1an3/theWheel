//////////////////////////////////////////////////////////////////////
// PowellOptimizer.h: interface for the PowellOptimizer 
//
// Copyright (C) 1996-2001
// $Id: PowellOptimizer.h,v 1.2 2004/03/28 22:35:09 default Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(POWELLOPTIMIZER_H)
#define POWELLOPTIMIZER_H

#include <MatrixNxM.h>

// optimizer base class
#include "Optimizer.h"

// the underlying Brent optimizer
#include "BrentOptimizer.h"

// the line function for the Brent optimizer
#include "LineFunction.h"

//////////////////////////////////////////////////////////////////////
// class PowellOptimizer
// 
// optimizer that implements the Powell algorithm explained in
//		Numerical Recipes
//////////////////////////////////////////////////////////////////////
class PowellOptimizer : public Optimizer
{
public:
	// construct a new Powell optimizer
	PowellOptimizer(ObjectiveFunction *pFunc);
	virtual ~PowellOptimizer();

	// performs the optimization
	virtual const VectorN<>& Optimize(const VectorN<>& vInit);

protected:
	// helper functions for optimization
	int IterateOverDirectionSet(REAL& del);
	void FindNewDirection(REAL fp, REAL fptt, REAL del, int nBigDir);

private:
	// line function that projects the objective function along a given line
	LineFunction m_lineFunction;

	// brent optimizer along the line function
	BrentOptimizer m_optimizeBrent;

	// the current direction set for the optimization
	MatrixNxM<> m_mDirections;

	// "locals" for the optimization, declared members to avoid initializing
	VectorN<> m_vScaledDir;
	VectorN<> m_vPt;
	VectorN<> m_vPtt;
	VectorN<> m_vXit;
};

// Backward compatibility
using CPowellOptimizer = PowellOptimizer;

#endif

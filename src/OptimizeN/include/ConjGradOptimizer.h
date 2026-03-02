//////////////////////////////////////////////////////////////////////
// ConjGradOptimizer.h: interface for the ConjGradOptimizer
//
// Copyright (C) 1996-2001
// $Id: ConjGradOptimizer.h,v 1.10 2007/05/09 01:45:20 Derek Lane Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(CONJGRADOPTIMIZER_H)
#define CONJGRADOPTIMIZER_H

// macros for attribute definition
// #include <Attributes.h>

// base class includes
#include "Optimizer.h"

// subordinate brent optimizer
#include "BrentOptimizer.h"

// the line function for the Brent optimizer
#include "LineFunction.h"


//////////////////////////////////////////////////////////////////////
// class ConjGradOptimizer
// 
// optimizer that implements the Powell algorithm explained in
//		Numerical Recipes
//////////////////////////////////////////////////////////////////////
class ConjGradOptimizer : public Optimizer
{
public:
	// construct a gradient optimizer for an objective function
	ConjGradOptimizer(ObjectiveFunction *pFunc);

	// returns a reference to the embedded Brent optimizer
	BrentOptimizer& GetBrentOptimizer();

	// optimize the objective function
	virtual const VectorN<>& Optimize(const VectorN<>& vInit);

	// flag to indicate that line opt tolerance should always be same
	DECLARE_ATTRIBUTE(LineToleranceEqual, bool);

	// used to set up the variance min / max calculation
	void SetAdaptiveVariance(bool bCalcVar, REAL varMin, REAL varMax);

private:
	// line function that projects the objective function along 
	//		a given line
	LineFunction m_lineFunction;

	// points to the line optimizer to be used
	Optimizer *m_pLineOptimizer;

	// brent optimizer along the line function
	BrentOptimizer m_optimizeBrent;

	// "statics" for the optimization routine
	VectorN<> m_vGrad;
	VectorN<> m_vGradPrev;
	VectorN<> m_vDir;
	VectorN<> m_vDirPrev;
	VectorN<> m_vLambdaScaled;

	// flag to indicate adaptive variance calculation
	bool m_bCalcVar;

	// AV min and max
	REAL m_varMin;
	REAL m_varMax;

	// stores orthogonal basis for searched directions (used to calculate adaptive variance)
	MatrixNxM<> m_mOrthoBasis;

	// stores the calculated AV
	VectorN<> m_vAdaptVariance;

};	// class ConjGradOptimizer


// Backward compatibility
using CConjGradOptimizer = ConjGradOptimizer;

#endif

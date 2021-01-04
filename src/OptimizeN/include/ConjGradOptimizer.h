//////////////////////////////////////////////////////////////////////
// ConjGradOptimizer.h: interface for the CConjGradOptimizer
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
// class CConjGradOptimizer
// 
// optimizer that implements the Powell algorithm explained in
//		Numerical Recipes
//////////////////////////////////////////////////////////////////////
class CConjGradOptimizer : public COptimizer
{
public:
	// construct a gradient optimizer for an objective function
	CConjGradOptimizer(CObjectiveFunction *pFunc);

	// returns a reference to the embedded Brent optimizer
	CBrentOptimizer& GetBrentOptimizer();

	// optimize the objective function
	virtual const CVectorN<>& Optimize(const CVectorN<>& vInit);

	// flag to indicate that line opt tolerance should always be same
	DECLARE_ATTRIBUTE(LineToleranceEqual, bool);

	// used to set up the variance min / max calculation
	void SetAdaptiveVariance(bool bCalcVar, REAL varMin, REAL varMax);

private:
	// line function that projects the objective function along 
	//		a given line
	CLineFunction m_lineFunction;

	// points to the line optimizer to be used
	COptimizer *m_pLineOptimizer;

	// brent optimizer along the line function
	CBrentOptimizer m_optimizeBrent;

	// "statics" for the optimization routine
	CVectorN<> m_vGrad;
	CVectorN<> m_vGradPrev;
	CVectorN<> m_vDir;
	CVectorN<> m_vDirPrev;
	CVectorN<> m_vLambdaScaled;

	// flag to indicate adaptive variance calculation
	bool m_bCalcVar;

	// AV min and max
	REAL m_varMin;
	REAL m_varMax;

	// stores orthogonal basis for searched directions (used to calculate adaptive variance)
	CMatrixNxM<> m_mOrthoBasis;

	// stores the calculated AV
	CVectorN<> m_vAdaptVariance;

};	// class CConjGradOptimizer


#endif

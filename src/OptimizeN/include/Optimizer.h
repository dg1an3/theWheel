//////////////////////////////////////////////////////////////////////
// Optimizer.h: interface for the Optimizer template
//
// Copyright (C) 1996-2001
// $Id: Optimizer.h,v 1.3 2006/04/01 16:47:16 HP_Administrator Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(OPTIMIZER_H)
#define OPTIMIZER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// include for the objective function
#include "ObjectiveFunction.h"

class Optimizer;

// callback function
typedef BOOL OptimizerCallback(Optimizer *pOpt, void *pParam);

//////////////////////////////////////////////////////////////////////
// class Optimizer
// 
// base template class for all optimizers
//////////////////////////////////////////////////////////////////////
class Optimizer
{
public:
	// construct a new Optimizer object
	Optimizer(ObjectiveFunction *pFunc);

	// destroy the optimizer
	virtual ~Optimizer();

	// sets the callback function
	void SetCallback(OptimizerCallback *pCallback, void *pParam = NULL);

	// the flag to indicate that gradient information should
	//		be used
	BOOL UseGradientInfo() const;
	void SetUseGradientInfo(BOOL bUseGradientInfo);

	// defines the tolerance for exit from optimization loop
	REAL GetTolerance() const;
	void SetTolerance(REAL tol);

	// holds the number of iterations needed for the optimization
	int GetIterations() const;

	// holds the final value of the optimization
	REAL GetFinalValue() const;

	// holds the final value of the parameters for the minimum f
	const VectorN<>& GetFinalParameter() const;

	// function to actually perform the optimization
	virtual const VectorN<>& Optimize(const VectorN<>& vInit) = 0;

protected:
	// the objective function over which optimization is to occur
	ObjectiveFunction *m_pFunc;

	// stores the callback info
	OptimizerCallback *m_pCallbackFunc;
	void *m_pCallbackParam;

	// holds the tolerance for the optimization
	REAL m_tolerance;

	// holds the number of iterations for the optimization
	int m_nIteration;

	// holds the value of the objective function at the final point
	REAL m_finalValue;

	// holds the final input parameter to the objective function (same as
	//		returned by Optimize)
	VectorN<> m_vFinalParam;

private:
	// flag to indicate whether gradient information should be used
	BOOL m_bUseGradientInfo;

};	// class Optimizer


// Backward compatibility
using COptimizer = Optimizer;

#endif // !defined(OPTIMIZER_H)

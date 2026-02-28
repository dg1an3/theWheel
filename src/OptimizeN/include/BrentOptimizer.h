//////////////////////////////////////////////////////////////////////
// BrentOptimizer.h: interface for the BrentOptimizer
//
// Copyright (C) 1996-2001
// $Id: BrentOptimizer.h,v 1.2 2004/02/16 15:55:22 default Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(BRENTOPTIMIZER_H)
#define BRENTOPTIMIZER_H

// include the optimizer base class
#include "Optimizer.h"

//////////////////////////////////////////////////////////////////////
// class BrentOptimizer
// 
// optimizer that implements the Brent algorithm explained in
//		Numerical Recipes
//////////////////////////////////////////////////////////////////////
class BrentOptimizer : public Optimizer
{
public:
	void SetParams(REAL Bracket, REAL GLimit);
	// construct a new Brent Optimizer
	BrentOptimizer(ObjectiveFunction *pFunc);

	// optimizes the initial input vector
	virtual const VectorN<>& Optimize(const VectorN<>& vInit = m_vBrentInit);

	// helper function to return an initial Zero vector
	static const VectorN<>& GetInitZero();

protected:
	// finds a bracket for the minimum value of the objective function
	void BracketMinimum(REAL& ax, REAL& bx, REAL& cx);

	// finds the minimum of the objective function
	REAL FindMinimum(REAL ax, REAL bx, REAL cx);

	// finds the minimum of the objective function, using gradient information
	REAL FindMinimumGrad(REAL ax, REAL bx, REAL cx);

private:
	// these are "local" variables that are initialized here so that we do not
	//		need to re-allocate them during optimization
	VectorN<> m_vAx, m_vBx, m_vCx;
	VectorN<> m_vX, m_vU;

	// holds the gradient
	VectorN<> m_vGrad;

	// parameters for optimization 
	REAL m_Bracket;	// initial bracket size
	REAL m_GLimit;	// parameter needed by function BracketMinimum  

	// starting value for the brent optimizer
	//		initialized to 
	static VectorN<> m_vBrentInit;
};

// Backward compatibility
using CBrentOptimizer = BrentOptimizer;

#endif
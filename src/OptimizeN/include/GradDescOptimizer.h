//////////////////////////////////////////////////////////////////////
// GradDescOptimizer.h: interface for the GradDescOptimizer
//
// Copyright (C) 1996-2001
// $Id: GradDescOptimizer.h,v 1.1 2002/06/11 04:01:01 default Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(GRADDESCOPTIMIZER_H)
#define GRADDESCOPTIMIZER_H

// include the optimizer base class
#include "Optimizer.h"

//////////////////////////////////////////////////////////////////////
// class GradDescOptimizer
// 
// optimizer that implements 1D gradient optimizer
//////////////////////////////////////////////////////////////////////
class GradDescOptimizer : public Optimizer
{
public:
	// construct a new Gradient Descent Optimizer
	GradDescOptimizer(ObjectiveFunction *pFunc);

	// optimizes the initial input vector
	virtual const VectorN<>& Optimize(const VectorN<>& vInit);

private:
	// holds the gradient
	VectorN<> m_vGrad;
};

// Backward compatibility
using CGradDescOptimizer = GradDescOptimizer;

#endif
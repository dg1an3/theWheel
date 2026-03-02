//////////////////////////////////////////////////////////////////////
// ObjectiveFunction.h: interface for the ObjectiveFunction
//		template base class
//
// Copyright (C) 1996-2001
// $Id: ObjectiveFunction.h,v 1.8 2006/11/22 00:29:26 HP_Administrator Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

#if !defined(OBJECTIVEFUNCTION_H)
#define OBJECTIVEFUNCTION_H

// objective functions are vector-domained functions
#include <VectorN.h>

#include <MatrixNxM.h>

//////////////////////////////////////////////////////////////////////
// class ObjectiveFunction
// 
// base class template for all objective functions.  allows the 
//		objective function to define a gradient, but a flag is provided
//		in the case that no gradient is available
//////////////////////////////////////////////////////////////////////
class ObjectiveFunction
{
public:
	// constructs an objective function; gets flag to indicate
	//		whether gradient information is available
	ObjectiveFunction(BOOL bHasGradientInfo);

	// virtual destructor
	virtual ~ObjectiveFunction() = default;

	// evaluates the objective function
	virtual REAL operator()(const VectorN<>& vInput, 
		VectorN<> *pGrad = NULL) const = 0;

	// whether gradient information is available
	BOOL HasGradientInfo() const;

	// transform function from linear to other parameter space
	virtual void Transform(VectorN<> *pvInOut) const;
	virtual void dTransform(VectorN<> *pvInOut) const;
	virtual void InvTransform(VectorN<> *pvInOut) const;

	// function to evaluate gradiant at a point (uses difference method)
	void Gradient(const VectorN<>& vIn, REAL epsilon, 
				VectorN<>& vGrad_out) const;

	// function to evaluate hessian at a point (uses difference method)
	void Hessian(const VectorN<>& vIn, REAL epsilon, 
				MatrixNxM<>& mHess_out) const;

	// sets the OF to use adaptive variance
	void SetAdaptiveVariance(VectorN<> *pAV, REAL varMin, REAL varMax);

protected:
	// pointer to adaptive variance vector, if enabled
	VectorN<> *m_pAV;

	// min and max for AV
	REAL m_varMin;
	REAL m_varMax;

private:
	// flag to indicate that gradient information is available
	BOOL m_bHasGradientInfo;

};	// class ObjectiveFunction

// Backward compatibility
using CObjectiveFunction = ObjectiveFunction;

#endif

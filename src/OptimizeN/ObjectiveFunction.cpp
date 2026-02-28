//////////////////////////////////////////////////////////////////////
// ObjectiveFunction.cpp: implementation of the ObjectiveFunction 
//		base class
//
// Copyright (C) 1996-2004  Derek G. Lane
// $Id: ObjectiveFunction.cpp,v 1.8 2006/11/22 00:29:24 HP_Administrator Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// the class definition
#include "ObjectiveFunction.h"


///////////////////////////////////////////////////////////////////////////////
// ObjectiveFunction::ObjectiveFunction
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
ObjectiveFunction::ObjectiveFunction(BOOL bHasGradientInfo)
	: m_bHasGradientInfo(bHasGradientInfo)
	, m_pAV(NULL)
{
}	// ObjectiveFunction::ObjectiveFunction

///////////////////////////////////////////////////////////////////////////////
// ObjectiveFunction::HasGradientInfo
// 
// whether gradient information is available
///////////////////////////////////////////////////////////////////////////////
BOOL ObjectiveFunction::HasGradientInfo() const
{
	return m_bHasGradientInfo;

}	// ObjectiveFunction::HasGradientInfo


///////////////////////////////////////////////////////////////////////////////
// ObjectiveFunction::Transform
// 
// over-ride for function to transform parameters
///////////////////////////////////////////////////////////////////////////////
void ObjectiveFunction::Transform(VectorN<> *pvInOut) const
{
	// default is identity transform
	ASSERT(false);

}	// ObjectiveFunction::Transform

///////////////////////////////////////////////////////////////////////////////
// ObjectiveFunction::dTransform
// 
// over-ride for derivative transform parameters
///////////////////////////////////////////////////////////////////////////////
void ObjectiveFunction::dTransform(VectorN<> *pvInOut) const
{
	// set to all 1.0 for identity transform
	ITERATE_VECTOR((*pvInOut), nAt, (*pvInOut)[nAt] = 1.0);

}	// ObjectiveFunction::dTransform

///////////////////////////////////////////////////////////////////////////////
// ObjectiveFunction::InvTransform
// 
// inverse of parameter transform
///////////////////////////////////////////////////////////////////////////////
void ObjectiveFunction::InvTransform(VectorN<> *pvInOut) const
{
	// default is identity transform
	ASSERT(false);

}	// ObjectiveFunction::InvTransform


///////////////////////////////////////////////////////////////////////////////
// ObjectiveFunction::Gradient
// 
// approximates gradient using difference method
///////////////////////////////////////////////////////////////////////////////
void ObjectiveFunction::Gradient(const VectorN<>& vIn, REAL epsilon, 
				VectorN<>& vGrad_out) const
{
	REAL res = 0.0;
	BEGIN_LOG_SECTION(ObjectiveFunction::Gradient());

	// get epsilon
	REAL elem_max = 0.0;
	for (int nAt = 0; nAt < vIn.GetDim(); nAt++)
	{
		elem_max = __max(elem_max, fabs(vIn[nAt]));
	}
	epsilon = elem_max * epsilon;

	// numerically evaluate the gradiant
	VectorN<> vParam = vIn;
	const REAL fp = (*this)(vParam);

	vGrad_out.SetDim(vParam.GetDim());
	for (int nAt = 0; nAt < vParam.GetDim(); nAt++)
	{
		vParam[nAt] += epsilon;

		const REAL fp_del = (*this)(vParam);
		vGrad_out[nAt] = (fp_del - fp) / epsilon;

		vParam[nAt] -= epsilon;
	}
	LOG_EXPR_EXT(vGrad_out);

	END_LOG_SECTION();	// ObjectiveFunction::Gradient

}	// ObjectiveFunction::Gradient


///////////////////////////////////////////////////////////////////////////////
// ObjectiveFunction::Hessian
// 
// approximates hessian using difference method
///////////////////////////////////////////////////////////////////////////////
void ObjectiveFunction::Hessian(const VectorN<>& vAtParam, REAL epsilon,
								MatrixNxM<>& mHess_out) const
{
	BEGIN_LOG_SECTION(ObjectiveFunction::Hessian());

	const REAL fp = (*this)(vAtParam);

	// numerically evaluate the hessian
	VectorN<> vParam = vAtParam;
	mHess_out.Reshape(vParam.GetDim(), vParam.GetDim());
	for (int nAtCol = 0; nAtCol < vParam.GetDim(); nAtCol++)
	{
		BEGIN_LOG_SECTION_(FMT("Col %i", nAtCol));

		vParam = vAtParam;
		vParam[nAtCol] += epsilon;
		const REAL fp_del_col = (*this)(vParam);

		for (int nAtRow = 0; nAtRow < mHess_out.GetRows(); nAtRow++)
		{
			BEGIN_LOG_SECTION_(FMT("Row %i", nAtRow));

			vParam = vAtParam;
			vParam[nAtRow] += epsilon;
			const REAL fp_del_row = (*this)(vParam);
			vParam[nAtCol] += epsilon;
			const REAL fp_del_row_del_col = (*this)(vParam);

			mHess_out[nAtCol][nAtRow] = fp_del_row_del_col - fp_del_row;
			mHess_out[nAtCol][nAtRow] -= fp_del_col - fp;
			mHess_out[nAtCol][nAtRow] /= epsilon * epsilon;
			LOG_EXPR(mHess_out[nAtCol][nAtRow]);

			END_LOG_SECTION();	// FormatString("Row %i", nAtRow);
		}
		END_LOG_SECTION();	// FormatString("Row %i", nAtCol);
	}
	LOG_EXPR_EXT(mHess_out);

	END_LOG_SECTION();	// ObjectiveFunction::Hessian

}	// ObjectiveFunction::Hessian


//////////////////////////////////////////////////////////////////////////////
void 
	ObjectiveFunction::SetAdaptiveVariance(VectorN<> *pAV, REAL varMin, REAL varMax)
	// sets the OF to use adaptive variance
{
	// store pointer to the AV vector
	m_pAV = pAV;

	// stores min / max
	m_varMin = varMin;
	m_varMax = varMax;

}	// ObjectiveFunction::SetAdaptiveVariance


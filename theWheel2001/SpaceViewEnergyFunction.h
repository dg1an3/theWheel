//////////////////////////////////////////////////////////////////////
// SpaceViewEnergyFunction.h: declaration of the 
//		CSpaceViewEnergyFunction class
//
// Copyright (C) 1999-2001
// U.S. Patent Pending
// $Id$
//////////////////////////////////////////////////////////////////////

#ifndef _SPACEVIEWENERGYFUNCTION_H
#define _SPACEVIEWENERGYFUNCTION_H

#include <LookupFunction.h>
#include <Optimizer.h>

//////////////////////////////////////////////////////////////////////
// forward declaration of the SpaceView class
//////////////////////////////////////////////////////////////////////
class CSpaceView;

//////////////////////////////////////////////////////////////////////
// typedef for the energy function type
//////////////////////////////////////////////////////////////////////
typedef float SPV_STATE_TYPE;

//////////////////////////////////////////////////////////////////////
// declaration for the dimensionality of the energy function state
//		vector
//////////////////////////////////////////////////////////////////////
const int SPV_STATE_DIM = 34;

//////////////////////////////////////////////////////////////////////
// class CSpaceViewEnergyFunction
//
// class that represents the energy function for a CSpaceView given
//		its state vector
//////////////////////////////////////////////////////////////////////
class CSpaceViewEnergyFunction 
		: public CObjectiveFunction<SPV_STATE_DIM, SPV_STATE_TYPE>
{
public:
	// construct the energy function, given the CSpaceView to which it
	//		is associated 
	CSpaceViewEnergyFunction(CSpaceView *pView);

	// evaluates the energy function
	virtual SPV_STATE_TYPE operator()(
		const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput);

	// evaluates the gradient of the energy function
	virtual CVector<SPV_STATE_DIM, SPV_STATE_TYPE> Grad(
		const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput);

private:
	// pointer to the energy function's spaceview
	CSpaceView *m_pView;

	// lookup function for the attraction field
	CLookupFunction<SPV_STATE_TYPE> m_attractFunc;

	// lookup function for the attraction field
	CLookupFunction<SPV_STATE_TYPE> m_dattractFunc_dx;

	// lookup function for the attraction field
	CLookupFunction<SPV_STATE_TYPE> m_dattractFunc_dy;

	// lookup function for the spacer field
	CLookupFunction<SPV_STATE_TYPE> m_spacerFunc;

	// lookup function for the spacer field
	CLookupFunction<SPV_STATE_TYPE> m_dspacerFunc_dx;

	// lookup function for the spacer field
	CLookupFunction<SPV_STATE_TYPE> m_dspacerFunc_dy;

	// caches previous input vector
	CVector<SPV_STATE_DIM, SPV_STATE_TYPE> m_vInput;

	// caches energy value for the previous input vector
	SPV_STATE_TYPE m_energy;

	// caches the gradient for the previous input vector
	CVector<SPV_STATE_DIM, SPV_STATE_TYPE> m_vGrad;
};


#endif // ndef _SPACEVIEWENERGYFUNCTION_H
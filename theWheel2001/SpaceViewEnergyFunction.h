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
const int SPV_STATE_DIM = 42;

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

	// loads the links and sizes for quick access
	void LoadSizesLinks();

	// evaluates the energy function
	virtual SPV_STATE_TYPE operator()(
		const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput);

	// evaluates the gradient of the energy function
	virtual CVector<SPV_STATE_DIM, SPV_STATE_TYPE> Grad(
		const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput);

	int m_nEvaluations;

private:
	// pointer to the energy function's spaceview
	CSpaceView *m_pView;

	// caches previous input vector
	CVector<SPV_STATE_DIM, SPV_STATE_TYPE> m_vInput;

	// caches energy value for the previous input vector
	SPV_STATE_TYPE m_energy;

	// caches the gradient for the previous input vector
	CVector<SPV_STATE_DIM, SPV_STATE_TYPE> m_vGrad;

	// stores the view sizes for quick access
	SPV_STATE_TYPE m_vSize[SPV_STATE_DIM][2];

	// stores the link weights for quick access
	SPV_STATE_TYPE m_mLinks[SPV_STATE_DIM][SPV_STATE_DIM];
};


#endif // ndef _SPACEVIEWENERGYFUNCTION_H
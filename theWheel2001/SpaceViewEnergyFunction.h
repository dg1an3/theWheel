#ifndef _SPACEVIEWENERGYFUNCTION_H
#define _SPACEVIEWENERGYFUNCTION_H

#include "Optimizer.h"

////////////////////////////////////
// Copyright (C) 1996-2000 DG Lane
// U.S. Patent Pending
////////////////////////////////////

typedef double SPV_STATE_TYPE;
const int SPV_STATE_DIM = 24;

class CSpaceView;

class CSpaceViewEnergyFunction : public CGradObjectiveFunction<SPV_STATE_DIM, SPV_STATE_TYPE>
{
public:
	CSpaceView *m_pView;

	// retrieves the threshold for the given dimensionality
	SPV_STATE_TYPE GetThreshold();

	// gets and sets the state vector for the associated CSpaceView
	CVector<SPV_STATE_DIM, SPV_STATE_TYPE> GetStateVector();
	void SetStateVector(const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vState);

	// evaluates the energy function
	virtual SPV_STATE_TYPE operator()(const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput);

	// evaluates the gradient of the energy function
	virtual CVector<SPV_STATE_DIM, SPV_STATE_TYPE> 
		Grad(const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput);

private:
	// caches previous computation
	CVector<SPV_STATE_DIM, SPV_STATE_TYPE> m_vInput;
	SPV_STATE_TYPE m_energy;
	CVector<SPV_STATE_DIM, SPV_STATE_TYPE> m_vGrad;
};


#endif // ndef _SPACEVIEWENERGYFUNCTION_H
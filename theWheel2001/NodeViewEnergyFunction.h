//////////////////////////////////////////////////////////////////////
// NodeViewEnergyFunction.h: declaration of the 
//		CNodeViewEnergyFunction class
//
// Copyright (C) 1999-2001
// U.S. Patent Pending
// $Id$
//////////////////////////////////////////////////////////////////////

#ifndef _NodeViewEnergyFUNCTION_H
#define _NodeViewEnergyFUNCTION_H

#include <Optimizer.h>

#include "NodeView.h"
#include "SpaceViewEnergyFunction.h"

//////////////////////////////////////////////////////////////////////
// forward declaration of the SpaceView class
//////////////////////////////////////////////////////////////////////
class CSpaceView;

//////////////////////////////////////////////////////////////////////
// typedef for the energy function type
//////////////////////////////////////////////////////////////////////
typedef float STATE_TYPE;

//////////////////////////////////////////////////////////////////////
// class CNodeViewEnergyFunction
//
// class that represents the energy function for a CSpaceView given
//		its state vector
//////////////////////////////////////////////////////////////////////
class CNodeViewEnergyFunction 
		: public CObjectiveFunction<2, STATE_TYPE>
{
public:
	// construct the energy function, given the CSpaceView to which it
	//		is associated 
	CNodeViewEnergyFunction(CSpaceView *pView);

	CNodeView *m_pNodeView;

	// typedef for the actual state vector
	typedef CVector<2, STATE_TYPE> CStateVector;

	// loads the links and sizes for quick access
	void LoadSizesLinks();

	// evaluates the energy function
	virtual STATE_TYPE operator()(const CStateVector& vInput);

	// evaluates the gradient of the energy function
	virtual CStateVector Grad(const CStateVector& vInput);

	// holds the number of evaluations that have been done
	int m_nEvaluations;

private:
	// pointer to the energy function's spaceview
	CSpaceView *m_pView;

	// caches previous input vector
	CStateVector m_vInput;

	// caches energy value for the previous input vector
	STATE_TYPE m_energy;

	// caches the gradient for the previous input vector
	CStateVector m_vGrad;

	// stores the view sizes for quick access
	STATE_TYPE m_vSize[STATE_DIM][2];
	STATE_TYPE m_act[STATE_DIM];

	// stores the link weights for quick access
	STATE_TYPE m_mLinks[STATE_DIM];
};


#endif // ndef _NodeViewEnergyFUNCTION_H
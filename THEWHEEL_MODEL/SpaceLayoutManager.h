//////////////////////////////////////////////////////////////////////
// SpaceLayoutManager.h: declaration of the 
//		CSpaceLayoutManager class
//
// Copyright (C) 1999-2001
// U.S. Patent Pending
// $Id$
//////////////////////////////////////////////////////////////////////

#ifndef _SPACELAYOUTMANAGER_H
#define _SPACELAYOUTMANAGER_H

#include <Optimizer.h>

//////////////////////////////////////////////////////////////////////
// forward declaration of the SpaceView class
//////////////////////////////////////////////////////////////////////
class CSpace;

//////////////////////////////////////////////////////////////////////
// declaration for the dimensionality of the energy function state
//		vector
//////////////////////////////////////////////////////////////////////
const int STATE_DIM = 36;

//////////////////////////////////////////////////////////////////////
// class CSpaceLayoutManager
//
// class that represents the energy function for a CSpaceView given
//		its state vector
//////////////////////////////////////////////////////////////////////
class CSpaceLayoutManager : public CObjectiveFunction
{
public:
	// construct the energy function, given the CSpaceView to which it
	//		is associated 
	CSpaceLayoutManager(CSpace *pSpace);

	// destroys the energy function
	virtual ~CSpaceLayoutManager();

	// typedef for the actual state vector
	typedef CVectorN<REAL> CStateVector;

	// loads the links and sizes for quick access
	void LoadSizesLinks();

	// gets and sets the state vector for this CSpaceView
	void Pos2StateVector();
	void StateVector2Pos();

	// evaluates the energy function
	virtual REAL operator()(const CStateVector& vInput, 
		CVectorN<> *pGrad = NULL);

	// performs the layout
	void LayoutNodes();

	// member function to rotate and translate the state vector
	//		to minimize the diff. w/ the previous state vector
	void RotateTranslateStateVector(CStateVector *pState, const CStateVector& vOldState);

private:
	// pointer to the energy function's spaceview
	CSpace *m_pSpace;

	// the optimizer for the layout
	COptimizer *m_pPowellOptimizer;
	COptimizer *m_pConjGradOptimizer;
	COptimizer *m_pGradDescOptimizer;

	// pointer to the optimizer to be used
	COptimizer *m_pOptimizer;

	// caches previous input vector
	CStateVector m_vInput;

	// caches energy value for the previous input vector
	REAL m_energy;

	// holds the current state
	CStateVector m_vState;

	// caches the gradient for the previous input vector
	CStateVector m_vGrad;

	// stores the view sizes for quick access
	REAL m_vSize[STATE_DIM][2];
	REAL m_act[STATE_DIM];

	// stores the link weights for quick access
	REAL m_mLinks[STATE_DIM][STATE_DIM];

	// holds the number of evaluations that have been done
	int m_nEvaluations;
};


#endif // ndef _SPACELAYOUTMANAGER_H
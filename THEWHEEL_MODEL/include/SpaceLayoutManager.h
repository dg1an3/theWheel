//////////////////////////////////////////////////////////////////////
// SpaceLayoutManager.h: declaration of the 
//		CSpaceLayoutManager class
//
// Copyright (C) 1999-2002 Derek Graham Lane
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
const int MAX_STATE_DIM = 256;

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

	double GetKPos();
	void SetKPos(double k_pos);

	double GetKRep();
	void SetKRep(double k_rep);

	double GetTolerance();
	void SetTolerance(double tolerance);

	double GetEnergy();

	// dimension of the state vector
	int GetStateDim() const;
	void SetStateDim(int nStateDim);

	// loads the links and sizes for quick access
	void LoadSizesLinks();

	// gets and sets the state vector for this CSpaceView
	void Pos2StateVector();
	void StateVector2Pos();

	// evaluates the energy function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL);

	// performs the layout
	void LayoutNodes();

	// member function to rotate and translate the state vector
	//		to minimize the diff. w/ the previous state vector
	void RotateTranslateStateVector(const CVectorN<>& vOldState, 
		CVectorN<>& vNewState);

private:
	// pointer to the energy function's spaceview
	CSpace *m_pSpace;

	// the optimizer for the layout
	COptimizer *m_pPowellOptimizer;
	COptimizer *m_pConjGradOptimizer;
	COptimizer *m_pGradDescOptimizer;

	// pointer to the optimizer to be used
	COptimizer *m_pOptimizer;

	// current state dimension
	int m_nStateDim;

	// caches previous input vector
	CVectorN<> m_vInput;

	// caches energy value for the previous input vector
	REAL m_energy;

	// holds the current state
	CVectorN<> m_vState;

	// caches the gradient for the previous input vector
	CVectorN<> m_vGrad;

	// stores the view sizes for quick access
	REAL m_vSize[MAX_STATE_DIM][2];
	REAL m_act[MAX_STATE_DIM];

	// stores the link weights for quick access
	REAL m_mLinks[MAX_STATE_DIM][MAX_STATE_DIM];

	// holds the number of evaluations that have been done
	int m_nEvaluations;
};


#endif // ndef _SPACELAYOUTMANAGER_H
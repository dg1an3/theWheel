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
// forward declaration of the CSpace class
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

	// accessors for optimization parameters
	REAL GetKPos();
	void SetKPos(REAL k_pos);

	REAL GetKRep();
	void SetKRep(REAL k_rep);

	REAL GetTolerance();
	void SetTolerance(REAL tolerance);

	// total energy
	REAL GetEnergy();

	// dimension of the state vector
	int GetStateDim() const;
	void SetStateDim(int nStateDim);

	// loads the links and sizes for quick access
	virtual void LoadSizesLinks(int nConstNodes, int nNodeCount);

	// evaluates the energy function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL);

	// returns the distance error between two nodes
	REAL GetDistError(CNode *pFrom, CNode *pTo);

	// performs the layout
	void LayoutNodes(CSpaceStateVector *pSSV, int nConstNodes);

protected:

	// optimal normalized distances
	static const REAL OPT_DIST;

	// scale for the sizes of the nodes
	static const REAL SIZE_SCALE;

	// constants for center repulsion
	static const REAL CENTER_REP_MAX_ACT;
	static const REAL CENTER_REP_SCALE;
	static const REAL CENTER_REP_WEIGHT;

	// pointer to the energy function's spaceview
	CSpace *m_pSpace;

	// current state dimension
	int m_nStateDim;

	// vector holding the const positions
	// CVectorN<> m_vConstPositions;
	int m_nConstNodes;

	// the optimizer for the layout
	COptimizer *m_pPowellOptimizer;
	COptimizer *m_pConjGradOptimizer;
	COptimizer *m_pGradDescOptimizer;

	// pointer to the optimizer to be used
	COptimizer *m_pOptimizer;

	// stores the objective function parameters
	REAL m_k_pos;
	REAL m_k_rep;
	REAL m_tolerance;

	// caches energy value for the previous input vector
	REAL m_energy;
	REAL m_energyConst;

	// flag to indicate that the center repulsion should be calculated
	BOOL m_bCalcCenterRep;

	// holds the current state
	CVectorN<> m_vState;

	// caches the gradient for the previous input vector
	CVectorN<> m_vGrad;

	// stores the view sizes for quick access
	REAL (*m_mSSX)[MAX_STATE_DIM];
	REAL (*m_mSSY)[MAX_STATE_DIM];

	// stores the link weights for quick access
	REAL (*m_mLinks)[MAX_STATE_DIM];
	REAL (*m_mAvgAct)[MAX_STATE_DIM];

	// holds the number of evaluations that have been done
	int m_nEvaluations;

};	// class CSpaceLayoutManager


#endif // ndef _SPACELAYOUTMANAGER_H
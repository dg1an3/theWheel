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
const int MAX_STATE_DIM = 100;

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
	DECLARE_ATTRIBUTE(KPos, REAL);
	DECLARE_ATTRIBUTE(KRep, REAL);
	DECLARE_ATTRIBUTE(Tolerance, REAL);

	// dimension of the state vector
	DECLARE_ATTRIBUTE_GI(StateDim, int);

	// total energy for current configuration
	REAL GetEnergy();

	// returns the distance error between two nodes
	REAL GetDistError(CNode *pFrom, CNode *pTo);

	// loads the links and sizes for quick access
	virtual void LoadSizesLinks(int nConstNodes, int nNodeCount);

	// evaluates the energy function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL) const;

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

	// pointer to the energy function's space
	CSpace *m_pSpace;

	// number of const positions
	int m_nConstNodes;

	// the optimizer for the layout
	COptimizer *m_pPowellOptimizer;
	COptimizer *m_pConjGradOptimizer;
	COptimizer *m_pGradDescOptimizer;
	COptimizer *m_pDFPOptimizer;

	// pointer to the optimizer to be used
	COptimizer *m_pOptimizer;

	// caches energy value for the previous input vector
	mutable REAL m_energy;
	mutable REAL m_energyConst;

	// flag to indicate that the center repulsion should be calculated
	BOOL m_bCalcCenterRep;

	// holds the current state
	mutable CVectorN<> m_vState;

	// caches the gradient for the previous input vector
	mutable CVectorN<> m_vGrad;

	// stores the view sizes for quick access
	REAL (*m_mSSX)[MAX_STATE_DIM];
	REAL (*m_mSSY)[MAX_STATE_DIM];

	// stores the link weights for quick access
	REAL (*m_mLinks)[MAX_STATE_DIM];
	REAL (*m_mAvgAct)[MAX_STATE_DIM];

	// holds the number of evaluations that have been done
	mutable int m_nEvaluations;

};	// class CSpaceLayoutManager


#endif // ndef _SPACELAYOUTMANAGER_H
//////////////////////////////////////////////////////////////////////
// SpaceLayoutManager.h: declaration of the 
//		CSpaceLayoutManager class
//
// Copyright (C) 1999-2002 Derek Graham Lane
// U.S. Patent Pending
// $Id: SpaceLayoutManager.h,v 1.17 2007/05/28 18:46:09 Derek Lane Exp $
//////////////////////////////////////////////////////////////////////

#pragma once

#include <Optimizer.h>

//////////////////////////////////////////////////////////////////////
// forward declaration of the CSpace class
//////////////////////////////////////////////////////////////////////
class CSpace;
class CSpaceStateVector;

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
class CSpaceLayoutManager : public ObjectiveFunction
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

	// accessors for the super node count
	int GetSuperNodeCount();
	void SetMaxSuperNodeCount(int nSuperNodeCount);

	// returns a pointer to the state vector
	DECLARE_ATTRIBUTE_PTR(StateVector, CSpaceStateVector);

	// total energy for current configuration
	REAL GetEnergy();

	// returns the distance error between two nodes
	REAL GetDistError(CNode *pFrom, CNode *pTo);

	//////////////////////////////////////////////////////////////////
	// Node Layout

	// layout the nodes
	void LayoutNodes();

	// helper functions for positioning nodes
	void PositionNewSuperNodes();

	// performs the layout
	void LayoutNodesPartial(int nConstNodes);

	// relaxes the node link weights
	void Relax(bool bSubThreshold = false);

	// loads the links and sizes for quick access
	virtual void LoadSizesLinks(int nConstNodes, int nNodeCount);

	// evaluates the energy function
	virtual REAL operator()(const VectorN<>& vInput, 
		VectorN<> *pGrad = NULL) const;

protected:
	// pointer to the energy function's space
	CSpace *m_pSpace;

	// number of const positions
	int m_nConstNodes;

	// the optimizer for the layout
	Optimizer *m_pPowellOptimizer;
	Optimizer *m_pConjGradOptimizer;
	Optimizer *m_pGradDescOptimizer;
	Optimizer *m_pDFPOptimizer;

	// pointer to the optimizer to be used
	Optimizer *m_pOptimizer;

	// caches energy value for the previous input vector
	mutable REAL m_energy;
	mutable REAL m_energyConst;

	// holds the current state
	mutable VectorN<> m_vState;

	// holds constant positions
	mutable VectorN<> m_vConstPositions;

	// caches the gradient for the previous input vector
	mutable VectorN<> m_vGrad;

	// stores the view sizes for quick access
	REAL (*m_mSS)[MAX_STATE_DIM];

	// stores the link weights for quick access
	REAL (*m_mLinks)[MAX_STATE_DIM];
	REAL (*m_mAvgAct)[MAX_STATE_DIM];

	// holds the number of evaluations that have been done
	mutable int m_nEvaluations;

};	// class CSpaceLayoutManager

//////////////////////////////////////////////////////////////////////
// SpaceLayoutManager.cpp: implementation of the 
//		CSpaceLayoutManager objective function.
//
// Copyright (C) 1996-2003 Derek Graham Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// defines the gaussian function
#include <MathUtil.h>

// intel float math
#include <mathf.h>

// for least-squares
#include <MatrixNxM.h>
#include <MatrixBase.inl>

// optimizer for the layout
#include <PowellOptimizer.h>
#include <ConjGradOptimizer.h>
#include <GradDescOptimizer.h>

// header file for the CSpace object
#include "Space.h"

// header files for the class
#include "SpaceLayoutManager.h"

/////////////////////////////////////////////////////////////////////////////
// Constants for the CSpaceLayoutManager
/////////////////////////////////////////////////////////////////////////////

// constant for the tolerance of the optimization
const REAL TOLERANCE = (REAL) 0.01;

// scale for the sizes of the nodes
const REAL CSpaceLayoutManager::SIZE_SCALE = 100.0;

// constants for the distance scale vs. activation curve
const REAL DIST_SCALE_MIN = 1.0;
const REAL DIST_SCALE_MAX = 1.35;
const REAL ACTIVATION_MIDPOINT = 0.25;

// compute the optimal normalized distance
const REAL MIDWEIGHT = 0.5;
const REAL CSpaceLayoutManager::OPT_DIST = DIST_SCALE_MIN
	+ (DIST_SCALE_MAX - DIST_SCALE_MIN) 
		* (1.0 - MIDWEIGHT / (MIDWEIGHT + ACTIVATION_MIDPOINT));


// constant for weighting the position energy
const REAL K_POS = 600.0;

// constant for weighting the repulsion energy
const REAL K_REP = 3200.0;

// constants for center repulsion
const REAL CSpaceLayoutManager::CENTER_REP_MAX_ACT = 0.15;
const REAL CSpaceLayoutManager::CENTER_REP_SCALE = SIZE_SCALE / 8.0;
const REAL CSpaceLayoutManager::CENTER_REP_WEIGHT = 9.0;

//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::CSpaceLayoutManager
// 
// constructs an energy function object for the associated space 
//		view
//////////////////////////////////////////////////////////////////////
CSpaceLayoutManager::CSpaceLayoutManager(CSpace *pSpace)
	: CObjectiveFunction(TRUE),
		m_pSpace(pSpace),

		m_nStateDim(80),

		m_k_pos(K_POS),
		m_k_rep(K_REP),
		m_tolerance(TOLERANCE),

		m_vInput(CVectorN<>()),
		m_energy(0.0),
		m_energyConst(0.0),
		m_bCalcCenterRep(TRUE),

		m_pPowellOptimizer(NULL)
{
	// create and initialize the Powell optimizer
	m_pPowellOptimizer = new CPowellOptimizer(this);
	m_pPowellOptimizer->SetTolerance(TOLERANCE);

	// create and initialize the conjugate gradient optimizer
	m_pConjGradOptimizer = new CConjGradOptimizer(this);
	m_pConjGradOptimizer->SetTolerance(TOLERANCE);

	// create and initialize the gradient descent optimizer
	m_pGradDescOptimizer = new CGradDescOptimizer(this);
	m_pGradDescOptimizer->SetTolerance(TOLERANCE);

	// set the optimizer to be used
	m_pOptimizer = m_pConjGradOptimizer;

}	// CSpaceLayoutManager::CSpaceLayoutManager


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::~CSpaceLayoutManager
// 
// destroys the layout manager
//////////////////////////////////////////////////////////////////////
CSpaceLayoutManager::~CSpaceLayoutManager()
{
	// delete the optimizers
	delete m_pPowellOptimizer;
	delete m_pConjGradOptimizer;
	delete m_pGradDescOptimizer;

}	// CSpaceLayoutManager::~CSpaceLayoutManager


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::GetStateDim
// 
// returns the state dimension
//////////////////////////////////////////////////////////////////////
int CSpaceLayoutManager::GetStateDim() const
{
	return m_nStateDim;

}	// CSpaceLayoutManager::GetStateDim


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::SetStateDim
// 
// sets the state dimension
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::SetStateDim(int nStateDim)
{
	m_nStateDim = nStateDim;

}	// CSpaceLayoutManager::SetStateDim


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::GetEnergy
// 
// returns the energy from the most recent optimization
//////////////////////////////////////////////////////////////////////
REAL CSpaceLayoutManager::GetEnergy()
{
	return m_energy;

}	// CSpaceLayoutManager::GetEnergy


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::GetKPos
// 
// returns the k_pos (positional) parameter
//////////////////////////////////////////////////////////////////////
REAL CSpaceLayoutManager::GetKPos()
{
	return m_k_pos;

}	// CSpaceLayoutManager::GetKPos


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::SetKPos
// 
// sets the positional paramater
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::SetKPos(REAL k_pos)
{
	m_k_pos = k_pos;

}	// CSpaceLayoutManager::SetKPos


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::GetKRep
// 
// returns the repulsion parameter
//////////////////////////////////////////////////////////////////////
REAL CSpaceLayoutManager::GetKRep()
{
	return m_k_rep;

}	// CSpaceLayoutManager::GetKRep


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::SetKRep
// 
// sets the repulsion parameter
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::SetKRep(REAL k_rep)
{
	m_k_rep = k_rep;

}	// CSpaceLayoutManager::SetKRep


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::GetTolerance
// 
// returns the tolerance parameter
//////////////////////////////////////////////////////////////////////
REAL CSpaceLayoutManager::GetTolerance()
{
	return m_tolerance;

}	// CSpaceLayoutManager::GetTolerance


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::SetTolerance()
// 
// sets the tolerance parameter
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::SetTolerance(REAL tolerance)
{
	m_tolerance = tolerance;

}	// CSpaceLayoutManager::SetTolerance()


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::LoadSizesLinks()
// 
// loads the sizes and links for quick access
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::LoadSizesLinks()
{
	// stores the computed sizes for the nodes, based on current act
	int nNodeCount = __min(GetStateDim() / 2, m_pSpace->GetNodeCount());

	// iterate over all current visualized node views
	int nAtNode;
	for (nAtNode = 0; nAtNode < nNodeCount; nAtNode++)
	{
		// get convenience pointers for the current node view and node
		CNode *pAtNode = m_pSpace->GetNodeAt(nAtNode);

		// store the activations
		m_act[nAtNode] = pAtNode->GetActivation();

		// compute the x- and y-scales for the fields (from the linked rectangle)
		const CVectorD<3> vSize = pAtNode->GetSize(m_act[nAtNode]);
		const REAL size_scale = SIZE_SCALE;

		// store the size -- add 10 to ensure non-zero sizes
		m_vSize[nAtNode][0] = size_scale * vSize[0] + (REAL) 10.0;
		m_vSize[nAtNode][1] = size_scale * vSize[1] + (REAL) 10.0;

	}

	// iterate over all current visualized node views
	for (nAtNode = 0; nAtNode < nNodeCount; nAtNode++)
	{
		// get convenience pointers for the current node view and node
		CNode *pAtNode = m_pSpace->GetNodeAt(nAtNode);

		m_mSSX[nAtNode][nAtNode] = 1.0;
		m_mSSY[nAtNode][nAtNode] = 1.0;
		m_mAvgAct[nAtNode][nAtNode] = 0.0;
		m_mLinks[nAtNode][nAtNode] = 0.0;

		// iterate over the potential linked views
		int nAtLinkedNode;
		for (nAtLinkedNode = nAtNode+1; nAtLinkedNode < nNodeCount; nAtLinkedNode++)
		{
			// get convenience pointers for the linked node view and node
			CNode *pAtLinkedNode = m_pSpace->GetNodeAt(nAtLinkedNode);

			const REAL ssx = (m_vSize[nAtNode][0] 
					+ m_vSize[nAtLinkedNode][0]) / (REAL) 2.0;
			m_mSSX[nAtNode][nAtLinkedNode] = 
				m_mSSX[nAtLinkedNode][nAtNode] = (ssx * ssx); 

			const REAL ssy = (m_vSize[nAtNode][1] 
					+ m_vSize[nAtLinkedNode][1]) / (REAL) 2.0;
			m_mSSY[nAtNode][nAtLinkedNode] = 
				m_mSSY[nAtLinkedNode][nAtNode] = (ssy * ssy); 

			// retrieve the link weight for layout
			REAL weight = (REAL) 0.5 *
				(pAtNode->GetLinkWeight(pAtLinkedNode)
					+ pAtLinkedNode->GetLinkWeight(pAtNode)) + (REAL) 1e-6;

			m_mAvgAct[nAtNode][nAtLinkedNode] = 0.0;
			m_mAvgAct[nAtLinkedNode][nAtNode] =
				(m_act[nAtNode] + m_act[nAtLinkedNode]);

			// store the link weight
			m_mLinks[nAtNode][nAtLinkedNode] = 0.0;
			m_mLinks[nAtLinkedNode][nAtNode] =
				weight * m_mAvgAct[nAtLinkedNode][nAtNode];

		}
	}

}	// CSpaceLayoutManager::LoadSizesLinks()


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::operator()
// 
// evaluates the energy function at the given point
//////////////////////////////////////////////////////////////////////
REAL CSpaceLayoutManager::operator()(const CVectorN<REAL>& vInput,
									 CVectorN<> *pGrad)
{
	// reset the energy
	m_energy = m_energyConst;

	if (NULL != pGrad)
	{
		// initialize the gradient vector to zeros
		m_vGrad.SetDim(GetStateDim());
		m_vGrad.SetZero();
	}

	// prepare the input vector for the full positional information
	ASSERT(m_vConstPositions.GetDim() + vInput.GetDim() == GetStateDim());
	m_vInput.SetDim(GetStateDim());
	m_vInput.CopyElements(m_vConstPositions, 0, 
		m_vConstPositions.GetDim());
	m_vInput.CopyElements(vInput, 0, vInput.GetDim(),
		m_vConstPositions.GetDim());

	// and the total number of nodes plus clusters
	int nNodeCount = __min(GetStateDim() / 2, m_pSpace->GetNodeCount());

	// compute the weighted center of the nodes, for the center repulsion
	REAL vCenter[2];
	REAL totalAct = 0.0;
	for (int nAt = 0; nAt < nNodeCount; // TRIAL: Removed m_vInput.GetDim() / 2; 
		nAt++)
	{
		vCenter[0] = m_act[nAt] * m_vInput[nAt * 2];
		vCenter[1] = m_act[nAt] * m_vInput[nAt * 2 + 1];
		totalAct += m_act[nAt];
	}

	// scale weighted center by total weight
	vCenter[0] /= totalAct;
	vCenter[1] /= totalAct;

	// iterate over all current visualized node views
	int nAtNode;
	for (nAtNode = nNodeCount-1; nAtNode >= m_vConstPositions.GetDim() / 2;
		nAtNode--)
	{
		// iterate over the potential linked views
		int nAtLinked;
		for (nAtLinked = nAtNode-1; nAtLinked >= 0; nAtLinked--)
		{
			if (nAtLinked == nAtNode)
			{
				continue;
			}

			//////////////////////////////////////////////////////////////
			// set up some common values

			// compute the x- and y-offset between the views
			const REAL x = m_vInput[nAtNode*2 + 0] - m_vInput[nAtLinked*2 + 0];
			const REAL y = m_vInput[nAtNode*2 + 1] - m_vInput[nAtLinked*2 + 1];

			// compute the x- and y-scales for the fields -- average of
			//		two rectangles
			const REAL& ssx_sq = m_mSSX[nAtNode][nAtLinked];
			const REAL& ssy_sq = m_mSSY[nAtNode][nAtLinked];

			//////////////////////////////////////////////////////////////
			// compute the attraction field

			// store the weight, for convenience
			const REAL& weight = m_mLinks[nAtNode][nAtLinked];

			// compute the energy due to this interation
			const REAL dx_ratio = x / ssx_sq;
			const REAL x_ratio = x * dx_ratio;
			const REAL dy_ratio = y / ssy_sq;
			const REAL y_ratio = y * dy_ratio;

			// compute the relative actual distance
			const REAL act_dist = (REAL) sqrt(x_ratio + y_ratio + ((REAL) 0.001));

			// compute the distance error
			const REAL dist_error = act_dist - OPT_DIST;

			// compute the factor controlling the importance of the
			//		attraction term
			const REAL factor = m_k_pos * weight;

			// and add the attraction term to the energy
			m_energy += factor * dist_error * dist_error;

			if (NULL != pGrad)
			{
				// compute d_energy
				const REAL dact_dist_dx = dx_ratio / act_dist;
				const REAL dact_dist_dy = dy_ratio / act_dist;

				// compute the gradient terms
				const REAL denergy_dx = factor * dist_error * dact_dist_dx;
				const REAL denergy_dy = factor * dist_error * dact_dist_dy;

				// add to the gradient vector
				m_vGrad[nAtNode*2   + 0] += denergy_dx + denergy_dx;
				m_vGrad[nAtNode*2   + 1] += denergy_dy + denergy_dy;
				m_vGrad[nAtLinked*2 + 0] -= denergy_dx + denergy_dx;
				m_vGrad[nAtLinked*2 + 1] -= denergy_dy + denergy_dy;
			}

			//////////////////////////////////////////////////////////////
			// compute the repulsion field

			// compute the energy term
			const REAL inv_sq = ((REAL) 1.0) / (x_ratio + y_ratio + ((REAL) 3.0));
			const REAL factor_rep = m_k_rep * m_mAvgAct[nAtNode][nAtLinked];

			// add to total energy
			m_energy += factor_rep * inv_sq;

			if (NULL != pGrad)
			{
				// compute gradient terms
				const REAL inv_sq_sq = inv_sq * inv_sq;
				const REAL dRepulsion_dx = factor_rep * dx_ratio * inv_sq_sq;
				const REAL dRepulsion_dy = factor_rep * dy_ratio * inv_sq_sq;

				// add to the gradient vectors
				m_vGrad[nAtNode*2   + 0] -= dRepulsion_dx + dRepulsion_dx;
				m_vGrad[nAtNode*2   + 1] -= dRepulsion_dy + dRepulsion_dy;
				m_vGrad[nAtLinked*2 + 0] += dRepulsion_dx + dRepulsion_dx;
				m_vGrad[nAtLinked*2 + 1] += dRepulsion_dy + dRepulsion_dy;
			}
		}
	}
	
	if (m_bCalcCenterRep)
	{
		for (nAtNode = nNodeCount-1; nAtNode >= 0; nAtNode--)
		{
			//////////////////////////////////////////////////////////////
			// compute the centering repulsion field

			if (m_pSpace->GetNodeAt(nAtNode)->GetActivation()
						<= CENTER_REP_MAX_ACT)
			{
				// compute the x- and y-offset between the views
				const REAL x = m_vInput[nAtNode*2 + 0] - vCenter[0];
				const REAL y = m_vInput[nAtNode*2 + 1] - vCenter[1];

				// compute the x- and y-scales for the fields -- average of
				//		two rectangles
				const REAL aspect_sq = (13.0 / 16.0) * (13.0 / 16.0);
				const REAL ssx_sq =
					(CENTER_REP_SCALE * CENTER_REP_SCALE / aspect_sq );
				const REAL ssy_sq = 
					(CENTER_REP_SCALE * CENTER_REP_SCALE * aspect_sq );

				// compute the energy due to this interation
				const REAL dx_ratio = x / ssx_sq; 
				const REAL x_ratio = x * dx_ratio; 
				const REAL dy_ratio = y / ssy_sq;
				const REAL y_ratio = y * dy_ratio;

				// compute the energy term
				const REAL inv_sq = ((REAL) 1.0) / (x_ratio + y_ratio + ((REAL) 3.0));
				const REAL factor_rep = m_k_rep * CENTER_REP_WEIGHT
					* (CENTER_REP_MAX_ACT - abs(m_act[nAtNode])) 
					* (CENTER_REP_MAX_ACT - abs(m_act[nAtNode]));

				// add to total energy
				m_energy += factor_rep * inv_sq;

				if (NULL != pGrad)
				{
					// compute gradient terms
					const REAL inv_sq_sq = inv_sq * inv_sq;
					const REAL dRepulsion_dx = factor_rep * dx_ratio * inv_sq_sq;
					const REAL dRepulsion_dy = factor_rep * dy_ratio * inv_sq_sq;

					// add to the gradient vectors
					m_vGrad[nAtNode*2   + 0] -= dRepulsion_dx + dRepulsion_dx;
					m_vGrad[nAtNode*2   + 1] -= dRepulsion_dy + dRepulsion_dy;
				}
			} 
		}
	}

	// store the gradient vector if one was passed
	if (NULL != pGrad)
	{
		// set correct dimensionality
		pGrad->SetDim(vInput.GetDim());

		// and assign
		pGrad->CopyElements(m_vGrad, m_vConstPositions.GetDim(), vInput.GetDim());
	}

	// increment evaluation count
	m_nEvaluations++;

	// return the total energy
	return m_energy;

}	// CSpaceLayoutManager::operator()


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::GetDistError
// 
// returns the distance error between two nodes
//////////////////////////////////////////////////////////////////////
REAL CSpaceLayoutManager::GetDistError(CNode *pFrom, CNode *pTo)
{
	// store the size -- add 10 to ensure non-zero sizes
	CVectorD<3> vSizeFrom = pFrom->GetSize(pFrom->GetActivation());
	vSizeFrom *= SIZE_SCALE;
	vSizeFrom += CVectorD<3>(10.0, 10.0, 0.0);

	CVectorD<3> vSizeTo = pTo->GetSize(pTo->GetActivation());
	vSizeTo *= SIZE_SCALE;
	vSizeTo += CVectorD<3>(10.0, 10.0, 0.0);

	CVectorD<3> vSizeAvg = (REAL) 0.5 * (vSizeFrom + vSizeTo);

	CVectorD<3> vOffset = pFrom->GetPosition() - pTo->GetPosition();

	// compute the relative actual distance
	const REAL act_dist = (REAL) sqrt(vOffset[0] * vOffset[0] 
											/ (vSizeAvg[0] * vSizeAvg[0])
		+ vOffset[1] * vOffset[1] 
				/ (vSizeAvg[1] * vSizeAvg[1])) + 0.001;

	// compute the distance error
	const REAL dist_error = act_dist - OPT_DIST;

	return dist_error;

}	// CSpaceLayoutManager::GetDistError


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::LayoutNodes
// 
// calls the optimizer to lay out the nodes
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::LayoutNodes(CSpaceStateVector *pSSV, 
									  int nConstNodes)
{
	nConstNodes = __min(nConstNodes, MAX_STATE_DIM / 2);

	// if all nodes are constant
	if (nConstNodes * 2 >= GetStateDim())
	{
		// do nothing
		return;
	}

	REAL tol = 1e-12  * (m_pOptimizer->GetFinalValue() + 1.0);
	m_pOptimizer->SetTolerance(tol);

	// set up the interaction matrices
	LoadSizesLinks();

	// if we have constant nodes,
	if (nConstNodes > 0)
	{
		// cache the old state dimension
		int nOldStateDim = GetStateDim();

		// set new state dimension to only constant nodes
		SetStateDim(nConstNodes * 2);
		m_vState.SetDim(nConstNodes * 2);

		// get the positions vector
		pSSV->GetPositionsVector(m_vState);

		// make sure no constant nodes
		m_vConstPositions.SetDim(0);

		// set up the interaction matrices
		LoadSizesLinks();

		// get the constant energy term
		m_bCalcCenterRep = FALSE;
		m_energyConst = 0.0;
		m_energyConst = (*this)(m_vState);
		m_bCalcCenterRep = TRUE;

		// restore old state dimension
		SetStateDim(nOldStateDim);
	}
	else
	{
		// no constant nodes, no constant energy term
		m_energyConst = 0.0;
	}

	// form the state vector
	m_vState.SetDim(GetStateDim());
	pSSV->GetPositionsVector(m_vState, TRUE);

	// store the constant positions
	m_vConstPositions.SetDim(nConstNodes * 2);
	m_vConstPositions.CopyElements(m_vState, 0, nConstNodes * 2);

	// reset evaluation count
	m_nEvaluations = 0;

	// form the new state vector
	static CVectorN<> vNewState;
	vNewState.SetDim(GetStateDim() - nConstNodes * 2);
	vNewState.CopyElements(m_vState, nConstNodes * 2, 
		GetStateDim() - nConstNodes * 2);

	// set up the interaction matrices
	LoadSizesLinks();

	// perform the optimization
	vNewState = m_pOptimizer->Optimize(vNewState);

	// copy the optimized elements
	m_vState.CopyElements(vNewState, 0, 
		GetStateDim() - nConstNodes * 2, nConstNodes * 2);

	if (nConstNodes == 0)
	{
		// set the resulting positions, rotate-translating in the process
		pSSV->RotateTranslateTo(m_vState);

		// retrieve the final state
		pSSV->GetPositionsVector(m_vState);
	}
	else
	{
		pSSV->SetPositionsVector(m_vState);
	}

}	// CSpaceLayoutManager::LayoutNodes



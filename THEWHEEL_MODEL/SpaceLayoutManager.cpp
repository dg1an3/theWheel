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
const REAL SIZE_SCALE = 100.0;

// constants for the distance scale vs. activation curve
const REAL DIST_SCALE_MIN = 1.0;
const REAL DIST_SCALE_MAX = 1.35;
const REAL ACTIVATION_MIDPOINT = 0.25;

// constant for weighting the position energy
const REAL K_POS = 600.0;

// constant for weighting the repulsion energy
const REAL K_REP = 600.0;


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
double CSpaceLayoutManager::GetEnergy()
{
	return m_energy;

}	// CSpaceLayoutManager::GetEnergy


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::GetKPos
// 
// returns the k_pos (positional) parameter
//////////////////////////////////////////////////////////////////////
double CSpaceLayoutManager::GetKPos()
{
	return m_k_pos;

}	// CSpaceLayoutManager::GetKPos


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::SetKPos
// 
// sets the positional paramater
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::SetKPos(double k_pos)
{
	m_k_pos = k_pos;

}	// CSpaceLayoutManager::SetKPos


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::GetKRep
// 
// returns the repulsion parameter
//////////////////////////////////////////////////////////////////////
double CSpaceLayoutManager::GetKRep()
{
	return m_k_rep;

}	// CSpaceLayoutManager::GetKRep


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::SetKRep
// 
// sets the repulsion parameter
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::SetKRep(double k_rep)
{
	m_k_rep = k_rep;

}	// CSpaceLayoutManager::SetKRep


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::GetTolerance
// 
// returns the tolerance parameter
//////////////////////////////////////////////////////////////////////
double CSpaceLayoutManager::GetTolerance()
{
	return m_tolerance;

}	// CSpaceLayoutManager::GetTolerance


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::SetTolerance()
// 
// sets the tolerance parameter
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::SetTolerance(double tolerance)
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
	// iterate over all current visualized node views
	int nAtNode;
	for (nAtNode = 0; nAtNode < m_pSpace->GetSuperNodeCount(); nAtNode++)
	{
		// get convenience pointers for the current node view and node
		CNode *pAtNode = m_pSpace->GetNodeAt(nAtNode);

		// compute the x- and y-scales for the fields (from the linked rectangle)
		CVectorD<3> vSize = pAtNode->GetSize(pAtNode->GetActivation());

		// store the size -- add 10 to ensure non-zero sizes
		m_vSize[nAtNode][0] = SIZE_SCALE * vSize[0] + 10.0;
		m_vSize[nAtNode][1] = SIZE_SCALE * vSize[1] + 10.0;

		// store the activations
		m_act[nAtNode] = pAtNode->GetActivation();

		// iterate over the potential linked views
		int nAtLinkedNode;
		for (nAtLinkedNode = 0; nAtLinkedNode < m_pSpace->GetSuperNodeCount(); nAtLinkedNode++)
		{
			// only process the linked view if it is in the vector
			if (nAtLinkedNode != nAtNode) 
			{
				// get convenience pointers for the linked node view and node
				CNode *pAtLinkedNode = m_pSpace->GetNodeAt(nAtLinkedNode);

				// retrieve the link weight for layout
				REAL weight = 0.5 *
					(pAtNode->GetLinkWeight(pAtLinkedNode)
						+ pAtLinkedNode->GetLinkWeight(pAtNode));

				// add some weight to make it non-zero
				weight += 0.0001f;

				// store the link weight
				m_mLinks[nAtNode][nAtLinkedNode] = weight;
			}
		}
	}

}	// CSpaceLayoutManager::LoadSizesLinks()


//////////////////////////////////////////////////////////////////////
// CSpaceView::Pos2StateVector
// 
// forms the state vector from the node positions
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::Pos2StateVector()
{
	// for the state vector
	m_vState.SetDim(m_nStateDim);

	// iterate for non-cluster nodes
	for (int nAt = 0; nAt < m_nStateDim / 2; nAt++)
	{
		if (nAt < m_pSpace->GetNodeCount())
		{
			// get the node
			CNode *pNode = m_pSpace->GetNodeAt(nAt);

			// store the nodes position, plus some random seeding
			m_vState[nAt*2] = pNode->GetPosition()[0]
				+ 0.0 * ( 1.0 - 2.0 * rand() / RAND_MAX);
			m_vState[nAt*2+1] = pNode->GetPosition()[1]
				+ 0.0 * ( 1.0 - 2.0 * rand() / RAND_MAX);
		}
	}

}	// CSpaceView::Pos2StateVector


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::StateVector2Pos
// 
// loads the node positions from the state vector
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::StateVector2Pos()
{
	// test for dimensionality
	ASSERT(m_vState.GetDim() == m_nStateDim);

	// form the number of currently visualized node views
	for (int nAt = 0; nAt < m_pSpace->GetSuperNodeCount(); nAt++)
	{
		// get the node
		CNode *pNode = m_pSpace->GetNodeAt(nAt);

		// form the node view's position
		CVectorD<3> vPosition(m_vState[nAt*2], m_vState[nAt*2+1]);

		// set the node's position
		pNode->SetPosition(vPosition);
	}

}	// CSpaceLayoutManager::StateVector2Pos


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::operator()
// 
// evaluates the energy function at the given point
//////////////////////////////////////////////////////////////////////
REAL CSpaceLayoutManager::operator()(const CVectorN<REAL>& vInput,
									 CVectorN<> *pGrad)
{
	// check dimensionality
	ASSERT(vInput.GetDim() == m_nStateDim);

	// reset the energy
	m_energy = 0.0f;

	if (NULL != pGrad)
	{
		// initialize the gradient vector to zeros
		m_vGrad.SetDim(vInput.GetDim());
		m_vGrad.SetZero();
	}

	// store this input
	m_vInput = vInput;

	// form the number of currently visualized node views
	int nRealNodeCount = m_pSpace->GetSuperNodeCount(); 

	// and the total number of nodes plus clusters
	int nVizNodeCount = m_pSpace->GetSuperNodeCount();

	// iterate over all current visualized node views
	int nAtNodeView;
	for (nAtNodeView = 0; nAtNodeView < nVizNodeCount; nAtNodeView++)
	{
		// iterate over the potential linked views
		int nAtLinkedView;
		for (nAtLinkedView = 0; nAtLinkedView < nVizNodeCount; nAtLinkedView++)
		{
			// only processs the linked view if it is in the vector
			if (nAtLinkedView != nAtNodeView) 
			{
				//////////////////////////////////////////////////////////////
				// set up some common values

				// compute the x- and y-offset between the views
				REAL x = vInput[nAtNodeView*2 + 0] - vInput[nAtLinkedView*2 + 0];
				REAL y = vInput[nAtNodeView*2 + 1] - vInput[nAtLinkedView*2 + 1];

				// compute the x- and y-scales for the fields -- average of
				//		two rectangles
				REAL ssx = (m_vSize[nAtNodeView][0] 
					+ m_vSize[nAtLinkedView][0]) / 2.0; 
				REAL ssy = (m_vSize[nAtNodeView][1] 
					+ m_vSize[nAtLinkedView][1]) / 2.0; ;

				//////////////////////////////////////////////////////////////
				// compute the attraction field

				// store the weight, for convenience
				REAL weight = m_mLinks[nAtNodeView][nAtLinkedView];

				// compute the relative actual distance
				REAL act_dist = (REAL) sqrt(x * x / (ssx * ssx)
					+ y * y / (ssy * ssy)) + 0.001;

				// compute the optimal distance
				const REAL MIDWEIGHT = 0.5;
				REAL opt_dist = DIST_SCALE_MIN
					+ (DIST_SCALE_MAX - DIST_SCALE_MIN) 
						* (1.0 - MIDWEIGHT / (MIDWEIGHT + ACTIVATION_MIDPOINT));

				// compute the distance error
				REAL dist_error = act_dist - opt_dist;

				// compute the factor controlling the importance of the
				//		attraction term
				REAL factor = m_k_pos * weight * m_act[nAtNodeView];

				// and add the attraction term to the energy
				m_energy += factor * dist_error * dist_error;

				if (NULL != pGrad)
				{
					// compute d_energy
					REAL dact_dist_dx = x / (ssx * ssx) / act_dist;
					REAL dact_dist_dy = y / (ssy * ssy) / act_dist;

					// compute the gradient terms
					REAL denergy_dx = factor * 2.0 * dist_error * dact_dist_dx;
					REAL denergy_dy = factor * 2.0 * dist_error * dact_dist_dy;

					// add to the gradient vector
					m_vGrad[nAtNodeView*2   + 0] += denergy_dx;
					m_vGrad[nAtNodeView*2   + 1] += denergy_dy;
					m_vGrad[nAtLinkedView*2 + 0] -= denergy_dx;
					m_vGrad[nAtLinkedView*2 + 1] -= denergy_dy;
				}

				//////////////////////////////////////////////////////////////
				// compute the repulsion field

				// add the repulsion field only for real nodes
				if (nAtLinkedView < nRealNodeCount)
				{
					// compute the energy due to this interation
					REAL dx_ratio = x / (ssx * ssx);
					REAL x_ratio = x * dx_ratio; // = (x * x) / (ssx * ssx);
					REAL dy_ratio = y / (ssy * ssy);
					REAL y_ratio = y * dy_ratio; // = (y * y) / (ssy * ssy);

					// compute the energy term
					REAL inv_sq = 1.0 / (x_ratio + y_ratio + 0.1);
					REAL factor = m_k_rep * m_act[nAtNodeView];

					// add to total energy
					m_energy += factor * inv_sq;

					if (NULL != pGrad)
					{
						// compute gradient terms
						REAL inv_sq_sq = inv_sq * inv_sq;
						REAL dRepulsion_dx = -2.0 * factor * dx_ratio * inv_sq_sq;
						REAL dRepulsion_dy = -2.0 * factor * dy_ratio * inv_sq_sq;

						// add to the gradient vectors
						m_vGrad[nAtNodeView*2   + 0] += dRepulsion_dx;
						m_vGrad[nAtNodeView*2   + 1] += dRepulsion_dy;
						m_vGrad[nAtLinkedView*2 + 0] -= dRepulsion_dx;
						m_vGrad[nAtLinkedView*2 + 1] -= dRepulsion_dy;
					}
				}
			}
		}
	}

	// store the gradient vector if one was passed
	if (NULL != pGrad)
	{
		// set correct dimensionality
		pGrad->SetDim(m_vGrad.GetDim());

		// and assign
		(*pGrad) = m_vGrad;
	}

	// increment evaluation count
	m_nEvaluations++;

	// return the total energy
	return m_energy;

}	// CSpaceLayoutManager::operator()


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::LayoutNodes
// 
// calls the optimizer to lay out the nodes
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::LayoutNodes()
{
	// set up the state vector
	Pos2StateVector();

	// now optimize
	LoadSizesLinks();

	// reset evaluation count
	m_nEvaluations = 0;

	// form the state vector
	CVectorN<> vNewState;
	vNewState.SetDim(m_nStateDim);

	// perform the optimization
	m_pOptimizer->SetTolerance(GetTolerance());
	vNewState = m_pOptimizer->Optimize(m_vState);

	// now determine a rotation/translation that minimizes the squared
	//		difference between previous and current state
	RotateTranslateStateVector(m_vState, vNewState);

	// assign the new state vector
	m_vState = vNewState;

	// set the resulting positions
	StateVector2Pos();

	for (int nAt = m_pSpace->GetSuperNodeCount(); 
			nAt < m_pSpace->GetNodeCount(); nAt++)
	{
		// get the node
		CNode *pNode = m_pSpace->GetNodeAt(nAt);

		// see if there is a max activator
		if (pNode->GetMaxActivator() != NULL)
		{
			// if so, set position to it
			pNode->SetPosition(pNode->GetMaxActivator()->GetPosition());
		}
	}

}	// CSpaceLayoutManager::LayoutNodes


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::RotateTranslateStateVector
// 
// called to minimize rotate/translate error between old state
//		and new
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::RotateTranslateStateVector(const CVectorN<>& vOldState,
													 CVectorN<>& vNewState)
{
	// form the Nx3 matrix of pre-coordinates (homogeneous)
	CMatrixNxM<> mOld(m_pSpace->GetSuperNodeCount(), 3);
	for (int nAt = 0; nAt < mOld.GetCols(); nAt++)
	{
		mOld[nAt][0] = vOldState[nAt * 2];
		mOld[nAt][1] = vOldState[nAt * 2 + 1];
		mOld[nAt][2] = 1.0;
	}

	// form the Nx3 matrix of post-coordinates (homogeneous)
	CMatrixNxM<> mNew(m_pSpace->GetSuperNodeCount(), 3);
	for (nAt = 0; nAt < mNew.GetCols(); nAt++)
	{
		mNew[nAt][0] = vNewState[nAt * 2];
		mNew[nAt][1] = vNewState[nAt * 2 + 1];
		mNew[nAt][2] = 1.0;
	}

	// work with the transpose of the problem, so that columns < rows
	mOld.Transpose();
	mNew.Transpose();

	// form the pseudo-inverse of the a coordinates
	CMatrixNxM<> mNew_ps = mNew;
	if (mNew_ps.Pseudoinvert())
	{
		// form the transform matrix
		CMatrixNxM<> mTransform = mNew_ps * mOld;

		// un-transpose the matrices
		mNew.Transpose();
		mOld.Transpose();
		mTransform.Transpose();

		// store the offset part
		CVectorN<> offset = mTransform[2];

		// reshape to isolate upper 2x2
		mTransform.Reshape(2, 2);

		// now turn the transform matrix upper 2x2
		//		into an orthogonal matrix
		CVectorN<> w(2);
		CMatrixNxM<> v(2, 2);
		if (mTransform.SVD(w, v))
		{
			// form the orthogonal matrix
			v.Transpose();
			mTransform *= v;

			// restore the translation part
			mTransform.Reshape(3, 3);
			mTransform[2][0] = offset[0];
			mTransform[2][1] = offset[1];
			mTransform[2][2] = 1.0;

			// transform the points
			mNew = mTransform * mNew;

			// now repopulate
			for (nAt = 0; nAt < mNew.GetCols(); nAt++)
			{
				vNewState[nAt * 2] = mNew[nAt][0];
				vNewState[nAt * 2 + 1] = mNew[nAt][1];
			}
		}
	}
	else
	{
		TRACE("CSpaceLayoutManager::RotateTranslateStateVector: No pseudoinverse\n");
	}

}	// CSpaceLayoutManager::RotateTranslateStateVector



//////////////////////////////////////////////////////////////////////
// SpaceLayoutManager.cpp: implementation of the 
//		CSpaceLayoutManager objective function.
//
// Copyright (C) 1996-2001
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// defines the gaussian function
#include <MathUtil.h>

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
const REAL TOLERANCE = (REAL) 0.1;

// scale for the sizes of the nodes
const REAL SIZE_SCALE = 100.0;

// constants for the distance scale vs. activation curve
const REAL DIST_SCALE_MIN = 1.0;
const REAL DIST_SCALE_MAX = 1.5;
const REAL ACTIVATION_MIDPOINT = 0.25;

// constant for weighting the position energy
const REAL K_POS = 600.0;

// constant for weighting the repulsion energy
const REAL K_REP = 200.0;


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::CSpaceLayoutManager
// 
// constructs an energy function object for the associated space 
//		view
//////////////////////////////////////////////////////////////////////
CSpaceLayoutManager::CSpaceLayoutManager(CSpace *pSpace)
	: CObjectiveFunction(TRUE),
		m_pSpace(pSpace),

		m_nStateDim(MAX_STATE_DIM / 2),

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
}


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
}


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
		CVector<3> vSize = pAtNode->GetSize(pAtNode->GetActivation());

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

	// now load the clusters
	int nAtCluster;
	for (nAtCluster = 0; nAtCluster < m_pSpace->GetClusterCount(); nAtCluster++)
	{
		// get convenience pointers for the current node view and node
		CNodeCluster *pCluster = m_pSpace->GetClusterAt(nAtCluster);

		// compute the x- and y-scales for the fields (from the linked rectangle)
		CVector<3> vSize = pCluster->GetSize(pCluster->GetActivation());

		// store the size -- add 10 to ensure non-zero sizes
		m_vSize[m_pSpace->GetSuperNodeCount() + nAtCluster][0] = 
			SIZE_SCALE * vSize[0] + 10.0;
		m_vSize[m_pSpace->GetSuperNodeCount() + nAtCluster][1] = 
			SIZE_SCALE * vSize[1] + 10.0f;

		// store the activation
		m_act[m_pSpace->GetSuperNodeCount() + nAtCluster] = 
			pCluster->GetActivation();

		// iterate over the potential linked views
		int nAtLinkedNode;
		for (nAtLinkedNode = 0; nAtLinkedNode < m_pSpace->GetSuperNodeCount(); 
				nAtLinkedNode++)
		{
			// get convenience pointers for the linked node view and node
			CNode *pAtLinkedNode = m_pSpace->GetNodeAt(nAtLinkedNode);

			// retrieve the link weight for layout
			REAL weight = (REAL) pCluster->GetLinkWeight(pAtLinkedNode);

			// add some weight to make it non-zero
			weight += 0.0001f;

			// store the link weight
			m_mLinks[m_pSpace->GetSuperNodeCount() + nAtCluster]
				[nAtLinkedNode] = weight;
			m_mLinks[nAtLinkedNode]
				[m_pSpace->GetSuperNodeCount() + nAtCluster] = weight;
		}

		// load cluster-cluster links
		int nAtOtherCluster;
		for (nAtOtherCluster = 0; nAtOtherCluster < m_pSpace->GetClusterCount(); nAtOtherCluster++)
		{
			// get convenience pointers for the current node view and node
			CNodeCluster *pOtherCluster = m_pSpace->GetClusterAt(nAtOtherCluster);

			// retrieve the link weight for layout
			REAL weight = (REAL) 0.5 *
				(pCluster->GetLinkWeight(pOtherCluster) +
				pOtherCluster->GetLinkWeight(pCluster));

			// add some weight to make it non-zero
			weight += 0.0001f;

			// store the link weight
			m_mLinks[m_pSpace->GetSuperNodeCount() + nAtCluster]
				[m_pSpace->GetSuperNodeCount() + nAtOtherCluster] = weight;
			m_mLinks[m_pSpace->GetSuperNodeCount() + nAtOtherCluster]
				[m_pSpace->GetSuperNodeCount() + nAtCluster] = weight;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// CSpaceView::Pos2StateVector
// 
// forms the state vector from the node positions
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::Pos2StateVector()
{
	// for the state vector
	m_vState.SetDim(m_nStateDim);

	// store the number of clusters
	int nClusters = m_pSpace->GetClusterCount();

	// iterate for non-cluster nodes
	for (int nAt = 0; nAt < m_nStateDim / 2 - nClusters; nAt++)
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

	// iterate over the clusters
	for (int nAtCluster = 0; nAtCluster < nClusters; nAtCluster++)
	{
		// get the cluster
		CNodeCluster *pCluster = m_pSpace->GetClusterAt(nAtCluster);

		// store the cluster's position
		m_vState[(m_nStateDim / 2 - nClusters + nAtCluster)*2] = 
			pCluster->GetPosition()[0];
		m_vState[(m_nStateDim / 2 - nClusters + nAtCluster)*2+1] = 
			pCluster->GetPosition()[1];
	}
}


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
		CVector<3> vPosition(m_vState[nAt*2], m_vState[nAt*2+1]);

		// set the node's position
		pNode->SetPosition(vPosition);
	}

	// compute the number of clusters
	int nClusters = m_pSpace->GetClusterCount();

	// iterate over the clusterss
	for (int nAtCluster = 0; nAtCluster < nClusters; nAtCluster++)
	{
		// get the cluster
		CNodeCluster *pCluster = m_pSpace->GetClusterAt(nAtCluster);

		// form the cluster's position
		CVector<3> vPosition(
			m_vState[(m_nStateDim / 2 - nClusters + nAtCluster)*2],
			m_vState[(m_nStateDim / 2 - nClusters + nAtCluster)*2+1]);

		// and set the cluster's position
		pCluster->SetPosition(vPosition);
	}
}


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
	int nVizNodeCount = m_pSpace->GetSuperNodeCount() 
		+ m_pSpace->GetClusterCount();

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
				REAL opt_dist = DIST_SCALE_MIN
					+ (DIST_SCALE_MAX - DIST_SCALE_MIN) 
						* (1.0 - weight / (weight + ACTIVATION_MIDPOINT));

				// compute the distance error
				REAL dist_error = act_dist - opt_dist;

				// compute the factor controlling the importance of the
				//		attraction term
				REAL factor = K_POS * weight * weight * m_act[nAtNodeView];

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
					REAL inv_sq = 1.0 / (x_ratio + y_ratio + 0.1f);
					REAL factor = K_REP * m_act[nAtNodeView];

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
}


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::LayoutNodes()
// 
// calls the optimizer to lay out the nodes
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::LayoutNodes()
{
#ifdef _DEBUG_MEM
	// used to check for memory leaks
	CMemoryState memState;
	memState.Checkpoint();
#endif

	// set up the state vector
	Pos2StateVector();

	// now optimize
	LoadSizesLinks();

	// reset evaluation count
	m_nEvaluations = 0;

#ifdef _DEBUG_MEM
	memState.DumpAllObjectsSince();
#endif

	// form the state vector
	CStateVector vNewState;
	vNewState.SetDim(m_nStateDim);

	// perform the optimization
	vNewState = m_pOptimizer->Optimize(m_vState);

#ifdef TRACE_ITERATIONS_PER_LAYOUT
	LOG_TRACE("Iterations for layout = %i\n", m_nEvaluations);
#endif

#ifdef _DEBUG_MEM
	memState.DumpAllObjectsSince();
#endif

	// now determine a rotation/translation that minimizes the squared
	//		difference between previous and current state
	RotateTranslateStateVector(&vNewState, m_vState);

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
		else
		{
			// otherwise, set the position to the nearest cluster
			CNodeCluster *pCluster = m_pSpace->GetClusterAt(0);
			if (NULL != pCluster)
			{
				CNodeCluster *pNearestCluster = 
					pCluster->GetNearestCluster(pNode);

				// set position to the closest cluster
				if (pNearestCluster)
				{
					pNode->SetPosition(pNearestCluster->GetPosition());
				}
			}
		}
	}

#ifdef _DEBUG_MEM
	memState.DumpAllObjectsSince();
#endif
}

//////////////////////////////////////////////////////////////////////
// class CLeastSquaresFit2D
// 
// objective function to compute a 2D rigid transform relating two
//		sets of points.
// input vector is a 3D vector with first two elements = x, y offset
//		and last element = angle
//////////////////////////////////////////////////////////////////////
class CLeastSquaresFit2D : public CObjectiveFunction
{
public:
	// construct the objective function
	CLeastSquaresFit2D(const CSpaceLayoutManager::CStateVector& vOldState, 
			const CSpaceLayoutManager::CStateVector& vNewState);

	// evaluates the objective function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL);

	// transforms the input state vector
	void Transform(const CSpaceLayoutManager::CStateVector& vState, 
			CVector<2> vOffset, double angle, 
			CSpaceLayoutManager::CStateVector *pXform);

private:
	// stores the old state
	CSpaceLayoutManager::CStateVector m_vOldState;

	// stores the new state
	CSpaceLayoutManager::CStateVector m_vNewState;
};


//////////////////////////////////////////////////////////////////////
// CLeastSquaresFit2D::CLeastSquaresFit2D
// 
// construct the objective function
//////////////////////////////////////////////////////////////////////
CLeastSquaresFit2D::CLeastSquaresFit2D(
		const CSpaceLayoutManager::CStateVector& vOldState, 
		const CSpaceLayoutManager::CStateVector& vNewState)
	: CObjectiveFunction(FALSE),
		m_vOldState(vOldState),
		m_vNewState(vNewState)
{
}

//////////////////////////////////////////////////////////////////////
// CLeastSquaresFit2D::operator()
// 
// evaluates the objective function
//////////////////////////////////////////////////////////////////////
REAL CLeastSquaresFit2D::operator()(const CVectorN<>& vInput, 
									CVectorN<> *pGrad)
{
	// check for valid input
	ASSERT(!_isnan(vInput[0]));
	ASSERT(!_isnan(vInput[1]));
	ASSERT(!_isnan(vInput[2]));

	// form the offset
	CVector<2> vOffset = CVector<2>(vInput);

	// stores the transformed state
	CSpaceLayoutManager::CStateVector vXformState;

	// transform the state
	Transform(m_vNewState, vOffset, vInput[2], &vXformState);

	// form the difference square
	double diff_sq = 0.0;
	for (int nAt = 0; nAt < vXformState.GetDim(); nAt += 2)
	{
		// compute difference squared
		diff_sq += (vXformState[nAt] - m_vOldState[nAt])
			 * (vXformState[nAt] - m_vOldState[nAt]);
	}

	// check for valid diff_sq
	ASSERT(!_isnan(diff_sq));

	// return as energy
	return diff_sq;
}

//////////////////////////////////////////////////////////////////////
// CLeastSquaresFit2D::Transform
// 
// transforms the input state vector
//////////////////////////////////////////////////////////////////////
void CLeastSquaresFit2D::Transform(
		const CSpaceLayoutManager::CStateVector& vState, 
		CVector<2> vOffset, double angle, 
		CSpaceLayoutManager::CStateVector *pXform)
{
	// set dimensions of receiving state vector
	pXform->SetDim(vState.GetDim());

	// create a rotation matrix
	CMatrix<2> mRotate = CreateRotate(angle);

	// for each pair of coordinates
	for (int nAt = 0; nAt < vState.GetDim(); nAt += 2)
	{
		// check state
		ASSERT(!_isnan(vState[nAt + 0]));
		ASSERT(!_isnan(vState[nAt + 1]));

		// form the position vector
		CVector<2> vPos;
		vPos[0] = vState[nAt + 0];
		vPos[1] = vState[nAt + 1];

		// rotate translate the position vector
		CVector<2> vXformPos = mRotate * vPos + vOffset;

		// check transformed coordinate
		ASSERT(!_isnan(vXformPos[0]));
		ASSERT(!_isnan(vXformPos[1]));

		// store in new state vector
		(*pXform)[nAt + 0] = vXformPos[0];
		(*pXform)[nAt + 1] = vXformPos[1];
	}
}

//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::RotateTranslateStateVector
// 
// called to minimize rotate/translate error between old state
//		and new
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::RotateTranslateStateVector(CStateVector *pState, 
											const CStateVector &vOldState)
{
	// objective function
	CLeastSquaresFit2D fitFunction(*pState, vOldState);
	
	// optimizer for the fitness function
	CPowellOptimizer optimizer(&fitFunction);
	optimizer.SetTolerance(0.001);

	// optimize and return the transform
	CVector<3> xform = CVector<3>(optimizer.Optimize(
		CVector<3>(0.0, 0.0, 0.0)));

	// trace output
	TRACE("Transforming by offset %lf, %lf, angle %lf\n", 
		xform[0], xform[1], xform[2]);

	// retrieve the resulting state vector
	CStateVector vXformState;
	fitFunction.Transform((*pState), CVector<2>(xform), xform[2], 
		&vXformState);

	// trace output
	TraceVector(vXformState);

	// and return
	(*pState) = vXformState;
}

int CSpaceLayoutManager::GetStateDim() const
{
	return m_nStateDim;
}

void CSpaceLayoutManager::SetStateDim(int nStateDim)
{
	m_nStateDim = nStateDim;
}

double CSpaceLayoutManager::GetKPos()
{
	return K_POS;
}

void CSpaceLayoutManager::SetKPos(double k_pos)
{
}

double CSpaceLayoutManager::GetKRep()
{
	return K_REP;
}

void CSpaceLayoutManager::SetKRep(double k_rep)
{
}

double CSpaceLayoutManager::GetTolerance()
{
	return TOLERANCE;
}

void CSpaceLayoutManager::SetTolerance(double tolerance)
{
}

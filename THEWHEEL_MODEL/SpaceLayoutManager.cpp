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

// header files for the class
#include "SpaceLayoutManager.h"
#include "Space.h"

/////////////////////////////////////////////////////////////////////////////
// Constants for the CSpaceView
/////////////////////////////////////////////////////////////////////////////

// constant for the tolerance of the optimization
const REAL TOLERANCE = (REAL) 0.1;

#define SIGMA_POINT 1.60

const REAL k = 
	(0.4f - (REAL) exp(-SIGMA_POINT * SIGMA_POINT / 2.0f) / (REAL) (sqrt(2.0f * PI)));

//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::CSpaceLayoutManager
// 
// constructs an energy function object for the associated space 
//		view
//////////////////////////////////////////////////////////////////////
CSpaceLayoutManager::CSpaceLayoutManager(CSpace *pSpace)
	: CObjectiveFunction(TRUE),
		m_pSpace(pSpace),

		m_vInput(CVectorN<>()),
		m_energy(0.0),

		m_pPowellOptimizer(NULL)
{
	m_pPowellOptimizer = new CPowellOptimizer(this);
	m_pPowellOptimizer->SetTolerance(TOLERANCE);

	m_pConjGradOptimizer = new CConjGradOptimizer(this);
	m_pConjGradOptimizer->SetTolerance(TOLERANCE);

	m_pGradDescOptimizer = new CGradDescOptimizer(this);
	m_pGradDescOptimizer->SetTolerance(TOLERANCE);
}


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::~CSpaceLayoutManager
// 
// destroys the layout manager
//////////////////////////////////////////////////////////////////////
CSpaceLayoutManager::~CSpaceLayoutManager()
{
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
		REAL ssx = 100.0f * (REAL) vSize[0];
		REAL ssy = 100.0f * (REAL) vSize[1];

		// store the size
		m_vSize[nAtNode][0] = ssx + 10.0f;
		m_vSize[nAtNode][1] = ssy + 10.0f;
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
				REAL weight = (REAL) 0.5 *
					(pAtNode->GetLinkWeight(pAtLinkedNode)
					+ pAtLinkedNode->GetLinkWeight(pAtNode));
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
		REAL ssx = 100.0f * (REAL) vSize[0];
		REAL ssy = 100.0f * (REAL) vSize[1];

		// store the size
		m_vSize[m_pSpace->GetSuperNodeCount() + nAtCluster][0] = ssx + 10.0f;
		m_vSize[m_pSpace->GetSuperNodeCount() + nAtCluster][1] = ssy + 10.0f;
		m_act  [m_pSpace->GetSuperNodeCount() + nAtCluster]    = pCluster->GetActivation();

		// iterate over the potential linked views
		int nAtLinkedNode;
		for (nAtLinkedNode = 0; nAtLinkedNode < m_pSpace->GetSuperNodeCount(); nAtLinkedNode++)
		{
			// get convenience pointers for the linked node view and node
			CNode *pAtLinkedNode = m_pSpace->GetNodeAt(nAtLinkedNode);

			// retrieve the link weight for layout
			REAL weight = (REAL) pCluster->GetLinkWeight(pAtLinkedNode);
			weight += 0.0001f;

			// store the link weight
			m_mLinks[m_pSpace->GetSuperNodeCount() + nAtCluster][nAtLinkedNode] = weight;
			m_mLinks[nAtLinkedNode][m_pSpace->GetSuperNodeCount() + nAtCluster] = weight;
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
	m_vState.SetDim(STATE_DIM);

	int nClusters = m_pSpace->GetClusterCount();

	for (int nAt = 0; nAt < STATE_DIM / 2 - nClusters; nAt++)
	{
		if (nAt < m_pSpace->GetNodeCount())
		{
			CNode *pNode = m_pSpace->GetNodeAt(nAt);

			m_vState[nAt*2] = pNode->GetPosition()[0];
			m_vState[nAt*2+1] = pNode->GetPosition()[1];
		}
	}

	for (int nAtCluster = 0; nAtCluster < nClusters; nAtCluster++)
	{
		CNodeCluster *pCluster = m_pSpace->GetClusterAt(nAtCluster);
		m_vState[(STATE_DIM / 2 - nClusters + nAtCluster)*2] = pCluster->GetPosition()[0];
		m_vState[(STATE_DIM / 2 - nClusters + nAtCluster)*2+1] = pCluster->GetPosition()[1];
	}
}


//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::StateVector2Pos
// 
// loads the node positions from the state vector
//////////////////////////////////////////////////////////////////////
void CSpaceLayoutManager::StateVector2Pos()
{
	ASSERT(m_vState.GetDim() == STATE_DIM);

	// form the number of currently visualized node views
	for (int nAt = 0; nAt < m_pSpace->GetSuperNodeCount(); nAt++)
	{
		CVector<3> vNewCenter = CVector<3>(m_vState[nAt*2], m_vState[nAt*2+1], 0.0);

		CNode *pNode = m_pSpace->GetNodeAt(nAt);
		pNode->SetPosition(vNewCenter);
	}

	int nClusters = m_pSpace->GetClusterCount();

	for (int nAtCluster = 0; nAtCluster < nClusters; nAtCluster++)
	{
		CNodeCluster *pCluster = m_pSpace->GetClusterAt(nAtCluster);

		CVector<3> vPosition(
			m_vState[(STATE_DIM / 2 - nClusters + nAtCluster)*2],
			m_vState[(STATE_DIM / 2 - nClusters + nAtCluster)*2+1],
			0.0);
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
	ASSERT(vInput.GetDim() == STATE_DIM);

	m_nEvaluations++;

	// reset the energy
	m_energy = 0.0f;

	// initialize the gradient vector to zeros
	m_vGrad.SetDim(vInput.GetDim());
	m_vGrad.SetZero();

	// store this input
	m_vInput = vInput;

	// form the number of currently visualized node views
	int nRealNodeCount = m_pSpace->GetSuperNodeCount(); 
	int nVizNodeCount = m_pSpace->GetSuperNodeCount() + m_pSpace->GetClusterCount();

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
				// compute the x- and y-offset between the views
				REAL x = vInput[nAtNodeView*2 + 0] - vInput[nAtLinkedView*2 + 0];
				REAL y = vInput[nAtNodeView*2 + 1] - vInput[nAtLinkedView*2 + 1];

				// compute the x- and y-scales for the fields (from the linked rectangle)
				// CRect rectLinked = pAtLinkedView->GetOuterRect();
				REAL ssx = m_vSize[nAtLinkedView][0]; 
				REAL ssy = m_vSize[nAtLinkedView][1];

				// compute the energy due to this interation
				REAL dx_ratio = x / (ssx * ssx);
				REAL x_ratio = x * dx_ratio; // (x * x) / (ssx * ssx);
				REAL dy_ratio = y / (ssy * ssy);
				REAL y_ratio = y * dy_ratio; // (y * y) / (ssy * ssy);

				// add the repulsion field
				if (nAtLinkedView < nRealNodeCount)
				{
					REAL inv_sq = 1.0f / (x_ratio + y_ratio + 0.1f);
					m_energy += 64.0f * inv_sq;

					// compute gradient
					REAL inv_sq_sq = inv_sq * inv_sq;
					REAL dRepulsion_dx = -2.0 * 64.0 * dx_ratio * inv_sq_sq;
					REAL dRepulsion_dy = -2.0 * 64.0 * dy_ratio * inv_sq_sq;
					m_vGrad[nAtNodeView*2   + 0] += dRepulsion_dx;
					m_vGrad[nAtNodeView*2   + 1] += dRepulsion_dy;
					m_vGrad[nAtLinkedView*2 + 0] -= dRepulsion_dx;
					m_vGrad[nAtLinkedView*2 + 1] -= dRepulsion_dy;
				}

				// add attraction * weight
				REAL weight = m_mLinks[nAtNodeView][nAtLinkedView] 
					* (REAL) 400.0;
				REAL inner_scale = 
					2.0f * (m_act[nAtNodeView] + m_act[nAtLinkedView]) * k;
				m_energy -= weight * (0.4f - inner_scale * (x_ratio + y_ratio));

				// compute gradient
				m_vGrad[nAtNodeView*2   + 0] -= -weight * inner_scale * 2.0 * dx_ratio;
				m_vGrad[nAtNodeView*2   + 1] -= -weight * inner_scale * 2.0 * dy_ratio;
				m_vGrad[nAtLinkedView*2 + 0] += -weight * inner_scale * 2.0 * dx_ratio;
				m_vGrad[nAtLinkedView*2 + 1] += -weight * inner_scale * 2.0 * dy_ratio;
			}
		}

#ifdef USE_POSITIONAL_ENERGY
		// now include a positional energy term
		// retrieve the parent's rectangle
		CRect rectSpaceView;
		m_pView->GetWindowRect(&rectSpaceView);

		// get convenience pointers for the current node view and node
		REAL x = vInput[nAtNodeView*2 + 0] - rectSpaceView.Width() / 2.0f;
		REAL y = vInput[nAtNodeView*2 + 1] - rectSpaceView.Height() / 2.0f;

		REAL ssx = rectSpaceView.Width() / 4.0f + 10.0f;
		REAL ssy = rectSpaceView.Height() / 4.0f + 10.0f;
		REAL ssz = rectSpaceView.Width() / 2.0f + 10.0f;

		m_energy +=  // 1000.0 * act * act 
			0.001f * m_vSize[nAtNodeView][0] // * m_vSize[nAtNodeView][1] 
				* (x * x / (ssx * ssx) + y * y / (ssy * ssy)); 
#endif
	}

	if (NULL != pGrad)
	{
		pGrad->SetDim(m_vGrad.GetDim());
		(*pGrad) = m_vGrad;
	}

	return m_energy;
}

void CSpaceLayoutManager::LayoutNodes()
{
#ifdef _DEBUG
	// used to check for memory leaks
	CMemoryState memState;
	memState.Checkpoint();
#endif

	// set up the state vector
	Pos2StateVector();

	// now optimize
	LoadSizesLinks();

	m_nEvaluations = 0;

#ifdef _DEBUG
	memState.DumpAllObjectsSince();
#endif

#define TRACE_ITERATIONS_PER_LAYOUT
#ifdef TRACE_ITERATIONS_PER_LAYOUT
	LOG_TRACE("Starting layout...");
#endif

#ifdef DUMP_GRADIENT
	{
	// test the gradient function
	CVectorN<> vGrad(STATE_DIM);
	double final_energy = (*this)(m_vState, &vGrad);
	for (int nAt = 0; nAt < STATE_DIM; nAt++)
	{
		m_vState[nAt] += 0.0001;
		double delta_energy = (*this)(m_vState);
		m_vState[nAt] -= 0.0001;

		TRACE("Gradient element %2i = %10.6lf, delta = %10.6lf\n",
			nAt, vGrad[nAt], (delta_energy - final_energy) * 10000.0);
	}
	}
#endif

	CStateVector vNewState;

	{
	for (int nAt = 0; nAt < m_vState.GetDim(); nAt++)
		ASSERT(!_isnan(m_vState[nAt]));
	}

	// perform the optimization
	if (false) // 
		// rand() > RAND_MAX / 2)
	{
		LOG_TRACE("Powell optimization\n");
		vNewState = m_pPowellOptimizer->Optimize(m_vState);
	}
	else if (true)
	{
		LOG_TRACE("Conj grad optimization\n");
		vNewState = m_pConjGradOptimizer->Optimize(m_vState);
	}
	else
	{
		LOG_TRACE("Grad desc optimization\n");
		vNewState = m_pGradDescOptimizer->Optimize(m_vState);
	}

	{
	for (int nAt = 0; nAt < m_vState.GetDim(); nAt++)
		ASSERT(!_isnan(vNewState[nAt]));
	}

#ifdef TRACE_ITERATIONS_PER_LAYOUT
	LOG_TRACE("Iterations for layout = %i\n", m_nEvaluations);
#endif

#ifdef _DEBUG
	memState.DumpAllObjectsSince();
#endif

	// now determine a rotation/translation that minimizes the squared
	//		difference between previous and current state
	RotateTranslateStateVector(&vNewState, m_vState);
	m_vState = vNewState;

	// set the resulting positions
	StateVector2Pos();

	// for the subthreshold nodes, set the position to the nearest cluster
	for (int nAt = m_pSpace->GetSuperNodeCount(); 
			nAt < m_pSpace->GetNodeCount(); nAt++)
	{
		CNode *pNode = m_pSpace->GetNodeAt(nAt);

		if (pNode->GetMaxActivator() != NULL)
		{
			pNode->SetPosition(pNode->GetMaxActivator()->GetPosition());
		}
		else
		{
			CNodeCluster *pCluster = m_pSpace->GetClusterAt(0);
			if (pCluster)
			{
				CNodeCluster *pNearestCluster = 
					pCluster->GetNearestCluster(pNode);

				// set position to the closest cluster
				if (pNearestCluster)
					pNode->SetPosition(pNearestCluster->GetPosition());
			}
		}
	}

#ifdef _DEBUG
	memState.DumpAllObjectsSince();
#endif
}

// objective function to compute a 2D rigid transform relating two
//		sets of points.
// input vector is a 3D vector with first two elements = x, y offset
//		and last element = angle
class CLeastSquaresFit2D : public CObjectiveFunction
{
public:
	CLeastSquaresFit2D(const CSpaceLayoutManager::CStateVector& vOldState, const CSpaceLayoutManager::CStateVector& vNewState)
		: CObjectiveFunction(FALSE),
			m_vOldState(vOldState),
			m_vNewState(vNewState)
	{
	}

	// evaluates the objective function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL)
	{
		ASSERT(!_isnan(vInput[0]));
		ASSERT(!_isnan(vInput[1]));
		ASSERT(!_isnan(vInput[2]));
		CVector<2> vOffset = CVector<2>(vInput);

		CSpaceLayoutManager::CStateVector vXformState;
		Transform(m_vNewState, vOffset, vInput[2], &vXformState);

		double diff_sq = 0.0;
		for (int nAt = 0; nAt < vXformState.GetDim(); nAt += 2)
		{
			// compute difference squared
			diff_sq += (vXformState[nAt] - m_vOldState[nAt])
				 * (vXformState[nAt] - m_vOldState[nAt]);
		}

		ASSERT(!_isnan(diff_sq));

		TRACE("diff_sq = %lf\n", diff_sq);
		return diff_sq;
	}

	void Transform(const CSpaceLayoutManager::CStateVector& vState, CVector<2> vOffset, double angle, 
					CSpaceLayoutManager::CStateVector *pXform)
	{
		// set dimensions of receiving state vector
		pXform->SetDim(vState.GetDim());

		// create a rotation matrix
		CMatrix<2> mRotate = CreateRotate(angle);

		for (int nAt = 0; nAt < vState.GetDim(); nAt += 2)
		{
			ASSERT(!_isnan(vState[nAt + 0]));
			ASSERT(!_isnan(vState[nAt + 1]));

			// form the position vector
			CVector<2> vPos;
			vPos[0] = vState[nAt + 0];
			vPos[1] = vState[nAt + 1];

			// rotate translate the position vector
			CVector<2> vXformPos = mRotate * vPos + vOffset;

			ASSERT(!_isnan(vXformPos[0]));
			ASSERT(!_isnan(vXformPos[1]));

			// store in new state vector
			(*pXform)[nAt + 0] = vXformPos[0];
			(*pXform)[nAt + 1] = vXformPos[1];
		}
	}

private:
	CSpaceLayoutManager::CStateVector m_vOldState;
	CSpaceLayoutManager::CStateVector m_vNewState;
};


void CSpaceLayoutManager::RotateTranslateStateVector(CStateVector *pState, 
													 const CStateVector &vOldState)
{
	CLeastSquaresFit2D fitFunction(*pState, vOldState);
	double v0 = fitFunction(CVector<3>(0.0, 0.0, 0.0));
	double v1 = fitFunction(CVector<3>(-100.0, -100.0, 0.0));
	double v2 = fitFunction(CVector<3>(0.0, 0.0, 3.1));

	CPowellOptimizer optimizer(&fitFunction);

	CVector<3> xform = CVector<3>(optimizer.Optimize(CVector<3>(0.0, 0.0, 0.0)));

	TRACE("Transforming by offset %lf, %lf, angle %lf\n", xform[0], xform[1], xform[2]);

	CStateVector vXformState;
	fitFunction.Transform((*pState), CVector<2>(xform), xform[2], &vXformState);
	TraceVector(vXformState);

	(*pState) = vXformState;
}


// computes the distance scale as a function of weight between two nodes
// the distance scale is multiplied by the average radius of the two nodes
//		to obtain the distance between the centers of the nodes
double distance_scale(double weight)
{
	const double min = 1.0;
	const double max = 4.0;
	const double k_slope = 8.0;
	const double midpoint = 0.5;

	return min + (max - min) * (1.0 - k_slope * weight / (weight + midpoint));
}

double node_radius(double act)
{
	return act * 100.0;
}

// computes the energy due to two interacting nodes
double energy_function(CVector<2> vPos1, double act1, CVector<2> vPos2, double act2, double weight)
{
	double act_dist = (vPos1 - vPos2).GetLength();
	double opt_dist = (node_radius(act1) + node_radius(act2)) / 2.0 * distance_scale(weight);

	double delta_dist = act_dist - opt_dist;

	// constant for weighting the position energy
	const double k_pos = 1.0;

	double factor = k_pos * weight * (act1 + act2);
	return  factor * delta_dist * delta_dist;

	// compute d_energy
	double dact_dist_dx = (vPos1[0] - vPos2[0]) / act_dist;
	double dact_dist_dy = (vPos1[1] - vPos2[1]) / act_dist;

	double denergy_dx = factor * 2.0 * delta_dist * dact_dist_dx;
	double denergy_dy = factor * 2.0 * delta_dist * dact_dist_dy;
}

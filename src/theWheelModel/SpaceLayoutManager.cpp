//////////////////////////////////////////////////////////////////////
// SpaceLayoutManager.cpp: implementation of the 
//		CSpaceLayoutManager objective function.
//
// Copyright (C) 1996-2003 Derek Graham Lane
// $Id: SpaceLayoutManager.cpp,v 1.25 2007/06/04 02:44:10 Derek Lane Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// defines the gaussian function
#include <MathUtil.h>


// for least-squares
#include <MatrixNxM.h>

// for the free energy log-determinant
#include <vector>

// optimizer for the layout
#include <PowellOptimizer.h>
#include <ConjGradOptimizer.h>
#include <GradDescOptimizer.h>
//#include <DFPOptimizer.h>

// header file for the CSpace object
#include "Space.h"

// header files for the class
#include "SpaceLayoutManager.h"

/////////////////////////////////////////////////////////////////////////////
// Constants for the CSpaceLayoutManager
/////////////////////////////////////////////////////////////////////////////

// constant for the tolerance of the optimization
const REAL TOLERANCE = (REAL) 1e+3;

// scale for the sizes of the nodes
const REAL SIZE_SCALE = 150.0;

// constants for the distance scale vs. activation curve
const REAL DIST_SCALE_MIN = 1.0f;
const REAL DIST_SCALE_MAX = 1.35f;
const REAL ACTIVATION_MIDPOINT = 0.25f;

// compute the optimal normalized distance
const REAL MIDWEIGHT = 0.5f;
const REAL OPT_DIST = DIST_SCALE_MIN
	+ (DIST_SCALE_MAX - DIST_SCALE_MIN) 
		* (1.0f - MIDWEIGHT / (MIDWEIGHT + ACTIVATION_MIDPOINT));

// constant for weighting the position energy
const REAL K_POS = 600.0f;

// constant for weighting the repulsion energy
const REAL K_REP = 3200.0f;

// extents of the wrapped space used by GetDistError
const REAL WRAP_WIDTH = 800.0f;
const REAL WRAP_HEIGHT = 400.0f;

// weak gaussian prior precision on positions for the laplace free
//		energy; regularizes the translation/rotation zero modes
const REAL FREE_ENERGY_PRIOR_PRECISION = 1e-3f;

// step size (in position units) for the finite-difference hessian
const REAL HESSIAN_STEP = 0.1f;


const REAL RELAX_SIGMOID_SHIFT = 1.20f; // 1.5; // 0.30; 
const REAL RELAX_SIGMOID_FACTOR = 1.5f; // 4.0; // 1.5; // 8.0; 

const REAL RELAX_NEW_GAIN_FACTOR = 0.4f;
const REAL RELAX_NEW_GAIN_FACTOR_SUBTHRESHOLD = 0.8f;


//////////////////////////////////////////////////////////////////////
CSpaceLayoutManager::CSpaceLayoutManager(CSpace *pSpace)
	// constructs an energy function object for the associated space 
	//		view
	: ObjectiveFunction(TRUE)
	, m_pSpace(pSpace)

	, m_KPos(K_POS)
	, m_KRep(K_REP)
	, m_Tolerance(TOLERANCE)

	, m_pStateVector(NULL)

	, m_energy(0.0)
	, m_energyConst(0.0)
	, m_freeEnergy(0.0)
	, m_logDetHessian(0.0)

	, m_mSS(NULL)
	, m_mAvgAct(NULL)
	, m_mLinks(NULL)

	, m_pPowellOptimizer(NULL)
	, m_pConjGradOptimizer(NULL)
	, m_pGradDescOptimizer(NULL)
{
	// sets initial state dimension
	SetStateDim(80);

	// construct state vector
	m_pStateVector = new CSpaceStateVector(this->m_pSpace);

	// create and initialize the Powell optimizer
	m_pPowellOptimizer = new PowellOptimizer(this);
	m_pPowellOptimizer->SetTolerance(TOLERANCE);

	// create and initialize the conjugate gradient optimizer
	ConjGradOptimizer *pCGO = new ConjGradOptimizer(this);
	pCGO->SetLineToleranceEqual(false);
	pCGO->SetTolerance(1e-3f);
	pCGO->GetBrentOptimizer().SetTolerance(1e-4f);

	// set the conj grad optimizer
	m_pConjGradOptimizer = pCGO;

	// create and initialize the gradient descent optimizer
	m_pGradDescOptimizer = new GradDescOptimizer(this);
	m_pGradDescOptimizer->SetTolerance(TOLERANCE);

	// create and initialize the dfp optimizer
	//m_pDFPOptimizer = new CDFPOptimizer(this);
	//m_pDFPOptimizer->SetTolerance(TOLERANCE);

	// set the optimizer to be used
	m_pOptimizer = m_pConjGradOptimizer;

}	// CSpaceLayoutManager::CSpaceLayoutManager


//////////////////////////////////////////////////////////////////////
CSpaceLayoutManager::~CSpaceLayoutManager()
	// destroys the layout manager
{
	// delete the state vector
	delete m_pStateVector;

	delete [] m_mSS;
	delete [] m_mAvgAct;
	delete [] m_mLinks;

	// delete the optimizers
	delete m_pPowellOptimizer;
	delete m_pConjGradOptimizer;
	delete m_pGradDescOptimizer;

}	// CSpaceLayoutManager::~CSpaceLayoutManager


//////////////////////////////////////////////////////////////////////
int 
	CSpaceLayoutManager::GetSuperNodeCount()
	// returns the number of super nodes
{
	return __min(GetStateDim() / 2,	m_pSpace->GetNodeCount());

}	// CSpaceLayoutManager::GetSuperNodeCount


//////////////////////////////////////////////////////////////////////
void 
	CSpaceLayoutManager::SetMaxSuperNodeCount(int nSuperNodeCount)
	// sets the maximum number of super nodes
{
	SetStateDim(nSuperNodeCount * 2);

}	// CSpaceLayoutManager::SetMaxSuperNodeCount


//////////////////////////////////////////////////////////////////////
void 
	CSpaceLayoutManager::SetStateDim(const int& nStateDim)
	// sets the state dimension
{
	m_StateDim = nStateDim;

	// re-initialize the state vector
	m_vState.SetDim(GetStateDim());

}	// CSpaceLayoutManager::SetStateDim


//////////////////////////////////////////////////////////////////////
REAL 
	CSpaceLayoutManager::GetEnergy()
	// returns the energy from the most recent optimization
{
	return m_energy;

}	// CSpaceLayoutManager::GetEnergy


//////////////////////////////////////////////////////////////////////
REAL 
	CSpaceLayoutManager::GetDistError(CNode *pFrom, CNode *pTo)
	// returns the distance error between two nodes
{
	/// TODO: clean this up
	// store the size -- add 10 to ensure non-zero sizes
	// CVectorD<3> vSizeFrom = // pFrom->GetSize(pFrom->GetActivation());
	//	pFrom->GetRadius() * CVectorD<3>(1.0, 1.0, 0.0);
	REAL sizeFrom = SIZE_SCALE * pFrom->GetRadius() + 10.0f;
	// vSizeFrom *= SIZE_SCALE;
	// vSizeFrom += CVectorD<3>(10.0, 10.0, 0.0);

	// CVectorD<3> vSizeTo = // pTo->GetSize(pTo->GetActivation());
	//	pTo->GetRadius() * CVectorD<3>(1.0, 1.0, 0.0);
	// vSizeTo *= SIZE_SCALE;
	// vSizeTo += CVectorD<3>(10.0, 10.0, 0.0);
	REAL sizeTo = SIZE_SCALE * pTo->GetRadius() + 10.0f;

	// CVectorD<3> vSizeAvg = (REAL) 0.5 * (vSizeFrom + vSizeTo);
	REAL sizeAvg = 0.5f * (sizeFrom + sizeTo);
	CVectorD<3> vOffset = pFrom->GetPosition() - pTo->GetPosition();

	// use the nearest wrapped image of the target node
	for (int shiftX = -1; shiftX <= 1; shiftX++)
	{
		for (int shiftY = -1; shiftY <= 1; shiftY++)
		{
			auto vNewTo = pTo->GetPosition();
			vNewTo[0] += shiftX * WRAP_WIDTH;
			vNewTo[1] += shiftY * WRAP_HEIGHT;
			auto vNewOffset = pFrom->GetPosition() - vNewTo;
			if (vNewOffset.GetLength() < vOffset.GetLength())
			{
				vOffset = vNewOffset;
			}
		}
	}


	// compute the relative actual distance
	const REAL act_dist = (REAL) sqrt(vOffset[0] * vOffset[0] 
											/ (sizeAvg * sizeAvg/*vSizeAvg[0] * vSizeAvg[0]*/)
		+ vOffset[1] * vOffset[1] 
				/ (sizeAvg * sizeAvg /*vSizeAvg[1] * vSizeAvg[1]*/)) + 0.001f;

	// compute the distance error
	const REAL dist_error = act_dist - OPT_DIST;

	return dist_error;

}	// CSpaceLayoutManager::GetDistError


//////////////////////////////////////////////////////////////////////
void 
	CSpaceLayoutManager::LayoutNodes()
	// lays out the nodes in the array
{
	// ensure nodes are sorted
	m_pSpace->SortNodes();

	// position any new super-threshold nodes
	PositionNewSuperNodes();

	// layout the nodes
	LayoutNodesPartial(0);

	// evaluate the free energy at the optimum
	UpdateFreeEnergy();

	// relax
	Relax();

	// perform xlate-rotate
	// set the resulting positions, rotate-translating in the process
	m_pStateVector->RotateTranslateTo(m_vState);

	// retrieve the final state
	m_pStateVector->GetPositionsVector(m_vState);

}	// CSpace::LayoutNodes


//////////////////////////////////////////////////////////////////////
static void 
	SymmetricEigenvalues(std::vector<double> mA, int nDim, 
			std::vector<double>& vEigen)
	// cyclic jacobi eigenvalues of the symmetric part of the 
	//		nDim x nDim matrix mA (row-major)
{
	// symmetrize
	for (int nAtRow = 0; nAtRow < nDim; nAtRow++)
	{
		for (int nAtCol = nAtRow + 1; nAtCol < nDim; nAtCol++)
		{
			const double avg = 0.5 * (mA[nAtRow * nDim + nAtCol] 
				+ mA[nAtCol * nDim + nAtRow]);
			mA[nAtRow * nDim + nAtCol] = avg;
			mA[nAtCol * nDim + nAtRow] = avg;
		}
	}

	for (int nSweep = 0; nSweep < 50; nSweep++)
	{
		// stop when the off-diagonal part is negligible
		double offDiag = 0.0;
		double diag = 0.0;
		for (int nAtRow = 0; nAtRow < nDim; nAtRow++)
		{
			diag += mA[nAtRow * nDim + nAtRow] * mA[nAtRow * nDim + nAtRow];
			for (int nAtCol = nAtRow + 1; nAtCol < nDim; nAtCol++)
			{
				offDiag += mA[nAtRow * nDim + nAtCol] * mA[nAtRow * nDim + nAtCol];
			}
		}
		if (offDiag <= 1e-24 * diag || offDiag == 0.0)
		{
			break;
		}

		for (int p = 0; p < nDim; p++)
		{
			for (int q = p + 1; q < nDim; q++)
			{
				const double apq = mA[p * nDim + q];
				if (apq == 0.0)
				{
					continue;
				}

				// rotation angle that zeros element (p, q)
				const double theta = (mA[q * nDim + q] - mA[p * nDim + p]) 
					/ (2.0 * apq);
				const double t = (theta >= 0.0 ? 1.0 : -1.0) 
					/ (fabs(theta) + sqrt(theta * theta + 1.0));
				const double c = 1.0 / sqrt(t * t + 1.0);
				const double sn = t * c;

				// apply the rotation to rows/columns p and q
				for (int k = 0; k < nDim; k++)
				{
					const double akp = mA[k * nDim + p];
					const double akq = mA[k * nDim + q];
					mA[k * nDim + p] = c * akp - sn * akq;
					mA[k * nDim + q] = sn * akp + c * akq;
				}
				for (int k = 0; k < nDim; k++)
				{
					const double apk = mA[p * nDim + k];
					const double aqk = mA[q * nDim + k];
					mA[p * nDim + k] = c * apk - sn * aqk;
					mA[q * nDim + k] = sn * apk + c * aqk;
				}
			}
		}
	}

	vEigen.resize(nDim);
	for (int nAt = 0; nAt < nDim; nAt++)
	{
		vEigen[nAt] = mA[nAt * nDim + nAt];
	}

}	// SymmetricEigenvalues


//////////////////////////////////////////////////////////////////////
REAL 
	CSpaceLayoutManager::GetFreeEnergy()
	// returns the laplace free energy from the most recent layout
{
	return m_freeEnergy;

}	// CSpaceLayoutManager::GetFreeEnergy


//////////////////////////////////////////////////////////////////////
REAL 
	CSpaceLayoutManager::GetLogDetHessian()
	// returns ln|H| from the most recent layout
{
	return m_logDetHessian;

}	// CSpaceLayoutManager::GetLogDetHessian


//////////////////////////////////////////////////////////////////////
void 
	CSpaceLayoutManager::UpdateFreeEnergy()
	// computes the laplace-approximated free energy at the current
	//		state:  F = E(mu) + 1/2 ln|H(mu) + lambda I|
	//		where the energy E is read as -ln p(x, o), H is its hessian,
	//		and lambda is a weak prior precision on positions
{
	const int nDim = 
		2 * __min(GetStateDim() / 2, m_pSpace->GetNodeCount());
	if (nDim == 0 || m_nConstNodes != 0)
	{
		m_logDetHessian = 0.0;
		m_freeEnergy = m_energy;
		return;
	}

	// copy the current (optimal) state
	VectorN<> vAt;
	vAt.SetDim(nDim);
	vAt.CopyElements(m_vState, 0, nDim, 0);

	// hessian by central differences of the analytic gradient
	std::vector<double> mHess(nDim * nDim);
	VectorN<> vParam = vAt;
	VectorN<> vGradPlus;
	VectorN<> vGradMinus;
	for (int nAtCol = 0; nAtCol < nDim; nAtCol++)
	{
		vParam[nAtCol] = vAt[nAtCol] + HESSIAN_STEP;
		(*this)(vParam, &vGradPlus);
		vParam[nAtCol] = vAt[nAtCol] - HESSIAN_STEP;
		(*this)(vParam, &vGradMinus);
		vParam[nAtCol] = vAt[nAtCol];

		for (int nAtRow = 0; nAtRow < nDim; nAtRow++)
		{
			mHess[nAtCol * nDim + nAtRow] = 
				(vGradPlus[nAtRow] - vGradMinus[nAtRow]) / (2.0 * HESSIAN_STEP);
		}
	}

	// ln|H + lambda I|; the layout optimizer stops at a loose 
	//		tolerance, so use |eigenvalue| for any residual negative 
	//		curvature
	std::vector<double> vEigen;
	SymmetricEigenvalues(mHess, nDim, vEigen);
	double logDet = 0.0;
	for (int nAt = 0; nAt < nDim; nAt++)
	{
		logDet += log(fabs(vEigen[nAt]) + FREE_ENERGY_PRIOR_PRECISION);
	}
	m_logDetHessian = (REAL) logDet;

	// re-evaluate at the optimum, restoring m_energy and m_vState
	const REAL energy = (*this)(vAt);
	m_freeEnergy = energy + 0.5f * m_logDetHessian;

	TRACE(_T("Layout energy = %f, free energy = %f, ln|H| = %f\n"),
		energy, m_freeEnergy, m_logDetHessian);

}	// CSpaceLayoutManager::UpdateFreeEnergy


//////////////////////////////////////////////////////////////////////
void 
	CSpaceLayoutManager::LayoutNodesPartial(int nConstNodes)
	// calls the optimizer to lay out the nodes
{
	nConstNodes = __min(nConstNodes, GetStateDim() / 2);

	// if all nodes are constant
	if (nConstNodes * 2 >= GetStateDim())
	{
		// do nothing
		return;
	}

	// set up the interaction matrices
	LoadSizesLinks(nConstNodes, GetStateDim() / 2);

	// form the state vector
	m_pStateVector->GetPositionsVector(m_vState);

	// reset evaluation count
	m_nEvaluations = 0;

	// form the partial state vector
	VectorN<> vPartState;
	vPartState.SetElements(GetStateDim() - m_nConstNodes * 2,
		&m_vState[m_nConstNodes * 2], FALSE);

	// perform the optimization
	vPartState = m_pOptimizer->Optimize(vPartState);

	m_pStateVector->SetPositionsVector(m_vState);

}	// CSpaceLayoutManager::LayoutNodes


//////////////////////////////////////////////////////////////////////
void 
	CSpaceLayoutManager::PositionNewSuperNodes()
	// positions the new super-nodes
{
	// stores the highest new supernode
	int nHighestNewSuper = GetSuperNodeCount();

	// TODO: use sort to accomplish this
	for (int nAt = 0; nAt < nHighestNewSuper; nAt++)  
	{
		// get the node
		CNode *pNode = m_pSpace->GetNodeAt(nAt);

		// is it a sub?
		if (pNode->GetIsSubThreshold()
			|| pNode->GetPostSuperCount() > 0)
		{
			std::iter_swap(m_pSpace->m_arrNodes.begin()+nAt,
				m_pSpace->m_arrNodes.begin()+nHighestNewSuper-1);

			/// TODO: use swap for this
			// remove from the list
//			m_pSpace->m_arrNodes.erase(m_pSpace->m_arrNodes.begin()+nAt);

			// repeat, since we moved the current
			nAt--;

			// re-insert after the super nodes
//			m_pSpace->m_arrNodes.insert(m_pSpace->m_arrNodes.begin() + m_pSpace->GetSuperNodeCount()-1, pNode);

			// we have a new highest
			nHighestNewSuper--;
		}
	}

	// move the new super-nodes to their updated position
	for_each(m_pSpace->m_arrNodes.begin(), m_pSpace->m_arrNodes.begin() + GetSuperNodeCount(),
		[](CNode* pNode) { pNode->PositionNewSuper(); });

	// now layout the new supernodes; three times over
	for (int nRep = 0; nRep < 1 /* 1 *//*3*/; nRep++)
	{
		// TODO: load state vector
		LayoutNodesPartial(nHighestNewSuper);

		// now relax the new super nodes
		Relax(true);
	}

	// finish post processing of super-threshold nodes 
	for_each(m_pSpace->m_arrNodes.begin(), m_pSpace->m_arrNodes.begin() + GetSuperNodeCount(),
		[](CNode* pNode) { pNode->UpdatePostSuper(); });

	// finish processing all subthreshold nodesby moving the new super-nodes to
	//		their updated position
	for_each(m_pSpace->m_arrNodes.begin() + GetSuperNodeCount(), m_pSpace->m_arrNodes.end(),
		[](CNode* pNode) { pNode->SetIsSubThreshold(TRUE); });		

}	// CSpace::PositionNewSuperNodes


//////////////////////////////////////////////////////////////////////
void 
	CSpaceLayoutManager::Relax(bool bSubThreshold)
	// relaxes the link weights (adjusts gain based on dist error)
{
	// choose the gain factor to use
	const REAL NEW_GAIN_FACTOR = bSubThreshold 
		? RELAX_NEW_GAIN_FACTOR_SUBTHRESHOLD
		: RELAX_NEW_GAIN_FACTOR;

	// iterate over all super nodes
	for (auto iterNode = m_pSpace->m_arrNodes.begin(); 
			iterNode != m_pSpace->m_arrNodes.begin() + GetSuperNodeCount(); 
			iterNode++)
	{
		// iterate over linked nodes
		for (auto iterLinked = m_pSpace->m_arrNodes.begin(); 
				iterLinked != m_pSpace->m_arrNodes.begin() + GetSuperNodeCount(); 
				iterLinked++)
		{
			// skip if we are at the same node
			if (iterNode != iterLinked)
			{
				if (bSubThreshold 
					&& !(*iterLinked)->GetIsSubThreshold()
					&& !(*iterNode)->GetIsSubThreshold()
					&& !((*iterLinked)->GetPostSuperCount() > 0)
					&& !((*iterNode)->GetPostSuperCount() > 0))
				{
					continue;
				}

				// get the link, if there is one
				CNodeLink *pLink = (*iterNode)->GetLinkTo(*iterLinked);
				if (pLink != NULL)
				{
					// compute the distance error
					REAL distErr = GetDistError((*iterNode), (*iterLinked));
					distErr *= ((*iterNode)->GetActivation() + (*iterLinked)->GetActivation());

					// compute the gain and set it
					REAL new_gain = 1.0f
						- Sigmoid(distErr - RELAX_SIGMOID_SHIFT, RELAX_SIGMOID_FACTOR);

					// and set, based on percentage of current
					pLink->SetGain(pLink->GetGain() * (1.0f - NEW_GAIN_FACTOR)
						+ new_gain * NEW_GAIN_FACTOR);
				}
			}
		}
	}

}	// CSpace::Relax

//////////////////////////////////////////////////////////////////////
void 
	CSpaceLayoutManager::LoadSizesLinks(int nConstNodes, int nNodeCount)
	// loads the sizes and links for quick access
{
	// stores the computed sizes for the nodes, based on current act
	nNodeCount = __min(nNodeCount, GetStateDim() / 2);
	nNodeCount = __min(nNodeCount, m_pSpace->GetNodeCount());

	// stores the sizes of the nodes
	static REAL arrSize[MAX_STATE_DIM];

	// allocate the buffers
	if (NULL == m_mSS)
	{
		m_mSS = new REAL[MAX_STATE_DIM][MAX_STATE_DIM];
		m_mAvgAct = new REAL[MAX_STATE_DIM][MAX_STATE_DIM];
		m_mLinks = new REAL[MAX_STATE_DIM][MAX_STATE_DIM];
	}

	// iterate over all current visualized node views
	auto iterNode = m_pSpace->m_arrNodes.begin();
	for (int nAtNode = 0; nAtNode < nNodeCount; nAtNode++, iterNode++)
	{
		// store the size -- add 10 to ensure non-zero sizes
		arrSize[nAtNode] = SIZE_SCALE * (*iterNode)->GetRadius() + 10.0f; 

		m_mSS[nAtNode][nAtNode] = 1.0f;
		m_mAvgAct[nAtNode][nAtNode] = 0.0f;
		m_mLinks[nAtNode][nAtNode] = 0.0f;

		// iterate over the potential linked views
		auto iterLinked = iterNode+1;
		for (int nAtLinkedNode = nAtNode+1; nAtLinkedNode < nNodeCount; 
				nAtLinkedNode++, iterLinked++)
		{
			// common size is average
			const REAL ss = (arrSize[nAtNode] + arrSize[nAtLinkedNode]) / 2.0f;

			// set size in size matrix
			m_mSS[nAtLinkedNode][nAtNode] = ss * ss;

			// get average activation

			REAL avgAct = m_mAvgAct[nAtLinkedNode][nAtNode] =
				((*iterNode)->GetActivation()+(*iterLinked)->GetActivation());


			// retrieve the link weight for layout
			REAL weight =
				((*iterNode)->GetActivation() / avgAct * (*iterNode)->GetLinkGainWeight(*iterLinked)
					+ (*iterLinked)->GetActivation() / avgAct * (*iterLinked)->GetLinkGainWeight((*iterNode))) 
						+ (REAL) 1e-6;

			// store the link weight
			m_mLinks[nAtLinkedNode][nAtNode] =
				weight * m_mAvgAct[nAtLinkedNode][nAtNode];
		}
	}

	// zero constant energy term
	m_energyConst = 0.0;

	// if we have constant nodes,
	if (nConstNodes > 0)
	{
		// get the positions vector
		m_vConstPositions.SetElements(nConstNodes * 2, &m_vState[0], FALSE);

		// cache the old state dimension
		int nOldStateDim = GetStateDim();

		// set new state dimension to only constant nodes
		m_StateDim = nConstNodes * 2;

		// make sure no constant nodes
		m_nConstNodes = 0;

		// get the constant energy term
		m_energyConst = CSpaceLayoutManager::operator()(m_vConstPositions);

		// restore old state dimension
		m_StateDim = nOldStateDim;
	}

	// save number of constant nodes
	m_nConstNodes = nConstNodes;

}	// CSpaceLayoutManager::LoadSizesLinks()


//////////////////////////////////////////////////////////////////////
REAL 
	CSpaceLayoutManager::operator()(const VectorN<REAL>& vInput,
			VectorN<> *pGrad) const
	// evaluates the energy function at the given point
{
	// reset the energy
	m_energy = m_energyConst;

	// are we calculating gradient?
	if (NULL != pGrad)
	{
		// initialize the gradient vector to zeros
		m_vGrad.SetDim(GetStateDim());
		m_vGrad.SetZero();
	}

	// prepare the input vector for the full positional information
	ASSERT(m_nConstNodes * 2 + vInput.GetDim() <= GetStateDim());
	m_vState.CopyElements(vInput, 0, vInput.GetDim(), m_nConstNodes * 2);

	// and the total number of nodes plus clusters
	int nNodeCount = __min(GetStateDim() / 2, m_pSpace->GetNodeCount());
	nNodeCount = __min(nNodeCount, vInput.GetDim() / 2 + m_nConstNodes);

	// iterate over all current visualized node views
	int nAtNode;
	for (nAtNode = nNodeCount-1; nAtNode >= m_nConstNodes; nAtNode--)
	{
		// iterate over the potential linked views
		int nAtLinked;
		for (nAtLinked = nAtNode-1; nAtLinked >= 0; nAtLinked--)
		{
			// skip if its self
			if (nAtLinked == nAtNode)
			{
				continue;
			}

			//////////////////////////////////////////////////////////////
			// set up some common values

			// compute the x- and y-offset between the views
			const REAL x = m_vState[nAtNode*2 + 0] - m_vState[nAtLinked*2 + 0];
			const REAL y = m_vState[nAtNode*2 + 1] - m_vState[nAtLinked*2 + 1];

			// compute the x- and y-scales for the fields -- average of
			//		two rectangles
			const REAL& ssx_sq = m_mSS[nAtNode][nAtLinked];
			const REAL& ssy_sq = m_mSS[nAtNode][nAtLinked];

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
			const REAL factor = GetKPos() * weight;

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
			const REAL factor_rep = GetKRep() * m_mAvgAct[nAtNode][nAtLinked];

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

	// store the gradient vector if one was passed
	if (NULL != pGrad)
	{
		// set correct dimensionality
		pGrad->SetDim(vInput.GetDim());

		// and assign
		pGrad->CopyElements(m_vGrad, m_nConstNodes * 2, vInput.GetDim());
	}

	// increment evaluation count
	m_nEvaluations++;

	// return the total energy
	return m_energy;

}	// CSpaceLayoutManager::operator()



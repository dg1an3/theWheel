// THEWHEEL_MODEL_Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "THEWHEEL_MODEL_Test.h"

#include "Node.h"
#include "SSELayoutManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <Space.h>


REAL RandFlt(REAL size)
{
	return size - (2.0 * size * rand() / (REAL) RAND_MAX);
}

REAL ComputeEpsilon(REAL forValue, REAL epsMin = 1e-4)
{
	return epsMin * (1.0 + forValue);
}

template<class BASE>
class CTestLayoutManager : public BASE
{
public:
	CTestLayoutManager(CSpace *pSpace)
		: BASE(pSpace) { }

	REAL Evaluate(const CVectorN<>& vInput);
	CVectorN<> EvaluateGrad(const CVectorN<>& vInput, REAL delta);

	REAL EvaluatePos(REAL x, REAL y, CVectorD<3> vSz, REAL act, REAL weight);
	REAL EvaluateRep(REAL x, REAL y, CVectorD<3> vSz, REAL act);

	// evaluates the energy function
	virtual REAL operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad = NULL);
};

template<class BASE>
REAL CTestLayoutManager<BASE>::EvaluatePos(REAL x, REAL y, CVectorD<3> vSz, REAL act, REAL weight)
{
	// compute the relative actual distance
	const REAL act_dist = (REAL) sqrt(x * x / (vSz[0] * vSz[0])
		+ y * y / (vSz[1] * vSz[1]) + 0.001);

	// compute the distance error
	const REAL dist_error = act_dist - OPT_DIST;

	// compute the factor controlling the importance of the
	//		attraction term
	const REAL factor = GetKPos() * act * weight;

	// and add the attraction term to the energy
	return factor * dist_error * dist_error;
}

template<class BASE>
REAL CTestLayoutManager<BASE>::EvaluateRep(REAL x, REAL y, CVectorD<3> vSz, REAL act)
{
	// compute the energy due to this interation
	const REAL x_ratio = x * x / (vSz[0] * vSz[0]);
	const REAL y_ratio = y * y / (vSz[1] * vSz[1]);

	// compute the energy term
	const REAL inv_sq = 1.0 / (x_ratio + y_ratio + 3.0);
	const REAL factor_rep = GetKRep() * act;

	// add to total energy
	return factor_rep * inv_sq;
}

template<class BASE>
REAL CTestLayoutManager<BASE>::Evaluate(const CVectorN<>& vInput)
{
	// reset the energy
	REAL energy = 0.0;

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
	for (nAtNode = 0; nAtNode < nNodeCount; nAtNode++)
	{
		CNode *pNode = m_pSpace->GetNodeAt(nAtNode);

		// iterate over the potential linked views
		int nAtLinked;
		for (nAtLinked = 0; nAtLinked < nNodeCount; nAtLinked++)
		{
			if (nAtLinked != nAtNode)
			{
				CNode *pLinked = m_pSpace->GetNodeAt(nAtLinked);
	
				//////////////////////////////////////////////////////////////
				// set up some common values

				// compute the x- and y-offset between the views
				const REAL x = vInput[nAtNode*2 + 0] - vInput[nAtLinked*2 + 0];
				const REAL y = vInput[nAtNode*2 + 1] - vInput[nAtLinked*2 + 1];

				// compute the x- and y-scales for the fields -- average of
				//		two rectangles
				CVectorD<3> vSz = (REAL) 0.5 * 
					(pNode->GetSize(pNode->GetActivation()) * SIZE_SCALE
					+ pLinked->GetSize(pLinked->GetActivation()) * SIZE_SCALE);
				vSz += CVectorD<3>(10.0, 10.0, 0.0);

				//////////////////////////////////////////////////////////////
				// compute the attraction field

				// store the weight, for convenience
				const REAL weight = 0.5 *
					(pNode->GetLinkWeight(pLinked)
						+ pLinked->GetLinkWeight(pNode)) + 1e-6;

				const REAL act = 0.5 * (pNode->GetActivation() 
					+ pLinked->GetActivation());

				energy += EvaluatePos(x, y, vSz, act, weight);

				//////////////////////////////////////////////////////////////
				// compute the repulsion field
				// add to total energy
				energy += EvaluateRep(x, y, vSz, act);
			} 
		}

		//////////////////////////////////////////////////////////////
		// compute the centering repulsion field

		if (m_bCalcCenterRep 
			&& pNode->GetActivation() <= CENTER_REP_MAX_ACT)
		{
			// compute the x- and y-offset between the views
			const REAL x = vInput[nAtNode*2 + 0] - vCenter[0];
			const REAL y = vInput[nAtNode*2 + 1] - vCenter[1];

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
				* (CENTER_REP_MAX_ACT - abs(pNode->GetActivation())) 
				* (CENTER_REP_MAX_ACT - abs(pNode->GetActivation()));

			// add to total energy
			energy += factor_rep * inv_sq;
		}  
	}

	return energy;
}

template<class BASE>
CVectorN<> CTestLayoutManager<BASE>::EvaluateGrad(const CVectorN<>& vInput, REAL delta)
{
	CVectorN<> vGrad;
	vGrad.SetDim(vInput.GetDim());

	// working copy of input
	CVectorN<> vInWork(vInput);

	for (int nAt = 0; nAt < vInput.GetDim(); nAt++)
	{
		vInWork[nAt] -= delta;
		REAL energy1 = Evaluate(vInWork);

		vInWork[nAt] += 2.0 * delta;
		REAL energy2 = Evaluate(vInWork);

		vInWork[nAt] -= delta;

		vGrad[nAt] = (energy2 - energy1) / (2.0 * delta);
	}

	return vGrad;
}

template<class BASE>
REAL CTestLayoutManager<BASE>::operator()(const CVectorN<>& vInput, 
		CVectorN<> *pGrad)
{
	// evaluate base class
	REAL energy = BASE::operator()(vInput, pGrad);

	// prepare the input vector for the full positional information
	ASSERT(m_vConstPositions.GetDim() + vInput.GetDim() == m_nStateDim);

	// compute full vector (in case there is a constant component)
	CVectorN<> vAll;
	vAll.SetDim(m_nStateDim);
	vAll.CopyElements(m_vConstPositions, 0, 
		m_vConstPositions.GetDim());
	vAll.CopyElements(vInput, 0, vInput.GetDim(),
		m_vConstPositions.GetDim());

	// make sure the synthetic full vector is equal to the one
	//		calculated by the base class
	ASSERT(vAll == m_vInput);

	// now evaluate for this
	REAL testEnergy = Evaluate(vAll);

	// compute the epsilon for comparison
	REAL epsilon = ComputeEpsilon(testEnergy);

	// test for approximate equality
	if (!IsApproxEqual(energy, testEnergy, epsilon))
	{
		cout << " **** FAILED: energy comparison" << endl;
		cout << "Energy " << energy << " vs. " << testEnergy << endl;
	}

	// are we testing gradient also?
	if (pGrad)
	{
		// compute the delta for numerical gradient approximation
		REAL delta = // 1e-2 * 
			epsilon;

		// numerically evaluate the gradient
		CVectorN<> vTestGrad = EvaluateGrad(vAll, delta);

		// extract the partial gradient (for comparison to pGrad)
		CVectorN<> vPartGrad;
		vPartGrad.SetDim(pGrad->GetDim());
		vPartGrad.CopyElements(vTestGrad, m_vConstPositions.GetDim(),
			vPartGrad.GetDim());

		// compute the error distance between the two gradient vectors
		// REAL errDist = (vPartGrad - (*pGrad)).GetLength();

		// compute the average length of the gradient, for the epsilon
		REAL avgGradLength = (vPartGrad.GetLength() 
			+ pGrad->GetLength()) / 2.0;

		// compute the epsilon for gradient comparison
		REAL epsGrad = ComputeEpsilon(avgGradLength, 1e-1);
		
		// test for gradient approximate equality
		if (!pGrad->IsApproxEqual(vPartGrad, epsGrad))
		{
			cout << " **** Error in gradient" << endl;
			cout << "vPartGrad " << vPartGrad << endl;
			cout << "pGrad " << (*pGrad) << endl;
		}
	}

	// return the total energy
	return energy;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;


void PrintInfo(CSpace *pSpace, BOOL bPosition = FALSE)
{
	// and print some activation values
/*	for (int nAt = 0; nAt < pSpace->GetSuperNodeCount(); nAt++)
	{
		CNode *pNode = pSpace->GetNodeAt(nAt);

		const char *strName = pNode->GetName();
		cout << "\tName = " << '"' << strName << '"';
		cout << ", Act = " << pNode->GetActivation();
		if (bPosition)
		{
			cout << ", Pos = " << pNode->GetPosition();
		}
		else
		{
			cout << endl;
		}
	} */

	cout << endl;
}


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		CSpace *pSpace = (CSpace *) RUNTIME_CLASS(CSpace)->CreateObject();

		if (argc < 2)
		{
			pSpace->OnNewDocument();
		}
		else
		{
			pSpace->OnOpenDocument(argv[1]);
		}

		// write the activation for all nodes (before)
		cout << "Total activation of all nodes = " 
			<< pSpace->GetTotalActivation()
			<< endl;

		// now normalize
		pSpace->NormalizeNodes();

		// write the activation for all nodes (after normalization)
		cout << "Total activation of all nodes = " 
			<< pSpace->GetTotalActivation()
			<< endl;

		// now try a propagation
		CNode *pMainNode = pSpace->GetNodeAt(0);

		// perform 1st activation
		cout << "**** 2nd Activation ****" << endl;

		// boost the root node up to about 100 times the other nodes
		pSpace->ActivateNode(pMainNode, 1.0 / 0.6);

		PrintInfo(pSpace);

		// perform 2nd activation
		cout << "**** 2nd Activation ****" << endl;

		// boost the root node up to about 100 times the other nodes
		pSpace->ActivateNode(pMainNode, 1.0 / 0.6);

		PrintInfo(pSpace);

		// perform 3rd activation
		cout << "**** 3rd Activation ****" << endl;

		// boost the root node up to about 100 times the other nodes
		pSpace->ActivateNode(pMainNode, 1.0 / 0.6);

		PrintInfo(pSpace);

		// output node w/ positions
		cout << "**** Layout Test ****" << endl;

		// set all nodes to zero position
		for (int nAt = 0; nAt < pSpace->GetNodeCount(); nAt++)
		{
			pSpace->GetNodeAt(nAt)->SetPosition(CVectorD<3>(
				RandFlt(10.0), RandFlt(10.0), RandFlt(10.0)));
		}

		// output node w/ positions
		cout << "**** Initial node positions ****" << endl;

		PrintInfo(pSpace, TRUE);

		// now test layout
		CTestLayoutManager<CSSELayoutManager> *pTestLayoutManager = 
			new CTestLayoutManager<CSSELayoutManager>(pSpace);

		pTestLayoutManager->SetStateDim(pSpace->GetLayoutManager()->GetStateDim());
		pTestLayoutManager->SetKPos(pSpace->GetLayoutManager()->GetKPos());
		pTestLayoutManager->SetKRep(pSpace->GetLayoutManager()->GetKRep());

		// pTestLayoutManager->LayoutNodes(pSpace->GetStateVector(), 0);

		// output node w/ positions
		cout << "**** Final node positions ****" << endl;

		// output node w/ positions
		PrintInfo(pSpace, TRUE);

		// output node w/ positions
		cout << "**** Final node positions -- second layout ****" << endl;

		// pTestLayoutManager->LayoutNodes(pSpace->GetStateVector(), 0);

		// output node w/ positions
		PrintInfo(pSpace, TRUE);

		for (int nCount = 0; nCount < 100; nCount++)
		{
			// output node w/ positions
			cout << "**** Node positions -- partial layout ****" << endl;

			int nConstNodes = pSpace->GetSuperNodeCount()-1;
			nConstNodes = 1 + nConstNodes * rand() / RAND_MAX;
			cout << "    Constant nodes = " << nConstNodes << endl;

			for (nAt = nConstNodes; nAt < pSpace->GetSuperNodeCount(); nAt++)
			{
				CVectorD<3> vRandomPosition( RandFlt(50.0), RandFlt(50.0), 0.0);
				pSpace->GetNodeAt(nAt)->SetPosition(vRandomPosition);
			}

			PrintInfo(pSpace, TRUE);

			pTestLayoutManager->LayoutNodes(pSpace->GetStateVector(), nConstNodes);

			// output node w/ positions
			PrintInfo(pSpace, TRUE);
		}

		delete pTestLayoutManager;
		delete pSpace;
	}

	return nRetCode;
}



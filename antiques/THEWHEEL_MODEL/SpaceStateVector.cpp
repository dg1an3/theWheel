//////////////////////////////////////////////////////////////////////
// SpaceStateVector.cpp: implementation of the CSpaceStateVector class.
//
// Copyright (C) 1999-2003 Derek Graham Lane
// $Id$
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// included for completness
#include <CastVectorD.h>
#include <MatrixNxM_pinv.h>
// #include <MatrixBase.inl>

// class definition
#include "SpaceStateVector.h"

// depends on CSpace
#include "Space.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CSpaceStateVector::CSpaceStateVector
// 
// returns positional RMSE
//////////////////////////////////////////////////////////////////////
CSpaceStateVector::CSpaceStateVector(CSpace *pSpace)
	: m_pSpace(pSpace)
{
}	// CSpaceStateVector::CSpaceStateVector


//////////////////////////////////////////////////////////////////////
// SpaceStateVector::~CSpaceStateVector
// 
// destroys the state vector
//////////////////////////////////////////////////////////////////////
CSpaceStateVector::~CSpaceStateVector()
{
}	// SpaceStateVector::~CSpaceStateVector


//////////////////////////////////////////////////////////////////////
// IMPLEMENT_SERIAL
// 
// allows state vector to be serialized
//////////////////////////////////////////////////////////////////////
IMPLEMENT_SERIAL(CSpaceStateVector, CObject, VERSIONABLE_SCHEMA | 1);


//////////////////////////////////////////////////////////////////////
// CSpaceStateVector::GetActivationsVector
// 
// accessor for the activation vector
//////////////////////////////////////////////////////////////////////
void CSpaceStateVector::GetActivationsVector(CVectorN<>& vActivations)
{
	if (!m_pSpace) return;

	int nDim = __min(m_pSpace->GetSuperNodeCount(),
		vActivations.GetDim());

	for (int nAt = 0; nAt < nDim; nAt++)
	{
		vActivations[nAt] = m_pSpace->GetNodeAt(nAt)->GetActivation();
	}

}	// CSpaceStateVector::GetActivationsVector


//////////////////////////////////////////////////////////////////////
// CSpaceStateVector::SetActivationsVector
// 
// sets the activation part of the state vector
//////////////////////////////////////////////////////////////////////
void CSpaceStateVector::SetActivationsVector(const CVectorN<>& vActivations)
{
	if (!m_pSpace) return;

	int nDim = __min(m_pSpace->GetSuperNodeCount(),
		vActivations.GetDim());

	for (int nAt = 0; nAt < nDim; nAt++)
	{
		m_pSpace->GetNodeAt(nAt)->SetActivation(vActivations[nAt]);
	}

}	// CSpaceStateVector::SetActivationsVector


//////////////////////////////////////////////////////////////////////
// CSpaceStateVector::GetPositionsVector
// 
// accessor for the positions vector
//////////////////////////////////////////////////////////////////////
void CSpaceStateVector::GetPositionsVector(CVectorN<>& vPositions, BOOL bPerturb)
{
	if (!m_pSpace) return;

	int nNodes = __min(m_pSpace->GetSuperNodeCount(),
		vPositions.GetDim() / 2);

	for (int nAt = 0; nAt < nNodes; nAt++)
	{
		CNode *pNode = m_pSpace->GetNodeAt(nAt);

		// get position vector
		vPositions[nAt*2 + 0] = pNode->GetPosition()[0];
		vPositions[nAt*2 + 1] = pNode->GetPosition()[1];

		// are we perturbing?
		if (bPerturb)
		{
			// yes, so get the size of the node
			REAL size = pNode->GetSize(pNode->GetActivation()).GetLength();

			// compute theoretical perturbation amount
			REAL perturb = 0.75 * size
				* pNode->GetRMSE() / (size + pNode->GetRMSE());

			// compute half-height of scaling sigma
			REAL half = m_pSpace->GetNodeAt(0)->GetActivation() / 10.0;

			// compute scale (proportional to activation
			REAL scale = half / (pNode->GetActivation() + half); 

			// perturb
			vPositions[nAt*2 + 0] += perturb * half
				* (0.5 - (REAL) rand() / (REAL) (RAND_MAX-1));
			vPositions[nAt*2 + 1] += perturb * half
				* (0.5 - (REAL) rand() / (REAL) (RAND_MAX-1)); 

#ifdef _PERTURB_FIXED
			Perturb(&vPositions[nAt*2 + 0], perturb);
			Perturb(&vPositions[nAt*2 + 1], perturb);
#endif
		}
	}

}	// CSpaceStateVector::GetPositionsVector


//////////////////////////////////////////////////////////////////////
// CSpaceStateVector::SetPositionsVector
// 
// sets the node positions
//////////////////////////////////////////////////////////////////////
void CSpaceStateVector::SetPositionsVector(const CVectorN<>& vPositions)
{
	if (!m_pSpace) return;

	int nNodes = __min(m_pSpace->GetSuperNodeCount(),
		vPositions.GetDim() / 2);

	for (int nAt = 0; nAt < nNodes; nAt++)
	{
		// form the node view's position
		static CVectorD<3> vPosition;
		vPosition[0] = vPositions[nAt*2 + 0];
		vPosition[1] = vPositions[nAt*2 + 1];
		vPosition[2] = 0.0;

		// load positions from nodes
		m_pSpace->GetNodeAt(nAt)->SetPosition(vPosition);
	}

}	// CSpaceStateVector::SetPositionsVector


//////////////////////////////////////////////////////////////////////
// CSpaceStateVector::Serialize
// 
// serialization of this state vector
//////////////////////////////////////////////////////////////////////
void CSpaceStateVector::Serialize(CArchive &ar)
{
	CObArray arrNodesOrder;
	arrNodesOrder.Serialize(ar);

	if (ar.IsLoading())
	{
		CVectorN<double> vActivations;
		ar >> vActivations;
		CVectorN<> vActReal;
		for (int nAt = 0; nAt < vActivations.GetDim(); nAt++)
			vActReal[nAt] = vActivations[nAt];
		SetActivationsVector(vActReal);

		CVectorN<double> vPositions;
		ar >> vPositions;
		CVectorN<> vPosReal;
		for (int nAt = 0; nAt < vActivations.GetDim(); nAt++)
			vPosReal[nAt] = vActivations[nAt];
		SetPositionsVector(vPosReal);

		// LoadMaps();
	}
	else
	{
		CVectorN<> vActReal;
		GetActivationsVector(vActReal);
		CVectorN<double> vActivations;
		for (int nAt = 0; nAt < vActReal.GetDim(); nAt++)
			vActivations[nAt] = vActReal[nAt];
		ar << vActivations;

		CVectorN<> vPosReal;
		GetPositionsVector(vPosReal);
		CVectorN<double> vPositions;
		for (int nAt = 0; nAt < vPosReal.GetDim(); nAt++)
			vPositions[nAt] = vPosReal[nAt];
		ar << vPositions;
	}
}	// CSpaceStateVector::Serialize



//////////////////////////////////////////////////////////////////////
// CSpaceStateVector::RotateTranslateStateVector
// 
// called to minimize rotate/translate error between old state
//		and new
//////////////////////////////////////////////////////////////////////
void CSpaceStateVector::RotateTranslateTo(const CVectorN<>& vPositions)
{
	if (TRUE) // m_pSpace->GetSuperNodeCount() <= 3)
	{
		SetPositionsVector(vPositions);
		return;
	}

	static CVectorN<> vActivations;
	vActivations.SetDim(m_pSpace->GetSuperNodeCount());
	GetActivationsVector(vActivations);
	vActivations.Normalize();

	static CVectorN<> vOldPositions;
	vOldPositions.SetDim(m_pSpace->GetSuperNodeCount() * 2);
	GetPositionsVector(vOldPositions);
	ASSERT(vOldPositions.GetDim() <= vPositions.GetDim());

	// form the Nx3 matrix of pre-coordinates (homogeneous)
	static CMatrixNxM<> mOld;
	mOld.Reshape(m_pSpace->GetSuperNodeCount(), 3);

	static CMatrixNxM<> mNew;
	mNew.Reshape(m_pSpace->GetSuperNodeCount(), 3);

	for (int nAt = 0; nAt < mOld.GetCols(); nAt++)
	{
		REAL act = 1.0 * vActivations[nAt];

		mOld[nAt][0] = act * vOldPositions[nAt * 2 + 0];
		mOld[nAt][1] = act * vOldPositions[nAt * 2 + 1];
		mOld[nAt][2] = act * 1.0;

		mNew[nAt][0] = act * vPositions[nAt * 2 + 0];
		mNew[nAt][1] = act * vPositions[nAt * 2 + 1];
		mNew[nAt][2] = act * 1.0;
	}

	// work with the transpose of the problem, so that columns < rows
	mOld.Transpose();
	mNew.Transpose();

	// form the pseudo-inverse of the a coordinates
	static CMatrixNxM<> mNew_ps;
	mNew_ps.Reshape(mNew.GetCols(), mNew.GetRows());
	// mNew_ps = mNew;
	mNew_ps = mNew;
	if (// FALSE) // 
		Pseudoinvert(mNew, mNew_ps))	// Pseudoinvert(mNew, mNew_ps))
	{
		// form the transform matrix
		static CMatrixNxM<> mTransform;
		mTransform.Reshape(mOld.GetCols(), mNew_ps.GetRows());
		mTransform = mNew_ps * mOld;

		// un-transpose the matrices
		mNew.Transpose();
		mOld.Transpose();
		mTransform.Transpose();

		// store the offset part
		CVectorD<3> offset = // CCastVectorD<3>(
			CCastVectorD<3>(mTransform[2]);

		// reshape to isolate upper 2x2
		mTransform.Reshape(2, 2);

		// now turn the transform matrix upper 2x2
		//		into an orthogonal matrix
		static CVectorN<> w;
		w.SetDim(2);

		static CMatrixNxM<> v(2, 2);
		v.SetIdentity();

		// SVD to factor the matrix
		if (SVD(mTransform, w, v)) // SVD(mTransform, w, v))
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
			static CVectorN<> vPositions;
			vPositions.SetDim(m_pSpace->GetSuperNodeCount() * 2);
			for (int nAt = 0; nAt < mNew.GetCols(); nAt++)
			{
				vPositions[nAt * 2 + 0] = mNew[nAt][0] / mNew[nAt][2];
				vPositions[nAt * 2 + 1] = mNew[nAt][1] / mNew[nAt][2];
			}

			if (FALSE) // GetPositionsError(vPositions) > 10.0 * vPositions.GetDim())
			{
				SetPositionsVector(vPositions);
			}
		}
	}

}	// CSpaceLayoutManager::RotateTranslateStateVector




//////////////////////////////////////////////////////////////////////
// CSpaceStateVector::GetPositionsError
// 
// compuptes the total positional error between vector and current
//////////////////////////////////////////////////////////////////////
REAL CSpaceStateVector::GetPositionsError(const CVectorN<>& vPositions)
{
	if (!m_pSpace) return 0.0;

	int nNodes = __min(m_pSpace->GetSuperNodeCount(),
		vPositions.GetDim() / 2);

	REAL error = 0.0;
	for (int nAt = 0; nAt < nNodes; nAt++)
	{
		// form the node view's position
		static CVectorD<3> vOffset;
		vOffset[0] = vPositions[nAt*2 + 0];
		vOffset[1] = vPositions[nAt*2 + 1];

		// load positions from nodes
		vOffset -= m_pSpace->GetNodeAt(nAt)->GetPosition();

		error += vOffset * vOffset;
	}

	return error;

}	// CSpaceStateVector::GetPositionsError


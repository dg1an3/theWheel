// SpaceStateVector.cpp: implementation of the CSpaceStateVector class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SpaceStateVector.h"

#include <Space.h>

#include <MatrixBase.inl>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSpaceStateVector::CSpaceStateVector(CSpace *pSpace)
	: m_pSpace(pSpace)
{
	// ASSERT(m_pSpace != NULL);
}

CSpaceStateVector::~CSpaceStateVector()
{
}

IMPLEMENT_SERIAL(CSpaceStateVector, CObject, VERSIONABLE_SCHEMA | 1);


// accessors for the activation vector
void CSpaceStateVector::GetActivationsVector(CVectorN<>& vActivations)
{
	if (!m_pSpace) return;

	int nDim = __min(m_pSpace->GetSuperNodeCount(),
		vActivations.GetDim());

	for (int nAt = 0; nAt < nDim; nAt++)
	{
		vActivations[nAt] = m_pSpace->GetNodeAt(nAt)->GetActivation();
	}
}

void CSpaceStateVector::SetActivationsVector(const CVectorN<>& vActivations)
{
	if (!m_pSpace) return;

	int nDim = __min(m_pSpace->GetSuperNodeCount(),
		vActivations.GetDim());

	for (int nAt = 0; nAt < nDim; nAt++)
	{
		m_pSpace->GetNodeAt(nAt)->SetActivation(vActivations[nAt]);
	}
}


// accessors for the positions vector
void CSpaceStateVector::GetPositionsVector(CVectorN<>& vPositions)
{
	if (!m_pSpace) return;

	int nNodes = __min(m_pSpace->GetSuperNodeCount(),
		vPositions.GetDim() / 2);

	for (int nAt = 0; nAt < nNodes; nAt++)
	{
		CNode *pNode = m_pSpace->GetNodeAt(nAt);

		vPositions[nAt*2 + 0] = pNode->GetPosition()[0];
		vPositions[nAt*2 + 1] = pNode->GetPosition()[1];
	}
}

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

		// load positions from nodes
		m_pSpace->GetNodeAt(nAt)->SetPosition(vPosition);
	}
}

// serialization of this node
void CSpaceStateVector::Serialize(CArchive &ar)
{
	CObArray arrNodesOrder;
	arrNodesOrder.Serialize(ar);

	if (ar.IsLoading())
	{
		CVectorN<> vActivations;
		ar >> vActivations;
		SetActivationsVector(vActivations);

		CVectorN<> vPositions;
		ar >> vPositions;
		SetPositionsVector(vPositions);

		// LoadMaps();
	}
	else
	{
		CVectorN<> vActivations;
		GetActivationsVector(vActivations);
		ar << vActivations;

		CVectorN<> vPositions;
		GetPositionsVector(vPositions);
		ar << vPositions;
	}
}

//////////////////////////////////////////////////////////////////////
// CSpaceLayoutManager::RotateTranslateStateVector
// 
// called to minimize rotate/translate error between old state
//		and new
//////////////////////////////////////////////////////////////////////
void CSpaceStateVector::RotateTranslateTo(const CVectorN<>& vPositions)
{
	if (m_pSpace->GetSuperNodeCount() <= 3)
		return;

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
		double act = vActivations[nAt];

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
	mNew_ps = mNew;
	if (mNew_ps.Pseudoinvert())
	{
		// form the transform matrix
		static CMatrixNxM<> mTransform;
		mTransform.Reshape(mOld.GetCols(), mOld.GetRows());
		mTransform = mNew_ps * mOld;

		// un-transpose the matrices
		mNew.Transpose();
		mOld.Transpose();
		mTransform.Transpose();

		// store the offset part
		CVectorD<3> offset = (CVectorD<3>) mTransform[2];

		// reshape to isolate upper 2x2
		mTransform.Reshape(2, 2);

		// now turn the transform matrix upper 2x2
		//		into an orthogonal matrix
		static CVectorN<> w;
		w.SetDim(2);

		static CMatrixNxM<> v(2, 2);
		v.SetIdentity();

		// SVD to factor the matrix
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
			static CVectorN<> vPositions;
			vPositions.SetDim(m_pSpace->GetSuperNodeCount() * 2);
			for (nAt = 0; nAt < mNew.GetCols(); nAt++)
			{
				vPositions[nAt * 2 + 0] = mNew[nAt][0] / mNew[nAt][2];
				vPositions[nAt * 2 + 1] = mNew[nAt][1] / mNew[nAt][2];
			}

			SetPositionsVector(vPositions);
		}
	}

}	// CSpaceLayoutManager::RotateTranslateStateVector




//////////////////////////////////////////////////////////////////////
// SpaceStateVector.cpp: implementation of the CSpaceStateVector class.
//
// Copyright (C) 1999-2003 Derek Graham Lane
// $Id: SpaceStateVector.cpp,v 1.8 2007/05/28 18:46:02 Derek Lane Exp $
// U.S. Patent Pending
//////////////////////////////////////////////////////////////////////

// pre-compiled headers
#include "stdafx.h"

// included for completness
#include <CastVectorD.h>
#include <MatrixD.h>
#include <Pseudoinverse.h>

// class definition
#include "SpaceStateVector.h"

// depends on CSpace
#include "Space.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CSpaceStateVector::CSpaceStateVector(CSpace *pSpace)
	: m_pSpace(pSpace)
{
}	// CSpaceStateVector::CSpaceStateVector


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
void 
	CSpaceStateVector::GetActivationsVector(CVectorN<>& vActivations)
	// accessor for the activation vector
{
	if (m_pSpace) 
	{
		// calculate how many dimensions to fill
		int nDim = __min(m_pSpace->GetLayoutManager()->GetSuperNodeCount(),
			vActivations.GetDim());

		// and fill them
#ifdef STL_COLL
		vector<CNode*>::iterator iter = m_pSpace->m_arrNodes.begin();
		for (int nAt = 0; nAt < nDim; nAt++, iter++)
		{
			ASSERT(iter != m_pSpace->m_arrNodes.end());
			vActivations[nAt] = (*iter)->GetActivation();
		}
#else
		for (int nAt = 0; nAt < nDim; nAt++)
		{
			vActivations[nAt] = m_pSpace->m_arrNodes[nAt]->GetActivation();
		}
#endif
	}

}	// CSpaceStateVector::GetActivationsVector


//////////////////////////////////////////////////////////////////////
void 
	CSpaceStateVector::SetActivationsVector(const CVectorN<>& vActivations)
	// sets the activation part of the state vector
{
	if (m_pSpace) 
	{
		// calculate how many dimensions to fill
		int nDim = __min(m_pSpace->GetLayoutManager()->GetSuperNodeCount(),
			vActivations.GetDim());

		// and fill them
#ifdef STL_COLL
		vector<CNode*>::iterator iter = m_pSpace->m_arrNodes.begin();
		for (int nAt = 0; nAt < nDim; nAt++, iter++)
		{
			ASSERT(iter != m_pSpace->m_arrNodes.end());
			(*iter)->SetActivation(vActivations[nAt]);
		}
#else
		for (int nAt = 0; nAt < nDim; nAt++)
		{
			m_pSpace->m_arrNodes[nAt]->SetActivation(vActivations[nAt]);
		}
#endif
	}

}	// CSpaceStateVector::SetActivationsVector


//////////////////////////////////////////////////////////////////////
void 
	CSpaceStateVector::GetPositionsVector(CVectorN<>& vPositions)
	// accessor for the positions vector
  /// TODO: should this be moved to CSpace???
{
	if (m_pSpace) 
	{
		int nNodes = __min(m_pSpace->GetLayoutManager()->GetSuperNodeCount(),
			vPositions.GetDim() / 2);

#ifdef STL_COLL
		vector<CNode*>::iterator iter = m_pSpace->m_arrNodes.begin();
		for (int nAt = 0; nAt < nNodes; nAt++, iter++)
		{
			ASSERT(iter != m_pSpace->m_arrNodes.end());
			const CVectorD<3>& vPos = (*iter)->GetPosition();
			vPositions[nAt*2 + 0] = vPos[0];
			vPositions[nAt*2 + 1] = vPos[1];
		}
#else
		for (int nAt = 0; nAt < nNodes; nAt++)
		{
			const CVectorD<3>& vPos = m_pSpace->m_arrNodes[nAt]->GetPosition();
			vPositions[nAt*2 + 0] = vPos[0];
			vPositions[nAt*2 + 1] = vPos[1];
		}
#endif
	}

}	// CSpaceStateVector::GetPositionsVector


//////////////////////////////////////////////////////////////////////
void 
	CSpaceStateVector::SetPositionsVector(const CVectorN<>& vPositions)
	// sets the node positions
{
	if (m_pSpace)
	{
		int nNodes = __min(m_pSpace->GetLayoutManager()->GetSuperNodeCount(),
			vPositions.GetDim() / 2);

		CVectorD<3> vPos;

#ifdef STL_COLL
		vector<CNode*>::iterator iter = m_pSpace->m_arrNodes.begin();
		for (int nAt = 0; nAt < nNodes; nAt++, iter++)
		{
			ASSERT(iter != m_pSpace->m_arrNodes.end());
			// form the node view's position
			vPos[0] = vPositions[nAt*2 + 0];
			vPos[1] = vPositions[nAt*2 + 1];

			// load positions from nodes
			(*iter)->SetPosition(vPos, (*iter)->IsPositionReset(false));
		}
#else
		for (int nAt = 0; nAt < nNodes; nAt++)
		{
			// form the node view's position
			vPos[0] = vPositions[nAt*2 + 0];
			vPos[1] = vPositions[nAt*2 + 1];

			// load positions from nodes
			m_pSpace->m_arrNodes[nAt]->SetPosition(vPos, 
				m_pSpace->m_arrNodes[nAt]->IsPositionReset(false));
		}
#endif
	}

}	// CSpaceStateVector::SetPositionsVector


//////////////////////////////////////////////////////////////////////
void 
	CSpaceStateVector::RotateTranslateTo(const CVectorN<>& vPositions)
	// called to minimize rotate/translate error between old state
	//		and new
{
	// can only work if super nodes > 3
	if (m_pSpace->GetLayoutManager()->GetSuperNodeCount() <= 4)
	{
		SetPositionsVector(vPositions);
		return;
	}

	// use the top 10 nodes
	const int nNodesToUse = __min(10, m_pSpace->GetLayoutManager()->GetSuperNodeCount());

	m_vOldPositions.SetDim(nNodesToUse * 2);
	GetPositionsVector(m_vOldPositions);

	// form the Nx3 matrix of pre-coordinates (homogeneous)
	m_mOldCoords.Reshape(nNodesToUse, 3);
	m_mNewCoords.Reshape(nNodesToUse, 3);

	for (int nAt = 0; nAt < m_mOldCoords.GetCols(); nAt++)
	{
		REAL act = 1.0; //  * vActivations[nAt];

		m_mOldCoords[nAt][0] = act * m_vOldPositions[nAt * 2 + 0];
		m_mOldCoords[nAt][1] = act * m_vOldPositions[nAt * 2 + 1];
		m_mOldCoords[nAt][2] = act * 1.0;

		m_mNewCoords[nAt][0] = act * vPositions[nAt * 2 + 0];
		m_mNewCoords[nAt][1] = act * vPositions[nAt * 2 + 1];
		m_mNewCoords[nAt][2] = act * 1.0;
	}

	// work with the transpose of the problem, so that columns < rows
	m_mOldCoords.Transpose();
	m_mNewCoords.Transpose();

	// form the pseudo-inverse of the a coordinates
	m_mNewCoords_psinv.Reshape(m_mNewCoords.GetCols(), m_mNewCoords.GetRows());
	if (true) // !Pseudoinvert(m_mNewCoords, m_mNewCoords_psinv))	
	{
		// just set the positions
		SetPositionsVector(vPositions);

		// and go
		return;
	}

	// form the transform matrix
	m_mTransform.Reshape(m_mOldCoords.GetCols(), m_mNewCoords_psinv.GetRows());
	m_mTransform = m_mNewCoords_psinv * m_mOldCoords;

	// un-transpose the transform
	m_mTransform.Transpose();

	// store the offset part of the transform
	CVectorD<3> vOffset = CCastVectorD<3>(m_mTransform[2]);

	// reshape to isolate upper 2x2
	m_mTransform.Reshape(2, 2);

	// now turn the transform matrix upper 2x2
	//		into an orthogonal matrix
	m_vSVD_w.SetDim(2);
	m_mSVD_v.Reshape(2, 2);
	m_mSVD_v.SetIdentity();

	// SVD to factor the matrix
	if (!SVD(m_mTransform, m_vSVD_w, m_mSVD_v))
	{
		// just set the positions
		SetPositionsVector(vPositions);

		// and go
		return;
	}

	// form the orthogonal matrix
	m_mSVD_v.Transpose();
	m_mTransform *= m_mSVD_v;

	// restore the translation part
	m_mTransform.Reshape(3, 3);
	m_mTransform[2][0] = vOffset[0];
	m_mTransform[2][1] = vOffset[1];
	m_mTransform[2][2] = 1.0; 

	// now repopulate
	m_vNewPositions.SetDim(m_pSpace->GetLayoutManager()->GetSuperNodeCount() * 2);
	CVectorN<> vOldPos(3);
	CVectorN<> vNewPos(3);
	for (int nAt = 0; nAt < m_pSpace->GetLayoutManager()->GetSuperNodeCount(); nAt++)
	{
		// set up HG of previous position
		vOldPos[0] = vPositions[nAt * 2 + 0];
		vOldPos[1] = vPositions[nAt * 2 + 1];
		vOldPos[2] = 1.0;
		
		// transform
		vNewPos = m_mTransform * vOldPos;

		// set the position in the new position vector
		m_vNewPositions[nAt * 2 + 0] = vNewPos[0] / vNewPos[2];
		m_vNewPositions[nAt * 2 + 1] = vNewPos[1] / vNewPos[2];
	}

	// set the new positions
	SetPositionsVector(m_vNewPositions);

}	// CSpaceLayoutManager::RotateTranslateStateVector


/// TODO: get rid of this
//////////////////////////////////////////////////////////////////////
void 
	CSpaceStateVector::Serialize(CArchive &ar)
	// serialization of this state vector
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


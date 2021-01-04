//////////////////////////////////////////////////////////////////////
// SpaceStateVector.h: interface for the CSpaceStateVector class.
//
// Copyright (C) 2003 Derek Graham Lane
// U.S. Patent Pending
// $Id: SpaceStateVector.h,v 1.3 2007/05/17 03:43:04 Derek Lane Exp $
//////////////////////////////////////////////////////////////////////

#pragma once

#include <VectorD.h>
#include <VectorN.h>
#include <MatrixNxM.h>

//////////////////////////////////////////////////////////////////////
// forward declaration of the CSpace class
//////////////////////////////////////////////////////////////////////
class CSpace;

//////////////////////////////////////////////////////////////////////
// class CSpaceStateVector
//
// class that represents the current state of the space, based on
//		activation values on node positions
//////////////////////////////////////////////////////////////////////
class CSpaceStateVector : public CObject  
{
public:
	// constructors / destructor
	CSpaceStateVector(CSpace *pSpace = NULL);
	~CSpaceStateVector();

	// state vectors can be serialized
	DECLARE_SERIAL(CSpaceStateVector);

	// returns the current node count for the vector
	int GetNodeCount() const;

	// accessors for the activation vector
	void GetActivationsVector(CVectorN<>& vActivation);
	void SetActivationsVector(const CVectorN<>& vActivation);

	// accessors for the positions vector
	void GetPositionsVector(CVectorN<>& vPositions);
	void SetPositionsVector(const CVectorN<>& vPositions);

	// lsq fit to the other state vector
	void RotateTranslateTo(const CVectorN<>& vMatchPositions);

	//////////////////////////////////////////////////////////////////
	// serialization

	// serialization of this node
	virtual void Serialize(CArchive &ar);

private:
	// the associated space
	CSpace *m_pSpace;

	// old positions vector
	CVectorN<> m_vOldPositions;

	// form the Nx3 matrix of pre-coordinates (homogeneous)
	CMatrixNxM<> m_mOldCoords;

	// the Nx3 matrix of post-coordinates
	CMatrixNxM<> m_mNewCoords;

	// pseudo-inverse of the new coordinates
	CMatrixNxM<> m_mNewCoords_psinv;

	// initial transform (calculated from pseudo-inverse)
	CMatrixNxM<> m_mTransform;

	// helpers for SVD
	CVectorN<> m_vSVD_w;
	CMatrixNxM<> m_mSVD_v;

	// new positions vector
	CVectorN<> m_vNewPositions;

};	// class CSpaceStateVector

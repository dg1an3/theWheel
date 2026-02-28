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
	void GetActivationsVector(VectorN<>& vActivation);
	void SetActivationsVector(const VectorN<>& vActivation);

	// accessors for the positions vector
	void GetPositionsVector(VectorN<>& vPositions);
	void SetPositionsVector(const VectorN<>& vPositions);

	// lsq fit to the other state vector
	void RotateTranslateTo(const VectorN<>& vMatchPositions);

	//////////////////////////////////////////////////////////////////
	// serialization

#ifdef _MSC_VER
	// serialization of this node
	virtual void Serialize(CArchive &ar);
#endif

private:
	// the associated space
	CSpace *m_pSpace;

	// old positions vector
	VectorN<> m_vOldPositions;

	// form the Nx3 matrix of pre-coordinates (homogeneous)
	MatrixNxM<> m_mOldCoords;

	// the Nx3 matrix of post-coordinates
	MatrixNxM<> m_mNewCoords;

	// pseudo-inverse of the new coordinates
	MatrixNxM<> m_mNewCoords_psinv;

	// initial transform (calculated from pseudo-inverse)
	MatrixNxM<> m_mTransform;

	// helpers for SVD
	VectorN<> m_vSVD_w;
	MatrixNxM<> m_mSVD_v;

	// new positions vector
	VectorN<> m_vNewPositions;

};	// class CSpaceStateVector

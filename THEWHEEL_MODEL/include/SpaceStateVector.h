//////////////////////////////////////////////////////////////////////
// SpaceStateVector.h: interface for the CSpaceStateVector class.
//
// Copyright (C) 2003 Derek Graham Lane
// U.S. Patent Pending
// $Id$
//////////////////////////////////////////////////////////////////////

#if !defined(_SPACESTATEVECTOR_H_)
#define _SPACESTATEVECTOR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

};	// class CSpaceStateVector

#endif // !defined(_SPACESTATEVECTOR_H_)

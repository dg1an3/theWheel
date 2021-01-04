#pragma once
#include "afx.h"

class CUnitNode :
	public CObject
{
public:
	CUnitNode(void);
	virtual ~CUnitNode(void);

	// stores unit node's reference vector
	DECLARE_ATTRIBUTE_GI(Reference, CVectorN<>);

	// stores unit node's position
	DECLARE_ATTRIBUTE(Position, CVectorD<2>);

	// gets designated neighbor
	CUnitNode * GetNeighbor(int nAt);

	// computes distance to other position on grid
	double MapDistTo(const CVectorD<2>& vMapPos);

	// computes offset to other position on grid
	CVectorD<2> MapOffsetTo(const CVectorD<2>& vMapPos);

	// updates the node's reference for the given input
	void Update(const CVectorN<>& vInput, CVectorD<2>& vInputMapPos,
		double alpha, double neighborhood);

	// interpolates the map position of the input vector
	CVectorD<2> InterpMapPos(const CVectorN<>& vInput);

	// holds grid repeat offset
	static vector< CVectorD<2> > m_arrGridOffset;

public:
	// pointers to neighbors
	vector<CUnitNode*> m_arrNeighbors;
};

#ifndef _SPACEVIEWENERGYFUNCTION_H
#define _SPACEVIEWENERGYFUNCTION_H

#include "Optimizer.h"

typedef double SPV_STATE_TYPE;
const int SPV_STATE_DIM = 20;

class CSpaceView;

class CSpaceViewEnergyFunction : public CObjectiveFunction<SPV_STATE_DIM, SPV_STATE_TYPE>
{
public:
	CSpaceView *m_pView;

	// retrieves the threshold for the given dimensionality
	SPV_STATE_TYPE GetThreshold();

	// retrieves the current mapping from CNodeView index to 
	//		state vector element index
	const CArray<int, int>& GetMap();

	// gets and sets the state vector for the associated CSpaceView
	CVector<SPV_STATE_DIM, SPV_STATE_TYPE> GetStateVector();
	void SetStateVector(const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vState);

	// evaluates the energy function
	virtual SPV_STATE_TYPE operator()(const CVector<SPV_STATE_DIM, SPV_STATE_TYPE>& vInput);

private:
	// array holding the mapping from CNodeViews (indexes into m_pView->nodeViews) to
	//		state vector elements
	CArray<int, int> m_arrElementMap;
};


#endif // ndef _SPACEVIEWENERGYFUNCTION_H
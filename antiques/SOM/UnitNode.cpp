#include "StdAfx.h"
#include "UnitNode.h"

// holds grid repeat offset
vector< CVectorD<2> > CUnitNode::m_arrGridOffset;

//////////////////////////////////////////////////////////////////////////////
CUnitNode::CUnitNode(void)
{
}	// CUnitNode::CUnitNode


//////////////////////////////////////////////////////////////////////////////
CUnitNode::~CUnitNode(void)
{
}	// CUnitNode::~CUnitNode


//////////////////////////////////////////////////////////////////////////////
void 
	CUnitNode::SetReference(const CVectorN<>& value)
{
	m_Reference.SetDim(value.GetDim());
	m_Reference = value;

}	// CUnitNode::SetReference


//////////////////////////////////////////////////////////////////////////////
CUnitNode * 
	CUnitNode::GetNeighbor(int nAt)
	// gets designated neighbor
{
	return m_arrNeighbors[nAt];

}	// CUnitNode::GetNeighbor


//////////////////////////////////////////////////////////////////////////////
//double 
//	CUnitNode::MapDistTo(const CVectorD<2>& vMapPos)
//	// computes distance to other node, including wrap-around
//{
//	double minDistSq = numeric_limits<double>::max();
//
//	vector< CVectorD<2> >::iterator offset = m_arrGridOffset.begin();
//	for (; offset != m_arrGridOffset.end(); offset++)
//	{
//		CVectorD<2> vPos = GetPosition();
//		vPos += (*offset);
//		CVectorD<2> vOfs = vMapPos - vPos;
//
//		minDistSq = __min(minDistSq, vOfs * vOfs);
//	}
//
//	return sqrt(minDistSq);
//
//}	// CUnitNode::MapDistTo


//////////////////////////////////////////////////////////////////////////////
CVectorD<2> 
	CUnitNode::MapOffsetTo(const CVectorD<2>& vMapPos)
	// computes distance to other node, including wrap-around
{
	double minDistSq = numeric_limits<double>::max();
	CVectorD<2> vPos = GetPosition();
	CVectorD<2> vMinOfs = vMapPos - vPos;

	//vector< CVectorD<2> >::iterator offset = m_arrGridOffset.begin();
	//for (; offset != m_arrGridOffset.end(); offset++)
	//{
	//	CVectorD<2> vPos = GetPosition();
	//	vPos += (*offset);
	//	CVectorD<2> vOfs = vMapPos - vPos;

	//	double distSq = vOfs * vOfs;
	//	if (distSq < minDistSq)
	//	{
	//		minDistSq = distSq;
	//		vMinOfs = vOfs;
	//	}
	//}

	return vMinOfs;

}	// CUnitNode::MapOffsetTo


//////////////////////////////////////////////////////////////////////////////
static
double 
	NeighborhoodWeight(double neighborhood, const CVectorD<2>& vOffset)
	// calculates weight for given offset
{
	return Gauss2D<double>(vOffset[0], vOffset[1], neighborhood, neighborhood);

}	// NeighborhoodWeight


//////////////////////////////////////////////////////////////////////////////
void 
	CUnitNode::Update(const CVectorN<>& vInput, 
			CVectorD<2>& vInputMapPos,
			double alpha, double neighborhood)
	// updates this unit node's reference
{
	const double weight = NeighborhoodWeight(neighborhood + 1e-8, 
		MapOffsetTo(vInputMapPos));

	CVectorN<> vLean(vInput);
	vLean -= m_Reference;
	vLean *= alpha * weight; 
	m_Reference += vLean;

	ASSERT(_finite(GetReference()[0]));

}	// CUnitNode::Update


//////////////////////////////////////////////////////////////////////////////
CVectorD<2> 
	CUnitNode::InterpMapPos(const CVectorN<>& vInput)
	// interpolates the map position of the input vector
{
	CUnitNode *pClosestNeigh[] = {NULL, NULL};

	// find nearest neighbor
	double minDistSq = numeric_limits<double>::max();
	int nClosestNeigh = -1;
	for (int nNeigh = 0; nNeigh < m_arrNeighbors.size(); nNeigh++)
	{
		CVectorN<> vOfs = m_arrNeighbors[nNeigh]->GetReference();
		vOfs -= vInput;
		double distSq = vOfs * vOfs;
		if (distSq < minDistSq)
		{
			distSq = minDistSq;
			nClosestNeigh = nNeigh;
		}
	}
	pClosestNeigh[0] = m_arrNeighbors[nClosestNeigh];

	// find next nearest neighbor
	int nNeighAfter = (nClosestNeigh + 1) % m_arrNeighbors.size();
	CVectorN<> vOfsAfter = m_arrNeighbors[nNeighAfter]->GetReference();
	vOfsAfter -= vInput;
	double distSqAfter = vOfsAfter * vOfsAfter;

	int nNeighBefore = (nClosestNeigh + m_arrNeighbors.size() - 1) % m_arrNeighbors.size();
	CVectorN<> vOfsBefore = m_arrNeighbors[nNeighAfter]->GetReference();
	vOfsBefore -= vInput;
	double distSqBefore = vOfsBefore * vOfsBefore;

	if (distSqAfter < distSqBefore)
	{
		pClosestNeigh[1] =  m_arrNeighbors[nNeighAfter];
	}
	else
	{
		pClosestNeigh[1] =  m_arrNeighbors[nNeighBefore];
	}

	CMatrixNxM<> mBasis;
	mBasis.Reshape(2, GetReference().GetDim());
	mBasis[0] = pClosestNeigh[0]->GetReference() - GetReference();
	mBasis[1] = pClosestNeigh[1]->GetReference() - GetReference();
	// TRACE_MATRIX("mBasis = ", mBasis);

	CMatrixNxM<> mBasis_trans = mBasis;
	mBasis_trans.Transpose();
	// TRACE_MATRIX("mBasis_trans = ", mBasis_trans);

	CMatrixNxM<> mBasis_self_inv = mBasis_trans * mBasis;
	mBasis_self_inv.Invert();
	// TRACE_MATRIX("mBasis_self_inv = ", mBasis_self_inv);

	CMatrixNxM<> mT = mBasis_self_inv * mBasis_trans;
	// TRACE_MATRIX("mT = ", mT);

	// calculate error residual in model space
	CVectorN<> vResid(vInput.GetDim());
	vResid = vInput - GetReference();
	// TRACE_VECTOR("vResid = ", vResid);
	//  TRACE("Residual length = %lf\n", vResid.GetLength());

	// calculate coordinates in local system
	CVectorN<> vA(2);
	vA = mT * vResid;

	if (vA[0] <= -1.0 || vA[0] >= 1.0
		|| vA[1] <= -1.0 && vA[1] >= 1.0) 
	{
		TRACE("out of range: %lf %lf\n", vA[0], vA[1]);
	}
	// TRACE_VECTOR("vA = ", vA);

	CMatrixNxM<> mBasis_grid;
	mBasis_grid.Reshape(2, 2);
	CVectorD<2> vOfs[2];
	vOfs[0] = MapOffsetTo(pClosestNeigh[0]->GetPosition());
	vOfs[1] = MapOffsetTo(pClosestNeigh[1]->GetPosition());
	mBasis_grid[0][0] = vOfs[0][0];
	mBasis_grid[0][1] = vOfs[0][1];
	mBasis_grid[1][0] = vOfs[1][0];
	mBasis_grid[1][1] = vOfs[1][1];

	// finally calculate position
	CVectorN<> vA_grid(2); 
	vA_grid = mBasis_grid * vA;
	// TraceVector(vA_grid);

	CVectorD<2> vPos = GetPosition();
	vPos[0] += vA_grid[0];
	vPos[1] += vA_grid[1];

	return vPos;

}	// CUnitNode::InterpMapPos

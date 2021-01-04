#include "StdAfx.h"
#include "LayoutEnergyFunction.h"

namespace theWeel
{

  //////////////////////////////////////////////////////////////////////////////
  LayoutEnergyFunction::LayoutEnergyFunction(ObjectRenderer::ArrayType& forRenderers)
    : mRenderers(forRenderers)
    , mSpaceDim(5)
    , mSuperThresholdObjects(40)
  {
    mDistanceMatrix.set_size(GetSuperThresholdObjects(), GetSuperThresholdObjects());
  }

  //////////////////////////////////////////////////////////////////////////////
  int 
    LayoutEnergyFunction::GetSpaceDim() 
  { 
    return mSpaceDim; 
  }
  
  //////////////////////////////////////////////////////////////////////////////
  int 
    LayoutEnergyFunction::GetSuperThresholdObjects() 
  { 
    return mSuperThresholdObjects; 
  }

  //////////////////////////////////////////////////////////////////////////////
  void 
    LayoutEnergyFunction::compute(Vector const& vPositions, double *f, Vector* g)
  {
    (*f) = 0.0;

    for (int nAtFrom = 0; nAtFrom < GetSuperThresholdObjects(); nAtFrom++)
    {
      for (int nAtTo = nAtFrom+1; nAtTo < GetSuperThresholdObjects(); nAtTo++)
      {
        // compute "ideal" distance
        double distanceStar = mDistanceMatrix(nAtTo, nAtFrom);

        // compute actual distance
        Point2D FromPosition(vPositions[nAtFrom*2+0], vPositions[nAtFrom*2+1]);
        Point2D ToPosition(vPositions[nAtTo*2+0], vPositions[nAtTo*2+1]);
        Point2D vDiff = FromPosition - ToPosition;
        double distanceCurrent = vDiff.magnitude();

        // term to be summed
        double term = distanceStar - distanceCurrent;
        term *= term;
        term /= distanceStar;

        (*f) += term;
      }
    }

    // scale by 
    (*f) /= mDistanceTotal;
  }

  //////////////////////////////////////////////////////////////////////////////
  void 
    LayoutEnergyFunction::ComputeDistanceMatrix()
  {
    mDistanceTotal = 0.0;
    for (int nAtFrom = 0; nAtFrom < GetSuperThresholdObjects(); nAtFrom++)
    {
      for (int nAtTo = nAtFrom+1; nAtTo < GetSuperThresholdObjects(); nAtTo++)
      {
        double distance = ComputeDistance(mRenderers[nAtFrom], mRenderers[nAtTo]);
        mDistanceMatrix.put(nAtTo, nAtFrom, distance);
        mDistanceTotal += distance;
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  double
    LayoutEnergyFunction::ComputeDistance(ObjectRenderer* fromRenderer, ObjectRenderer* toRenderer)
  {
    Vector vFromPosition;
    ComputePositionNSpace(fromRenderer, vFromPosition);

    Vector vToPosition;
    ComputePositionNSpace(toRenderer, vToPosition);

    Vector vDiff = vFromPosition - vToPosition;
    return vDiff.magnitude();
  }

  //////////////////////////////////////////////////////////////////////////////
  void
    LayoutEnergyFunction::ComputePositionNSpace(ObjectRenderer* forRenderer, Vector& vPositionNSpace)
  {
    vPositionNSpace.set_size(GetSpaceDim());
    for (int nAt = 0; nAt < GetSpaceDim(); nAt++)
    {
      float weight = 1.0;
      if (mRenderers[nAt] != forRenderer)
        weight = forRenderer->GetLinkWeight(mRenderers[nAt]);

      float axisActivation = mRenderers[nAt]->GetActivation();
      vPositionNSpace[nAt] = weight * axisActivation;
    }
  }

}

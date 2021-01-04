#pragma once

#include <vnl/vnl_vector.h>
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_cost_function.h>

#include "ObjectRenderer.h"

namespace theWeel
{
  /**
   * Vnl types for Vector and Matrix
   */
  typedef vnl_vector<double> Vector;
  typedef vnl_matrix<double> Matrix;

  /**
   * Cost function to evaluate layout, as well as helpers for forming the position vector
   */
  class LayoutEnergyFunction
    : public vnl_cost_function
  {	
  public:
    /**
     * Constructs the layout energy function for the given renderer array
     * Renderer array should be sorted by decreasing activation
     */
    LayoutEnergyFunction(ObjectRenderer::ArrayType& forRenderers);

    /**
     *
     */
    int GetSpaceDim();
    int GetSuperThresholdObjects();

    /**
     *
     */
    virtual void compute(Vector const& x, double *f, Vector* g);

    /**
     * 
     */
    void PositionsToVector(Vector& vPositions);
    void PositionsFromVector(const Vector& vPositions);

    /**
     * 
     */
    void ComputeDistanceMatrix();

    /**
     * 
     */
    double ComputeDistance(ObjectRenderer* fromRenderer, ObjectRenderer* toRenderer);

    /**
     *
     */
    void ComputePositionNSpace(ObjectRenderer* forRenderer, Vector& vPosition);

  private:
    
    // the renderers to be laid-out
    ObjectRenderer::ArrayType& mRenderers;

    // distance matrix between renderers
    Matrix mDistanceMatrix;

    // stores the total distance from the matrix
    double mDistanceTotal;

    // constants for the optimization
    const int mSpaceDim;
    const int mSuperThresholdObjects;
  };

} // thWeel
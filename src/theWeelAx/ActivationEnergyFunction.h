#pragma once

#include "LayoutEnergyFunction.h"

namespace theWeel
{

  /**
   *
   */
  class ActivationEnergyFunction
	  : public vnl_cost_function
  {	
  public:

    /**
     *
     */
    ActivationEnergyFunction(ObjectRenderer::ArrayType& forRenderers);

    /**
     * 
     */
	  virtual void compute(vnl_vector<double> const& x, double *f, vnl_vector<double>* g)
	  {
	  }

    /**
     * 
     */
    void ActivationsToVector(Vector& vActivations);
    void ActivationsFromVector(const Vector& vActivations);

    /**
     * 
     */
	  void SortNodesByActivation();

  private:

    // the renderers to be activated
    ObjectRenderer::ArrayType& mRenderers;
  };

} // thWeel


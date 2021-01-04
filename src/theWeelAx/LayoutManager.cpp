#include "StdAfx.h"
#include "LayoutManager.h"

namespace theWeel
{

  ////////////////////////////////////////////////////////////////////////////
  LayoutManager::LayoutManager(ObjectRenderer::MapType& forMap)
  {
    // populate the array from the map
    POSITION pos = forMap.GetStartPosition();
    while (pos != NULL)
    {
      ObjectRenderer* renderer = forMap.GetNextValue(pos);
      mRenderers.Add(renderer);
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  void 
    LayoutManager::Activate(ObjectRenderer* pRenderer, double activationTarget, double dt)
  {
    // change renderer activation (using spring and dt?)

    // propagate activation
    ActivationEnergyFunction energyFunction(mRenderers);

  }

  ////////////////////////////////////////////////////////////////////////////
  void 
    LayoutManager::Layout(double dt)
  {
    LayoutEnergyFunction energyFunction(mRenderers);

    Vector vInitPositions;
    energyFunction.PositionsToVector(vInitPositions);

    //vnl_optimizer optimizer;
    //optimizer.optimize(vInitPositions);

    energyFunction.PositionsFromVector(vInitPositions);

    // update springs using dt
  }

}

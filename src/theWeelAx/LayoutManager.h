/** @file
 * LayoutManager for theWeelSpaceView
 * Copyright (C) 2010 Derek G. Lane 
 */

#pragma once

#include "ActivationEnergyFunction.h"
#include "LayoutEnergyFunction.h"
#include "ObjectRenderer.h"

namespace theWeel
{

  /**
   * Represents a layout manager for a view
   */
  class LayoutManager
  {
  public:
    /**
     * Constructs for a map
     */
    LayoutManager(ObjectRenderer::MapType& forMap);

    /**
     *
     */
    void StartActivationEvent(ObjectRenderer* renderer);
    void EndActivationEvent();

    /**
     * 
     */
    void Activate(ObjectRenderer* pRenderer, double activationTarget, double dt);

    /**
     * 
     */
    void Layout(double dt);

  private:

    /**
     * 
     */
    ObjectRenderer::ArrayType mRenderers;
  };

} // namespace theWeel
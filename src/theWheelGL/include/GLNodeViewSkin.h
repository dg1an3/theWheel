/// GLNodeViewSkin.h
/// OpenGL ES port of NodeViewSkin2007 — caches GLPlaque meshes by activation level.
///
/// Copyright (C) 1996-2007 Derek G Lane

#pragma once

#include <vector>
#include "GLRenderer.h"
#include "GLPlaque.h"

// Forward declarations from theWheelModel
class CNode;

namespace theWheelGL {

class GLNodeViewSkin {
public:
    GLNodeViewSkin();
    ~GLNodeViewSkin();

    /// Render a node at the given activation with the given world transform already set.
    /// Sets material, selects and renders the appropriate plaque.
    void Render(GLRenderer& renderer, float activation,
                float centerX, float centerY, float centerZ);

    /// Calculate the extent rectangle for a given activation.
    /// Returns width and height via outW, outH.
    void CalcRectForActivation(float activation, float& outW, float& outH);

private:
    int getPlaqueIndex(float activation);

    std::vector<GLPlaque*> m_plaques;
};

} // namespace theWheelGL

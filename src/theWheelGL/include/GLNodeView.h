/// GLNodeView.h
/// Cross-platform per-node rendering via OpenGL ES.
/// Holds node state for GL rendering (position, activation).
///
/// Copyright (C) 1996-2007 Derek G Lane

#pragma once

#include "GLRenderer.h"
#include "GLNodeViewSkin.h"
#include "GLMath.h"

// Forward declarations from theWheelModel
class CNode;
class CNodeLink;

namespace theWheelGL {

/// Renders a single node and its links via OpenGL ES.
/// This is a lightweight rendering helper — the actual animation state
/// (springs, activation) is managed by CNodeView (MFC) or NodeViewData (wx).
class GLNodeView {
public:
    /// Draw a node's plaque at the given screen-space position and activation.
    static void Draw(GLRenderer& renderer, GLNodeViewSkin& skin,
                     float centerX, float centerY, float centerZ,
                     float activation);

    /// Draw a link line between two nodes using GL_LINES.
    /// fromAct/toAct control line width via glLineWidth (clamped).
    static void DrawLink(GLRenderer& renderer,
                         float fromX, float fromY, float fromZ,
                         float toX, float toY, float toZ,
                         float fromAct, float toAct);

private:
    // Link line VBO (shared, lazily created)
    static GLuint s_linkVBO;
    static void ensureLinkVBO();
};

} // namespace theWheelGL

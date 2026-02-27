/// GLNodeView.cpp
/// Implementation of GLNodeView static rendering helpers.
///
/// Copyright (C) 1996-2007 Derek G Lane

#include "GLNodeView.h"

#include <algorithm>
#include <cmath>

namespace theWheelGL {

GLuint GLNodeView::s_linkVBO = 0;

void GLNodeView::Draw(GLRenderer& renderer, GLNodeViewSkin& skin,
                      float centerX, float centerY, float centerZ,
                      float activation)
{
    skin.Render(renderer, activation, centerX, centerY, centerZ);
}

void GLNodeView::ensureLinkVBO()
{
    if (s_linkVBO == 0) {
        // A simple 2-vertex line buffer; positions set per-draw via buffer sub-data
        float lineVerts[12] = {
            0, 0, 0, 0, 0, 1,  // vertex 0: pos + normal
            0, 0, 0, 0, 0, 1   // vertex 1: pos + normal
        };
        glGenBuffers(1, &s_linkVBO);
        glBindBuffer(GL_ARRAY_BUFFER, s_linkVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(lineVerts), lineVerts, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void GLNodeView::DrawLink(GLRenderer& renderer,
                           float fromX, float fromY, float fromZ,
                           float toX, float toY, float toZ,
                           float fromAct, float toAct)
{
    float totalAct = fromAct + toAct;
    if (totalAct < 0.04f)
        return;

    ensureLinkVBO();

    // Update link vertices
    float verts[12] = {
        fromX, fromY, fromZ, 0, 0, 1,
        toX,   toY,   toZ,   0, 0, 1
    };

    glBindBuffer(GL_ARRAY_BUFFER, s_linkVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    GLint aPos = renderer.GetPositionAttrib();
    GLint aNorm = renderer.GetNormalAttrib();

    glEnableVertexAttribArray(aPos);
    glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(aNorm);
    glVertexAttribPointer(aNorm, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    // Set link material color — darker shade of background
    float shade = 0.85f;
    if (totalAct < 0.06f) shade = 0.95f;
    else if (totalAct < 0.08f) shade = 0.90f;

    renderer.SetMaterial(shade, shade, shade, 1.0f, shade, shade, shade);
    renderer.SetWorldMatrix(Mat4f()); // identity
    renderer.ApplyUniforms();

    // Line width based on activation (clamped to GL limits)
    float lineW = 1.0f + std::sqrt(totalAct) * 4.0f;
    lineW = std::min(lineW, 5.0f);
    glLineWidth(lineW);

    glDrawArrays(GL_LINES, 0, 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glLineWidth(1.0f);
}

} // namespace theWheelGL

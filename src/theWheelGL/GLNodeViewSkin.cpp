/// GLNodeViewSkin.cpp
/// Implementation of GLNodeViewSkin — port of NodeViewSkin2007.
///
/// Copyright (C) 1996-2007 Derek G Lane

#include "GLNodeViewSkin.h"
#include "Elliptangle.h"

#include <cmath>
#include <algorithm>

namespace theWheelGL {

// Constants matching NodeViewSkin2007.cpp
static const float BORDER_RADIUS = 2.0f;
static const float RECT_SIZE_MAX = 13.0f / 16.0f;
static const float RECT_SIZE_SCALE = 6.0f / 16.0f;

static float computeEllipticalness(float activation)
{
    float scale = 1.0f - 1.0f / std::sqrt(2.0f);
    return 1.0f - scale * std::exp(-1.5f * activation + 0.01f);
}

GLNodeViewSkin::GLNodeViewSkin()
{
}

GLNodeViewSkin::~GLNodeViewSkin()
{
    for (auto p : m_plaques)
        delete p;
}

void GLNodeViewSkin::CalcRectForActivation(float activation, float& outW, float& outH)
{
    float aspectRatio = RECT_SIZE_MAX - RECT_SIZE_SCALE * std::exp(-2.0f * activation);
    outW = std::sqrt(activation / aspectRatio);
    outH = std::sqrt(activation * aspectRatio);
}

int GLNodeViewSkin::getPlaqueIndex(float activation)
{
    int index = (int)std::round(activation * 200.0f);
    index = std::max(index, 1);

    if (index >= (int)m_plaques.size()) {
        m_plaques.resize(index + 1, nullptr);
    }

    if (!m_plaques[index]) {
        float indexActivation = (float)index / 100.0f;

        // Build the elliptangle shape
        float w, h;
        CalcRectForActivation(indexActivation, w, h);

        CExtent<2, REAL> rectInner;
        rectInner.Deflate(-w, -h, -w, -h);

        float ellipt = computeEllipticalness(indexActivation);
        auto* pElliptangle = new theWheel2007::Elliptangle(rectInner, ellipt);

        float borderRadius = BORDER_RADIUS;
        if (rectInner.GetSize(1) < borderRadius * 4.0f) {
            borderRadius = (float)rectInner.GetSize(1) / 4.0f;
        }

        m_plaques[index] = new GLPlaque(pElliptangle, borderRadius);
    }

    return index;
}

void GLNodeViewSkin::Render(GLRenderer& renderer, float activation,
                            float centerX, float centerY, float centerZ)
{
    int index = getPlaqueIndex(activation);
    GLPlaque* plaque = m_plaques[index];
    if (!plaque) return;

    // Compute scaling (matching NodeViewSkin2007::Render)
    float w, h;
    CalcRectForActivation(activation, w, h);

    float sizeUp = 100.0f + 1200.0f * activation * activation;

    auto* shape = plaque->GetShape();
    float shapeW = (float)shape->InnerRect.GetSize(0);
    float shapeH = (float)shape->InnerRect.GetSize(1);

    float scaleX = (shapeW > 0.0f) ? sizeUp * w / shapeW : 1.0f;
    float scaleY = (shapeH > 0.0f) ? sizeUp * h / shapeH : 1.0f;
    float scaleZ = (scaleX + scaleY) / 2.0f;

    // Build world matrix: translate to node center, then scale plaque
    Mat4f translate = Mat4f::Translation(-centerX, -centerY, centerZ);
    Mat4f scale = Mat4f::Scaling(scaleX, scaleY, scaleZ);
    Mat4f world = translate * scale;

    renderer.SetWorldMatrix(world);
    plaque->Render(renderer);
}

} // namespace theWheelGL

/// GLPlaque.h
/// OpenGL ES plaque mesh — port of D3D9 Plaque class.
/// Creates GL VBOs from RadialShape parametric curves.
///
/// Copyright (C) 1996-2007 Derek G Lane

#pragma once

#include <GLES2/gl2.h>
#include <vector>
#include "GLMath.h"
#include "GLRenderer.h"

// Forward declaration — defined in theWheelView
namespace theWheel2007 { class RadialShape; }

namespace theWheelGL {

class GLPlaque {
public:
    /// Construct a plaque for the given shape and bevel radius.
    /// Takes ownership of pShape.
    GLPlaque(theWheel2007::RadialShape* pShape, float borderRadius);
    ~GLPlaque();

    /// Render the plaque using the current GL state.
    void Render(GLRenderer& renderer);

    /// Access the underlying shape.
    theWheel2007::RadialShape* GetShape() { return m_pShape; }

private:
    struct Vertex {
        float px, py, pz;
        float nx, ny, nz;
    };

    void buildGeometry();
    void calcSections(std::vector<Vertex>& verts);
    void calcVertsForSection(float theta, const Vec3f& offset,
                             std::vector<Vertex>& verts);
    void buildStripIndices();
    void buildFanGeometry();
    void uploadBuffers();

    theWheel2007::RadialShape* m_pShape;
    float m_borderRadius;
    int m_numSteps;

    // Bevel strip geometry
    std::vector<Vertex> m_stripVerts;
    std::vector<unsigned short> m_stripIndices;
    std::vector<int> m_sectionStarts;
    int m_vertPerSection;

    // Face (center fan, converted to triangles) geometry
    std::vector<Vertex> m_faceVerts;
    std::vector<unsigned short> m_faceIndices;

    // GL buffers
    GLuint m_stripVBO;
    GLuint m_stripIBO;
    GLuint m_faceVBO;
    GLuint m_faceIBO;

    bool m_built;
};

} // namespace theWheelGL

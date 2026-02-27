/// GLPlaque.cpp
/// Implementation of the GLPlaque class.
/// Port of D3D9 Plaque.cpp geometry generation to OpenGL ES VBOs.
///
/// Copyright (C) 1996-2007 Derek G Lane

#include "GLPlaque.h"
#include "RadialShape.h"

#include <cmath>

#ifndef PIf
#define PIf 3.14159265358979323846f
#endif

namespace theWheelGL {

GLPlaque::GLPlaque(theWheel2007::RadialShape* pShape, float borderRadius)
    : m_pShape(pShape)
    , m_borderRadius(borderRadius)
    , m_numSteps(8)
    , m_vertPerSection(0)
    , m_stripVBO(0)
    , m_stripIBO(0)
    , m_faceVBO(0)
    , m_faceIBO(0)
    , m_built(false)
{
}

GLPlaque::~GLPlaque()
{
    if (m_stripVBO) glDeleteBuffers(1, &m_stripVBO);
    if (m_stripIBO) glDeleteBuffers(1, &m_stripIBO);
    if (m_faceVBO) glDeleteBuffers(1, &m_faceVBO);
    if (m_faceIBO) glDeleteBuffers(1, &m_faceIBO);
    delete m_pShape;
}

void GLPlaque::Render(GLRenderer& renderer)
{
    if (!m_built) {
        buildGeometry();
        uploadBuffers();
        m_built = true;
    }

    GLint aPos = renderer.GetPositionAttrib();
    GLint aNorm = renderer.GetNormalAttrib();
    const GLsizei stride = sizeof(Vertex);

    // --- Draw bevel strip sections ---
    if (m_stripVBO && m_stripIBO && m_sectionStarts.size() > 1) {
        glBindBuffer(GL_ARRAY_BUFFER, m_stripVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_stripIBO);

        glEnableVertexAttribArray(aPos);
        glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(aNorm);
        glVertexAttribPointer(aNorm, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

        // Set bevel material (light blue-gray, matching D3D9 Plaque::Render)
        renderer.SetMaterial(0.8f, 0.8f, 0.9f, 1.0f, 0.8f, 0.8f, 0.9f);
        renderer.ApplyUniforms();

        // Each section pair draws the same index pattern with different base vertex.
        // In GLES2, we cannot use glDrawElementsBaseVertex, so we generated
        // absolute indices for each section pair.
        int indicesPerPair = (int)m_stripIndices.size() / ((int)m_sectionStarts.size() - 1);
        for (size_t i = 0; i + 1 < m_sectionStarts.size(); i++) {
            int indexOffset = (int)i * indicesPerPair;
            glDrawElements(GL_TRIANGLE_STRIP, indicesPerPair, GL_UNSIGNED_SHORT,
                           (void*)(indexOffset * sizeof(unsigned short)));
        }
    }

    // --- Draw center face ---
    if (m_faceVBO && m_faceIBO) {
        glBindBuffer(GL_ARRAY_BUFFER, m_faceVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_faceIBO);

        glEnableVertexAttribArray(aPos);
        glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(aNorm);
        glVertexAttribPointer(aNorm, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

        // Set face material (warm red-brown, matching D3D9 Plaque::Render)
        renderer.SetMaterial(0.7f, 0.4f, 0.4f, 1.0f, 0.7f, 0.4f, 0.4f);
        renderer.ApplyUniforms();

        glDrawElements(GL_TRIANGLES, (GLsizei)m_faceIndices.size(),
                       GL_UNSIGNED_SHORT, (void*)0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLPlaque::buildGeometry()
{
    m_stripVerts.clear();
    m_sectionStarts.clear();
    calcSections(m_stripVerts);
    buildStripIndices();
    buildFanGeometry();
}

void GLPlaque::calcSections(std::vector<Vertex>& verts)
{
    auto& arrDiscont = m_pShape->Discontinuities();
    size_t nNextDiscont = 0;

    const float step = 2.0f * PIf / (float)(m_numSteps * 4);

    float theta = 0.0f;
    for (int nSlice = 0; nSlice < m_numSteps * 4; nSlice++) {
        if (nSlice + 1 == m_numSteps * 4) {
            theta = 0.0f; // close the loop
        }

        CVectorD<2, REAL> vPt = m_pShape->Eval(theta);
        m_sectionStarts.push_back((int)verts.size());
        calcVertsForSection(theta, Vec3f((float)vPt[0], (float)vPt[1], 0.0f), verts);

        theta += step;

        if (nNextDiscont < arrDiscont.size() && theta > arrDiscont[nNextDiscont]) {
            theta = (float)arrDiscont[nNextDiscont++];
            nSlice--; // extra slice at discontinuity
        }
    }
}

void GLPlaque::calcVertsForSection(float theta, const Vec3f& offset,
                                   std::vector<Vertex>& verts)
{
    int origLen = (int)verts.size();
    Mat4f mRotate = Mat4f::RotationZ(theta);

    const float step = (PIf / 2.0f) / (float)(m_numSteps - 1);

    float angle = 0.0f;
    for (int nAt = 0; nAt < m_numSteps; nAt++) {
        Vec3f pos(std::cos(angle), 0.0f, std::sin(angle));
        Vec3f normal = mRotate.transformNormal(pos);
        Vec3f tpos = mRotate.transformCoord(pos);
        tpos *= m_borderRadius;
        tpos += offset;

        Vertex v;
        v.px = tpos.x; v.py = tpos.y; v.pz = tpos.z;
        v.nx = normal.x; v.ny = normal.y; v.nz = normal.z;
        verts.push_back(v);

        angle += step;
    }

    // Pie-wedge tip vertex
    Vertex tipVert;
    tipVert.px = offset.x; tipVert.py = offset.y; tipVert.pz = m_borderRadius;
    // Reuse last normal for the tip
    if (!verts.empty()) {
        auto& last = verts.back();
        tipVert.nx = last.nx; tipVert.ny = last.ny; tipVert.nz = last.nz;
    } else {
        tipVert.nx = 0; tipVert.ny = 0; tipVert.nz = 1.0f;
    }
    verts.push_back(tipVert);

    m_vertPerSection = (int)verts.size() - origLen;
}

void GLPlaque::buildStripIndices()
{
    m_stripIndices.clear();

    if (m_sectionStarts.size() < 2)
        return;

    // For each adjacent section pair, create a triangle strip
    for (size_t s = 0; s + 1 < m_sectionStarts.size(); s++) {
        int base0 = m_sectionStarts[s];
        int base1 = m_sectionStarts[s + 1];

        // Bevel surface: interleave vertices from section s and s+1
        for (int nAt = 0; nAt < m_numSteps - 1; nAt++) {
            m_stripIndices.push_back((unsigned short)(base0 + nAt));
            m_stripIndices.push_back((unsigned short)(base1 + nAt));
            m_stripIndices.push_back((unsigned short)(base0 + nAt + 1));
            m_stripIndices.push_back((unsigned short)(base1 + nAt + 1));
        }

        // Pie-wedge fill to interior
        m_stripIndices.push_back((unsigned short)(base0 + m_numSteps - 1));
        m_stripIndices.push_back((unsigned short)(base1 + m_numSteps - 1));
        // Tip vertex (last vertex in each section)
        m_stripIndices.push_back((unsigned short)(base1 + m_numSteps));
    }
}

void GLPlaque::buildFanGeometry()
{
    m_faceVerts.clear();
    m_faceIndices.clear();

    auto& arrDiscont = m_pShape->Discontinuities();
    size_t nNextDiscont = 0;

    // Calculate Y position for the face (matches D3D9 fan)
    float Y = (float)(m_pShape->InnerRect.GetMin(1)
                      + m_pShape->InnerRect.GetSize(1) * 0.75f);
    float angleStart = 0.0f, angleEnd = PIf;
    m_pShape->InvEvalY(Y, angleStart, angleEnd);

    // Center vertex
    Vertex center;
    center.px = 0.0f; center.py = Y; center.pz = m_borderRadius + 0.1f;
    center.nx = 0.0f; center.ny = 0.0f; center.nz = 1.0f;
    m_faceVerts.push_back(center);

    // Edge vertices
    float angleDiff = angleEnd - angleStart;
    float curAngle = angleStart;
    for (int n = 0; n <= m_numSteps * 4; n++) {
        CVectorD<2, REAL> vPt = m_pShape->Eval(curAngle);
        Vertex v;
        v.px = (float)vPt[0]; v.py = (float)vPt[1]; v.pz = m_borderRadius - 0.1f;
        v.nx = 0.0f; v.ny = 0.0f; v.nz = 1.0f;
        m_faceVerts.push_back(v);

        curAngle += angleDiff / (float)(m_numSteps * 4);

        // Extra vertex at discontinuity
        if (nNextDiscont < arrDiscont.size() && curAngle > arrDiscont[nNextDiscont]) {
            CVectorD<2, REAL> vPtD = m_pShape->Eval(arrDiscont[nNextDiscont++]);
            Vertex vd;
            vd.px = (float)vPtD[0]; vd.py = (float)vPtD[1]; vd.pz = m_borderRadius - 0.1f;
            vd.nx = 0.0f; vd.ny = 0.0f; vd.nz = 1.0f;
            m_faceVerts.push_back(vd);
        }
    }

    // Convert fan (center + N edge verts) to triangles
    int numEdge = (int)m_faceVerts.size() - 1;
    for (int i = 0; i < numEdge - 1; i++) {
        m_faceIndices.push_back(0);                          // center
        m_faceIndices.push_back((unsigned short)(i + 1));    // edge i
        m_faceIndices.push_back((unsigned short)(i + 2));    // edge i+1
    }
}

void GLPlaque::uploadBuffers()
{
    // Strip VBO
    if (!m_stripVerts.empty()) {
        glGenBuffers(1, &m_stripVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_stripVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     m_stripVerts.size() * sizeof(Vertex),
                     m_stripVerts.data(), GL_STATIC_DRAW);
    }

    // Strip IBO
    if (!m_stripIndices.empty()) {
        glGenBuffers(1, &m_stripIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_stripIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     m_stripIndices.size() * sizeof(unsigned short),
                     m_stripIndices.data(), GL_STATIC_DRAW);
    }

    // Face VBO
    if (!m_faceVerts.empty()) {
        glGenBuffers(1, &m_faceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_faceVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     m_faceVerts.size() * sizeof(Vertex),
                     m_faceVerts.data(), GL_STATIC_DRAW);
    }

    // Face IBO
    if (!m_faceIndices.empty()) {
        glGenBuffers(1, &m_faceIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_faceIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     m_faceIndices.size() * sizeof(unsigned short),
                     m_faceIndices.data(), GL_STATIC_DRAW);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

} // namespace theWheelGL

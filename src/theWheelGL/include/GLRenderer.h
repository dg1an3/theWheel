/// GLRenderer.h
/// Cross-platform OpenGL ES 2.0 renderer via ANGLE (EGL).
/// Manages EGL context, shader program, and render state.
///
/// Copyright (C) 1996-2007 Derek G Lane

#pragma once

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "GLMath.h"

namespace theWheelGL {

/// Manages EGL display/surface/context and the shader program.
class GLRenderer {
public:
    GLRenderer();
    ~GLRenderer();

    /// Initialize EGL from a native window handle.
    /// On Windows: pass HWND. On macOS: pass NSView* or CALayer*.
    bool InitFromWindow(void* nativeWindow, int width, int height);

    /// Resize the surface (call on window resize).
    void Resize(int width, int height);

    /// Begin a frame: make context current, clear, set viewport.
    void BeginFrame(float r, float g, float b);

    /// End a frame: swap buffers.
    void EndFrame();

    /// Release EGL resources.
    void Shutdown();

    /// Check if initialized.
    bool IsInitialized() const { return m_context != EGL_NO_CONTEXT; }

    // --- Shader uniform setters ---

    void SetWorldMatrix(const Mat4f& mat);
    void SetViewMatrix(const Mat4f& mat);
    void SetProjectionMatrix(const Mat4f& mat);

    /// Set directional light (direction is toward the light source).
    void SetLight(const Vec3f& direction, const Vec3f& color);

    /// Set ambient light color.
    void SetAmbientLight(const Vec3f& color);

    /// Set material diffuse and ambient colors.
    void SetMaterial(float dr, float dg, float db, float da,
                     float ar, float ag, float ab);

    // --- Attribute locations ---
    GLint GetPositionAttrib() const { return m_aPosition; }
    GLint GetNormalAttrib() const { return m_aNormal; }

    /// Upload current uniforms to the shader. Call before draw.
    void ApplyUniforms();

    /// Get viewport dimensions.
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    bool initShaders();
    GLuint compileShader(GLenum type, const char* source);

    // EGL state
    EGLDisplay m_display;
    EGLSurface m_surface;
    EGLContext m_context;

    // Viewport
    int m_width;
    int m_height;

    // Shader program
    GLuint m_program;

    // Attribute locations
    GLint m_aPosition;
    GLint m_aNormal;

    // Uniform locations
    GLint m_uWorld;
    GLint m_uView;
    GLint m_uProj;
    GLint m_uLightDir;
    GLint m_uLightColor;
    GLint m_uAmbientColor;
    GLint m_uMaterialDiffuse;
    GLint m_uMaterialAmbient;

    // Cached uniform values
    Mat4f m_world;
    Mat4f m_view;
    Mat4f m_proj;
    Vec3f m_lightDir;
    Vec3f m_lightColor;
    Vec3f m_ambientColor;
    float m_materialDiffuse[4];
    float m_materialAmbient[3];
};

} // namespace theWheelGL

/// GLRenderer.cpp
/// Implementation of the GLRenderer class.
///
/// Copyright (C) 1996-2007 Derek G Lane

#include "GLRenderer.h"
#include "../Shaders.h"

#include <cstring>
#include <cstdio>

namespace theWheelGL {

GLRenderer::GLRenderer()
    : m_display(EGL_NO_DISPLAY)
    , m_surface(EGL_NO_SURFACE)
    , m_context(EGL_NO_CONTEXT)
    , m_width(0)
    , m_height(0)
    , m_program(0)
    , m_aPosition(-1)
    , m_aNormal(-1)
    , m_uWorld(-1)
    , m_uView(-1)
    , m_uProj(-1)
    , m_uLightDir(-1)
    , m_uLightColor(-1)
    , m_uAmbientColor(-1)
    , m_uMaterialDiffuse(-1)
    , m_uMaterialAmbient(-1)
{
    m_lightDir = Vec3f(0.0f, 0.0f, -1.0f);
    m_lightColor = Vec3f(1.0f, 1.0f, 1.0f);
    m_ambientColor = Vec3f(0.25f, 0.25f, 0.25f);
    m_materialDiffuse[0] = m_materialDiffuse[1] = m_materialDiffuse[2] = 0.8f;
    m_materialDiffuse[3] = 1.0f;
    m_materialAmbient[0] = m_materialAmbient[1] = m_materialAmbient[2] = 0.8f;
}

GLRenderer::~GLRenderer()
{
    Shutdown();
}

bool GLRenderer::InitFromWindow(void* nativeWindow, int width, int height)
{
    m_width = width;
    m_height = height;

    // Get EGL display
    m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (m_display == EGL_NO_DISPLAY)
        return false;

    // Initialize EGL
    EGLint majorVersion, minorVersion;
    if (!eglInitialize(m_display, &majorVersion, &minorVersion))
        return false;

    // Choose config
    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(m_display, configAttribs, &config, 1, &numConfigs) || numConfigs == 0)
        return false;

    // Create window surface
    m_surface = eglCreateWindowSurface(m_display, config,
        static_cast<EGLNativeWindowType>(nativeWindow), nullptr);
    if (m_surface == EGL_NO_SURFACE)
        return false;

    // Create GLES2 context
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    m_context = eglCreateContext(m_display, config, EGL_NO_CONTEXT, contextAttribs);
    if (m_context == EGL_NO_CONTEXT)
        return false;

    // Make current
    if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context))
        return false;

    // Initialize shaders
    if (!initShaders())
        return false;

    // Set default GL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);

    return true;
}

void GLRenderer::Resize(int width, int height)
{
    m_width = width;
    m_height = height;
    if (m_context != EGL_NO_CONTEXT) {
        eglMakeCurrent(m_display, m_surface, m_surface, m_context);
    }
}

void GLRenderer::BeginFrame(float r, float g, float b)
{
    eglMakeCurrent(m_display, m_surface, m_surface, m_context);
    glViewport(0, 0, m_width, m_height);
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_program);
}

void GLRenderer::EndFrame()
{
    eglSwapBuffers(m_display, m_surface);
}

void GLRenderer::Shutdown()
{
    if (m_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (m_program) {
            glDeleteProgram(m_program);
            m_program = 0;
        }

        if (m_surface != EGL_NO_SURFACE) {
            eglDestroySurface(m_display, m_surface);
            m_surface = EGL_NO_SURFACE;
        }

        if (m_context != EGL_NO_CONTEXT) {
            eglDestroyContext(m_display, m_context);
            m_context = EGL_NO_CONTEXT;
        }

        eglTerminate(m_display);
        m_display = EGL_NO_DISPLAY;
    }
}

// --- Uniform setters ---

void GLRenderer::SetWorldMatrix(const Mat4f& mat) { m_world = mat; }
void GLRenderer::SetViewMatrix(const Mat4f& mat) { m_view = mat; }
void GLRenderer::SetProjectionMatrix(const Mat4f& mat) { m_proj = mat; }

void GLRenderer::SetLight(const Vec3f& direction, const Vec3f& color)
{
    m_lightDir = direction.normalized();
    m_lightColor = color;
}

void GLRenderer::SetAmbientLight(const Vec3f& color) { m_ambientColor = color; }

void GLRenderer::SetMaterial(float dr, float dg, float db, float da,
                             float ar, float ag, float ab)
{
    m_materialDiffuse[0] = dr; m_materialDiffuse[1] = dg;
    m_materialDiffuse[2] = db; m_materialDiffuse[3] = da;
    m_materialAmbient[0] = ar; m_materialAmbient[1] = ag; m_materialAmbient[2] = ab;
}

void GLRenderer::ApplyUniforms()
{
    glUniformMatrix4fv(m_uWorld, 1, GL_FALSE, m_world.data());
    glUniformMatrix4fv(m_uView, 1, GL_FALSE, m_view.data());
    glUniformMatrix4fv(m_uProj, 1, GL_FALSE, m_proj.data());
    glUniform3f(m_uLightDir, m_lightDir.x, m_lightDir.y, m_lightDir.z);
    glUniform3f(m_uLightColor, m_lightColor.x, m_lightColor.y, m_lightColor.z);
    glUniform3f(m_uAmbientColor, m_ambientColor.x, m_ambientColor.y, m_ambientColor.z);
    glUniform4fv(m_uMaterialDiffuse, 1, m_materialDiffuse);
    glUniform3fv(m_uMaterialAmbient, 1, m_materialAmbient);
}

// --- Shader compilation ---

bool GLRenderer::initShaders()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, kVertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFragmentShaderSource);
    if (!vs || !fs) return false;

    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);

    GLint linked;
    glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(m_program, sizeof(log), nullptr, log);
        fprintf(stderr, "GLRenderer: shader link error: %s\n", log);
        glDeleteProgram(m_program);
        m_program = 0;
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    // Cache attribute locations
    m_aPosition = glGetAttribLocation(m_program, "a_position");
    m_aNormal = glGetAttribLocation(m_program, "a_normal");

    // Cache uniform locations
    m_uWorld = glGetUniformLocation(m_program, "u_world");
    m_uView = glGetUniformLocation(m_program, "u_view");
    m_uProj = glGetUniformLocation(m_program, "u_proj");
    m_uLightDir = glGetUniformLocation(m_program, "u_lightDir");
    m_uLightColor = glGetUniformLocation(m_program, "u_lightColor");
    m_uAmbientColor = glGetUniformLocation(m_program, "u_ambientColor");
    m_uMaterialDiffuse = glGetUniformLocation(m_program, "u_materialDiffuse");
    m_uMaterialAmbient = glGetUniformLocation(m_program, "u_materialAmbient");

    return true;
}

GLuint GLRenderer::compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        fprintf(stderr, "GLRenderer: shader compile error (%s): %s\n",
                type == GL_VERTEX_SHADER ? "vertex" : "fragment", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

} // namespace theWheelGL

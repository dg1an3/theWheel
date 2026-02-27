/// Shaders.h
/// Embedded GLSL ES 2.0 shaders replicating D3D9 fixed-function pipeline.
/// Gouraud lighting: 1 directional light + ambient, material color uniform.
///
/// Copyright (C) 1996-2007 Derek G Lane

#pragma once

namespace theWheelGL {

/// Vertex shader: transforms position and computes Gouraud lighting per-vertex.
/// Uniforms:
///   u_world, u_view, u_proj — model/view/projection matrices
///   u_lightDir — normalized direction TO light (world space)
///   u_lightColor — directional light RGB
///   u_ambientColor — ambient light RGB
///   u_materialDiffuse — material diffuse RGBA
///   u_materialAmbient — material ambient RGB
static const char* const kVertexShaderSource = R"GLSL(
attribute vec3 a_position;
attribute vec3 a_normal;

uniform mat4 u_world;
uniform mat4 u_view;
uniform mat4 u_proj;

uniform vec3 u_lightDir;
uniform vec3 u_lightColor;
uniform vec3 u_ambientColor;
uniform vec4 u_materialDiffuse;
uniform vec3 u_materialAmbient;

varying vec4 v_color;

void main() {
    // Transform position
    vec4 worldPos = u_world * vec4(a_position, 1.0);
    gl_Position = u_proj * u_view * worldPos;

    // Transform normal to world space (using upper-left 3x3 of world matrix)
    // For uniform scaling this is correct; for non-uniform, use inverse transpose
    vec3 worldNormal = normalize(mat3(u_world) * a_normal);

    // Gouraud lighting
    float NdotL = max(dot(worldNormal, -u_lightDir), 0.0);
    vec3 diffuse = u_lightColor * u_materialDiffuse.rgb * NdotL;
    vec3 ambient = u_ambientColor * u_materialAmbient;

    v_color = vec4(diffuse + ambient, u_materialDiffuse.a);
}
)GLSL";

/// Fragment shader: passes through interpolated vertex color.
static const char* const kFragmentShaderSource = R"GLSL(
precision mediump float;
varying vec4 v_color;

void main() {
    gl_FragColor = v_color;
}
)GLSL";

} // namespace theWheelGL

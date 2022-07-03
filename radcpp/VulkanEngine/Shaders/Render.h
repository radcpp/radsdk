#ifndef RENDER_H
#define RENDER_H

layout(set = 0, binding = 0) uniform FrameUniforms
{
    mat4 viewProjectionMatrix;
} g_frameUniforms;

layout(set = 0, binding = 1) uniform MeshUniforms
{
    mat4 modelToWorld;
} g_meshUniforms;

struct FragAttribs
{
    vec3 worldPosition;
#ifdef HAS_NORMAL
#ifdef HAS_TANGENT
    mat3 tangentToWorld;
#else
    vec3 worldNormal;
#endif
#endif
#ifdef HAS_COLOR
    vec4 color;
#endif
#if NUM_UV_CHANNELS > 0
    vec2 texCoord[NUM_UV_CHANNELS];
#endif
};

#endif // RENDER_H
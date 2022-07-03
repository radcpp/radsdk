#version 450 core

#extension GL_AMD_shader_explicit_vertex_parameter : enable

#include "Render.h"

layout(location = 0) in FragAttribs g_fragAttribs;
layout(location = 0) out vec4 g_fragColor;

#ifdef HAS_BASE_COLOR_TEXTURE
layout(set = 1, binding = 0) uniform texture2D g_baseColorTexture;
#endif

layout(set = 2, binding = 0) uniform sampler g_sampler;

float getEdgeFactor(float width, float feather)
{
    width = width / 3.0f;
    vec3 bary = vec3(gl_BaryCoordSmoothAMD.xy, 1.0 - gl_BaryCoordSmoothAMD.x - gl_BaryCoordSmoothAMD.y);
    vec3 d = fwidth(bary);
    bary = smoothstep(d * width, d * (width + feather), bary);
    float factor = min(min(bary.x, bary.y), bary.z);
    factor = 1.0 - factor;
    return factor;
}

void main()
{
    vec4 baseColor = vec4(0, 0, 0, 0);
#ifdef HAS_COLOR
    baseColor = g_fragAttribs.color;
#endif
#ifdef HAS_BASE_COLOR_TEXTURE
    baseColor = texture(sampler2D(g_baseColorTexture, g_sampler), g_fragAttribs.texCoord[0]);
#endif
    const vec4 wireframeColor = vec4(1.0, 1.0, 1.0, 1.0);
    float edgeFactor = getEdgeFactor(1.0, 2.0);
    g_fragColor = mix(baseColor, wireframeColor, edgeFactor);
}

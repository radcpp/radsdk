#version 450 core

#include "Render.h"

#ifdef HAS_POSITION
layout(location = 0) in vec3 g_inputPosition;
#endif
#ifdef HAS_NORMAL
layout(location = 1) in vec3 g_inputNormal;
#endif
#ifdef HAS_TANGENT
layout(location = 2) in vec4 g_inputTangent;
#endif
#ifdef HAS_COLOR
layout(location = 3) in vec4 g_inputColor;
#endif
#if NUM_UV_CHANNELS > 0
layout(location = 4) in vec2 g_inputTexCoord[NUM_UV_CHANNELS];
#endif

layout(location = 0) out FragAttribs g_fragAttribs;

void main()
{
    vec4 worldPosition = g_meshUniforms.modelToWorld * vec4(g_inputPosition, 1.0f);
    gl_Position = g_frameUniforms.viewProjectionMatrix * worldPosition;

    g_fragAttribs.worldPosition = worldPosition.xyz;

#ifdef HAS_NORMAL
#ifdef HAS_TANGENT
    vec3 worldNormal = normalize(vec3(g_meshUniforms.modelToWorld * vec4(g_inputNormal, 0.0)));
    vec3 worldTangent = normalize(vec3(g_meshUniforms.modelToWorld * vec4(g_inputTangent.xyz, 0.0)));
    vec3 worldBitangent = cross(worldNormal, worldTangent) * g_inputTangent.w;
    g_fragAttribs.tangentToWorld = mat3(worldTangent, worldBitangent, worldNormal);
#else
    g_fragAttribs.worldNormal = normalize(vec3(g_meshUniforms.normalMatrix * vec4(g_inputNormal, 0.0)));
#endif
#endif

#ifdef HAS_COLOR
    g_fragAttribs.color = g_inputColor;
#endif

#if NUM_UV_CHANNELS > 0
    for(uint channelIndex = 0; channelIndex < NUM_UV_CHANNELS; channelIndex++)
    {
        g_fragAttribs.texCoord[channelIndex] = g_inputTexCoord[channelIndex];
    }
#endif
}
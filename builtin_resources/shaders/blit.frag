#version 460
#extension GL_GOOGLE_include_directive : enable

#include "utils/toneMapping.glsl"

layout(set = 0, binding = 0) uniform sampler2D src;

layout(location = 0) in vec2 texCoords;
layout(location = 0) out vec4 outputColor;

void main()
{
    vec3 srcColor = texture(src, texCoords).rgb;
    outputColor   = vec4(pow(ACESToneMapping(srcColor), vec3(.6f)), 1.f);
}
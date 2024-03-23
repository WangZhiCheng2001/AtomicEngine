#version 460

layout(set = 0, binding = 0) uniform sampler2D src;

layout(location = 0) in vec2 texCoords;
layout(location = 0) out vec4 outputColor;

void main()
{
    outputColor = texture(src, texCoords);
}
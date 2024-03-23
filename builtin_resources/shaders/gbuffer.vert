#version 460

#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;
layout(location = 0) out vec3 positionWorld;

layout(push_constant) uniform PushConstants { mat4 matrixView; mat4 matrixProj; };

void main()
{
    gl_Position = matrixProj * matrixView * vec4(pos, 1.0);
    positionWorld = pos;
}
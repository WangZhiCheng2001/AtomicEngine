#version 460

layout(location = 0) in vec3 positionWorld;
layout(location = 0) out vec4 gbuffer;

void main()
{
    gbuffer = vec4(positionWorld, uintBitsToFloat(gl_PrimitiveID));
}
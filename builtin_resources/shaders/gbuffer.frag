#version 460

layout(location = 0) flat in uint drawIndex;
layout(location = 0) out vec4 gbuffer;

void main() { gbuffer = unpackUnorm4x8(((drawIndex & 255) << 23) | ((gl_PrimitiveID + 1) & ((1 << 23) - 1))); }
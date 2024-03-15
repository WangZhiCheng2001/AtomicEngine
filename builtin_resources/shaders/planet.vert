#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 0) out vec2 fragTexCoord;

layout(push_constant) uniform PushConstants { mat4 mvp; };

void main()
{
    gl_Position = mvp * vec4(pos, 1.0);
    fragTexCoord = texCoord;
}
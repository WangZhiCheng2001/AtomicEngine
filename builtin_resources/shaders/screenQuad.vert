#version 460

layout(location = 0) out vec2 v_texCoords;

// use a larger triangle mesh to cover screen quad
// then regions outside screen quad will be culled
void main()
{
	v_texCoords = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(v_texCoords * 2 - 1, 1.f, 1.f);
}
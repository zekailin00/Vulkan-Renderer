#version 450

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoords;

layout (location = 0) out vec3 outUVW;

layout (set = 1, binding = 0) uniform ViewProjection 
{
    mat4 view;
    mat4 projection;
} vp;

void main() 
{
	outUVW = Position;

	mat4 newView = vp.view;
	newView[3][0] = 0;
	newView[3][1] = 0;
	newView[3][2] = 0;

	gl_Position = vp.projection * newView * vec4(Position.xyz, 1.0);
	gl_Position.y = -gl_Position.y;
}

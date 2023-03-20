#version 450

layout (set = 0, binding = 0) uniform samplerCube samplerCubeMap;

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = texture(samplerCubeMap, inUVW);
	outFragColor = pow(outFragColor, vec4(0.4545)); // Gamma correction
}
#version 450

layout (location = 0) in vec3 inColor;

layout (location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(inColor, 1);
}
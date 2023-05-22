#version 450
#extension GL_EXT_scalar_block_layout : require

// glslc shader.vert -o vert.spv

layout (location = 0) out vec3 oFragPos;
layout (location = 1) out vec3 oNormal;
layout (location = 2) out vec2 oTexCoords;
layout (location = 3) out vec3 oViewPos;

layout (set = 1, binding = 0, std430) uniform MeshCoordinates
{
    mat4 model;
} m;

layout (set = 2, binding = 0, std430) uniform ViewProjection 
{
    mat4 view;
    mat4 projection;
} vp;



layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoords;

void main()
{
    oFragPos = vec3(m.model * vec4(Position, 1.0));
    oNormal =  vec3(m.model * vec4(Normal, 0.0));;  
    oTexCoords = TexCoords;
    mat4 camera = inverse(vp.view);
    oViewPos = vec3(camera[3][0], camera[3][1], camera[3][2]);
    gl_Position = vp.projection * vp.view * vec4(oFragPos, 1);
}

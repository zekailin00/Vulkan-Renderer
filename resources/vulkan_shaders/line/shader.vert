#version 450

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoords;

layout (location = 3) in vec3 BeginPoint;
layout (location = 4) in vec3 EndPoint;

layout (location = 0) out vec3 VertColor;

layout (set = 0, binding = 0) uniform ViewProjection 
{
    mat4 view;
    mat4 projection;
} vp;

layout (set = 1, binding = 0) uniform LineProperties
{
    mat4 model;

    vec3  color;
    float _0;

    float width;
    int   useGlobalTransform;
    vec2  resolution;
} line;


void main()
{
    vec4 clip0;
    vec4 clip1;

    // https://wwwtyro.net/2019/11/18/instanced-lines.html
    if (line.useGlobalTransform != 0)
    {
        clip0 = vp.projection * vp.view * line.model * vec4(BeginPoint, 1.0);
        clip1 = vp.projection * vp.view * line.model * vec4(EndPoint, 1.0);
    }
    else
    {
        clip0 = vp.projection * vp.view * vec4(BeginPoint, 1.0);
        clip1 = vp.projection * vp.view * vec4(EndPoint, 1.0);
    }

    vec2 screen0 = line.resolution * (0.5 * clip0.xy/clip0.w + 0.5);
    vec2 screen1 = line.resolution * (0.5 * clip1.xy/clip1.w + 0.5);

    vec2 xBasis = normalize(screen1 - screen0);
    vec2 yBasis = vec2(-xBasis.y, xBasis.x);
    vec2 pt0 = screen0 + line.width * (Position.x * xBasis + Position.y * yBasis);
    vec2 pt1 = screen1 + line.width * (Position.x * xBasis + Position.y * yBasis);
    vec2 pt = mix(pt0, pt1, Position.z);

    vec4 clip = mix(clip0, clip1, Position.z);

    gl_Position = vec4(clip.w * ((2.0 * pt) / line.resolution - 1.0), clip.z, clip.w);

    VertColor = line.color;
}
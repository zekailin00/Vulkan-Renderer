#version 450

layout(push_constant) uniform PushConstants {
	vec3 beginPoint;
	float width;
	vec3 endPoint;
	float _1;
	vec3 color;
	float _2;
} lineInfo;

layout (set = 0, binding = 0) uniform ViewProjection 
{
    mat4 view;
    mat4 projection;
} vp;

const vec3 LINE[6] = vec3[] (
	vec3( 0.01,  1.0, 0.0),
	vec3(-0.01, -1.0, 0.0),
	vec3(-0.01,  1.0, 0.0),
    vec3(-0.01, -1.0, 0.0),
	vec3( 0.01,  1.0, 0.0),
	vec3( 0.01, -1.0, 0.0)
);

layout (location = 0) out vec3 oColor;

void main() 
{
	vec3 up = normalize(lineInfo.beginPoint - lineInfo.endPoint);
	vec3 _view = normalize(vec3(vp.view[0][2], vp.view[1][2], vp.view[2][2]));
	vec3 right = normalize(cross(up, _view));
	vec3 view = normalize(cross(right, up));

	vec3 pos = (lineInfo.beginPoint + lineInfo.endPoint) / 2;
	float len = length(lineInfo.beginPoint - lineInfo.endPoint) / 2;

	// translation * rotation * scale(length)
	mat4 model = mat4 (
		vec4(right, 0),
		vec4(up, 0),
		vec4(view, 0),
		vec4(pos, 1)
	) * mat4(
		vec4(lineInfo.width, 0, 0, 0),
		vec4(0, len, 0, 0),
		vec4(0, 0, 1, 0),
		vec4(0, 0, 0, 1)
	);

	vec4 camCoord = vp.view * model * vec4(LINE[gl_VertexIndex].xyz, 1.0);

	// The width of the wire is depth independent
	// FIXME: unknown transform bug; might related to camera or cross
	camCoord.x = -camCoord.x * camCoord.z / 6;
	camCoord.y = -camCoord.y * camCoord.z / 6;

	gl_Position = vp.projection * camCoord;
	oColor = lineInfo.color;
}

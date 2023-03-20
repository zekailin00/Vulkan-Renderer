#version 450
#extension GL_EXT_scalar_block_layout : require

// glslc shader.frag -o frag.spv

layout (location = 0) in vec3 FragPos;  
layout (location = 1) in vec3 Normal;  
layout (location = 2) in vec2 TexCoords;
layout (location = 3) in vec3 ViewPos;

layout (location = 0) out vec4 FragColor;

layout (set = 0, binding = 0, std430) uniform MeshProperties
{
    vec4  albedo;

    float metallic;
    float smoothness;
    float _1;
    float _2;

    float useAlbedoTex;
    float useMetallicTex;
    float useSmoothnessTex;
    float useNormalTex;
} meshProperties;

layout (set = 0, binding = 1) uniform sampler2D AlbedoTexture;
layout (set = 0, binding = 2) uniform sampler2D MetallicTexture;
layout (set = 0, binding = 3) uniform sampler2D SmoothnessTexture;
layout (set = 0, binding = 4) uniform sampler2D NormalTexture;

struct DirLight
{
    vec4 direction;
    vec4 color;
} dirLight;

layout (set = 3, binding = 0, std430) uniform SceneProperties
{
    uvec4 lightCount; // only 1st is used
    DirLight lights[5];

} scene;

const float PI = 3.14159265359;


// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom); 
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// Fresnel function ----------------------------------------------------
vec3 F_Schlick(float cosTheta, float metallic, vec3 albedo)
{
	vec3 F0 = mix(vec3(0.04), albedo, metallic);
	vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0); 
	return F;    
}


vec3 BRDF(vec3 L, vec3 V, vec3 N, float metallic, float roughness, vec3 albedo, vec3 lightColor)
{
	// Precalculate vectors and dot products	
	vec3 H = normalize (V + L);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);
	float dotLH = clamp(dot(L, H), 0.0, 1.0);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);

	vec3 color = vec3(0.0);
	if (dotNL > 0.0)
	{
		float rroughness = max(0.05, roughness);
		// D = Normal distribution (Distribution of the microfacets)
		float D = D_GGX(dotNH, roughness); 
		// G = Geometric shadowing term (Microfacets shadowing)
		float G = G_SchlicksmithGGX(dotNL, dotNV, rroughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		vec3 F = F_Schlick(dotNV, metallic, albedo);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

		vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.00001);

		color += (kD * albedo / PI + spec) * dotNL * lightColor;
	}

	return color;
}

void main()
{
	vec3 N = normalize(Normal);
	vec3 V = normalize(ViewPos - FragPos);

    float metallicFrag;
    float roughnessFrag;
    vec3 albedoFrag;

    if (meshProperties.useMetallicTex == 0)
        metallicFrag = meshProperties.metallic;
    else
        metallicFrag = texture(MetallicTexture, TexCoords).x;

    if (meshProperties.useSmoothnessTex == 0)
        roughnessFrag = clamp(1 - meshProperties.smoothness, 0.0, 1.0);
    else
        roughnessFrag = clamp(1 - texture(SmoothnessTexture, TexCoords).x, 0.0, 1.0);

    if (meshProperties.useAlbedoTex == 0)
        albedoFrag = meshProperties.albedo.rgb;
    else
        albedoFrag = texture(AlbedoTexture, TexCoords).rgb;

	// Specular contribution
	vec3 Lo = vec3(0.0);
	for (int i = 0; i < scene.lightCount.x; i++) {
		vec3 L = normalize(-scene.lights[i].direction.xyz);
		L.y = -L.y;
        vec3 lightColor = scene.lights[i].color.rgb;
		Lo += BRDF(L, V, N, metallicFrag, roughnessFrag, albedoFrag, lightColor);
	}

	// Combine with ambient
	vec3 color = albedoFrag * 0.02;
	color += Lo;

    // HDR mapping
    color = color / (color + vec3(1.0));

	// Gamma correct
	color = pow(color, vec3(0.4545));

	FragColor = vec4(color, 1.0);
}



// void main()
// {
//     // ambient
//     vec3 ambient;
//     if (meshProperties.useAlbedoTex == 0)
//         ambient = sceneProperties.lights[0].ambient * meshProperties.albedo;
//     else
//         ambient = sceneProperties.lights[0].ambient * texture(AlbedoTexture, TexCoords).rgb;

  	
//     // diffuse 
//     vec3 norm = normalize(Normal);
//     vec3 lightDir = sceneProperties.lights[0].direction;
//     float diff = max(dot(norm, lightDir), 0.0);
//     vec3 diffuse = sceneProperties.lights[0].diffuse * diff * texture(AlbedoTexture, TexCoords).rgb;  
    
//     // // specular
//     // vec3 viewDir = normalize(ViewPos - FragPos);
//     // vec3 reflectDir = reflect(-lightDir, norm);  
//     // float spec = pow(max(dot(viewDir, reflectDir), 0.0001), meshProperties.specularShininess);
//     // vec3 specular = meshProperties.specular * 0.0;

//     vec3 result = diffuse + ambient; // + specular;
//     FragColor = vec4(result, 1);
// } 




#include "shader_builtin.h"

const char* sk_shader_builtin_pbr = R"_(// [name] sk/default_pbr
cbuffer GlobalBuffer : register(b0) {
	float4x4 sk_view;
	float4x4 sk_proj;
	float4x4 sk_viewproj;
	float4   sk_light;
	float4   sk_light_color;
	float4   sk_camera_pos;
	float4   sk_camera_dir;
};
struct Inst {
	float4x4 world;
	float4   color;
};
cbuffer TransformBuffer : register(b1) {
	Inst sk_inst[800];
};
TextureCube sk_cubemap : register(t11);
SamplerState tex_cube_sampler;

cbuffer ParamBuffer : register(b2) {
	// [param] color color {1,1,1,1}
	float4 _color;
	// [param] float metallic 0
	float metallic;
	// [param] float roughness 1
	float roughness;
	// [param] float tex_scale 1
	float tex_scale;
};
struct vsIn {
	float4 pos : SV_POSITION;
	float3 norm : NORMAL;
	float2 uv : TEXCOORD0;
	float3 color : COLOR0;
};
struct psIn {
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float3 world : TEXCOORD1;
};

// [texture] diffuse white
Texture2D tex : register(t0);
SamplerState tex_sampler;

// [texture] emission black
Texture2D tex_emission : register(t1);
SamplerState tex_e_sampler;

// [texture] metal white
Texture2D tex_metal : register(t2);
SamplerState tex_metal_sampler;

// [texture] normal flat
Texture2D tex_normal : register(t3);
SamplerState tex_normal_sampler;

// [texture] occlusion white
Texture2D tex_occ : register(t4);
SamplerState tex_occ_sampler;

float3 sRGBToLinear(float3 sRGB);
float3 LinearTosRGB(float3 lin);

float3x3 CotangentFrame(float3 N, float3 p, float2 uv);

float DistributionGGX(float3 normal, float3 half_vec, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float NdotL, float NdotV, float roughness);
float3 FresnelSchlick(float NdotV, float3 surfaceColor, float metalness);

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	output.world = mul(float4(input.pos.xyz, 1), sk_inst[id].world).xyz;
	output.pos   = mul(float4(output.world,  1), sk_viewproj);

	output.normal = normalize(mul(float4(input.norm, 0), sk_inst[id].world).xyz);
	output.uv     = input.uv * tex_scale;
	output.color  = input.color * sk_inst[id].color.rgb * _color.rgb;
	return output;
}

float4 ps(psIn input) : SV_TARGET{
	float3 albedo      = sRGBToLinear(tex         .Sample(tex_sampler,       input.uv).rgb) * input.color.rgb;
	float3 emissive    = sRGBToLinear(tex_emission.Sample(tex_e_sampler,     input.uv).rgb);
	float3 metal_rough = sRGBToLinear(tex_metal   .Sample(tex_metal_sampler, input.uv).rgb); // b is metallic, rough is g
	float3 tex_norm    = (tex_normal.Sample(tex_normal_sampler,input.uv).xyz) * 2 - 1;
	//float  occlusion   = (tex_occ     .Sample(tex_occ_sampler,input.uv).r);

	float metal = metal_rough.b * metallic;
	float rough = max(1-(metal_rough.g * roughness), 0.001);

	float3 normal = normalize(input.normal);
	float3 view   = normalize(sk_camera_pos.xyz - input.world);
	tex_norm = mul(tex_norm, CotangentFrame(normal, -view, input.uv));
	normal   = normalize(tex_norm);
	float3 light    = -normalize(sk_light.xyz);
	float3 half_vec =  normalize(light + normal);
	float  NdotL    = (dot(normal,light));
	float  NdotV    = (dot(normal,view));

	float w, h;
	uint mip_levels;
	sk_cubemap.GetDimensions(0, w, h, mip_levels);

	float3 reflection = reflect(-view, normal);
	float3 irradiance = sk_cubemap.SampleLevel(tex_cube_sampler, normal, (1-rough)*mip_levels).rgb; // This should be Spherical Harmonics eventually
	float3 reflection_color = sk_cubemap.SampleLevel(tex_cube_sampler, reflection, (1-rough)*mip_levels).rgb;

	// Lighting an object is a combination of two types of light reflections,
	// a diffuse reflection, and a specular reflection. These reflections
	// are approximated using functions called BRDFs (Bidirectional Reflectance
	// Distribuition Function).

	// This formula is a common format for the specular BRDF,
	// Unreal calls this a 'generalized microfacet model', as different functions
	// can easily be swapped out for D, F, and G
	// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf
	//
	//          D(h) F(l,h) G(l,v,h)
	// f(l,v) = --------------------
	//              4(n*l)(n*v)
	//
	// Where:
	// D() is specular distribution function
	// F() is fresnel function
	// G() is geometric shadowing function
	// n is the surface normal
	// l is the vector from the surface to the light position (light direction)
	// v is view direction, or direction from the surface to the camera
	// h is the half vector, h = normalize(l + v)
	float  D = DistributionGGX(normal, half_vec, rough);
	float3 F = FresnelSchlick(max(NdotV, 0), albedo, metal);
	float  G = GeometrySmith(max(NdotL, 0), max(NdotV, 0), rough);
	float3 specular =
		(D * G * F)
			/
		(4 * NdotL * NdotV);

	// For the diffuse BRDF, we'll use Lambert's, since it's simple, effective,
	// and research has said you can't do much better for cheap!
	// https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
	//
	// L = c_diffuse/PI * c_light(n*l)
	//
	// Where:
	// c_diffuse is the diffuse color
	// c_light is the color of the light
	// n is the surface normal
	// l is the vector from the surface to the light position (light direction)

	// Find the diffuse contribution to the reflectance, this is based on the Fresnel
	float3 specular_contribution = F;
	float3 diffuse_contribution  = 1 - specular_contribution;
	diffuse_contribution *= 1 - metal;

	// Combine it all together
	float3 diffuse = (albedo*irradiance);
	float3 ambient = diffuse_contribution * diffuse + reflection_color*F;
	float3 reflectance = ambient;// + (diffuse_contribution * albedo / 3.14159) * sk_light_color.rgb * saturate(NdotL);
	return float4(LinearTosRGB(reflectance + emissive), _color.a);
}

// From: http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
float3 sRGBToLinear(float3 sRGB) {
	return sRGB * (sRGB * (sRGB * 0.305306011 + 0.682171111) + 0.012522878);
}
float3 LinearTosRGB(float3 lin) {
	float3 S1 = sqrt(lin);
	float3 S2 = sqrt(S1);
	float3 S3 = sqrt(S2);
	return 0.585122381 * S1 + 0.783140355 * S2 - 0.368262736 * S3;
}

// http://www.thetenthplanet.de/archives/1180
// For creating normals without a precalculated tangent
float3x3 CotangentFrame(float3 N, float3 p, float2 uv)
{
	// get edge vectors of the pixel triangle
	float3 dp1  = ddx(p);
	float3 dp2  = ddy(p);
	float2 duv1 = ddx(uv);
	float2 duv2 = ddy(uv);

	// solve the linear system
	float3 dp2perp = cross(dp2, N);
	float3 dp1perp = cross(N, dp1);
	float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame 
	float invmax = rsqrt(max(dot(T, T), dot(B, B)));
	return float3x3(T * invmax, B * invmax, N);
}

// https://learnopengl.com/PBR/Theory
// This is a normal distribution function called, Trowbridge-Reitz GGX
float DistributionGGX(float3 normal, float3 half_vec, float roughness)
{
	float roughness2 = roughness * roughness;
	float NdotH      = max(dot(normal, half_vec), 0.0);
	float NdotH2     = NdotH * NdotH;

	float nom   = roughness2;
	float denom = (NdotH2 * (roughness2 - 1.0) + 1.0);
	denom = 3.14159 * denom * denom;

	return nom / denom;
}

// These are Geometry functions
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float nom   = NdotV;
	float denom = NdotV * (1.0 - roughness) + roughness;

	return nom / denom;
}
float GeometrySmith(float NdotL, float NdotV, float roughness)
{
	//float NdotV = max(dot(normal, view), 0.0);
	//float NdotL = max(dot(normal, light_dir), 0.0);
	float ggx1 = GeometrySchlickGGX(NdotV, roughness);
	float ggx2 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

// and Fresnel
float3 FresnelSchlick(float NdotV, float3 surfaceColor, float metalness)
{
	float3 F0 = 0.04;//float3(0.04);
	F0 = lerp(F0, surfaceColor, metalness);
	return F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);
})_";
#include <stereokit.hlsli>
#include <stereokit_pbr.hlsli>

//--name = sk/pbr

float4 color           = {1,1,1,1};
float4 emission_factor = {0,0,0,0};
float4 tex_trans       = {0,0,1,1};
float  metallic        = 0;
float  roughness       = 1;

//--diffuse  = white
//--emission = black
//--metal    = rough
Texture2D    diffuse    : register(t0);
SamplerState diffuse_s  : register(s0);
Texture2D    emission   : register(t1);
SamplerState emission_s : register(s1);
Texture2D    metal      : register(t2);
SamplerState metal_s    : register(s2);

struct vsIn {
	float4 pos       : SV_Position;
	float3 norm      : NORMAL0;
	float2 uv        : TEXCOORD0;
	float4 color     : COLOR0;
};
struct psIn {
	float4      pos       : SV_POSITION;
	min16float3 normal    : NORMAL0;
	float2      uv        : TEXCOORD0;
	min16float4 color     : COLOR0;
	min16float3 irradiance: COLOR1;
	float3      view_dir  : TEXCOORD1;
};

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;
	float3x3 world3x3 = (float3x3)sk_inst[ids.inst].world;
	float3   world    = mul(input.pos.xyz, world3x3) + sk_inst[ids.inst].world[3].xyz;
	o.pos             = mul(float4(world, 1), sk_viewproj[ids.view]);

	o.normal     = normalize(mul(input.norm, world3x3));
	o.uv         = (input.uv * tex_trans.zw) + tex_trans.xy;
	o.color      = input.color * sk_inst[ids.inst].color * color;
	o.irradiance = sk_lighting(o.normal);
	o.view_dir   = sk_camera_pos[ids.view].xyz - world;
	return o;
}

min16float4 ps(psIn input) : SV_TARGET {
	min16float2 metal_rough = (min16float2)metal    .Sample(metal_s,     input.uv).gb; // rough is g, b is metallic
	min16float4 albedo      = (min16float4)diffuse  .Sample(diffuse_s,   input.uv);
	min16float3 emissive    = (min16float3)emission .Sample(emission_s,  input.uv).rgb;

	min16float metallic_final = metal_rough.y * (min16float)metallic;
	min16float rough_final    = metal_rough.x * (min16float)roughness;

	min16float4 color = sk_pbr_shade(albedo * input.color, input.irradiance, 1, metallic_final, rough_final, input.view_dir, input.normal);
	color.rgb += emissive * (min16float3)emission_factor.rgb;
	return color;
}
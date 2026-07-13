#include "stereokit.hlsli"

//--name = sk/lightmap

float4 tex_trans = {0,0,1,1};

//--diffuse = white
Texture2D    diffuse    : register(t0);
SamplerState diffuse_s  : register(s0);
//--lightmap = white
Texture2D    lightmap   : register(t1);
SamplerState lightmap_s : register(s1);

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float2 uv1   : TEXCOORD0;
	float2 uv2   : TEXCOORD1;
};

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;
	float3 world = mul(float4(input.pos.xyz, 1), sk_inst[ids.inst].world).xyz;
	o.pos        = mul(float4(world,         1), sk_viewproj[ids.view]);

	o.uv1   = (input.uv * tex_trans.zw) + tex_trans.xy;
	o.uv2   = input.norm.xy;
	return o;
}
float4 ps(psIn input) : SV_TARGET {
	float4 col   = diffuse .Sample(diffuse_s,  input.uv1);
	float4 light = lightmap.Sample(lightmap_s, input.uv2);
	
	return col * light;
}
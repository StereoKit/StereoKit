#include "stereokit.hlsli"

//--name = sk/default_ui

float4 color = {1,1,1,1};

//--diffuse = white
Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 color: COLOR0;
};
struct psIn {
	float4      pos   : SV_POSITION;
	float2      uv    : TEXCOORD0;
	float3      world : TEXCOORD1;
	min16float4 color : COLOR0;
};

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;
	float3 normal = normalize(mul(input.norm, (float3x3) sk_inst[ids.inst].world));
	float4 world  = mul(input.pos, sk_inst[ids.inst].world);
	o.pos   = mul(world, sk_viewproj[ids.view]);
	o.world = world.xyz;
	o.uv    = input.uv;
	o.color.rgb = color.rgb * input.color.rgb * sk_inst[ids.inst].color.rgb * sk_lighting(normal);
	o.color.a   = input.color.a;
	return o;
}

min16float4 ps(psIn input) : SV_TARGET {
	min16float  glow = sk_finger_glow(input.world);
	min16float4 col  = min16float4(lerp(input.color.rgb, min16float3(1, 1, 1), glow), input.color.a);

	return (min16float4)diffuse.Sample(diffuse_s, input.uv) * col;
}
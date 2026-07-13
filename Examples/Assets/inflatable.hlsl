#include <stereokit.hlsli>

//--name = app/inflatable
//--color:color = 1, 1, 1, 1
//--inflate     = 0.02
//--diffuse     = white
float4       color;
float        inflate;

Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float2 uv    : TEXCOORD0;
	float4 color : COLOR0;
};

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;

	float3 world  = mul(float4(input.pos.xyz, 1), sk_inst[ids.inst].world).xyz;
	float3 normal = mul(input.norm, (float3x3)sk_inst[ids.inst].world);
	world += normal * inflate;
	o.pos = mul(float4(world, 1), sk_viewproj[ids.view]);

	o.uv    = input.uv;
	o.color = input.col * color * sk_inst[ids.inst].color;
	return o;
}
float4 ps(psIn input) : SV_TARGET {
	float4 col = diffuse.Sample(diffuse_s, input.uv);

	col = col * input.color;

	return col; 
}
#include "stereokit.hlsli"

//--color:color = 1, 1, 1, 1
//--tex_trans   = 0,0,1,1
//--diffuse     = white
//--cutoff      = 0.01

float4       color;
float4       tex_trans;
float        cutoff;
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
	half4  color : COLOR0;
	uint view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	float4 world = mul(float4(input.pos.xyz, 1), sk_inst[id].world);
	o.pos        = mul(world,                    sk_viewproj[o.view_id]);

	o.uv    = (input.uv * tex_trans.zw) + tex_trans.xy;
	o.color = input.col * color * sk_inst[id].color;
	return o;
}

half4 ps(psIn input) : SV_TARGET {
	half4 col = diffuse.Sample(diffuse_s, input.uv);
	if (col.a < cutoff) discard;
	
	return col * input.color;
}
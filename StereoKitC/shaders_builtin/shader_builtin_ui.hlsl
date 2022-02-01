#include "stereokit.hlsli"

//--name = sk/default_ui
//--color:color = 1, 1, 1, 1
//--diffuse     = white
float4       color;
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
	float3 normal: NORMAL0;
	float2 uv    : TEXCOORD0;
	float4 color : COLOR0;
	float4 world : TEXCOORD1;
	uint view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	o.world = mul(input .pos, sk_inst    [id].world);
	o.pos   = mul(o.world,    sk_viewproj[o.view_id]);

	o.normal = normalize(mul(input.norm, (float3x3)sk_inst[id].world));

	o.uv         = input.uv;
	o.color      = color * input.col * sk_inst[id].color * sk_inst[id].color.a;
	o.color.rgb *= Lighting(o.normal);

	return o;
}
float4 ps(psIn input) : SV_TARGET {
	float  glow = FingerGlow(input.world.xyz, input.normal);
	float4 col  = float4(lerp(input.color.rgb, float3(2,2,2), glow), input.color.a);

	return diffuse.Sample(diffuse_s, input.uv) * col;
}
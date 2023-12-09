#include "stereokit.hlsli"

//--name = sk/default_ui
//--color:color = 1, 1, 1, 1
//--ui_tint     = 0
//--diffuse     = white
float4       color;
bool         ui_tint;
Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 color: COLOR0;
};
struct psIn {
	float4 pos     : SV_POSITION;
	float2 uv      : TEXCOORD0;
	float3 world   : TEXCOORD1;
	half3  normal  : NORMAL0;
	half3  color   : COLOR0;
	uint   view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	float4 world = mul(input.pos, sk_inst[id].world);
	o.pos    = mul(world, sk_viewproj[o.view_id]);
	o.normal = normalize(mul(input.norm, (float3x3)sk_inst[id].world));
	o.world  = world.xyz;

	o.uv    = input.uv;
	o.color = (ui_tint == true
		? lerp(color, sk_inst[id].color, input.color.a)
		: (color * input.color * sk_inst[id].color)).rgb * sk_lighting(o.normal);

	return o;
}
float4 ps(psIn input) : SV_TARGET {
	float  glow = sk_finger_glow(input.world, input.normal);
	float4 col  = float4(lerp(input.color, float3(2,2,2), glow), 1);

	return diffuse.Sample(diffuse_s, input.uv) * col;
}
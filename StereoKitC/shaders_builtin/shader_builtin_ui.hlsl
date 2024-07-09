#include "stereokit.hlsli"

//--color:color = 1, 1, 1, 1
//--diffuse     = white
float4       color;
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
	half4  color   : COLOR0;
	uint   view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	float3 normal = normalize(mul(input.norm, (float3x3) sk_inst[id].world));
	float4 world  = mul(input.pos, sk_inst[id].world);
	o.pos   = mul(world, sk_viewproj[o.view_id]);
	o.world = world.xyz;
	o.uv    = input.uv;
	o.color.rgb = color.rgb * input.color.rgb * sk_inst[id].color.rgb * sk_lighting(normal);
	o.color.a   = input.color.a;
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	float  glow = pow(1 - saturate(sk_finger_distance(input.world) / 0.12), 10);
	float4 col  = float4(lerp(input.color.rgb, half3(1, 1, 1), glow), input.color.a);

	return diffuse.Sample(diffuse_s, input.uv) * col;
}
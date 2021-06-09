#include "stereokit.hlsli"

//--name        = sk/lines
//--color:color = 1,1,1,1
//--diffuse = white
float4       color;
Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

struct vsIn {
	float4 pos    : SV_Position;
	float3 tangent: NORMAL0;
	float2 uv     : TEXCOORD0;
	float4 col    : COLOR0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float2 uv    : TEXCOORD0;
	float4 color : COLOR0;
	uint view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	float3 view_pos = mul(float4(input.pos.xyz, 1), sk_view[o.view_id]).xyz;
	float2 view_tan = normalize(mul(float4(input.tangent, 0), sk_view[o.view_id]).xy);
	view_pos.xy += float2(view_tan.y, -view_tan.x) * input.uv.x;

	o.pos   = mul(float4(view_pos, 1), sk_proj[o.view_id]);
	o.uv    = float2(0, input.uv.y);
	o.color = input.col * color;
	return o;
}
float4 ps(psIn input) : SV_TARGET {
	float4 col = diffuse.Sample(diffuse_s, input.uv);

	col = col * input.color;
	// Anti-alias the line edge
	//col.a = (0.5f-abs(input.uv.y - 0.5f)) / fwidth(input.uv.y);

	return col;
}
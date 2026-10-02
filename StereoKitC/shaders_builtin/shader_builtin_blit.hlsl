#include "stereokit.hlsli"
//--name = sk/blit
//--source = white

// Shrinks the sampled region of the source, so a blit can read just the
// upper-left corner of a render target that's only partially drawn. uv_clamp
// is the last texel center of that region, keeping linear samples at the far
// edges from reading past what was drawn.
float2 uv_scale = {1,1};
float2 uv_clamp = {1,1};

Texture2D    source   : register(t0);
SamplerState source_s : register(s0);

struct psIn {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

psIn vs(uint id : SV_VertexID) {
	psIn o;
	o.pos = float4(o.uv * float2(2, -2) + float2(-1, 1), 0, 1);
	float2 quad = float2(id & 2, (id << 1) & 2);
	o.uv  = quad * uv_scale;
	o.pos = float4(quad * float2(2, -2) + float2(-1, 1), 0, 1);
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	return source.Sample(source_s, min(input.uv, uv_clamp));
}

#include "stereokit.hlsli"
//--name = sk/blit
//--source = white

Texture2D    source   : register(t0);
SamplerState source_s : register(s0);

struct psIn {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

psIn vs(uint id : SV_VertexID) {
	psIn o;
	o.uv  = float2(id & 2, (id << 1) & 2);
	o.pos = float4(o.uv * float2(2, -2) + float2(-1, 1), 0, 1);
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	return source.Sample(source_s, input.uv);
}

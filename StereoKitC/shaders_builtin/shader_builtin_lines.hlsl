#include "stereokit.hlsli"

//--name = sk/lines

float4 color = {1,1,1,1};

struct vsIn {
	float4 pos    : SV_Position;
	float3 next   : NORMAL0;
	float2 uv     : TEXCOORD0;
	float4 col    : COLOR0;
};
struct psIn {
	float4      pos   : SV_POSITION;
	min16float4 color : COLOR0;
};

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;
	float  aspect   = sk_aspect_ratio(ids.view);
	float4 pos      = mul(float4(input.pos.xyz, 1), sk_viewproj[ids.view]);
	float4 next     = mul(float4(input.next,    1), sk_viewproj[ids.view]);
	float2 proj_pos = pos .xy / pos .w;
	float2 proj_next= next.xy / next.w;
	proj_pos.x     *= aspect;
	proj_next.x    *= aspect;
	float2 dir = normalize(proj_next - proj_pos);
	dir = float2(dir.y, -dir.x) * input.uv.x * sign(next.w) * sign(pos.w); // Multiply by signs to fix a flipping issue when point is offscreen
	dir = mul(float4(dir.x, dir.y, 0, 1), sk_proj[ids.view]).xy;
	pos.xy += dir;

	o.pos   = pos;
	o.color = input.col * color;
	return o;
}
min16float4 ps(psIn input) : SV_TARGET {
	return input.color;
}
#include "stereokit.hlsli"

//--name        = sk/lines
//--color:color = 1,1,1,1
float4 color;

struct vsIn {
	float4 pos    : SV_Position;
	float3 next   : NORMAL0;
	float2 uv     : TEXCOORD0;
	float4 col    : COLOR0;
};
struct psIn : sk_ps_input_t {
	float4 pos   : SV_POSITION;
	float4 color : COLOR0;
};

psIn vs(vsIn input, sk_vs_input_t sk_in) {
	psIn o;
	uint view_id = sk_view_init(sk_in, o);
	uint id      = sk_inst_id  (sk_in);

	float  aspect   = sk_aspect_ratio(view_id);
	float4 pos      = mul(float4(input.pos.xyz, 1), sk_viewproj[view_id]);
	float4 next     = mul(float4(input.next,    1), sk_viewproj[view_id]);
	float2 proj_pos = pos .xy / pos .w;
	float2 proj_next= next.xy / next.w;
	proj_pos.x     *= aspect;
	proj_next.x    *= aspect;
	float2 dir = normalize(proj_next - proj_pos);
	dir = float2(dir.y, -dir.x) * input.uv.x * sign(next.w) * sign(pos.w); // Multiply by signs to fix a flipping issue when point is offscreen
	dir = mul(float4(dir.x, dir.y, 0, 1), sk_proj[view_id]).xy;
	pos.xy += dir;

	o.pos   = pos;
	o.color = input.col * color;
	return o;
}
float4 ps(psIn input) : SV_TARGET {
	float4 col = input.color;

	// Anti-alias the line edge
	//col.a = (0.5f-abs(input.uv.y - 0.5f)) / fwidth(input.uv.y);

	return col;
}
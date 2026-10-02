#include <stereokit.hlsli>

//--name = app/spec_constant_test
// Test shader for specialization constants. Each [[vk::constant_id]] constant
// feeds the output color, so overriding one (via Material.SetInt/SetFloat/
// SetBool/SetUInt) produces a visibly different pipeline variant.

[[vk::constant_id(0)]] const int   COLOR_STEPS  = 2;
[[vk::constant_id(1)]] const float BRIGHTNESS   = 0.25;
[[vk::constant_id(2)]] const bool  USE_TINT     = true;
[[vk::constant_id(3)]] const uint  BLUE_SCALE   = 4;

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
	float4 world = mul(input.pos, sk_inst[ids.inst].world);
	o.pos        = mul(world,     sk_viewproj[ids.view]);
	o.uv         = input.uv;
	o.color      = input.col;
	return o;
}

float4 ps(psIn input) : SV_Target {
	// Fold each spec constant into a channel. The loop count, the float, the
	// bool branch and the uint are all compile-time constants inside the
	// baked pipeline.
	float r = 0;
	for (int i = 0; i < COLOR_STEPS; i++)
		r += 0.2;

	float3 col = float3(r, BRIGHTNESS, (float)BLUE_SCALE / 16.0);
	if (USE_TINT)
		col *= float3(1, 0.5, 1);

	return float4(col, 1);
}

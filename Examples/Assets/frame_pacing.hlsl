#include <stereokit.hlsli>

//--name = app/frame_pacing
// Picket fence for the frame pacing test, see TestFramePacing.cs. The quad's
// UVs map to a virtual pixel grid, so the pattern is the same on any display
// and in XR.

float4 fence = {0, 64, 32, 0}; // x scroll px, y bar period px, z bar width px, w marker left edge px
float2 size_px = {1600, 900};
int    frame_index;

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4 pos : SV_POSITION;
	float2 px  : TEXCOORD0;
};

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;
	float3 world = mul(float4(input.pos.xyz, 1), sk_inst[ids.inst].world).xyz;
	o.pos = mul(float4(world, 1), sk_viewproj[ids.view]);
	// The quad's u runs right to left as seen from the front, v top to bottom
	o.px  = float2(1 - input.uv.x, input.uv.y) * size_px;
	return o;
}

float3 hue_rgb(float h) {
	float3 p = abs(frac(h + float3(1.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - 3.0);
	return saturate(p - 1.0);
}

float picket(float x, float period, float bar) {
	return frac(x / period) * period < bar ? 1.0 : 0.0;
}

float4 ps(psIn input) : SV_TARGET {
	float2 px = input.px;
	float  y  = px.y / size_px.y;

	// Left strip: a new hue every frame. In a high speed camera capture a
	// repeated color is a repeated frame, and a skipped hue is a dropped one.
	if (px.x < 24) {
		float v = (frame_index & 1) ? 1.0 : 0.55;
		return float4(hue_rgb(frac(frame_index * 0.618034)) * v, 1);
	}

	float3 col = 0.12;
	if      (y > 0.08 && y < 0.42) col = picket(px.x - fence.x,       fence.y, fence.z);
	else if (y > 0.47 && y < 0.68) col = picket(px.x - fence.x * 2.0, fence.y, fence.z);
	else if (y > 0.73 && y < 0.84) {
		float d = px.x - fence.w;
		col = (d >= 0 && d < 32) ? float3(1.0, 0.6, 0.1) : 0.04;
	}
	else if (y > 0.89 && y < 0.96) {
		// 16 cells, one lit per frame, so a capture can count displayed frames
		int cell = (int)floor(px.x / size_px.x * 16.0);
		col = cell == (frame_index & 15) ? float3(0.2, 1.0, 0.3) : 0.04;
	}
	return float4(col, 1);
}

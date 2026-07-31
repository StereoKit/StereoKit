#include "stereokit.hlsli"

//--name = skt/postfx_depth_fog
//--fog_density = 0.5
//--fog_color   = 0.4, 0.5, 0.65

// A depth-reading post-process! Depth arrives as a second input attachment
// at index 1, always single-sample - under MSAA, StereoKit resolves depth
// on-tile first, so the same shader works at every MSAA setting. Including
// stereokit.hlsli gives postfx shaders the system constants, so depth can
// be linearized with the real per-view projection.
float  fog_density;
float3 fog_color;

[[vk::input_attachment_index(0)]] SubpassInput<float4> color;
[[vk::input_attachment_index(1)]] SubpassInput<float>  depth;

struct psIn {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

psIn vs(uint id : SV_VertexID) {
	psIn o;
	float2 uv = float2(id & 2, (id << 1) & 2);
	o.pos = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
	o.uv  = uv;
	return o;
}

float4 ps(psIn input, uint view_id : SV_ViewID) : SV_TARGET {
	float4 c = color.SubpassLoad();
	float  d = depth.SubpassLoad();

	// Unproject to view space using this eye's inverse projection. uv is
	// texture-space (0,0 top-left) while NDC has y up, so v inverts.
	float2 ndc  = float2(input.uv.x * 2 - 1, 1 - input.uv.y * 2);
	float4 view = mul(float4(ndc, d, 1), sk_proj_inv[view_id]);
	float  dist = length(view.xyz / view.w);

	float fog = saturate(exp(-dist * fog_density));
	return float4(lerp(fog_color, c.rgb, fog), c.a);
}

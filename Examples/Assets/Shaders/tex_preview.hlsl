#include "stereokit.hlsli"

// Shows one mip and layer of a 2D, array, or cubemap texture. sk_renderer
// binds cubes and plain 2D textures here through a 2D array view.

//--mip      = 0
//--layer    = 0
//--channels = 1, 1, 1, 0
//--exposure = 1
//--source   = white

float  mip;
float  layer;
float4 channels;
float  exposure;

Texture2DArray source   : register(t0);
SamplerState   source_s : register(s0);

struct vsIn {
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD0;
};
struct psIn {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;
	float4 world = mul(float4(input.pos.xyz, 1), sk_inst[ids.inst].world);
	o.pos = mul(world, sk_viewproj[ids.view]);
	o.uv  = input.uv;
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	float4 color = source.SampleLevel(source_s, float3(input.uv, layer), mip);
	color.rgb *= exposure;

	// One channel on its own reads clearest as grayscale
	if (dot(channels, 1) == 1)
		return float4(dot(color, channels).xxx, 1);
	return float4(color.rgb * channels.rgb, 1);
}

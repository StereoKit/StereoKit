//--name = sk/blit/fxaa
//--source = white

// Reference: https://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf

#include "stereokit.hlsli"

Texture2D    source   : register(t0);
SamplerState source_s : register(s0);
float4       source_i;

static const float fxaa_threshold     = .125;
static const float fxaa_threshold_min = 1/32.0;
static const float fxaa_subpixel  = .75;

cbuffer TransformBuffer : register(b2) {
	float sk_width;
	float sk_height;
	float sk_pixel_width;
	float sk_pixel_height;
};

struct vsIn {
	float4 pos  : SV_Position;
	float2 uv   : TEXCOORD0;
};
struct psIn {
	float4 pos : SV_POSITION;
	float2 uv   : TEXCOORD0;
};

psIn vs(vsIn input) {
	psIn o;
	o.pos = input.pos;
	o.uv  = input.uv;
	return o;
}

float get_luma(float2 uv) {
	// As an optimization, luminance is estimated strictly from Red and Green channels
	// using a single fused multiply add operation. In practice pure blue aliasing rarely
	// appears in typical game content.
	float4 col = source.Sample(source_s, uv);
	return col.g * (0.587 / 0.299) + col.r;
}

struct luma_t {
	float m, n, e, s, w, ne, se, sw, nw;
	float highest, lowest, range;
};

luma_t get_luma_neighborhood(float2 uv) {
	luma_t luma;
	luma.m  = get_luma(uv);
	luma.n  = get_luma(uv + float2( 0,               sk_pixel_height));//,  0.0,  1.0);
	luma.e  = get_luma(uv + float2( sk_pixel_width,  0              ));//,  1.0,  0.0);
	luma.s  = get_luma(uv + float2( 0,              -sk_pixel_height));//,  0.0, -1.0);
	luma.w  = get_luma(uv + float2(-sk_pixel_width,  0              ));//, -1.0,  0.0);
	luma.ne = get_luma(uv + float2( sk_pixel_width,  sk_pixel_height));//,  1.0,  1.0);
	luma.se = get_luma(uv + float2( sk_pixel_width, -sk_pixel_height));//,  1.0, -1.0);
	luma.sw = get_luma(uv + float2(-sk_pixel_width, -sk_pixel_height));//, -1.0, -1.0);
	luma.nw = get_luma(uv + float2(-sk_pixel_width,  sk_pixel_height));//, -1.0,  1.0);

	luma.highest = max(max(max(max(luma.m, luma.n), luma.e), luma.s), luma.w);
	luma.lowest  = min(min(min(min(luma.m, luma.n), luma.e), luma.s), luma.w);
	luma.range   = luma.highest - luma.lowest;
	return luma;
}

struct fxaa_edge_t {
	bool  is_horizontal;
	float pixel_step;
};

bool is_horizontal(luma_t luma) {
	float horizontal =
		2.0 * abs(luma.n  + luma.s  - 2.0 * luma.m) +
		      abs(luma.ne + luma.se - 2.0 * luma.e) +
		      abs(luma.nw + luma.sw - 2.0 * luma.w);
	float vertical =
		2.0 * abs(luma.e  + luma.w  - 2.0 * luma.m) +
		      abs(luma.ne + luma.nw - 2.0 * luma.n) +
		      abs(luma.se + luma.sw - 2.0 * luma.s);
	return horizontal >= vertical;
}

fxaa_edge_t get_edge(luma_t luma) {
	fxaa_edge_t edge;
	edge.is_horizontal = is_horizontal(luma);

	float lumaP, lumaN;
	if (edge.is_horizontal) {
		edge.pixel_step = sk_pixel_height;
		lumaP = luma.n;
		lumaN = luma.s;
	} else {
		edge.pixel_step = sk_pixel_width;
		lumaP = luma.e;
		lumaN = luma.w;
	}
	float gradientP = abs(lumaP - luma.m);
	float gradientN = abs(lumaN - luma.m);

	if (gradientP < gradientN) {
		edge.pixel_step = -edge.pixel_step;
		//edge.lumaGradient = gradientN;
		//edge.otherLuma = lumaN;
	}
	else {
		//edge.lumaGradient = gradientP;
		//edge.otherLuma = lumaP;
	}

	return edge;
}

float subpixel_blend(luma_t luma) {
	/*float filter = luma.n + luma.e + luma.s + luma.w;
	filter *= 1.0 / 4;
	return filter;*/
	float filter = 2.0 * (luma.n + luma.e + luma.s + luma.w);
	filter += luma.ne + luma.nw + luma.se + luma.sw;
	filter *= 1.0 / 12.0;
	filter = saturate(filter / luma.range);
	filter = smoothstep(0, 1, filter);
	return filter * filter * fxaa_subpixel;
}

float4 ps(psIn input) : SV_TARGET{
	if (sk_time % 4.0 < 2)
		return source.Sample(source_s, input.uv);

	luma_t luma = get_luma_neighborhood(input.uv);
	if (luma.range < max(fxaa_threshold_min, fxaa_threshold * luma.highest)) {
		return source.Sample(source_s, input.uv);
	}

	fxaa_edge_t edge = get_edge(luma);
	float  blend         = subpixel_blend(luma);
	float2 blend_uv      = input.uv;
	if (edge.is_horizontal) {
		//return float4(0, 1, 0, 1);
		blend_uv.y += blend * edge.pixel_step;
	} else {
		//return float4(1, 0, 0, 1);
		blend_uv.x += blend * edge.pixel_step;
	}
	//return edge.pixel_step > 0.0 ? float4(1.0, 0.0, 0.0, 0.0) : 1.0;
	return source.Sample(source_s, blend_uv);
}
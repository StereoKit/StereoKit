#include <stereokit.hlsli>

//--name = app/shader_arrays
//--color:color = 1, 1, 1, 1
float4 color;

float4 offsets[10];

//--diffuse     = white
Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn : sk_ps_input_t {
	float4 pos   : SV_POSITION;
	float2 uv    : TEXCOORD0;
	float4 color : COLOR0;
};

psIn vs(vsIn input, sk_vs_input_t sk_in) {
	psIn o;
	uint view_id = sk_view_init(sk_in, o);
	uint id      = sk_inst_id  (sk_in);

	float4 world = mul(input.pos, sk_inst[id].world);
	o.pos        = mul(world,     sk_viewproj[view_id]);

	o.uv    = input.uv + offsets[((uint)(sk_time*4))%10].xy;
	o.color = input.col * color * sk_inst[id].color;
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	float4 col = diffuse.Sample(diffuse_s, input.uv);

	col = col * input.color;

	return col; 
}
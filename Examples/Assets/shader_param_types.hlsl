#include <stereokit.hlsli>

//--name = app/param_types
// This shader is a test for using matrices as parameters. Instead of using the
// instance transform that StereoKit provides, it uses a transform provided
// from the user.

struct data_t {
	uint2 a;
	int2 b;
};

//--color:color = 1, 1, 1, 1
float4 color;
int4 id_set;
uint4 id_set2;
bool on_off;
data_t data;

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

	o.uv    = input.uv;
	o.color = input.col * color * sk_inst[id].color;

	// Just some code to prevent vars from being optimized out.
	float4 id_set3 = id_set + id_set2;
	float4 stuff = id_set3 + float4(.1,.1,.1,.1);
	if (on_off) {
		stuff += float4(data.a.x, data.a.y, data.b.x, data.b.y);
	}
	if (length(stuff.xyz) < 1) {
		o.color *= 1.2;
	}
	return o;
}
float4 ps(psIn input) : SV_TARGET {
	float4 col = diffuse.Sample(diffuse_s, input.uv);

	col = col * input.color;

	return col; 
}
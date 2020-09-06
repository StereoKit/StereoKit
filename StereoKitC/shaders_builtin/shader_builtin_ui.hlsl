#include "stereokit.hlslinc"

//--name = sk/default_ui
//--color:color = 1, 1, 1, 1
//--diffuse     = white
float4       color;
Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

struct vsIn {
	float4 pos  : SV_POSITION;
	float3 norm : NORMAL;
	float4 col  : COLOR;
	float2 uv   : TEXCOORD0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float4 color : COLOR0;
	float4 world : TEXCOORD1;
	float3 normal: NORMAL;
	float2 uv    : TEXCOORD0;
	uint view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	output.world = mul(input .pos,   sk_inst[id].world);
	output.pos   = mul(output.world, sk_viewproj[sk_inst[id].view_id]);

	output.normal = normalize(mul(input.norm, (float3x3)sk_inst[id].world));

	output.view_id    = sk_inst[id].view_id;
	output.uv         = input.uv;
	output.color      = color * input.col * sk_inst[id].color;
	output.color.rgb *= Lighting(output.normal);
	return output;
}
float4 ps(psIn input) : SV_TARGET {
	float dist = 1;
	float ring = 0;
	for	(int i=0;i<2;i++) {
		float3 delta = sk_fingertip[i].xyz - input.world.xyz;
		float3 norm  = normalize(delta);
		float  d     = dot(delta,delta) / (0.08 * 0.08);
		ring = max( ring, min(1, 1 - abs(max(0,dot(input.normal, norm))-0.9)*200*d) );
		dist = min( dist, d );
	}

	float  pct = pow(1-dist, 5);
	float4 col = float4(lerp(input.color.rgb, float3(1,1,1), pct*0.6 + (ring*pct)), input.color.a);

	return diffuse.Sample(diffuse_s, input.uv) * col; 
}
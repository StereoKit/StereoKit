#include <stereokit.hlsli>

//--name = lighting/light_only

struct Light {
	float4 pos;
	float4 col;
};
cbuffer ParamBuffer : register(b3) {
	Light lights[6];
};

struct vsIn {
	float4 pos  : SV_POSITION;
	float3 norm : NORMAL;
};
struct psIn : sk_ps_input_t {
	float4 pos   : SV_POSITION;
	float3 world : TEXCOORD0;
	float3 normal: NORMAL;
	uint view_id : SV_RenderTargetArrayIndex;
};

// y = 1/(x^2+1) * (1-x/6)
float3 sample_light(float3 world_pos, float3 world_norm, float4 light_pos, float4 light_col) {
	float3 delta = light_pos.xyz - world_pos;
	float  d     = sqrt(dot(delta, delta));
	float  NdotL = max(0, dot(delta/d, world_norm));
	d = d / (light_pos.w * 0.25);
	float  atten = 1/(d*d + 1); //1/(d + ((0.1*0.1)/2));
	return light_col.rgb * light_col.w * atten * NdotL;
}
float3 sample_lights(float3 world_pos, float3 world_norm) {
	float3 result;
	for (int i = 0; i < 6; i++) {
		result += sample_light(world_pos, world_norm, lights[i].pos, lights[i].col);
	}
	return result;
}

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	output.view_id = id % sk_view_count;
	id             = id / sk_view_count;

	float4 world  = mul(input.pos, sk_inst[id].world);
	output.pos    = mul(world,     sk_viewproj[output.view_id]);
	output.world  = world.xyz;
	output.normal = normalize(mul(input.norm, (float3x3)sk_inst[id].world));
	return output;
}
float4 ps(psIn input) : SV_TARGET{
	float4 col;
	col.rgb = sample_lights(input.world, input.normal);
	col.a   = col.r*0.3 + col.g*0.59 + col.b*0.11; // Convert color to grayscale value for alpha
	return col;
}
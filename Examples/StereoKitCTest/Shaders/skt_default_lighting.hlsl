#include <stereokit.hlsli>

//--name = lighting/default

//--color : color = 1, 1, 1, 1
//--tex_scale     = 1
//--diffuse       = white
//--emission      = black

float4       color;
float        tex_scale;
Texture2D    diffuse    : register(t0);
SamplerState diffuse_s  : register(s0);
Texture2D    emission   : register(t1);
SamplerState emission_s : register(s1);

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
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR;
};
struct psIn {
	float4 pos   : SV_Position;
	float4 color : COLOR0;
	float2 uv    : TEXCOORD0;
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
	float3 result = float3(0,0,0);
	for (int i = 0; i < 6; i++) {
		result += sample_light(world_pos, world_norm, lights[i].pos, lights[i].col);
	}
	return result;
}

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	output.view_id = id % sk_view_count;
	id             = id / sk_view_count;

	float4 world = mul(input.pos, sk_inst[id].world);
	output.pos   = mul(world,     sk_viewproj[output.view_id]);

	float3 normal = normalize(mul(input.norm, (float3x3)sk_inst[id].world));

	output.uv      = input.uv * tex_scale;
	output.color   = color * input.col * sk_inst[id].color;
	output.color.rgb = sample_lights(world.xyz, normal) + Lighting(normal);
	return output;
}
float4 ps(psIn input) : SV_TARGET {
	float4 col = diffuse.Sample(diffuse_s, input.uv);
	col.rgb = col.rgb * input.color.rgb;
	col.rgb += emission.Sample(emission_s, input.uv).rgb;
	return col;
}
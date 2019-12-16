const char* sk_shader_builtin_default = R"_(
// [name] sk/default
cbuffer GlobalBuffer : register(b0) {
	float4x4 sk_view[2];
	float4x4 sk_proj[2];
	float4x4 sk_viewproj[2];
	float4   sk_lighting_sh[9];
	float4   sk_camera_pos[2];
	float4   sk_camera_dir[2];
	float4   sk_fingertip[2];
	float    sk_time;
};
struct Inst {
	float4x4 world;
	float4   color;
	uint     view_id;
};
cbuffer TransformBuffer : register(b1) {
	Inst sk_inst[682];
};
TextureCube sk_cubemap : register(t11);
SamplerState tex_cube_sampler;

cbuffer ParamBuffer : register(b2) {
	// [param] color color {1, 1, 1, 1}
	float4 _color;
	// [param] float tex_scale 1
	float tex_scale;
};
struct vsIn {
	float4 pos  : SV_POSITION;
	float3 norm : NORMAL;
	float4 col  : COLOR;
	float2 uv   : TEXCOORD0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float4 color : COLOR0;
	float2 uv    : TEXCOORD0;
	float3 world : TEXCOORD1;
	float3 normal: NORMAL;
	uint view_id : SV_RenderTargetArrayIndex;
};

// [texture] diffuse white
Texture2D tex : register(t0);
SamplerState tex_sampler;

float3 Lighting(float3 normal) {
	float3 result = 0;

	static const float CosineA0 = 3.141592654;
	static const float CosineA1 = (2.0f * CosineA0) / 3.0f;
	static const float CosineA2 = CosineA0 * 0.25f;

	// Band 0
	result += sk_lighting_sh[0].xyz * (0.282095 * CosineA0);

	// Band 1
	float3 n = normal * (0.488603 * CosineA1);
	result += sk_lighting_sh[1].xyz * n.y;
	result += sk_lighting_sh[2].xyz * n.z;
	result += sk_lighting_sh[3].xyz * n.x;

	// Band 2
	n = normal.xyz * normal.yzx;
	float3 n2 = normal * normal;
	result += sk_lighting_sh[4].xyz * (1.092548 * CosineA2 * n.x );
	result += sk_lighting_sh[5].xyz * (1.092548 * CosineA2 * n.y );
	result += sk_lighting_sh[6].xyz * (0.315392 * CosineA2 * (3.0f * n2.z - 1.0f) );
	result += sk_lighting_sh[7].xyz * (1.092548 * CosineA2 * n.z);
	result += sk_lighting_sh[8].xyz * (0.546274 * CosineA2 * (n2.x - n2.y) );

	return result;
}

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	output.world = mul(float4(input.pos.xyz, 1), sk_inst[id].world).xyz;
	output.pos   = mul(float4(output.world,  1), sk_viewproj[sk_inst[id].view_id]);

	output.normal = normalize(mul(float4(input.norm, 0), sk_inst[id].world).xyz);

	float w, h;
	uint mip_levels;
	sk_cubemap.GetDimensions(0, w, h, mip_levels);
	float4 irradiance = float4(sk_cubemap.SampleLevel(tex_cube_sampler, output.normal, (0.7)*mip_levels).rgb, 1);

	output.view_id = sk_inst[id].view_id;
	output.uv      = input.uv * tex_scale;
	output.color   = _color * input.col * sk_inst[id].color;
	output.color.rgb *= Lighting(output.normal);
	return output;
}
float4 ps(psIn input) : SV_TARGET {
	float4 col = tex.Sample(tex_sampler, input.uv);
	return col * input.color;
})_";
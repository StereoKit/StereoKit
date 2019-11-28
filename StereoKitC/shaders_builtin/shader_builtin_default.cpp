const char* sk_shader_builtin_default = R"_(
// [name] sk/default
cbuffer GlobalBuffer : register(b0) {
	float4x4 sk_view[2];
	float4x4 sk_proj[2];
	float4x4 sk_viewproj[2];
	float4   sk_light;
	float4   sk_light_color;
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
	float3 col  : COLOR;
	float2 uv   : TEXCOORD0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float3 color : COLOR0;
	float2 uv    : TEXCOORD0;
	float3 world : TEXCOORD1;
	float3 normal: NORMAL;
	uint view_id : SV_RenderTargetArrayIndex;
};

// [texture] diffuse white
Texture2D tex : register(t0);
SamplerState tex_sampler;

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	output.world = mul(float4(input.pos.xyz, 1), sk_inst[id].world).xyz;
	output.pos   = mul(float4(output.world,  1), sk_viewproj[sk_inst[id].view_id]);

	output.normal = normalize(mul(float4(input.norm, 0), sk_inst[id].world).xyz);

	float w, h;
	uint mip_levels;
	sk_cubemap.GetDimensions(0, w, h, mip_levels);
	float3 irradiance = sk_cubemap.SampleLevel(tex_cube_sampler, output.normal, (0.9)*mip_levels).rgb;

	output.view_id = sk_inst[id].view_id;
	output.uv      = input.uv * tex_scale;
	output.color   = _color.rgb * input.col * sk_inst[id].color.rgb * irradiance;
	return output;
}
float4 ps(psIn input) : SV_TARGET {
	float3 col = tex.Sample(tex_sampler, input.uv).rgb;

	col = col * input.color;

	return float4(col, _color.a); 
})_";
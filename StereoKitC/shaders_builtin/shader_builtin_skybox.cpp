extern const char* sk_shader_builtin_skybox = R"_(
// [name] sk/skybox
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
};
cbuffer TransformBuffer : register(b1) {
	Inst sk_inst[800];
};
TextureCube sk_cubemap : register(t11);
SamplerState tex_cube_sampler;

cbuffer ParamBuffer : register(b2) {
	// [param] float blur 0.0
	float blur;
};
struct vsIn {
	float4 pos  : SV_POSITION;
	float3 norm : NORMAL0;
};
struct psIn {
	float4 pos  : SV_POSITION;
	float3 norm : NORMAL0;
};
struct psOut {
    float4 color : SV_Target;
    float  depth : SV_Depth;
};

psIn vs(vsIn input, uint id : SV_InstanceID, uint view_id : SV_RenderTargetArrayIndex) {
	psIn output;
	output.pos  = mul(float4(input.pos.xyz, 0), sk_viewproj[view_id]);
	output.norm = input.norm;
	return output;
}

psOut ps(vsIn input) {
	psOut result;

	float w, h;
	uint mip_levels;
	sk_cubemap.GetDimensions(0, w, h, mip_levels);

	result.color = sk_cubemap.SampleLevel(tex_cube_sampler, input.norm, blur*mip_levels);
	result.depth = 0.99999;
	return result;
}
)_";
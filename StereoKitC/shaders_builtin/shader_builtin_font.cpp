const char* sk_shader_builtin_font = R"_(
// [name] sk/font
cbuffer GlobalBuffer : register(b0) {
	float4x4 sk_view[2];
	float4x4 sk_proj[2];
	float4x4 sk_viewproj[2];
	float3   sk_lighting_sh[9];
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
	// [param] color color {1,1,1,1}
	float4 color;
};
struct vsIn {
	float4 pos : SV_POSITION;
	float3 norm : NORMAL;
	float2 uv : TEXCOORD0;
	float4 color : COLOR0;
};
struct psIn {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	uint view_id : SV_RenderTargetArrayIndex;
};

// [texture] diffuse white
Texture2D tex : register(t0);
SamplerState tex_sampler;

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	float3 world = mul(float4(input.pos.xyz, 1), sk_inst[id].world).xyz;
	output.pos   = mul(float4(world,         1), sk_viewproj[sk_inst[id].view_id]);

	output.view_id = sk_inst[id].view_id;
	output.normal  = normalize(mul(float4(input.norm, 0), sk_inst[id].world).xyz);
	output.uv      = input.uv;
	output.color   = input.color * color;
	return output;
}

float4 ps(psIn input, bool frontface : SV_IsFrontFace) : SV_TARGET{
	// From an excellent article about text rendering by Ben Golus:
	// https://medium.com/@bgolus/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec
	float2 dx = ddx(input.uv);
	float2 dy = ddy(input.uv); // manually calculate the per axis mip level, clamp to 0 to 1

	float2 uvOffsets = float2(0.125, 0.375);
	float4 offsetUV = float4(0.0, 0.0, 0.0, -1); // supersampled using 2x2 rotated grid
	half4  col = 0;

	offsetUV.xy = input.uv.xy + uvOffsets.x * dx + uvOffsets.y * dy;
	col += tex.SampleBias(tex_sampler, offsetUV.xy, offsetUV.w);
	offsetUV.xy = input.uv.xy - uvOffsets.x * dx - uvOffsets.y * dy;
	col += tex.SampleBias(tex_sampler, offsetUV.xy, offsetUV.w);
	offsetUV.xy = input.uv.xy + uvOffsets.y * dx - uvOffsets.x * dy;
	col += tex.SampleBias(tex_sampler, offsetUV.xy, offsetUV.w);
	offsetUV.xy = input.uv.xy - uvOffsets.y * dx + uvOffsets.x * dy;
	col += tex.SampleBias(tex_sampler, offsetUV.xy, offsetUV.w);
	col *= 0.25;
	float text_value = col.r;
	clip(text_value-0.004); // .004 is 1/255, or one 8bit pixel value!

	float3 albedo = input.color.rgb;
	float3 normal = normalize(input.normal) * (frontface ? -1:1);

	float w, h;
	uint mip_levels;
	sk_cubemap.GetDimensions(0, w, h, mip_levels);
	float3 irradiance = sk_cubemap.SampleLevel(tex_cube_sampler, normal, 0.8*mip_levels).rgb; // This should be Spherical Harmonics eventually

	return float4(albedo, text_value);
}
)_";
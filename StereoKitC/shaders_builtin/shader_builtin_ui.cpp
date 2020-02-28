const char* sk_shader_builtin_ui = R"_(
// [name] sk/default_ui
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
SamplerState tex_cube_sampler : register(s11);

cbuffer ParamBuffer : register(b2) {
	// [param] color color {1, 1, 1, 1}
	float4 _color;
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
	float4 world : TEXCOORD1;
	float3 normal: NORMAL;
	float2 uv    : TEXCOORD0;
	uint view_id : SV_RenderTargetArrayIndex;
};

// [texture] diffuse white
Texture2D tex : register(t0);
SamplerState tex_sampler : register(s0);

// A spherical harmonics lighting lookup!
// Some calculations have been offloaded to 'sh_to_fast'
// in StereoKitC
float3 Lighting(float3 normal) {
	// Band 0
	float3 result = sk_lighting_sh[0].xyz;

	// Band 1
	result += sk_lighting_sh[1].xyz * normal.y;
	result += sk_lighting_sh[2].xyz * normal.z;
	result += sk_lighting_sh[3].xyz * normal.x;

	// Band 2
	float3 n  = normal.xyz * normal.yzx;
	float3 n2 = normal * normal;
	result += sk_lighting_sh[4].xyz * n.x;
	result += sk_lighting_sh[5].xyz * n.y;
	result += sk_lighting_sh[6].xyz * (3.0f * n2.z - 1.0f);
	result += sk_lighting_sh[7].xyz * n.z;
	result += sk_lighting_sh[8].xyz * (n2.x - n2.y);
	return result;
}

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	output.world = mul(input .pos,   sk_inst[id].world);
	output.pos   = mul(output.world, sk_viewproj[sk_inst[id].view_id]);

	output.normal = normalize(mul(input.norm, (float3x3)sk_inst[id].world));

	output.view_id    = sk_inst[id].view_id;
	output.uv         = input.uv;
	output.color      = _color * input.col * sk_inst[id].color;
	output.color.rgb *= Lighting(output.normal);
	return output;
}
float4 ps(psIn input) : SV_TARGET {
	float dist = 1;
	float ring = 0;
	for	(int i=0;i<2;i++) {
		float3 delta = sk_fingertip[i].xyz - input.world.xyz;
		float3 norm = normalize(delta);
		float d = dot(delta,delta) / (0.08 * 0.08);
		ring = max( ring, min(1, 1 - abs(max(0,dot(input.normal, norm))-0.9)*200*d) );
		dist = min( dist, d );
	}

	float  pct = pow(1-dist, 5);
	float4 col = float4(lerp(input.color.rgb, float3(1,1,1), pct*0.6 + (ring*pct)), input.color.a);

	return tex.Sample(tex_sampler, input.uv) * col; 
})_";
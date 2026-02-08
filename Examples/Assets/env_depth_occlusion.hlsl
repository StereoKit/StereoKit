#include <stereokit.hlsli>

//--name = app/env_depth_occlusion

static const float POS_INF = asfloat(0x7F800000);

//--color:color = 1,1,1,1
float4 color;

//--diffuse = white
Texture2D    diffuse       : register(t0);
SamplerState diffuse_s     : register(s0);

//--env_depth
Texture2DArray env_depth   : register(t1);
SamplerState   env_depth_s : register(s1);

//--depth_near = 0.1
float depth_near;
//--depth_far = 10
float depth_far;

// Per-eye depth camera view-projection matrices
//--depth_view_proj_l = 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1
float4x4 depth_view_proj_l;
//--depth_view_proj_r = 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1
float4x4 depth_view_proj_r;

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4 pos     : SV_Position;
	float2 uv      : TEXCOORD0;
	float4 color   : COLOR0;
	float3 world   : TEXCOORD1;
	uint   view_id : SV_RenderTargetArrayIndex;
};

float decode_depth(float z, float nearZ, float farZ, float invalidValue)
{
	if (z <= 0 || z >= 1)
		return invalidValue;

	float d = isinf(farZ)
        ? nearZ / (1.0 - z)
        : (nearZ * farZ) / (farZ - z * (farZ - nearZ));

	return d;
}

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	float4 world = mul(input.pos, sk_inst[id].world);
	o.pos        = mul(world,     sk_viewproj[o.view_id]);
	o.world      = world.xyz;

	float3 normal = normalize(mul(input.norm, (float3x3)sk_inst[id].world));

	o.uv    = input.uv;
	o.color = color * input.col * sk_inst[id].color;
	o.color.rgb *= sk_lighting(normal);
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	float4x4 dvp = (input.view_id == 0) ? depth_view_proj_l : depth_view_proj_r;

	// Project world position into depth camera clip space
	float4 depth_clip = mul(dvp, float4(input.world, 1));

	// Reject fragments behind the depth camera
	clip(depth_clip.w);

	// Convert to depth texture UVs
	float2 depth_ndc = depth_clip.xy / depth_clip.w;
	float2 depth_uv  = depth_ndc * 0.5 + 0.5;

	// Reject outside depth texture bounds
	clip(min(depth_uv.x, depth_uv.y));
	clip(min(1.0 - depth_uv.x, 1.0 - depth_uv.y));

	float depth_raw = env_depth.SampleLevel(env_depth_s, float3(depth_uv, (float)input.view_id), 0).r;
	float depth_m   = decode_depth(depth_raw, depth_near, depth_far, POS_INF);

	// Occlude if virtual surface is behind real geometry
	if (depth_clip.w > depth_m)
		discard;

	float4 col = diffuse.Sample(diffuse_s, input.uv);
	col *= input.color;
	return col;
}
#include <stereokit.hlsli>

//--name = app/env_depth_point_cloud

//--diffuse = white
Texture2DArray diffuse   : register(t0);
SamplerState   diffuse_s : register(s0);

//--color:color = 1,1,1,1
float4 color;
//--point_size = 0.01
float point_size;
//--screen_size = 0
float screen_size;
//--depth_near = 0.1
float depth_near;
//--depth_far = 10
float depth_far;
//--depth_scale = 1
float depth_scale;
//--eye_tans = -1,1,1,-1
float4 eye_tans;
//--eye_layer = 0
float eye_layer;
//--eye_pose = 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1
float4x4 eye_pose;
//--near_clip = 0.1
float near_clip;
//--color_by_depth = 0
float color_by_depth;

struct vsIn {
	float4 pos       : SV_POSITION;
	float3 sample_uv : NORMAL0;
	float2 off       : TEXCOORD0;
	float4 color     : COLOR0;
};
struct psIn {
	float4 pos     : SV_POSITION;
	float2 uv      : TEXCOORD0;
	float4 color   : COLOR0;
	uint   view_id : SV_RenderTargetArrayIndex;
};

float decode_depth(float z, float nearZ, float farZ, float scale, float invalidValue)
{
	if (z <= 0 || z >= 1)
		return invalidValue;

	float d = isinf(farZ)
        ? nearZ / (1.0 - z)
        : (nearZ * farZ) / (farZ - z * (farZ - nearZ));

	return d * scale;
}

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	float2 sample_uv = input.sample_uv.xy;
	float3 tex_coord = float3(sample_uv.x, 1.0 - sample_uv.y, eye_layer);

	float depth_raw = diffuse.SampleLevel(diffuse_s, tex_coord, 0).r;
	float depth_m   = decode_depth(depth_raw, depth_near, depth_far, depth_scale, 0.0);

	// Cull invalid/near points by pushing outside the clip volume
	if (depth_m <= near_clip) {
		o.pos   = float4(0, 0, -2, 1);
		o.color = 0;
		return o;
	}

	// Unproject depth texel to eye-space using per-eye FOV tangents
	float2 tan_xy = float2(
		lerp(eye_tans.x, eye_tans.y, sample_uv.x),
		lerp(eye_tans.z, eye_tans.w, sample_uv.y));
	float3 eye_pos = float3(tan_xy * depth_m, -depth_m);
	float4 world   = mul(eye_pose, float4(eye_pos, 1));

	// Apply point size in view-space (meters) or clip-space (pixels)
	float4 view = mul(world, sk_view[o.view_id]);
	if (screen_size <= 0.1)
		view.xy = point_size * input.off + view.xy;
	o.pos = mul(view, sk_proj[o.view_id]);

	if (screen_size > 0.1) {
		float aspect = sk_proj[o.view_id]._m11 / sk_proj[o.view_id]._m00;
		o.pos.xy = (point_size * input.off / float2(aspect, 1)) * o.pos.w + o.pos.xy;
	}

	// Optional: Map depth to a color gradient over a fixed 0–5m range for visualizing depth
	if (color_by_depth > 0.5) {
		float t = saturate(depth_m / 5.0);
		o.color = float4(1.0 - t, 1.0 - abs(t - 0.5) * 2.0, t, (input.color * color).a);
	} else {
		o.color = input.color * color;
	}

	return o;
}

float4 ps(psIn input) : SV_TARGET {
	return input.color;
}
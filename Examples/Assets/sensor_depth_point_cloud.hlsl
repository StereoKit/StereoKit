#include <stereokit.hlsli>

//--name = app/sensor_depth_point_cloud

//--diffuse = white
Texture2DArray diffuse   : register(t0);
SamplerState   diffuse_s : register(s0);
//--confidence = white
Texture2DArray confidence   : register(t1);
SamplerState   confidence_s : register(s1);

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
//--color_mode = 0
float color_mode;
//--conf_scale = 1
float conf_scale;
//--flip_v = 0
float flip_v;
//--depth_format = 0
float depth_format;

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
};

// Converts a sampled depth value to metric meters. format 0 = ndc (linearized via
// near/far), 1 = metric meters (used directly). 0 and +inf are treated as invalid.
float decode_depth(float format, float z, float nearZ, float farZ, float scale)
{
	if (format >= 0.5)
		return (z <= 0 || isinf(z)) ? 0.0 : z * scale;

	if (z <= 0 || z >= 1)
		return 0.0;

	float d = isinf(farZ)
        ? nearZ / (1.0 - z)
        : (nearZ * farZ) / (farZ - z * (farZ - nearZ));

	return d * scale;
}

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;

	float2 sample_uv = input.sample_uv.xy;
	// Flip the texture row order (independent of depth_format)
	float  tex_v     = flip_v > 0.5 ? sample_uv.y : (1.0 - sample_uv.y);
	float3 tex_coord = float3(sample_uv.x, tex_v, eye_layer);

	float depth_raw = diffuse.SampleLevel(diffuse_s, tex_coord, 0).r;
	float depth_m   = decode_depth(depth_format, depth_raw, depth_near, depth_far, depth_scale);

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
	float4 view = mul(world, sk_view[ids.view]);
	if (screen_size <= 0.1)
		view.xy = point_size * input.off + view.xy;
	o.pos = mul(view, sk_proj[ids.view]);

	if (screen_size > 0.1) {
		float aspect = sk_proj[ids.view]._m11 / sk_proj[ids.view]._m00;
		o.pos.xy = (point_size * input.off / float2(aspect, 1)) * o.pos.w + o.pos.xy;
	}

	// color_mode: 0 = vertex/eye color, 1 = depth gradient, 2 = confidence gradient
	if (color_mode > 1.5) {
		float conf = saturate(confidence.SampleLevel(confidence_s, tex_coord, 0).r * conf_scale);
		o.color = float4(1.0 - conf, conf, 0, (input.color * color).a);
	} else if (color_mode > 0.5) {
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
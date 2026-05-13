#include <stereokit.hlsli>

//--name = app/texture3d
// Raymarches a Texture3D through a unit cube. Used by Test3DTex to
// validate StereoKit's 3D texture support.

//--world_inv = 1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1
float4x4 world_inv;

Texture3D    volume   : register(t0);
SamplerState volume_s : register(s0);

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4 pos       : SV_POSITION;
	float3 local_pos : TEXCOORD0;
	float3 cam_local : TEXCOORD1;
};

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;

	float4 world = mul(input.pos, sk_inst[ids.inst].world);
	o.pos        = mul(world,     sk_viewproj[ids.view]);

	o.local_pos  = input.pos.xyz;
	o.cam_local  = mul(float4(sk_camera_pos[ids.view].xyz, 1), world_inv).xyz;
	return o;
}

// Ray-AABB slab test. Returns (tmin, tmax) along the ray.
float2 ray_box_intersect(float3 ray_origin, float3 ray_dir, float3 box_min, float3 box_max) {
	float3 inv_dir = 1.0 / ray_dir;
	float3 t0      = (box_min - ray_origin) * inv_dir;
	float3 t1      = (box_max - ray_origin) * inv_dir;
	float3 tmin3   = min(t0, t1);
	float3 tmax3   = max(t0, t1);
	float  tmin    = max(max(tmin3.x, tmin3.y), tmin3.z);
	float  tmax    = min(min(tmax3.x, tmax3.y), tmax3.z);
	return float2(tmin, tmax);
}

float4 ps(psIn input) : SV_TARGET {
	// Mesh.Cube is a unit cube spanning ±0.5 in local space.
	float3 ray_dir = normalize(input.local_pos - input.cam_local);
	float2 t       = ray_box_intersect(input.cam_local, ray_dir, float3(-0.5,-0.5,-0.5), float3(0.5,0.5,0.5));

	float t_start = max(t.x, 0.0);
	float t_end   = t.y;
	if (t_end <= t_start) discard;

	const int   STEPS     = 64;
	const float step_size = (t_end - t_start) / float(STEPS);

	float4 accum   = float4(0,0,0,0);
	float3 ray_pos = input.cam_local + ray_dir * t_start;

	for (int i = 0; i < STEPS && accum.a < 0.95; i++) {
		float3 uvw          = ray_pos + 0.5;
		float4 sample_color = volume.SampleLevel(volume_s, uvw, 0);

		float  alpha = sample_color.a * step_size * 4.0;
		accum.rgb   += (1.0 - accum.a) * alpha * sample_color.rgb;
		accum.a     += (1.0 - accum.a) * alpha;

		ray_pos += ray_dir * step_size;
	}

	if (accum.a < 0.01) discard;
	return float4(accum.rgb, accum.a);
}

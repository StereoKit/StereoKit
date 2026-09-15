#ifndef _STEREOKIT_HLSLI
#define _STEREOKIT_HLSLI

// Maximum number of views StereoKit can render in a single pass. Must
// match SK_MAX_VIEWS in renderer.cpp.
#define SK_MAX_VIEWS 6

///////////////////////////////////////////

cbuffer stereokit_buffer : register(b1) {
	float4x4 sk_view       [SK_MAX_VIEWS];
	float4x4 sk_proj       [SK_MAX_VIEWS];
	float4x4 sk_proj_inv   [SK_MAX_VIEWS];
	float4x4 sk_viewproj   [SK_MAX_VIEWS];
	float4   sk_lighting_sh[7];
	float4   sk_light_dir_to; // Main light: .xyz = normalized direction toward it
	float4   sk_light_color;  // .rgb = linear color, .w = 1 when a light exists
	float4   sk_camera_pos [SK_MAX_VIEWS];
	float4   sk_camera_dir [SK_MAX_VIEWS];
	float4   sk_fingertip  [2];
	float4   sk_cubemap_i;   // Reflection: .xy = size, .z = last mip index, .w = footprint mip constant
	float4   sk_screen_size; // .xy = width/height, .zw = 1/width, 1/height
	float    sk_time;
	uint     sk_view_count;
	uint     sk_eye_offset;
};
struct inst_t {
	float4x4 world;
	float4   color;
};
StructuredBuffer<inst_t> sk_inst : register(t12);
TextureCube  sk_cubemap   : register(t11);
SamplerState sk_cubemap_s : register(s11);

///////////////////////////////////////////

// L2 spherical harmonics lighting lookup, packed into dot-product
// form for efficient evaluation. Coefficients are pre-baked by
// 'sh_to_fast' in StereoKitC into 7 float4s:
//   [0..2] = band 0+1 per channel (R,G,B)
//   [3..5] = band 2   per channel (R,G,B)
//   [6]    = last band 2 term (.rgb)
float3 sk_lighting(float3 normal) {
	float4 vA = float4(normal, 1);
	float4 vB = normal.xyzz * normal.yzzx;
	float  vC = normal.x * normal.x - normal.y * normal.y;
	return float3(
		dot(sk_lighting_sh[0], vA) + dot(sk_lighting_sh[3], vB) + sk_lighting_sh[6].x * vC,
		dot(sk_lighting_sh[1], vA) + dot(sk_lighting_sh[4], vB) + sk_lighting_sh[6].y * vC,
		dot(sk_lighting_sh[2], vA) + dot(sk_lighting_sh[5], vB) + sk_lighting_sh[6].z * vC);
}
// Legacy name, use sk_lighting
float3 Lighting(float3 normal) { return sk_lighting(normal); }

///////////////////////////////////////////

struct finger_dist_t {
	float from_finger;
	float on_plane;
};

finger_dist_t sk_finger_distance_info(float3 world_pos, float3 world_norm) {
	float3 to_0 = sk_fingertip[0].xyz - world_pos;
	float3 to_1 = sk_fingertip[1].xyz - world_pos;
	float  sq_0 = dot(to_0, to_0);
	float  sq_1 = dot(to_1, to_1);
	float  d_0  = dot(world_norm, to_0);
	float  d_1  = dot(world_norm, to_1);

	// The tangential leg is |to_finger|^2 - d^2, so both distances fall out
	// of dots we already need, and only the winners get a sqrt.
	finger_dist_t result;
	result.from_finger = sqrt(min(sq_0, sq_1)) * sign(sq_0 < sq_1 ? d_0 : d_1);
	result.on_plane    = sqrt(max(min(sq_0 - d_0*d_0, sq_1 - d_1*d_1), 0));
	return result;
}

///////////////////////////////////////////

float sk_finger_distance_sq(float3 world_pos) {
	float3 d0 = sk_fingertip[0].xyz - world_pos;
	float3 d1 = sk_fingertip[1].xyz - world_pos;
	return min(dot(d0, d0), dot(d1, d1));
}

///////////////////////////////////////////

float sk_finger_distance(float3 world_pos) {
	return sqrt(sk_finger_distance_sq(world_pos));
}

///////////////////////////////////////////

min16float sk_finger_glow(float3 world_pos) {
	min16float d_sq = sk_finger_distance_sq(world_pos);
	// The reciprocal can't exceed 1, so saturate covers the low clamp and
	// rides along as a free output modifier.
	return saturate(1/(1+10000*d_sq)-0.0069h);
}

///////////////////////////////////////////

float sk_aspect_ratio(uint view_id) {
	// Load the full matrix first to avoid deep ViewIndex access chains
	// in SPIR-V. Adreno's multiview linker fails when ViewIndex is a
	// non-terminal index in an OpAccessChain.
	float4x4 proj = sk_proj[view_id];
	return proj._m11 / proj._m00;
}

///////////////////////////////////////////

// System vertex IDs. Multiview provides SV_ViewID via the driver,
// SV_InstanceID is the real instance index (no view packing).
struct sk_ids_t {
	uint inst : SV_InstanceID;
	uint view : SV_ViewID;
};

///////////////////////////////////////////

#endif
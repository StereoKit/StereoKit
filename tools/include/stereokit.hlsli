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
	float4   sk_camera_pos [SK_MAX_VIEWS];
	float4   sk_camera_dir [SK_MAX_VIEWS];
	float4   sk_fingertip  [2];
	float4   sk_cubemap_i;   // .xy = width/height, .z  = mip count, .w = unused
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
	finger_dist_t result;
	result.from_finger = 10000;
	result.on_plane    = 10000;
	
	for	(int i=0;i<2;i++) {
		float3 to_finger = sk_fingertip[i].xyz - world_pos;
		float  d         = dot(world_norm, to_finger);
		float3 on_plane  = sk_fingertip[i].xyz - d*world_norm;

		// Also make distances behind the plane negative
		float finger_dist = length(to_finger);
		if (abs(result.from_finger) > finger_dist)
			result.from_finger = finger_dist * sign(d);
		
		result.on_plane = min(result.on_plane, length(world_pos - on_plane));
	}

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
	return max(0, 1/(1+10000*d_sq)-0.0069h);
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

// System vertex IDs — multiview provides SV_ViewID via the driver,
// SV_InstanceID is the real instance index (no view packing).
struct sk_ids_t {
	uint inst : SV_InstanceID;
	uint view : SV_ViewID;
};

///////////////////////////////////////////

#endif
#ifndef _STEREOKIT_HLSLI
#define _STEREOKIT_HLSLI

///////////////////////////////////////////

cbuffer stereokit_buffer : register(b1) {
	float4x4 sk_view       [2];
	float4x4 sk_proj       [2];
	float4x4 sk_proj_inv   [2];
	float4x4 sk_viewproj   [2];
	float4   sk_lighting_sh[9];
	float4   sk_camera_pos [2];
	float4   sk_camera_dir [2];
	float4   sk_fingertip  [2];
	float4   sk_cubemap_i;
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

// A spherical harmonics lighting lookup!
// Some calculations have been offloaded to 'sh_to_fast'
// in StereoKitC
float3 sk_lighting(float3 normal) {
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
// Legacy name, use sk_finger_distance_info
finger_dist_t FingerDistanceInfo(float3 world_pos, float3 world_norm) { return sk_finger_distance_info(world_pos, world_norm); }

///////////////////////////////////////////

float2 sk_finger_glow_ex(float3 world_pos, float3 world_norm) {
	float dist = 1;
	float ring = 0;
	for (int i = 0; i < 2; i++) {
		float3 to_finger = sk_fingertip[i].xyz - world_pos;
		float  d         = dot(world_norm, to_finger);
		float3 on_plane  = sk_fingertip[i].xyz - d * world_norm;

		float dist_from_finger = length(to_finger);
		float dist_on_plane    = length(world_pos - on_plane);
		ring = max(ring, saturate(1 - abs(d * 0.5 - dist_on_plane) * 600));
		dist = min(dist, dist_from_finger);
	}

	return float2(dist, ring);
}
// Legacy name, use sk_finger_glow_ex
float2 FingerGlowEx(float3 world_pos, float3 world_norm) { return sk_finger_glow_ex(world_pos, world_norm); }

///////////////////////////////////////////

float sk_finger_distance(float3 world_pos) {
	return min(
		length(sk_fingertip[0].xyz - world_pos),
		length(sk_fingertip[1].xyz - world_pos));
}

///////////////////////////////////////////

float sk_finger_glow(float3 world_pos, float3 world_norm) {
	float2 glow = sk_finger_glow_ex(world_pos, world_norm);
	glow.x = pow(saturate(1 - glow.x / 0.12), 2);
	return (glow.x * 0.2) + (glow.y * glow.x);
}
// Legacy name, use sk_finger_glow
float FingerGlow(float3 world_pos, float3 world_norm) { return sk_finger_glow(world_pos, world_norm); }

///////////////////////////////////////////

float sk_aspect_ratio(uint view_id) {
	return sk_proj[view_id]._m11 / sk_proj[view_id]._m00;
}

///////////////////////////////////////////

#endif
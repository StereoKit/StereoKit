//--name = sk/sh_compute
// Computes spherical harmonics coefficients from a cubemap texture.
// Designed for small mip levels (typically 16x16 to 32x32 per face).
// Dispatched as (1, 1, 1) - single workgroup processes all faces.

TextureCube<float4> source   : register(t0);
SamplerState        source_s : register(s0);

// Output: 9 RGB coefficients = 27 floats
// Layout: [coef0.rgb, coef1.rgb, ..., coef8.rgb]
RWStructuredBuffer<float> sh_output : register(u0);

// Cubemap face size and mip level
uint face_size;
uint mip_level;

// SH basis function coefficients (L2 spherical harmonics)
static const float SH_C0   = 0.282094791773878140f;
static const float SH_C1   = 0.488602511902919920f;
static const float SH_C2_0 = 0.946174695757560080f;
static const float SH_C2_1 = 0.315391565252520050f;
static const float SH_C2_2 = 1.092548430592079200f;
static const float SH_C2_3 = 0.546274215296039590f;

// CosWtInt normalization factor
static const float PI = 3.14159265358979323846f;
static const float fRet = PI / (0.25f + 0.5f);

// Windowing factor to prevent SH ringing
static const float WINDOW_WIDTH = 0.01f;

// Convert UV to cubemap direction for a face
float3 uv_to_direction(float2 uv, uint face) {
	float2 ndc = uv * 2.0 - 1.0;
	float3 dir;
	if      (face == 0) { dir = float3( 1.0, -ndc.y, -ndc.x); }
	else if (face == 1) { dir = float3(-1.0, -ndc.y,  ndc.x); }
	else if (face == 2) { dir = float3( ndc.x,  1.0,  ndc.y); }
	else if (face == 3) { dir = float3( ndc.x, -1.0, -ndc.y); }
	else if (face == 4) { dir = float3( ndc.x, -ndc.y,  1.0); }
	else                { dir = float3(-ndc.x, -ndc.y, -1.0); }
	return normalize(dir);
}

// Add a sample's SH contribution
void sh_add(inout float3 coeffs[9], float3 dir, float3 color) {
	float3 light_dir = float3(-dir.x, -dir.y, dir.z);
	color = color * fRet;

	float z2 = light_dir.z * light_dir.z;
	float s1 = light_dir.y;
	float c1 = light_dir.x;
	float s2 = light_dir.x * s1 + light_dir.y * c1;
	float c2 = light_dir.x * c1 - light_dir.y * s1;
	float p_2_1 = -SH_C2_2 * light_dir.z;

	coeffs[0] += color * SH_C0;
	coeffs[1] += color * (-SH_C1 * s1);
	coeffs[2] += color * ( SH_C1 * light_dir.z);
	coeffs[3] += color * (-SH_C1 * c1);
	coeffs[4] += color * (SH_C2_3 * s2);
	coeffs[5] += color * (p_2_1 * s1);
	coeffs[6] += color * (SH_C2_0 * z2 - SH_C2_1);
	coeffs[7] += color * (p_2_1 * c1);
	coeffs[8] += color * (SH_C2_3 * c2);
}

// Apply windowing to prevent ringing
void sh_windowing(inout float3 coeffs[9]) {
	uint idx = 0;
	for (int band = 0; band <= 2; band++) {
		float s = 1.0f / (1.0f + WINDOW_WIDTH * band * band * (band + 1.0f) * (band + 1.0f));
		for (int m = -band; m <= band; m++) {
			coeffs[idx++] *= s;
		}
	}
}

// Shared memory for reduction (9 coefficients * 64 threads)
groupshared float3 sh_shared[9][64];

[numthreads(64, 1, 1)]
void cs(uint local_idx : SV_GroupIndex) {
	// Each thread processes multiple pixels across all faces
	// Total pixels = face_size * face_size * 6
	uint total_pixels = face_size * face_size * 6;
	uint pixels_per_thread = (total_pixels + 63) / 64;

	float3 local_coeffs[9];
	for (uint i = 0; i < 9; i++) {
		local_coeffs[i] = float3(0, 0, 0);
	}

	float half_px = 0.5f / (float)face_size;

	// Process assigned pixels
	for (uint p = 0; p < pixels_per_thread; p++) {
		uint pixel_idx = local_idx * pixels_per_thread + p;
		if (pixel_idx >= total_pixels) break;

		uint face = pixel_idx / (face_size * face_size);
		uint within_face = pixel_idx % (face_size * face_size);
		uint y = within_face / face_size;
		uint x = within_face % face_size;

		float u = (float)x / (float)face_size + half_px;
		float v = (float)y / (float)face_size + half_px;

		float3 dir = uv_to_direction(float2(u, v), face);
		float3 color = source.SampleLevel(source_s, dir, mip_level).rgb;

		sh_add(local_coeffs, dir, color);
	}

	// Store to shared memory
	for (uint i = 0; i < 9; i++) {
		sh_shared[i][local_idx] = local_coeffs[i];
	}
	GroupMemoryBarrierWithGroupSync();

	// Parallel reduction
	for (uint stride = 32; stride > 0; stride >>= 1) {
		if (local_idx < stride) {
			for (uint i = 0; i < 9; i++) {
				sh_shared[i][local_idx] += sh_shared[i][local_idx + stride];
			}
		}
		GroupMemoryBarrierWithGroupSync();
	}

	// Thread 0 writes final result
	if (local_idx == 0) {
		float3 result[9];
		float count = (float)total_pixels;

		// Copy and normalize
		for (uint i = 0; i < 9; i++) {
			result[i] = sh_shared[i][0] / count;
		}

		// Apply windowing
		sh_windowing(result);

		// Write output
		for (uint i = 0; i < 9; i++) {
			sh_output[i * 3 + 0] = result[i].x;
			sh_output[i * 3 + 1] = result[i].y;
			sh_output[i * 3 + 2] = result[i].z;
		}
	}
}

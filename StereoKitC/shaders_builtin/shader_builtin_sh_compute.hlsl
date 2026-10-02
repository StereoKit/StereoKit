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

// Projection normalization: uniform radiance-1 environment reads exactly 1
// from sh_lookup. Must match SH_PROJECT_NORM in spherical_harmonics.cpp.
static const float PI   = 3.14159265358979323846f;
static const float fRet = 4.0f;

// Irradiance convolution kernel, matching sh_lookup's CosineA constants.
static const float COS_A0 = PI;
static const float COS_A1 = (2.0f * PI) / 3.0f;
static const float COS_A2 = PI * 0.25f;

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

// Attenuate higher bands to suppress ringing
void sh_windowing(inout float3 coeffs[9], float width) {
	uint idx = 0;
	for (int band = 0; band <= 2; band++) {
		float s = 1.0f / (1.0f + width * band * band * (band + 1.0f) * (band + 1.0f));
		for (int m = -band; m <= band; m++) {
			coeffs[idx++] *= s;
		}
	}
}

// Minimum of the irradiance reconstruction over 26 probe directions,
// mirroring sh_lookup in spherical_harmonics.cpp.
float sh_min_lookup(float3 c[9]) {
	float lowest = 1e30;
	for (int i = 0; i < 27; i++) {
		int3 v = int3(i % 3, (i / 3) % 3, i / 9) - 1;
		if (all(v == 0)) continue;
		float3 n   = normalize(float3(v));
		float3 val = c[0] * (0.282095f * COS_A0)
			+ (c[1] * n.y + c[2] * n.z + c[3] * n.x)                    * (0.488603f * COS_A1)
			+ (c[4] * n.x * n.y + c[5] * n.y * n.z + c[7] * n.x * n.z) * (1.092548f * COS_A2)
			+  c[6] * (0.315392f * (3.0f * n.z * n.z - 1.0f) * COS_A2)
			+  c[8] * (0.546274f * (n.x * n.x - n.y * n.y)   * COS_A2);
		lowest = min(lowest, min(val.r, min(val.g, val.b)));
	}
	return lowest;
}

// Dering adaptively: widen the window only until the irradiance
// reconstruction stops going negative, so well-behaved environments keep
// their full directionality. Window scale is monotonic, bisection is valid.
void sh_window_fit(inout float3 coeffs[9]) {
	if (sh_min_lookup(coeffs) >= 0) return;
	float lo = 0, hi = 4.0f;
	for (int i = 0; i < 10; i++) {
		float  mid = (lo + hi) * 0.5f;
		float3 test[9];
		for (uint j = 0; j < 9; j++) test[j] = coeffs[j];
		sh_windowing(test, mid);
		if (sh_min_lookup(test) >= 0) hi = mid;
		else                          lo = mid;
	}
	sh_windowing(coeffs, hi);
}

// Shared memory for reduction (9 coefficients + weight sum, 64 threads)
groupshared float3 sh_shared[9][64];
groupshared float  w_shared[64];

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
	float local_weight = 0;

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

		// Cube texels shrink in solid angle toward face corners; without this
		// weight, corner directions count up to (sqrt(3))^3 = 5.2x too much.
		float2 ndc    = float2(u, v) * 2.0 - 1.0;
		float  weight = pow(1.0 + dot(ndc, ndc), -1.5);

		float3 dir = uv_to_direction(float2(u, v), face);
		float3 color = source.SampleLevel(source_s, dir, mip_level).rgb;

		sh_add(local_coeffs, dir, color * weight);
		local_weight += weight;
	}

	// Store to shared memory
	for (uint i = 0; i < 9; i++) {
		sh_shared[i][local_idx] = local_coeffs[i];
	}
	w_shared[local_idx] = local_weight;
	GroupMemoryBarrierWithGroupSync();

	// Parallel reduction
	for (uint stride = 32; stride > 0; stride >>= 1) {
		if (local_idx < stride) {
			for (uint i = 0; i < 9; i++) {
				sh_shared[i][local_idx] += sh_shared[i][local_idx + stride];
			}
			w_shared[local_idx] += w_shared[local_idx + stride];
		}
		GroupMemoryBarrierWithGroupSync();
	}

	// Thread 0 writes final result
	if (local_idx == 0) {
		float3 result[9];

		// Copy and normalize by total solid-angle weight
		for (uint i = 0; i < 9; i++) {
			result[i] = sh_shared[i][0] / w_shared[0];
		}

		// Apply windowing
		sh_window_fit(result);

		// Write output
		for (uint i = 0; i < 9; i++) {
			sh_output[i * 3 + 0] = result[i].x;
			sh_output[i * 3 + 1] = result[i].y;
			sh_output[i * 3 + 2] = result[i].z;
		}
	}
}

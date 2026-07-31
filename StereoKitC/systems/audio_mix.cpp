#include "audio.h"
#include "input.h"
#include "../asset_types/sound.h"
#include "../sk_memory.h"
#include "../sk_math.h"
#include "../sk_math_dx.h"
#include "../libraries/atomic_util.h"

#include <string.h>

using namespace DirectX;

namespace sk {

///////////////////////////////////////////

// Play commands only: stops travel through the voice's atomic stop_request
// param instead, which can't overflow or fail.
struct au_cmd_t {
	int16_t  slot;
	uint16_t id;
};

// Pushed by the audio thread as voices finish, drained on main where freeing
// is safe. A -1 slot is steal cleanup, freeing without touching voice state.
struct au_ret_t {
	sound_t     sound;
	ma_decoder* decoder;
	ma_pcm_rb*  rb;
	float*      ring_data;
	int16_t     slot;
};

#define AU_RET_RING_SIZE 512

au_voice_t au_voices[AU_VOICE_COUNT] = {};
bool32_t   au_offline = false;

// SPSC rings. Head/tail are free-running counters, masked on use. All
// arithmetic is done in uint32_t to keep wraparound defined.
static au_cmd_t au_cmd_ring[AU_CMD_RING_SIZE] = {};
static int32_t  au_cmd_head = 0; // Main writes
static int32_t  au_cmd_tail = 0; // Audio writes
static au_ret_t au_ret_ring[AU_RET_RING_SIZE] = {};
static int32_t  au_ret_head = 0; // Audio writes
static int32_t  au_ret_tail = 0; // Main writes

// Listener pose, published field-by-field through the atomics. A read may mix
// two consecutive frames' components, but individual fields never tear.
static pose_t au_listener_pose = {{0,0,0}, {0,0,0,1}};

static uint64_t au_mix_temp_size = 0;
static float*   au_mix_temp      = nullptr;
static bool     au_mix_truncate_warned = false;

// Pitch resampling reads sources at up to AU_PITCH_MAX rate.
static uint64_t au_resample_temp_size = 0;
static float*   au_resample_temp      = nullptr;

// The first order ambisonic bus, interleaved ambiX ACN order - one W, Y, Z, X
// float4 per frame, so encode and decode are single SIMD ops per sample. It's
// a power-of-two ring with room past the current block, so early reflections
// can write *ahead* of the read head without per-voice memory; an untouched,
// rung-down bus skips the decode entirely.
static float*   au_foa         = nullptr; // Decode input scratch, linear
static float*   au_foa_ring    = nullptr;
static uint32_t au_foa_mask    = 0;     // Ring length in frames, minus one
static uint32_t au_foa_head    = 0;     // Read head, audio thread
static int32_t  au_foa_future  = 0;     // Frames written ahead of head
static bool     au_foa_touched = false; // Any encode this block, audio thread
static int32_t  au_foa_tail    = 0;     // Decode ring-down frames left

///////////////////////////////////////////
// Binaural decode: 6 virtual speakers on an octahedron - hard left/right,
// up/down, front/back. The symmetric layout makes the coherent sum exactly
// 1.0 in every direction, so calibration passes straight through, and the
// lateral pair carries the full interaural delay. The 12 ear paths (lane
// p = speaker*2 + ear) run structure-of-arrays in three SIMD lane groups,
// so the filter bank steps 4 paths per op; only the fractional ITD delays
// stay scalar, they need per-lane buffer indexing.

#define AU_SPEAKER_COUNT 6
#define AU_PATH_COUNT    (AU_SPEAKER_COUNT * 2)
#define AU_PATH_GROUPS   (AU_PATH_COUNT / 4)
#define AU_DECODE_STAGES AU_VOICING_STAGES
#define AU_EAR_DELAY_MAX 64   // Pow2 for masked wrap

// Far-ear high band lands at 20*log10(1-wet) dB, scaled by lateralness in
// use. Real heads shadow 15-20dB up top; 0.87 lands at ~-18dB.
#define AU_SHADOW_WET    0.87f

// Spherical head radius, meters - shared by the interaural delay, the
// coherence crossover, and the near-field ear distances.
#define AU_HEAD_RADIUS   0.0875f

// Shoulder reflectors ~0.17m below the ears: an elevation-tracked comb up to
// ~1ms - the low frequency up/down cue, the pinna notches live above 5kHz.
#define AU_SHOULDER_M    0.17f
#define AU_SHOULDER_GAIN 0.35f

// Schroeder allpass delays per [ear][section]: distinct primes so the ears'
// phase responses never line up, sums matched so the image stays centered.
#define AU_DECO_SECTIONS 2
#define AU_DECO_MAX      256
#define AU_DECO_GAIN     0.5f
static const int32_t au_deco_len[2][AU_DECO_SECTIONS] = {{89, 199}, {107, 181}};

struct au_biquad_t {
	float b0, b1, b2, a1, a2;
};

struct au_decode_t {
	vec3     speaker_dir[AU_SPEAKER_COUNT]; // Head-relative
	// Biquad coefficients and filter states, [stage][lane group]
	XMVECTOR b0[AU_DECODE_STAGES][AU_PATH_GROUPS], b1[AU_DECODE_STAGES][AU_PATH_GROUPS];
	XMVECTOR b2[AU_DECODE_STAGES][AU_PATH_GROUPS], a1[AU_DECODE_STAGES][AU_PATH_GROUPS];
	XMVECTOR a2[AU_DECODE_STAGES][AU_PATH_GROUPS];
	XMVECTOR x1[AU_DECODE_STAGES][AU_PATH_GROUPS], x2[AU_DECODE_STAGES][AU_PATH_GROUPS];
	XMVECTOR y1[AU_DECODE_STAGES][AU_PATH_GROUPS], y2[AU_DECODE_STAGES][AU_PATH_GROUPS];
	// Head shadow one-pole at ~1.5kHz, mixed in by lateralness, 0 = bypass
	XMVECTOR shadow_wet  [AU_PATH_GROUPS];
	XMVECTOR shadow_state[AU_PATH_GROUPS];
	// Brown-Duda ITD as a fractional delay per lane, 0 = bypass
	float    delay    [AU_PATH_COUNT];
	float    delay_buf[AU_PATH_COUNT][AU_EAR_DELAY_MAX];
	int32_t  delay_at;
	// Dual-band directional weighting: one-pole low-band state of the bus
	// channels, split at ~700Hz where ITD hands off to energy cues.
	XMVECTOR foa_lp;
	float    xover_k;
	// Diffuse-field coherence: above the crossover each ear runs its own
	// allpass chain, wet share riding psi so plane waves stay coherent.
	float    psi;            // Smoothed diffuseness, 0 = plane wave, 1 = diffuse
	float    deco_k;         // Crossover one-pole coefficient, ~980Hz
	float    deco_lp [2];    // Crossover state per ear
	float    deco_buf[2][AU_DECO_SECTIONS][AU_DECO_MAX];
	int32_t  deco_at [2][AU_DECO_SECTIONS];
};
static au_decode_t au_decode = {};

// RBJ cookbook peaking EQ, gain_db < 0 makes a dip.
static au_biquad_t au_biquad_peaking(float hz, float q, float gain_db) {
	float A     = powf(10.0f, gain_db / 40.0f);
	float w0    = 6.2831853f * hz / AU_SAMPLE_RATE;
	float alpha = sinf(w0) / (2.0f * q);
	float a0    = 1 + alpha / A;
	au_biquad_t r = {};
	r.b0 = (1 + alpha * A) / a0;
	r.b1 = (-2 * cosf(w0)) / a0;
	r.b2 = (1 - alpha * A) / a0;
	r.a1 = (-2 * cosf(w0)) / a0;
	r.a2 = (1 - alpha / A) / a0;
	return r;
}

// RBJ cookbook high shelf, S = 1.
static au_biquad_t au_biquad_highshelf(float hz, float gain_db) {
	float A     = powf(10.0f, gain_db / 40.0f);
	float w0    = 6.2831853f * hz / AU_SAMPLE_RATE;
	float cw    = cosf(w0);
	float alpha = sinf(w0) / 2.0f * 1.4142135f;
	float sq    = 2.0f * sqrtf(A) * alpha;
	float a0    = (A + 1) - (A - 1) * cw + sq;
	au_biquad_t r = {};
	r.b0 = ( A * ((A + 1) + (A - 1) * cw + sq)) / a0;
	r.b1 = (-2 * A * ((A - 1) + (A + 1) * cw)) / a0;
	r.b2 = ( A * ((A + 1) + (A - 1) * cw - sq)) / a0;
	r.a1 = ( 2 *     ((A - 1) - (A + 1) * cw)) / a0;
	r.a2 = (     (A + 1) - (A - 1) * cw - sq ) / a0;
	return r;
}

// Direction voicing shared by the direct path and the decode's speakers,
// AU_VOICING_STAGES biquads; dir is head-relative unit length. Elevation is
// Iida-style N1/N2 pinna acoustics: notches rise and fade as the source
// rises - deeply notched below, smooth and bright overhead - judged against
// P1, the elevation-independent concha peak.
static void au_voicing(vec3 dir, au_biquad_t out[AU_VOICING_STAGES]) {
	float el01  = dir.y * 0.5f + 0.5f;
	float n1_hz = math_lerp(4800, 11000, el01);
	float n_db  = math_lerp(-16, -1.5f, el01);
	out[0] = au_biquad_peaking(n1_hz,         7, n_db);
	out[1] = au_biquad_peaking(n1_hz * 1.45f, 8, n_db * 0.7f);
	out[2] = au_biquad_peaking(4200, 1.5f, 3);
	// Below the horizon leans deliberately dark, steeper than physical -
	// listeners compress elevation toward the horizon, so the gradient is
	// exaggerated. Behind loses sparkle; a zero-gain shelf is exact identity.
	float dull = -10.0f * fmaxf(0, -dir.y) - 3.0f * fmaxf(0, dir.z);
	out[3] = au_biquad_highshelf(5000, dull);
}

// Brown-Duda spherical head per-ear delay in samples. cos_e is the cosine of
// the source direction against that ear's axis; only the difference between
// ears is audible, the common offset just keeps delays causal.
static inline float au_ear_delay(float cos_e) {
	const float r = (AU_HEAD_RADIUS / 343.0f) * AU_SAMPLE_RATE;
	if (cos_e >= 0) return r * (1.0f - cos_e);
	return r * (1.0f + acosf(fmaxf(-1.0f, cos_e)) - 1.5707963f);
}

static void au_speakers_init() {
	memset(&au_decode, 0, sizeof(au_decode));

	// Coherence crossover at c/(2 * ear spacing) - it falls out of the
	// head's size, the same radius the ITD model uses.
	float fc = 343.0f / (4.0f * AU_HEAD_RADIUS);
	float RC = 1.0f / (6.2831853f * fc);
	float dt = 1.0f / AU_SAMPLE_RATE;
	au_decode.deco_k = dt / (RC + dt);

	// Dual-band decode crossover, ~700Hz: the frequency below which the
	// ear localizes by interaural time rather than level/energy.
	float xRC = 1.0f / (6.2831853f * 700.0f);
	au_decode.xover_k = dt / (xRC + dt);

	// Default until the first block measures: silence is diffuse, so
	// ring-downs and fresh starts stay wide.
	au_decode.psi = 1;

	// Coefficients build per-lane in scalar staging, packed at the end.
	float b0[AU_DECODE_STAGES][AU_PATH_COUNT], b1[AU_DECODE_STAGES][AU_PATH_COUNT];
	float b2[AU_DECODE_STAGES][AU_PATH_COUNT], a1[AU_DECODE_STAGES][AU_PATH_COUNT];
	float a2[AU_DECODE_STAGES][AU_PATH_COUNT];
	float shadow[AU_PATH_COUNT] = {};
	for (int32_t s = 0; s < AU_DECODE_STAGES; s++) {
		for (int32_t p = 0; p < AU_PATH_COUNT; p++) {
			b0[s][p] = 1; // Identity biquad for unused stages
			b1[s][p] = 0; b2[s][p] = 0; a1[s][p] = 0; a2[s][p] = 0;
		}
	}

	// Octahedron: the lateral pair carries the full interaural delay and
	// shadow, the other axes anchor the voicing. SK frame, -Z forward.
	static const vec3 dirs[AU_SPEAKER_COUNT] = {
		{ 0, 0,-1}, {-1, 0, 0}, { 1, 0, 0},
		{ 0, 0, 1}, { 0, 1, 0}, { 0,-1, 0} };
	for (int32_t s = 0; s < AU_SPEAKER_COUNT; s++) {
		vec3 dir = dirs[s];
		au_decode.speaker_dir[s] = dir;

		// The same Brown-Duda ear model the direct path uses, near ear
		// normalized to zero. Lane p = speaker*2 + ear.
		float dl   = au_ear_delay(-dir.x);
		float dr   = au_ear_delay( dir.x);
		float base = fminf(dl, dr);
		au_decode.delay[s*2 + 0] = dl - base;
		au_decode.delay[s*2 + 1] = dr - base;
		float lateral = fabsf(dir.x);
		if (lateral > 0)
			shadow[s*2 + (dir.x > 0 ? 0 : 1)] = AU_SHADOW_WET * lateral;

		// The direct path's per-source voicing at the speaker directions, so
		// the render paths stay matched where the spread crossfade hands off.
		au_biquad_t vc[AU_VOICING_STAGES];
		au_voicing(dir, vc);
		for (int32_t ear = 0; ear < 2; ear++) {
			int32_t lane = s*2 + ear;
			for (int32_t f = 0; f < AU_VOICING_STAGES; f++) {
				b0[f][lane] = vc[f].b0; b1[f][lane] = vc[f].b1; b2[f][lane] = vc[f].b2;
				a1[f][lane] = vc[f].a1; a2[f][lane] = vc[f].a2;
			}
		}
	}

	for (int32_t g = 0; g < AU_PATH_GROUPS; g++) {
		for (int32_t s = 0; s < AU_DECODE_STAGES; s++) {
			au_decode.b0[s][g] = XMVectorSet(b0[s][g*4], b0[s][g*4+1], b0[s][g*4+2], b0[s][g*4+3]);
			au_decode.b1[s][g] = XMVectorSet(b1[s][g*4], b1[s][g*4+1], b1[s][g*4+2], b1[s][g*4+3]);
			au_decode.b2[s][g] = XMVectorSet(b2[s][g*4], b2[s][g*4+1], b2[s][g*4+2], b2[s][g*4+3]);
			au_decode.a1[s][g] = XMVectorSet(a1[s][g*4], a1[s][g*4+1], a1[s][g*4+2], a1[s][g*4+3]);
			au_decode.a2[s][g] = XMVectorSet(a2[s][g*4], a2[s][g*4+1], a2[s][g*4+2], a2[s][g*4+3]);
		}
		au_decode.shadow_wet[g] = XMVectorSet(shadow[g*4], shadow[g*4+1], shadow[g*4+2], shadow[g*4+3]);
	}
}

// Decodes the FOA bus additively into interleaved stereo. Rotation applies to
// the pickup directions rather than the field - correct under head roll too.
static void audio_decode_foa(float* output, ma_uint32 frame_count, pose_t head) {
	// Pickup gains for the rotated speakers, two per lane group with ears
	// adjacent. Dual-band: velocity-matched (t=3) lows keep the superposed
	// ITD physical, max-rE (t=sqrt3) highs focus energy; W weighs 1 in both
	// bands so the coherent-sum calibration holds. Ambisonic +X is front.
	float gw = (2.0f / AU_SPEAKER_COUNT) * 0.5f;
	float gys[AU_SPEAKER_COUNT], gzs[AU_SPEAKER_COUNT], gxs[AU_SPEAKER_COUNT];
	for (int32_t s = 0; s < AU_SPEAKER_COUNT; s++) {
		vec3 u = head.orientation * au_decode.speaker_dir[s];
		gxs[s] = gw * -u.z;
		gys[s] = gw * -u.x;
		gzs[s] = gw *  u.y;
	}
	XMVECTOR gwv = XMVectorReplicate(gw);
	XMVECTOR gy[AU_PATH_GROUPS], gz[AU_PATH_GROUPS], gx[AU_PATH_GROUPS];
	for (int32_t g = 0; g < AU_PATH_GROUPS; g++) {
		int32_t a = g*2, b = g*2 + 1;
		gy[g] = XMVectorSet(gys[a], gys[a], gys[b], gys[b]);
		gz[g] = XMVectorSet(gzs[a], gzs[a], gzs[b], gzs[b]);
		gx[g] = XMVectorSet(gxs[a], gxs[a], gxs[b], gxs[b]);
	}
	XMVECTOR earl   = XMVectorSet(1, 0, 1, 0);
	XMVECTOR earr   = XMVectorSet(0, 1, 0, 1);
	XMVECTOR sh_k   = XMVectorReplicate(0.178f);
	XMVECTOR xk     = XMVectorReplicate(au_decode.xover_k);
	XMVECTOR t_hi   = XMVectorSet(1, 1.7320508f, 1.7320508f, 1.7320508f);
	XMVECTOR t_del  = XMVectorSet(0, 1.2679492f, 1.2679492f, 1.2679492f); // t_lo - t_hi
	XMVECTOR foa_lp = au_decode.foa_lp;
	XMVECTOR acc_wv = XMVectorZero(), acc_e = XMVectorZero();
	float    psi    = au_decode.psi;

	for (ma_uint32 i = 0; i < frame_count; i++) {
		// Speaker signals as multiply-adds on the broadcast W, Y, Z, X frame;
		// both ears share a signal and diverge in the filters below.
		XMVECTOR foa = XMLoadFloat4((XMFLOAT4*)(au_foa + i*4));
		acc_wv = XMVectorMultiplyAdd(XMVectorSplatX(foa), foa, acc_wv);
		acc_e  = XMVectorMultiplyAdd(foa, foa, acc_e);
		foa_lp = XMVectorMultiplyAdd(xk, XMVectorSubtract(foa, foa_lp), foa_lp);
		foa    = XMVectorMultiplyAdd(t_del, foa_lp, XMVectorMultiply(t_hi, foa));
		XMVECTOR ambw = XMVectorSplatX(foa), amby = XMVectorSplatY(foa);
		XMVECTOR ambz = XMVectorSplatZ(foa), ambx = XMVectorSplatW(foa);
		XMFLOAT4A st[AU_PATH_GROUPS];
		for (int32_t g = 0; g < AU_PATH_GROUPS; g++) {
			XMVECTOR sig = XMVectorMultiply   (gwv,   ambw);
			         sig = XMVectorMultiplyAdd(gy[g], amby, sig);
			         sig = XMVectorMultiplyAdd(gz[g], ambz, sig);
			         sig = XMVectorMultiplyAdd(gx[g], ambx, sig);
			XMStoreFloat4A(&st[g], sig);
		}
		float*  lanes  = (float*)st;
		int32_t at_idx = au_decode.delay_at;
		for (int32_t p = 0; p < AU_PATH_COUNT; p++) {
			float d = au_decode.delay[p];
			if (d <= 0) continue;
			float* buf = au_decode.delay_buf[p];
			buf[at_idx] = lanes[p];
			float   read_at = (float)(at_idx + AU_EAR_DELAY_MAX) - d;
			int32_t i0      = (int32_t)read_at;
			lanes[p] = math_lerp(buf[i0 & (AU_EAR_DELAY_MAX-1)], buf[(i0+1) & (AU_EAR_DELAY_MAX-1)], read_at - (float)i0);
		}
		au_decode.delay_at = (at_idx + 1) & (AU_EAR_DELAY_MAX-1);

		XMVECTOR sum = XMVectorZero();
		for (int32_t g = 0; g < AU_PATH_GROUPS; g++) {
			XMVECTOR x = XMLoadFloat4A(&st[g]);

			// Head shadow one-pole, bypass lanes have wet 0.
			XMVECTOR sh = au_decode.shadow_state[g];
			sh = XMVectorMultiplyAdd(sh_k, XMVectorSubtract(x, sh), sh);
			au_decode.shadow_state[g] = sh;
			x = XMVectorMultiplyAdd(au_decode.shadow_wet[g], XMVectorSubtract(sh, x), x);

			// Two direct-form-I biquad stages, 4 paths per step.
			for (int32_t s = 0; s < AU_DECODE_STAGES; s++) {
				XMVECTOR y = XMVectorMultiply(au_decode.b0[s][g], x);
				y = XMVectorMultiplyAdd             (au_decode.b1[s][g], au_decode.x1[s][g], y);
				y = XMVectorMultiplyAdd             (au_decode.b2[s][g], au_decode.x2[s][g], y);
				y = XMVectorNegativeMultiplySubtract(au_decode.a1[s][g], au_decode.y1[s][g], y);
				y = XMVectorNegativeMultiplySubtract(au_decode.a2[s][g], au_decode.y2[s][g], y);
				au_decode.x2[s][g] = au_decode.x1[s][g]; au_decode.x1[s][g] = x;
				au_decode.y2[s][g] = au_decode.y1[s][g]; au_decode.y1[s][g] = y;
				x = y;
			}
			sum = XMVectorAdd(sum, x);
		}

		// The coherence split: lows pass, highs decorrelate. The allpasses
		// are unity magnitude, so per-ear energy and calibration hold.
		float lr[2] = {
			XMVectorGetX(XMVector4Dot(sum, earl)),
			XMVectorGetX(XMVector4Dot(sum, earr)) };
		for (int32_t e = 0; e < 2; e++) {
			float lp = au_decode.deco_lp[e] + au_decode.deco_k * (lr[e] - au_decode.deco_lp[e]);
			au_decode.deco_lp[e] = lp;
			float hi  = lr[e] - lp;
			float wet = hi;
			for (int32_t a = 0; a < AU_DECO_SECTIONS; a++) {
				float*  buf = au_decode.deco_buf[e][a];
				int32_t at2 = au_decode.deco_at [e][a];
				float   d   = buf[at2];
				float   v   = wet + AU_DECO_GAIN * d;
				wet         = d  - AU_DECO_GAIN * v;
				buf[at2]    = v;
				au_decode.deco_at[e][a] = at2 + 1 >= au_deco_len[e][a] ? 0 : at2 + 1;
			}
			lr[e] = lp + hi + psi * (wet - hi);
		}
		output[i*2  ] += lr[0];
		output[i*2+1] += lr[1];
	}
	au_decode.foa_lp = foa_lp;

	// Block diffuseness: intensity vector length against total energy - plane
	// wave 0, ambience 1, silence keeps the last estimate. Smoothed ~40ms and
	// applied next block; one block of lag is well under audibility.
	XMFLOAT4A wv, en;
	XMStoreFloat4A(&wv, acc_wv);
	XMStoreFloat4A(&en, acc_e);
	float e_total = en.x + en.y + en.z + en.w;
	if (e_total > 1e-9f) {
		float iv  = sqrtf(wv.y*wv.y + wv.z*wv.z + wv.w*wv.w);
		float dif = fmaxf(0.0f, fminf(1.0f, 1.0f - 2.0f * iv / e_total));
		float k   = 1.0f - expf(-(float)frame_count / (0.04f * AU_SAMPLE_RATE));
		au_decode.psi += k * (dif - au_decode.psi);
	}
}

///////////////////////////////////////////
// Environment reverb: spatial voices feed a mono send whose gain ignores
// distance - reverberant energy in a real space barely depends on where the
// source sits - so the direct/reverb balance carries absolute distance for
// free. One shared FDN turns the send into a tail on the bus's W channel.
// Parameters are perceptual, and wet 0 - the default - runs nothing at all.

#define AU_ENV_LINES    8
#define AU_ENV_MAX_LINE 12480 // Longest line at the 40m size clamp, frames
#define AU_ENV_APS      4     // Series input diffusion allpasses
#define AU_ENV_AP_MAX   611   // Longest allpass delay, frames
#define AU_ER_DELAY_MAX 4096  // Reflection write-ahead cap, frames, ~85ms
#define AU_ER_LEVEL     0.5f  // Tap level against an equal-distance direct
// Line read modulation depth, frames - just enough to keep the modes moving.
// Bigger depths make linear interp's HF rolloff flutter, a sparkle on tails.
#define AU_ENV_MOD      2.5f

// Mutually prime line lengths at size 10m, ~23-62ms, scaled by size/10.
// Scaling loses exact primality, but stays incommensurate enough.
static const int32_t au_env_base_len[AU_ENV_LINES] =
	{ 1097, 1327, 1559, 1801, 2099, 2371, 2677, 2971 };
// Injection/tap signs decorrelate the lines' contributions.
static const float au_env_in_sign [AU_ENV_LINES] = { 1,-1, 1,-1, 1,-1, 1,-1 };
static const float au_env_out_sign[AU_ENV_LINES] = { 1, 1,-1,-1, 1,-1,-1, 1 };
// Series diffusion delays, ~3.6-12.7ms - Dattorro-style spacing.
static const int32_t au_env_ap_len[AU_ENV_APS] = { 173, 229, 447, 611 };
// Mutually detuned read modulation rates, Hz. A static FDN's modes freeze
// into a metallic comb; a cent of wobble breaks it, far below audible chorus.
static const float au_env_mod_rate[AU_ENV_LINES] =
	{ 0.31f, 0.43f, 0.37f, 0.53f, 0.41f, 0.59f, 0.35f, 0.47f };

struct au_env_t {
	float*  lines;              // AU_ENV_LINES max-length lines, contiguous
	float*  send;               // Mono send accumulator, au_mix_temp_size
	int32_t len[AU_ENV_LINES];  // Active line lengths for the current size
	int32_t at [AU_ENV_LINES];
	float   fb [AU_ENV_LINES];  // Per-line decay gain from RT60
	float   lp [AU_ENV_LINES];  // Damping one-pole state
	float   mod_phase[AU_ENV_LINES];
	float   ap[AU_ENV_APS][AU_ENV_AP_MAX];
	int32_t ap_at[AU_ENV_APS];
	float   in_lp;              // Input tone one-pole state
	float*  er_temp;            // Absorbed-mono scratch for the tap writes
	float   send_ref;           // Send rolloff knee, meters - ~half of size
	// Early reflection shoebox, listener-centered, world-aligned.
	bool    er_on;              // Gates all per-voice tap work
	float   er_hx;              // Wall half-extent, x and z
	float   er_floor, er_ceil;  // Surface heights relative to the head
	float   er_reflect;
	float   er_k;               // Surface absorption one-pole coefficient
	float   wet_cur;            // Glides toward the target, audio thread
	float   size_cur;           // Size the lines are laid out for
	int32_t tail_left;          // Frames of tail worth rendering
	bool    on;                 // This block: sends accumulate
	bool    sent;               // A voice touched the send this block
};
static au_env_t au_env = {};

// Cross-thread environment params, atomic f32, main writes.
static float au_env_wet     = 0;
static float au_env_decay   = 0.45f;
static float au_env_damp    = 0.35f;
static float au_env_size    = 5;
static float au_env_scatter = 0.5f;
static float au_env_reflect = 0.5f;

void audio_set_env(audio_env_t env) {
	atomic_store_f32(&au_env_wet,     fmaxf(0,     fminf(1,  env.wet)));
	atomic_store_f32(&au_env_decay,   fmaxf(0.05f, fminf(10, env.decay)));
	atomic_store_f32(&au_env_damp,    fmaxf(0,     fminf(1,  env.damp)));
	atomic_store_f32(&au_env_size,    fmaxf(2,     fminf(40, env.size)));
	atomic_store_f32(&au_env_scatter, fmaxf(0,     fminf(1,  env.scatter)));
	atomic_store_f32(&au_env_reflect, fmaxf(0,     fminf(1,  env.reflect)));
}

audio_env_t audio_get_env() {
	audio_env_t result;
	result.wet     = atomic_load_f32(&au_env_wet);
	result.decay   = atomic_load_f32(&au_env_decay);
	result.damp    = atomic_load_f32(&au_env_damp);
	result.size    = atomic_load_f32(&au_env_size);
	result.scatter = atomic_load_f32(&au_env_scatter);
	result.reflect = atomic_load_f32(&au_env_reflect);
	return result;
}

// Audio thread, before the voice sweep: glide the wet level and ready the
// send buffer. `on` gates every per-voice send, so off costs nothing.
static void audio_env_begin(ma_uint32 frame_count) {
	float wet = atomic_load_f32(&au_env_wet);
	float k   = 1.0f - expf(-(float)frame_count / (0.05f * AU_SAMPLE_RATE));
	au_env.wet_cur += k * (wet - au_env.wet_cur);
	if (wet <= 0 && au_env.wet_cur < 0.001f) au_env.wet_cur = 0;

	au_env.on = au_env.wet_cur > 0;
	if (!au_env.on) { au_env.er_on = false; return; }

	// A size change re-lays the lines and restarts the tail - which reads
	// as the space changing anyway. Other params glide.
	float size = atomic_load_f32(&au_env_size);
	au_env.send_ref = fmaxf(2.0f, size * 0.5f);

	// Shoebox: walls from size, floor at head height, slow-growing ceiling.
	// Absorption rides damp - a perfect bounce combs against the direct.
	au_env.er_on      = true;
	au_env.er_hx      = fmaxf(2.0f, size * 0.5f);
	au_env.er_floor   = -1.6f;
	au_env.er_ceil    = fminf(8.0f, fmaxf(1.2f, size * 0.25f));
	au_env.er_reflect = atomic_load_f32(&au_env_reflect);
	float er_RC  = 1.0f / (6.2831853f * math_lerp(6000, 2000, atomic_load_f32(&au_env_damp)));
	au_env.er_k  = (1.0f / AU_SAMPLE_RATE) / (er_RC + 1.0f / AU_SAMPLE_RATE);
	if (size != au_env.size_cur) {
		au_env.size_cur = size;
		float scale = size * 0.1f;
		for (int32_t l = 0; l < AU_ENV_LINES; l++) {
			au_env.len[l] = (int32_t)fminf(AU_ENV_MAX_LINE, fmaxf(96, au_env_base_len[l] * scale));
			au_env.at [l] = 0;
			au_env.lp [l] = 0;
		}
		memset(au_env.lines, 0, sizeof(float) * AU_ENV_LINES * AU_ENV_MAX_LINE);
	}

	// Per-line feedback for -60dB after `decay` seconds of round trips.
	float decay = atomic_load_f32(&au_env_decay);
	for (int32_t l = 0; l < AU_ENV_LINES; l++)
		au_env.fb[l] = powf(10.0f, -3.0f * (float)au_env.len[l] / (decay * AU_SAMPLE_RATE));

	memset(au_env.send, 0, sizeof(float) * frame_count);
	au_env.sent = false;
}

// Audio thread, after the voice sweep: run the FDN over the block's send and
// add the tail to the bus. A send silent longer than the tail turns this off.
static void audio_env_mix(ma_uint32 frame_count) {
	if (!au_env.on) return;
	if (au_env.sent)
		au_env.tail_left = (int32_t)(atomic_load_f32(&au_env_decay) * 1.5f * AU_SAMPLE_RATE);
	if (au_env.tail_left <= 0) return;
	au_env.tail_left -= mini((int32_t)frame_count, au_env.tail_left);

	float damp    = atomic_load_f32(&au_env_damp);
	float scatter = atomic_load_f32(&au_env_scatter);
	float wet     = au_env.wet_cur * 0.22f; // Tail level norm, tuned by ear

	// Input tone: a real space's tail is never as bright as its source,
	// and full-bandwidth injection reads as harsh. Cutoff rides damp.
	float tone_RC = 1.0f / (6.2831853f * math_lerp(8000, 2500, damp));
	float tone_k  = (1.0f / AU_SAMPLE_RATE) / (tone_RC + 1.0f / AU_SAMPLE_RATE);
	// Diffusion gain from scatter, floored so line injections never
	// arrive as discrete slapback copies of a transient.
	float g_ap    = 0.35f + 0.4f * scatter;

	// Read-tap modulation offsets, stepped linearly across the block.
	float mod_at[AU_ENV_LINES], mod_step[AU_ENV_LINES];
	for (int32_t l = 0; l < AU_ENV_LINES; l++) {
		float ph0 = au_env.mod_phase[l];
		float ph1 = ph0 + au_env_mod_rate[l] * (float)frame_count / AU_SAMPLE_RATE;
		au_env.mod_phase[l] = ph1 - (int32_t)ph1;
		float o0 = 1.5f + AU_ENV_MOD * (0.5f + 0.5f * sinf(ph0 * 6.2831853f));
		float o1 = 1.5f + AU_ENV_MOD * (0.5f + 0.5f * sinf(ph1 * 6.2831853f));
		mod_at  [l] = o0;
		mod_step[l] = (o1 - o0) / (float)frame_count;
	}

	for (ma_uint32 i = 0; i < frame_count; i++) {
		// Tone, then four series Schroeder allpasses smear the send so
		// the lines fill with a wash instead of echo copies.
		float in = au_env.send[i];
		au_env.in_lp += tone_k * (in - au_env.in_lp);
		in = au_env.in_lp;
		for (int32_t a = 0; a < AU_ENV_APS; a++) {
			float d = au_env.ap[a][au_env.ap_at[a]];
			float v = in + g_ap * d;
			in      = d  - g_ap * v;
			au_env.ap[a][au_env.ap_at[a]] = v;
			au_env.ap_at[a] = au_env.ap_at[a] + 1 >= au_env_ap_len[a] ? 0 : au_env.ap_at[a] + 1;
		}

		// Modulated fractional line reads, then Householder feedback - an
		// orthogonal reflection, so decay comes only from fb.
		float y[AU_ENV_LINES];
		float sum = 0;
		for (int32_t l = 0; l < AU_ENV_LINES; l++) {
			float*  buf = au_env.lines + l * AU_ENV_MAX_LINE;
			float   rp  = (float)au_env.at[l] + mod_at[l];
			mod_at[l] += mod_step[l];
			int32_t r0  = (int32_t)rp;
			float   fr  = rp - (float)r0;
			if (r0 >= au_env.len[l]) r0 -= au_env.len[l];
			int32_t r1  = r0 + 1 >= au_env.len[l] ? 0 : r0 + 1;
			y[l] = math_lerp(buf[r0], buf[r1], fr);
			sum += y[l];
		}
		float h   = sum * (2.0f / AU_ENV_LINES);
		float out = 0;
		for (int32_t l = 0; l < AU_ENV_LINES; l++) {
			float fbv = y[l] - h;
			au_env.lp[l] = fbv + damp * (au_env.lp[l] - fbv);
			au_env.lines[l * AU_ENV_MAX_LINE + au_env.at[l]] =
				in * au_env_in_sign[l] * 0.5f + au_env.fb[l] * au_env.lp[l];
			au_env.at[l] = au_env.at[l] + 1 >= au_env.len[l] ? 0 : au_env.at[l] + 1;
			out += au_env_out_sign[l] * y[l];
		}
		au_foa_ring[((au_foa_head + i) & au_foa_mask) * 4] += out * wet;
	}
	au_foa_touched = true;
}

///////////////////////////////////////////
// Early reflection taps, computed per voice per block and written from
// the voice's filtered mono signal as a post-pass.

// A spatial voice's reflection tap set for one block.
struct au_er_taps_t {
	int32_t  count;
	int32_t  slot[AU_ER_TAPS]; // Shoebox surface index, addresses state
	float    tgt [AU_ER_TAPS]; // Target delay in frames, fractional
	XMVECTOR enc [AU_ER_TAPS];
};

// First-order images in the listener-centered shoebox, each tap a delayed,
// attenuated copy encoded from the image's direction. Sources past the walls
// clamp into the box, but taps still fade with *true* distance (far_k) -
// frozen reflections under a fading direct read as the source approaching.
static void au_er_setup(au_er_taps_t* out, vec3 dir, float dist, float decibels, float trim_gain, float spread, int32_t grant) {
	out->count = 0;
	if (grant == 0 || !au_env.er_on || spread >= 1) return;

	float hx = au_env.er_hx, fy = au_env.er_floor, cy = au_env.er_ceil;
	vec3  pc = vec3{
		fmaxf(-hx*0.9f, fminf(hx*0.9f, dir.x)),
		fmaxf( fy*0.9f, fminf(cy*0.9f, dir.y)),
		fmaxf(-hx*0.9f, fminf(hx*0.9f, dir.z)) };
	float dist_c = fmaxf(AU_MIN_DISTANCE, vec3_magnitude(pc));
	float far_k  = fminf(1.0f, dist_c / dist);
	vec3  imgs[AU_ER_TAPS] = {
		{pc.x, 2*fy - pc.y, pc.z}, {pc.x, 2*cy - pc.y, pc.z},
		{ 2*hx - pc.x, pc.y, pc.z}, {-2*hx - pc.x, pc.y, pc.z},
		{pc.x, pc.y,  2*hx - pc.z}, {pc.x, pc.y, -2*hx - pc.z} };

	float delay_max = 0;
	for (int32_t t = 0; t < AU_ER_TAPS; t++) {
		float lvl = fmaxf(t == 0 ? 0.35f : 0, au_env.er_reflect) * AU_ER_LEVEL * (1.0f - spread) * far_k;
		if (lvl < 0.01f) continue;
		float d_img = fmaxf(AU_MIN_DISTANCE, vec3_magnitude(imgs[t]));
		float tgt   = (d_img - dist_c) * (AU_SAMPLE_RATE / 343.0f);
		tgt = fmaxf(1.0f, fminf((float)(AU_ER_DELAY_MAX - 3), tgt));
		if (tgt > delay_max) delay_max = tgt;
		vec3  u_img = imgs[t] / d_img;
		float g     = decibels_to_signal(decibels - 20.0f*log10f(d_img)) * trim_gain * lvl;
		out->enc [out->count] = XMVectorSet(g, g * -u_img.x, g * u_img.y, g * -u_img.z);
		out->tgt [out->count] = tgt;
		out->slot[out->count] = t;
		out->count += 1;
	}
	if (out->count > 0) {
		au_foa_touched = true;
		au_foa_future  = maxi(au_foa_future, (int32_t)delay_max + 2);
	}
}

// Writes a voice's taps from its filtered mono block. Delays slew across the
// block through two-point interpolated writes - integer jumps on sustained
// content read as a rattle. The absorption softens each copy's attack, and
// runs once for the whole tap set: same input, same coefficient, same result.
static void au_er_write(au_voice_t* voice, const au_er_taps_t* taps, const float* mono, ma_uint32 frame_offset, ma_uint32 count) {
	if (taps->count == 0 || count == 0) return;

	float k = au_env.er_k, lp = voice->er_lp, lp2 = voice->er_lp2;
	for (ma_uint32 i = 0; i < count; i++) {
		lp  += k * (mono[i] - lp);
		lp2 += k * (lp - lp2);
		au_env.er_temp[i] = lp2;
	}
	voice->er_lp  = lp;
	voice->er_lp2 = lp2;

	uint32_t base = au_foa_head + frame_offset;
	for (int32_t t = 0; t < taps->count; t++) {
		int32_t  slot = taps->slot[t];
		float    cur  = voice->er_delay[slot];
		if (cur < 0) cur = taps->tgt[t];
		float    step = (taps->tgt[t] - cur) / (float)count;
		XMVECTOR enc  = taps->enc[t];
		for (ma_uint32 i = 0; i < count; i++) {
			float    v    = au_env.er_temp[i];
			float    fi   = (float)i + cur;
			uint32_t at0  = base + (uint32_t)fi;
			float    frac = fi - (float)(uint32_t)fi;
			float*   tap0 = au_foa_ring + (( at0      & au_foa_mask) * 4);
			float*   tap1 = au_foa_ring + (((at0 + 1) & au_foa_mask) * 4);
			XMStoreFloat4((XMFLOAT4*)tap0, XMVectorMultiplyAdd(XMVectorReplicate(v * (1.0f - frac)), enc, XMLoadFloat4((XMFLOAT4*)tap0)));
			XMStoreFloat4((XMFLOAT4*)tap1, XMVectorMultiplyAdd(XMVectorReplicate(v * frac),          enc, XMLoadFloat4((XMFLOAT4*)tap1)));
			cur += step;
		}
		voice->er_delay[slot] = taps->tgt[t];
	}
}

///////////////////////////////////////////
// Direct binaural path for point sources: per-voice exact ITD, head
// shadow, and parametric elevation/back voicing. Precision cues the FOA
// bus can't provide - a source's own interaural delay is what makes it
// pointable. Spread crossfades a voice from here onto the bus.

// A/B hook: force everything through the bus for listening comparisons.
static int32_t au_force_bus = 0;
void audio_test_force_bus(bool32_t enable) {
	atomic_store_i32(&au_force_bus, enable ? 1 : 0);
}

// Direct-form-I biquad with caller-owned state, st = [x1 x2 y1 y2].
static inline float au_biquad_apply(const au_biquad_t* c, float* st, float x) {
	float y = c->b0*x + c->b1*st[0] + c->b2*st[1] - c->a1*st[2] - c->a2*st[3];
	st[1] = st[0]; st[0] = x;
	st[3] = st[2]; st[2] = y;
	return y;
}

// Soft clip limiter, transparent below the -1dBFS knee.
static inline float au_limit(float x) {
	const float knee = 0.89f;
	float a = fabsf(x);
	if (a <= knee) return x;
	float y = knee + (1.0f - knee) * tanhf((a - knee) / (1.0f - knee));
	return x < 0 ? -y : y;
}

// Master/bus trims and the output meter, all atomic f32.
static float au_master_volume = 1;
static float au_bus_volumes[AU_BUS_COUNT] = {1, 1, 1, 1};
static float au_output_dbfs  = -120;

///////////////////////////////////////////

void audio_set_volume(float volume) {
	atomic_store_f32(&au_master_volume, fmaxf(0, fminf(1, volume)));
}
float audio_get_volume() {
	return atomic_load_f32(&au_master_volume);
}
void audio_set_bus_volume(sound_bus_ bus, float volume) {
	if ((uint32_t)bus >= AU_BUS_COUNT) { log_errf("audio_set_bus_volume: invalid bus %d", bus); return; }
	atomic_store_f32(&au_bus_volumes[bus], fmaxf(0, fminf(1, volume)));
}
float audio_get_bus_volume(sound_bus_ bus) {
	if ((uint32_t)bus >= AU_BUS_COUNT) return 0;
	return atomic_load_f32(&au_bus_volumes[bus]);
}
float audio_get_output_decibels() {
	return atomic_load_f32(&au_output_dbfs);
}

///////////////////////////////////////////

void audio_listener_publish(pose_t pose) {
	atomic_store_f32(&au_listener_pose.position.x,    pose.position.x);
	atomic_store_f32(&au_listener_pose.position.y,    pose.position.y);
	atomic_store_f32(&au_listener_pose.position.z,    pose.position.z);
	atomic_store_f32(&au_listener_pose.orientation.x, pose.orientation.x);
	atomic_store_f32(&au_listener_pose.orientation.y, pose.orientation.y);
	atomic_store_f32(&au_listener_pose.orientation.z, pose.orientation.z);
	atomic_store_f32(&au_listener_pose.orientation.w, pose.orientation.w);
}

pose_t audio_listener_get() {
	pose_t result;
	result.position.x    = atomic_load_f32(&au_listener_pose.position.x);
	result.position.y    = atomic_load_f32(&au_listener_pose.position.y);
	result.position.z    = atomic_load_f32(&au_listener_pose.position.z);
	result.orientation.x = atomic_load_f32(&au_listener_pose.orientation.x);
	result.orientation.y = atomic_load_f32(&au_listener_pose.orientation.y);
	result.orientation.z = atomic_load_f32(&au_listener_pose.orientation.z);
	result.orientation.w = atomic_load_f32(&au_listener_pose.orientation.w);
	return result;
}

///////////////////////////////////////////

void audio_mix_init(int32_t period_frames) {
	// Sizes are in frames, buffers allocate 4x for interleaved ambisonics.
	au_mix_temp_size = (uint64_t)period_frames + AU_SAMPLE_BUFFER_SIZE * 2;
	au_mix_temp      = sk_malloc_t(float, au_mix_temp_size * 4);
	memset(au_mix_temp, 0, sizeof(float) * au_mix_temp_size * 4);

	au_resample_temp_size = ((uint64_t)(period_frames * AU_PITCH_MAX) + AU_SAMPLE_BUFFER_SIZE * 2 + 8) * 4;
	au_resample_temp      = sk_malloc_t(float, au_resample_temp_size);
	memset(au_resample_temp, 0, sizeof(float) * au_resample_temp_size);

	au_foa = sk_malloc_t(float, au_mix_temp_size * 4);
	memset(au_foa, 0, sizeof(float) * au_mix_temp_size * 4);

	uint32_t ring = 1;
	while (ring < au_mix_temp_size + AU_ER_DELAY_MAX + 64) ring <<= 1;
	au_foa_ring = sk_malloc_t(float, (uint64_t)ring * 4);
	memset(au_foa_ring, 0, sizeof(float) * ring * 4);
	au_foa_mask   = ring - 1;
	au_foa_head   = 0;
	au_foa_future = 0;

	au_env.lines = sk_malloc_t(float, AU_ENV_LINES * AU_ENV_MAX_LINE);
	memset(au_env.lines, 0, sizeof(float) * AU_ENV_LINES * AU_ENV_MAX_LINE);
	au_env.send  = sk_malloc_t(float, au_mix_temp_size);
	memset(au_env.send, 0, sizeof(float) * au_mix_temp_size);
	au_env.er_temp = sk_malloc_t(float, au_mix_temp_size);
	memset(au_env.er_temp, 0, sizeof(float) * au_mix_temp_size);

	au_speakers_init();
	audio_listener_publish(pose_identity);
}

///////////////////////////////////////////

static void voice_free_resources(sound_t sound, ma_decoder* decoder, ma_pcm_rb* rb, float* ring_data) {
	if (decoder != nullptr) {
		ma_decoder_uninit(decoder);
		sk_free(decoder);
	}
	if (rb != nullptr) {
		ma_pcm_rb_uninit(rb);
		sk_free(rb);
	}
	sk_free(ring_data);
	sound_release(sound);
}

///////////////////////////////////////////

// Estimated audibility of a voice at the listener's ear. Distance-less
// sounds (ambisonic fields, head-locked audio) rank infinite: nothing
// displaces them, and they're never dormant.
static float voice_audibility(const au_voice_t* voice, vec3 listener_pos) {
	// Atomic because activation writes it on the audio thread. The pointer
	// stays valid regardless: frees happen on this thread's own drain.
	_sound_t* snd = (_sound_t*)atomic_load_ptr(&voice->sound);
	if (snd == nullptr) snd = (_sound_t*)atomic_load_ptr(&voice->pending_sound);
	if (snd == nullptr) return 0;

	int32_t flags = atomic_load_i32(&voice->params.flags);
	if (snd->channels != sound_channels_mono || (flags & sound_flags_head_locked) != 0)
		return 1e30f;

	vec3 pos = {
		atomic_load_f32(&voice->params.pos_x),
		atomic_load_f32(&voice->params.pos_y),
		atomic_load_f32(&voice->params.pos_z) };
	float dist = fmaxf(AU_MIN_DISTANCE, vec3_magnitude(listener_pos - pos));
	float db   = atomic_load_f32(&snd->decibels);
	return decibels_to_signal(db - 20.0f*log10f(dist))
	     * atomic_load_f32(&voice->params.volume);
}

static float sound_play_audibility(sound_t sound, vec3 at, float volume, vec3 listener_pos) {
	if (sound->channels != sound_channels_mono) return 1e30f;
	float dist = fmaxf(AU_MIN_DISTANCE, vec3_magnitude(listener_pos - at));
	return decibels_to_signal(atomic_load_f32(&sound->decibels) - 20.0f*log10f(dist)) * volume;
}

int16_t audio_voice_reserve(sound_t sound, vec3 at, float volume) {
	for (int16_t i = 0; i < AU_VOICE_COUNT; i++) {
		if (atomic_cas_i32(&au_voices[i].state, au_voice_free, au_voice_reserved)) {
			au_voices[i].id += 1;
			return i;
		}
	}

	// No free slot - finished voices may just be waiting on their drain,
	// which is main-thread work, and this is the main thread.
	audio_mix_drain_returns();
	for (int16_t i = 0; i < AU_VOICE_COUNT; i++) {
		if (atomic_cas_i32(&au_voices[i].state, au_voice_free, au_voice_reserved)) {
			au_voices[i].id += 1;
			return i;
		}
	}

	// Genuinely full: steal the quietest playing voice, but only if the new
	// sound is *more* audible - otherwise refusing it is the least-bad
	// choice. Distance-less voices rank infinite and are never victims.
	pose_t  head      = audio_listener_get();
	int16_t best_slot = -1;
	float   best_gain = 1e30f;
	for (int16_t i = 0; i < AU_VOICE_COUNT; i++) {
		if (atomic_load_i32(&au_voices[i].state) != au_voice_playing) continue;
		float gain = voice_audibility(&au_voices[i], head.position);
		if (gain < best_gain) { best_gain = gain; best_slot = i; }
	}
	if (best_slot < 0 || sound_play_audibility(sound, at, volume, head.position) <= best_gain)
		return -1;

	if (!atomic_cas_i32(&au_voices[best_slot].state, au_voice_playing, au_voice_reserved))
		return -1; // Lost a race with the audio thread, just refuse this play.

	// The displaced play's resources stay untouched until the audio thread
	// hands them back through the return ring at activation.
	au_voices[best_slot].id += 1;
	return best_slot;
}

///////////////////////////////////////////

// Main thread, once per frame: rank active voices by audibility, grant the
// top AU_MIX_VOICES the mix budget. The rest go dormant in place - cursor
// frozen, slot and handle intact - and resume when they rank back in.
void audio_voice_rank() {
	pose_t  head = audio_listener_get();
	float   gains[AU_VOICE_COUNT];
	int32_t active = 0;

	for (int16_t i = 0; i < AU_VOICE_COUNT; i++) {
		int32_t state = atomic_load_i32(&au_voices[i].state);
		if (state != au_voice_playing && state != au_voice_reserved) {
			gains[i] = -1;
			continue;
		}
		gains[i] = voice_audibility(&au_voices[i], head.position);
		active  += 1;
	}

	// Reflection budget: taps for the AU_ER_VOICES loudest spatial voices.
	// Distance-less voices never render taps, so they can't waste grants.
	float   er_gain[AU_VOICE_COUNT];
	int32_t er_n = 0;
	for (int16_t i = 0; i < AU_VOICE_COUNT; i++)
		if (gains[i] >= 0 && gains[i] < 1e29f) er_gain[er_n++] = gains[i];
	float er_thresh = 0;
	if (er_n > AU_ER_VOICES) {
		for (int32_t i = 1; i < er_n; i++) {
			float   v = er_gain[i];
			int32_t j = i - 1;
			while (j >= 0 && er_gain[j] < v) { er_gain[j+1] = er_gain[j]; j -= 1; }
			er_gain[j+1] = v;
		}
		er_thresh = er_gain[AU_ER_VOICES - 1];
	}
	int32_t er_granted = 0;
	for (int16_t i = 0; i < AU_VOICE_COUNT; i++) {
		bool er = gains[i] >= 0 && gains[i] < 1e29f
		       && gains[i] >= er_thresh && er_granted < AU_ER_VOICES;
		if (er) er_granted += 1;
		atomic_store_i32(&au_voices[i].er_grant, er ? 1 : 0);
	}

	// Under budget: everyone plays, skip the ranking work.
	if (active <= AU_MIX_VOICES) {
		for (int16_t i = 0; i < AU_VOICE_COUNT; i++)
			if (gains[i] >= 0) atomic_store_i32(&au_voices[i].audible, 1);
		return;
	}

	// The budget's threshold gain: the AU_MIX_VOICES'th loudest. A simple
	// selection over 128 entries, this is small.
	float sorted[AU_VOICE_COUNT];
	int32_t n = 0;
	for (int16_t i = 0; i < AU_VOICE_COUNT; i++)
		if (gains[i] >= 0) sorted[n++] = gains[i];
	for (int32_t i = 1; i < n; i++) {
		float   v = sorted[i];
		int32_t j = i - 1;
		while (j >= 0 && sorted[j] < v) { sorted[j+1] = sorted[j]; j -= 1; }
		sorted[j+1] = v;
	}
	float threshold = sorted[AU_MIX_VOICES - 1];

	int32_t granted = 0;
	for (int16_t i = 0; i < AU_VOICE_COUNT; i++) {
		if (gains[i] < 0) continue;
		bool in = gains[i] >= threshold && granted < AU_MIX_VOICES;
		if (in) granted += 1;
		atomic_store_i32(&au_voices[i].audible, in ? 1 : 0);
	}
}

///////////////////////////////////////////

// The command ring publish is the release point for every pending_* field
// main wrote during reservation, the audio thread acquires them via head.
bool audio_voice_submit(int16_t slot) {
	uint32_t head = (uint32_t)atomic_load_i32    (&au_cmd_head);
	uint32_t tail = (uint32_t)atomic_load_i32_acq(&au_cmd_tail);
	if (head - tail >= AU_CMD_RING_SIZE) return false;

	au_cmd_t cmd = {};
	cmd.slot = slot;
	cmd.id   = au_voices[slot].id;
	au_cmd_ring[head % AU_CMD_RING_SIZE] = cmd;
	atomic_store_i32_rel(&au_cmd_head, (int32_t)(head + 1));
	return true;
}

///////////////////////////////////////////

// Audio thread. Hand a voice's resources to main for freeing.
static void audio_ret_push(sound_t sound, ma_decoder* decoder, ma_pcm_rb* rb, float* ring_data, int16_t slot) {
	uint32_t head = (uint32_t)atomic_load_i32    (&au_ret_head);
	uint32_t tail = (uint32_t)atomic_load_i32_acq(&au_ret_tail);
	if (head - tail >= AU_RET_RING_SIZE) {
		// Can't free from this thread, and by design this can't happen: the
		// ring outsizes every event source combined.
		return;
	}
	au_ret_t* entry = &au_ret_ring[head % AU_RET_RING_SIZE];
	entry->sound     = sound;
	entry->decoder   = decoder;
	entry->rb        = rb;
	entry->ring_data = ring_data;
	entry->slot      = slot;
	atomic_store_i32_rel(&au_ret_head, (int32_t)(head + 1));
}

// Audio thread. The common finish: hand off and mark for the drain.
static void voice_finish(au_voice_t* voice, int16_t slot) {
	if (atomic_cas_i32(&voice->state, au_voice_playing, au_voice_finished))
		audio_ret_push(voice->sound, voice->stream_decoder, voice->stream_ring, voice->stream_ring_data, slot);
	// A failed CAS means the slot was just stolen, activation cleans up.
}

// Main thread, called from audio_step and shutdown.
void audio_mix_drain_returns() {
	uint32_t tail = (uint32_t)atomic_load_i32    (&au_ret_tail);
	uint32_t head = (uint32_t)atomic_load_i32_acq(&au_ret_head);
	while (tail != head) {
		au_ret_t* entry = &au_ret_ring[tail % AU_RET_RING_SIZE];
		voice_free_resources(entry->sound, entry->decoder, entry->rb, entry->ring_data);
		if (entry->slot >= 0) {
			// Finished voices are off-limits to the audio thread, clearing
			// the freed pointers here is safe and keeps shutdown single-path.
			au_voice_t* voice = &au_voices[entry->slot];
			voice->sound            = nullptr;
			voice->stream_decoder   = nullptr;
			voice->stream_ring      = nullptr;
			voice->stream_ring_data = nullptr;
			atomic_cas_i32(&voice->state, au_voice_finished, au_voice_free);
		}
		tail += 1;
	}
	atomic_store_i32_rel(&au_ret_tail, (int32_t)tail);
}

///////////////////////////////////////////

// Audio thread. Swap the pending play in, handing any displaced resources
// (a stolen slot's previous play) back to main first.
static void voice_activate(au_voice_t* voice) {
	if (voice->sound != nullptr)
		audio_ret_push(voice->sound, voice->stream_decoder, voice->stream_ring, voice->stream_ring_data, -1);

	// Atomic stores, the audibility scans on main read both pointers.
	atomic_store_ptr(&voice->sound, voice->pending_sound);
	atomic_store_ptr(&voice->pending_sound, nullptr);
	voice->cursor           = voice->pending_cursor;
	voice->stream_decoder   = voice->pending_decoder;
	voice->stream_ring      = voice->pending_ring;
	voice->stream_ring_data = voice->pending_ring_data;
	voice->pending_decoder  = nullptr;
	voice->pending_ring     = nullptr;
	voice->pending_ring_data= nullptr;
	voice->delay_left       = voice->pending_delay;
	voice->bus              = voice->pending_bus;
	voice->fade_gain        = voice->pending_fade ? 0.0f : 1.0f;
	voice->resample_frac    = 0;
	memset(voice->resample_last, 0, sizeof(voice->resample_last));
	memset(voice->lpf_state,     0, sizeof(voice->lpf_state));
	voice->er_lp  = 0;
	voice->er_lp2 = 0;
	for (int32_t t = 0; t < AU_ER_TAPS; t++) voice->er_delay[t] = -1;
	memset(voice->dir_ring,   0, sizeof(voice->dir_ring));
	memset(voice->dir_filter, 0, sizeof(voice->dir_filter));
	voice->dir_ring_at      = 0;
	voice->dir_shoulder     = -1;
	voice->dir_delay[0]     = -1;
	voice->dir_delay[1]     = -1;
	voice->dir_shadow[0]    = 0;
	voice->dir_shadow[1]    = 0;
	atomic_store_i32(&voice->audible,  1);
	atomic_store_i32(&voice->er_grant, 1);
	// Release so main's prefetch sees the stream fields once it's playing.
	atomic_store_i32_rel(&voice->state, au_voice_playing);
}

///////////////////////////////////////////

// Audio thread, start of each block.
static void audio_drain_commands() {
	uint32_t tail = (uint32_t)atomic_load_i32    (&au_cmd_tail);
	uint32_t head = (uint32_t)atomic_load_i32_acq(&au_cmd_head);
	while (tail != head) {
		au_cmd_t*   cmd   = &au_cmd_ring[tail % AU_CMD_RING_SIZE];
		au_voice_t* voice = &au_voices[cmd->slot];
		if (voice->id == cmd->id && atomic_load_i32(&voice->state) == au_voice_reserved)
			voice_activate(voice);
		tail += 1;
	}
	atomic_store_i32_rel(&au_cmd_tail, (int32_t)tail);
}

///////////////////////////////////////////

// Audio thread. Reads source-rate *frames* into dest (interleaved), honoring
// looping. Live-source underruns zero-fill so the voice stays alive, a short
// return means done for good. The cursor stores atomically for main's getter.
static ma_uint64 voice_read_source(au_voice_t* voice, float* dest, ma_uint64 frames, bool loop) {
	_sound_t* sound = voice->sound;
	int32_t   ch    = sound_channel_count(sound->channels);
	switch (sound->data_type) {
	case sound_data_pcm: {
		ma_uint64 total  = 0;
		uint64_t  cursor = voice->cursor;
		while (total < frames) {
			if (cursor >= sound->pcm_count) {
				if (!loop || sound->pcm_count == 0) break;
				cursor = 0;
			}
			ma_uint64 read = mini(frames - total, sound->pcm_count - cursor);
			memcpy(dest + total*ch, sound->pcm + cursor*ch, (size_t)(read * ch) * sizeof(float));
			cursor += read;
			total  += read;
		}
		atomic_store_u64(&voice->cursor, cursor);
		return total;
	}
	case sound_data_stream_file: {
		// Prefetch handles looping by re-seeking the decoder, from here it's
		// a ring that never runs eof. Ring counts are channel-aware frames.
		ma_uint64 read = 0;
		while (read < frames) {
			ma_uint32 request = (ma_uint32)(frames - read);
			void*     from    = nullptr;
			if (ma_pcm_rb_acquire_read(voice->stream_ring, &request, &from) != MA_SUCCESS || request == 0) break;
			memcpy(dest + read*ch, from, (size_t)(request * ch) * sizeof(float));
			ma_pcm_rb_commit_read(voice->stream_ring, request);
			read += request;
		}
		atomic_store_u64(&voice->cursor, voice->cursor + read);
		if (read < frames && atomic_load_i32_acq(&voice->stream_eof) == 0) {
			// Prefetch underrun, not the end of the file. Pad with silence.
			memset(dest + read*ch, 0, (size_t)((frames - read) * ch) * sizeof(float));
			read = frames;
		}
		return read;
	}
	case sound_data_ring: {
		// Broadcast read at this voice's own absolute cursor: lock-free,
		// nothing consumed, every voice hears the whole stream.
		uint64_t  cursor = voice->cursor;
		ma_uint64 read   = sound_ring_read_at(sound, &cursor, dest, frames);
		atomic_store_u64(&voice->cursor, cursor);
		// Live streams idle at the end of their data instead of finishing.
		memset(dest + read*ch, 0, (size_t)((frames - read) * ch) * sizeof(float));
		return frames;
	}
	default: return 0;
	}
}

///////////////////////////////////////////

// 4-point Catmull-Rom between p1 and p2 - flat to near Nyquist, where linear
// interpolation shaves the top octave and folds images down as aliasing.
static inline float au_catmull(float p0, float p1, float p2, float p3, float t) {
	return p1 + 0.5f*t*(p2 - p0 + t*(2*p0 - 5*p1 + 4*p2 - p3 + t*(3*(p1 - p2) + p3 - p0)));
}

// Mid-waveform starts (deferred play catch-up, seeks) are a step function
// otherwise, an audible click. Applied to the raw source read, so every
// render path inherits the ramp.
static inline void voice_fade_apply(au_voice_t* voice, float* dest, ma_uint64 frames, int32_t ch) {
	float g = voice->fade_gain;
	if (g >= 1.0f) return;

	const float step = 1.0f / AU_FADE_FRAMES;
	for (ma_uint64 i = 0; i < frames && g < 1.0f; i++) {
		for (int32_t c = 0; c < ch; c++) dest[i*ch + c] *= g;
		g += step;
	}
	voice->fade_gain = fminf(1.0f, g);
}

// Audio thread. voice_read_source plus Catmull-Rom resampling for both pitch
// and source rate. resample_last carries three frames across blocks so the
// 4-point window stays continuous. `rate` is source frames per output frame.
static ma_uint64 voice_read(au_voice_t* voice, float* dest, ma_uint64 frames, float rate, bool loop) {
	int32_t ch = sound_channel_count(voice->sound->channels);
	if (rate == 1.0f) {
		ma_uint64 read = voice_read_source(voice, dest, frames, loop);
		// History rolls forward anyway, a live pitch change resumes clean.
		for (int32_t k = 0; k < 3; k++) {
			int64_t f = (int64_t)read - 3 + k;
			for (int32_t c = 0; c < ch; c++)
				voice->resample_last[k][c] = f >= 0
					? dest[f*ch + c]
					: voice->resample_last[k + (int32_t)read][c];
		}
		voice->resample_frac = 0;
		voice_fade_apply(voice, dest, read, ch);
		return read;
	}

	ma_uint64 needed = (ma_uint64)(voice->resample_frac + (double)frames * rate) + 2;
	if (needed > au_resample_temp_size / ch - 3)
		needed = au_resample_temp_size / ch - 3;

	// Temp frames 0-2 are history; frame 2 is the last consumed frame, so
	// position 0 interpolates the span from frame 2 to frame 3.
	for (int32_t k = 0; k < 3; k++)
		for (int32_t c = 0; c < ch; c++)
			au_resample_temp[k*ch + c] = voice->resample_last[k][c];
	ma_uint64 src_read = voice_read_source(voice, au_resample_temp + 3*ch, needed, loop);

	// Mono is the hot path - every pitched spatial voice lands here - so it
	// gets its own loop without the per-frame channel iteration.
	ma_uint64 out  = 0;
	float     frac = voice->resample_frac;
	if (ch == 1) {
		// Four outputs per pass: each lane loads its own 4-point window, one
		// transpose makes p0..p3 across lanes, the polynomial runs 4-wide.
		// The scalar Horner chain is latency-bound, so this wins ~2x.
		XMVECTOR rate4 = XMVectorSet(0, rate, rate * 2, rate * 3);
		for (; out + 4 <= frames; out += 4) {
			float     base = frac + (float)out * rate;
			ma_uint64 idx3 = (ma_uint64)(base + rate * 3);
			if (idx3 + 2 > src_read) break;

			XMVECTOR p = XMVectorAdd(XMVectorReplicate(base), rate4);
			XMMATRIX m;
			m.r[0] = XMLoadFloat4((XMFLOAT4*)(au_resample_temp + (ma_uint64)(base)            + 1));
			m.r[1] = XMLoadFloat4((XMFLOAT4*)(au_resample_temp + (ma_uint64)(base + rate)     + 1));
			m.r[2] = XMLoadFloat4((XMFLOAT4*)(au_resample_temp + (ma_uint64)(base + rate * 2) + 1));
			m.r[3] = XMLoadFloat4((XMFLOAT4*)(au_resample_temp + idx3                         + 1));
			m = XMMatrixTranspose(m);

			// au_catmull per lane, t = p - floor(p)
			XMVECTOR t    = XMVectorSubtract(p, XMVectorFloor(p));
			XMVECTOR poly = XMVectorSubtract(
				XMVectorAdd(XMVectorScale(m.r[0], 2), XMVectorScale(m.r[2], 4)),
				XMVectorAdd(XMVectorScale(m.r[1], 5), m.r[3]));
			poly = XMVectorMultiplyAdd(t, XMVectorSubtract(XMVectorAdd(XMVectorScale(XMVectorSubtract(m.r[1], m.r[2]), 3), m.r[3]), m.r[0]), poly);
			poly = XMVectorMultiplyAdd(t, poly, XMVectorSubtract(m.r[2], m.r[0]));
			poly = XMVectorMultiply(XMVectorScale(t, 0.5f), poly);
			XMStoreFloat4((XMFLOAT4*)(dest + out), XMVectorAdd(m.r[1], poly));
		}
		for (; out < frames; out++) {
			float     p   = frac + (float)out * rate;
			ma_uint64 idx = (ma_uint64)p;
			if (idx + 2 > src_read) break;
			const float* s = au_resample_temp + idx;
			dest[out] = au_catmull(s[1], s[2], s[3], s[4], p - (float)idx);
		}
	} else {
		for (; out < frames; out++) {
			float     p   = frac + (float)out * rate;
			ma_uint64 idx = (ma_uint64)p;
			if (idx + 2 > src_read) break;
			float t = p - (float)idx;
			for (int32_t c = 0; c < ch; c++) {
				const float* s = au_resample_temp + (idx+1)*ch + c;
				dest[out*ch + c] = au_catmull(s[0], s[ch], s[ch*2], s[ch*3], t);
			}
		}
	}

	float     adv   = frac + (float)out * rate;
	ma_uint64 adv_i = (ma_uint64)adv;
	voice->resample_frac = adv - (float)adv_i;
	if (adv_i <= src_read)
		for (int32_t k = 0; k < 3; k++)
			for (int32_t c = 0; c < ch; c++)
				voice->resample_last[k][c] = au_resample_temp[(adv_i + k)*ch + c];
	voice_fade_apply(voice, dest, out, ch);
	return out;
}

///////////////////////////////////////////

// Audio thread. Per-voice processing: read at pitch, gain from the decibel
// model, air absorption, then a head-locked add, an FOA bus encode, or the
// direct binaural render, all at frame_offset.
static ma_uint32 voice_mix(au_voice_t* voice, pose_t head, float* output, ma_uint32 frame_offset, ma_uint64 frame_count) {
	int32_t flags  = atomic_load_i32(&voice->params.flags);
	float   volume = atomic_load_f32(&voice->params.volume);
	float   pitch  = atomic_load_f32(&voice->params.pitch);
	pitch = pitch <= 0 ? 1 : fminf(AU_PITCH_MAX, fmaxf(AU_PITCH_MIN, pitch));
	bool loop = (flags & sound_flags_loop) != 0;

	// The sound declares real-world loudness at 1m, attenuated -6dB per
	// distance doubling. Normalization makes declared decibels truthful, and
	// volume/bus/master are plain 0-1 trims.
	float decibels  = atomic_load_f32(&voice->sound->decibels);
	float trim_gain = voice->sound->norm_gain * volume
	                * atomic_load_f32(&au_bus_volumes[voice->bus])
	                * atomic_load_f32(&au_master_volume);

	// Source rate and pitch resample through the same interpolator.
	float rate = pitch * ((float)voice->sound->sample_rate / (float)AU_SAMPLE_RATE);

	// Stereo plays head-locked and untouched, position is ignored.
	if (voice->sound->channels == sound_channels_stereo) {
		float     gain = decibels_to_signal(decibels) * trim_gain;
		ma_uint64 read = voice_read(voice, au_mix_temp, frame_count, rate, loop);
		float     peak = 0;
		float*    out  = output + frame_offset * 2;
		for (ma_uint64 i = 0; i < read; i++) {
			float l = au_mix_temp[i*2], r = au_mix_temp[i*2+1];
			if (peak < fabsf(l)) peak = fabsf(l);
			if (peak < fabsf(r)) peak = fabsf(r);
			out[i*2  ] += l*gain;
			out[i*2+1] += r*gain;
		}
		if (peak > atomic_load_f32(&voice->intensity))
			atomic_store_f32(&voice->intensity, peak);
		return (ma_uint32)read;
	}

	// Ambisonic sounds add straight onto the bus, world-fixed - the decode
	// counter-rotates them. Spread fades the directional channels out.
	if (voice->sound->channels == sound_channels_ambisonic1) {
		float     gain      = decibels_to_signal(decibels) * trim_gain;
		float     spread    = fmaxf(0, fminf(1, atomic_load_f32(&voice->params.spread)));
		float     dir_scale = (1.0f - spread) * gain;
		ma_uint64 read      = voice_read(voice, au_mix_temp, frame_count, rate, loop);
		float     peak      = 0;
		XMVECTOR  enc       = XMVectorSet(gain, dir_scale, dir_scale, dir_scale);
		for (ma_uint64 i = 0; i < read; i++) {
			float w = au_mix_temp[i*4];
			if (peak < fabsf(w)) peak = fabsf(w);
			float* bus = au_foa_ring + (((uint32_t)(au_foa_head + frame_offset + i) & au_foa_mask) * 4);
			XMStoreFloat4((XMFLOAT4*)bus, XMVectorMultiplyAdd(
				XMLoadFloat4((XMFLOAT4*)(au_mix_temp + i*4)), enc,
				XMLoadFloat4((XMFLOAT4*)bus)));
		}
		if (read > 0) au_foa_touched = true;
		if (peak > atomic_load_f32(&voice->intensity))
			atomic_store_f32(&voice->intensity, peak);
		return (ma_uint32)read;
	}

	// Head-locked voices skip spatialization entirely: no attenuation, no
	// panning, no filtering, both ears get the sound as-is.
	if (flags & sound_flags_head_locked) {
		float     gain = decibels_to_signal(decibels) * trim_gain;
		ma_uint64 read = voice_read(voice, au_mix_temp, frame_count, rate, loop);
		float     peak = 0;
		float*    out  = output + frame_offset * 2;
		for (ma_uint64 i = 0; i < read; i++) {
			float s = au_mix_temp[i];
			if (peak < fabsf(s)) peak = fabsf(s);
			out[i*2  ] += s*gain;
			out[i*2+1] += s*gain;
		}
		if (peak > atomic_load_f32(&voice->intensity))
			atomic_store_f32(&voice->intensity, peak);
		return (ma_uint32)read;
	}

	vec3   position = {
		atomic_load_f32(&voice->params.pos_x),
		atomic_load_f32(&voice->params.pos_y),
		atomic_load_f32(&voice->params.pos_z) };

	// Distance falloff is amplitude's 1/d, not intensity's 1/d^2 - loudness
	// tracks pressure. The min clamp stands in for "inside your head".
	vec3  dir    = position - head.position;
	float dist2  = vec3_magnitude_sq(dir);
	float dist   = fmaxf(AU_MIN_DISTANCE, sqrtf(dist2));
	float gain   = decibels_to_signal(decibels - 20.0f*log10f(dist)) * trim_gain;

	// Sources at (or inside) the head have no direction, they go diffuse.
	float spread = fmaxf(0, fminf(1, atomic_load_f32(&voice->params.spread)));
	vec3  u      = vec3{0,0,0};
	if (dist2 > 0.0001f) u = dir / sqrtf(dist2);
	else                 spread = 1;

	// Point sources render direct binaural for per-source ITD precision, and
	// crossfade onto the FOA bus as spread widens - width is the bus's job.
	float k_bus = fminf(1.0f, spread / AU_DIRECT_SPREAD);
	if (atomic_load_i32(&au_force_bus)) k_bus = 1;

	// gain*dist strips the distance term back out, keeping the send constant
	// while the source shares the space; past ~half the size the min() lets
	// it parallel the direct falloff so the tail doesn't swell as the source
	// fades. Wide sources are already diffuse and send less.
	float send_gain = au_env.on ? gain * fminf(dist, au_env.send_ref) * (1.0f - spread) : 0;

	// Reflection taps write after the render loops from the block's filtered
	// mono - both branches store it back over the read buffer as they go.
	au_er_taps_t er_taps;
	au_er_setup(&er_taps, dir, dist, decibels, trim_gain, spread, atomic_load_i32(&voice->er_grant));

	ma_uint64 read = voice_read(voice, au_mix_temp, frame_count, rate, loop);

	// Air absorption as four cascaded poles - a single pole's shallow skirt
	// leaks enough far hiss that dense distant fields sound synthetic. A
	// per-voice override replaces the automatic distance model:
	// https://www.desmos.com/calculator/h5tssewqbl
	float override = atomic_load_f32(&voice->params.cutoff);
	float cutoff   = override > 0 ? override
		: fmaxf(1000, -4000.f * logf(fmaxf(1, dist-3.5f)) + 22200);
	// Poles sit above the requested knee, sqrt(2^(1/4)-1), so the cascade's
	// -3dB point lands at `cutoff` with the skirt falling ~24dB/oct.
	float RC    = 0.43496f / (2.0f * 3.14159265359f * cutoff);
	float dt    = 1.0f / AU_SAMPLE_RATE;
	float alpha = dt / (RC + dt);
	// Inside ~4.5m the cutoff model plateaus but the cascade still shaves
	// highs, so the filter fades to a true bypass - close sources keep their
	// sparkle. It keeps running dry, primed for the fade back in.
	float lpf_wet = fminf(1.0f, fmaxf(0.0f, (22200.0f - cutoff) * (1.0f / 2200.0f)));
	float lp0   = voice->lpf_state[0], lp1 = voice->lpf_state[1];
	float lp2   = voice->lpf_state[2], lp3 = voice->lpf_state[3];
	float peak  = 0;

	// Bus encode is in the *world* ambisonic frame, head rotation applies
	// at decode. SK -> ambisonic axes: +X front, +Y left, +Z up.
	float    dir_scale = (1.0f - spread) * k_bus;
	XMVECTOR enc       = XMVectorSet(gain * k_bus, gain * -u.x * dir_scale,
	                                 gain * u.y * dir_scale, gain * -u.z * dir_scale);

	if (k_bus >= 1.0f) {
		for (ma_uint64 i = 0; i < read; i++) {
			float s = au_mix_temp[i];
			if (peak < fabsf(s)) peak = fabsf(s);
			lp0 += alpha * (s   - lp0);
			lp1 += alpha * (lp0 - lp1);
			lp2 += alpha * (lp1 - lp2);
			lp3 += alpha * (lp2 - lp3);
			s += lpf_wet * (lp3 - s);
			if (send_gain > 0) au_env.send[frame_offset+i] += s * send_gain;
			au_mix_temp[i] = s;

			float* bus = au_foa_ring + (((uint32_t)(au_foa_head + frame_offset + i) & au_foa_mask) * 4);
			XMStoreFloat4((XMFLOAT4*)bus, XMVectorMultiplyAdd(XMVectorReplicate(s), enc, XMLoadFloat4((XMFLOAT4*)bus)));
		}
		voice->lpf_state[0] = lp0; voice->lpf_state[1] = lp1;
		voice->lpf_state[2] = lp2; voice->lpf_state[3] = lp3;
		if (read > 0) au_foa_touched = true;
		// Direct state is stale after bus-only rendering, snap on return.
		voice->dir_delay[0] = -1;
	} else if (read > 0) {
		// The ear model works in head-relative space. Ears sit on +-X.
		vec3  uh   = quat_inverse(head.orientation) * u;
		float cosL = -uh.x;
		float cosR =  uh.x;

		// Delays slew across the block so a moving source's ITD sweeps, not
		// steps. The near ear normalizes to zero - onsets stay sample exact.
		float tgt_l = au_ear_delay(cosL);
		float tgt_r = au_ear_delay(cosR);
		float base  = fminf(tgt_l, tgt_r);
		tgt_l -= base;
		tgt_r -= base;
		if (voice->dir_delay[0] < 0) { voice->dir_delay[0] = tgt_l; voice->dir_delay[1] = tgt_r; }
		float delay_l = voice->dir_delay[0], step_l = (tgt_l - delay_l) / (float)read;
		float delay_r = voice->dir_delay[1], step_r = (tgt_r - delay_r) / (float)read;

		// Head shadow on the far side, scaled by how far around it sits.
		float wet_l = AU_SHADOW_WET * fmaxf(0, -cosL);
		float wet_r = AU_SHADOW_WET * fmaxf(0, -cosR);

		// Elevation/back voicing, shared mono before the ear split - the same
		// voicing the decode's speakers carry. Rebuilt each block.
		au_biquad_t vc[AU_VOICING_STAGES];
		au_voicing(uh, vc);

		// Shoulder bounce for sources above the horizon, slewed like the
		// ear delays so a rising source's comb glides.
		float sh_g    = AU_SHOULDER_GAIN * fmaxf(0, uh.y);
		float sh_tgt  = (2.0f * AU_SHOULDER_M / 343.0f) * AU_SAMPLE_RATE * fmaxf(0, uh.y);
		float sh_del  = voice->dir_shoulder < 0 ? sh_tgt : voice->dir_shoulder;
		float sh_step = (sh_tgt - sh_del) / (float)read;

		// Inside ~1m the ears' own path lengths diverge audibly: per-ear gain
		// follows true distance, the broadband near-field ILD a head-centered
		// gain misses - about +-3dB per ear at the minimum distance.
		float    d_l      = sqrtf(dist*dist + AU_HEAD_RADIUS*AU_HEAD_RADIUS - 2.0f*dist*AU_HEAD_RADIUS*cosL);
		float    d_r      = sqrtf(dist*dist + AU_HEAD_RADIUS*AU_HEAD_RADIUS - 2.0f*dist*AU_HEAD_RADIUS*cosR);
		float    g_dir_l  = gain * (1.0f - k_bus) * (dist / d_l);
		float    g_dir_r  = gain * (1.0f - k_bus) * (dist / d_r);
		float    shadow_l = voice->dir_shadow[0];
		float    shadow_r = voice->dir_shadow[1];
		float*   ring     = voice->dir_ring;
		int32_t  ring_at  = voice->dir_ring_at;
		float*   out      = output + frame_offset * 2;
		for (ma_uint64 i = 0; i < read; i++) {
			float s = au_mix_temp[i];
			if (peak < fabsf(s)) peak = fabsf(s);
			lp0 += alpha * (s   - lp0);
			lp1 += alpha * (lp0 - lp1);
			lp2 += alpha * (lp1 - lp2);
			lp3 += alpha * (lp2 - lp3);
			s += lpf_wet * (lp3 - s);
			if (send_gain > 0) au_env.send[frame_offset+i] += s * send_gain;
			au_mix_temp[i] = s;

			if (k_bus > 0) {
				float* bus = au_foa_ring + (((uint32_t)(au_foa_head + frame_offset + i) & au_foa_mask) * 4);
				XMStoreFloat4((XMFLOAT4*)bus, XMVectorMultiplyAdd(XMVectorReplicate(s), enc, XMLoadFloat4((XMFLOAT4*)bus)));
			}

			for (int32_t f = 0; f < AU_VOICING_STAGES; f++)
				s = au_biquad_apply(&vc[f], voice->dir_filter[f], s);
			ring[ring_at] = s;

			// Fractional ear taps - positive-offset masked wrap, matching
			// the batch path - then the far-side shadow one-pole.
			float rp_l = (float)(ring_at + AU_ITD_MAX) - delay_l;
			float rp_r = (float)(ring_at + AU_ITD_MAX) - delay_r;
			int32_t l0 = (int32_t)rp_l, r0 = (int32_t)rp_r;
			float   vl = math_lerp(ring[l0 & (AU_ITD_MAX-1)], ring[(l0+1) & (AU_ITD_MAX-1)], rp_l - (float)l0);
			float   vr = math_lerp(ring[r0 & (AU_ITD_MAX-1)], ring[(r0+1) & (AU_ITD_MAX-1)], rp_r - (float)r0);
			if (sh_g > 0.01f) {
				float   rp_s = (float)(ring_at + AU_ITD_MAX) - sh_del;
				int32_t s0   = (int32_t)rp_s;
				float   vb   = math_lerp(ring[s0 & (AU_ITD_MAX-1)], ring[(s0+1) & (AU_ITD_MAX-1)], rp_s - (float)s0);
				vl += sh_g * vb;
				vr += sh_g * vb;
				sh_del += sh_step;
			}
			shadow_l += 0.178f * (vl - shadow_l);
			shadow_r += 0.178f * (vr - shadow_r);
			vl += wet_l * (shadow_l - vl);
			vr += wet_r * (shadow_r - vr);

			out[i*2  ] += vl * g_dir_l;
			out[i*2+1] += vr * g_dir_r;

			delay_l += step_l;
			delay_r += step_r;
			ring_at  = (ring_at + 1) & (AU_ITD_MAX-1);
		}
		voice->lpf_state[0]  = lp0; voice->lpf_state[1] = lp1;
		voice->lpf_state[2]  = lp2; voice->lpf_state[3] = lp3;
		voice->dir_shoulder  = sh_tgt;
		voice->dir_delay[0]  = tgt_l;
		voice->dir_delay[1]  = tgt_r;
		voice->dir_shadow[0] = shadow_l;
		voice->dir_shadow[1] = shadow_r;
		voice->dir_ring_at   = ring_at;
		if (k_bus > 0) au_foa_touched = true;
	}
	au_er_write(voice, &er_taps, au_mix_temp, frame_offset, (ma_uint32)read);
	if (send_gain > 0 && read > 0) au_env.sent = true;

	// Track the block's peak for the intensity getter. Main zeroes this
	// each frame, a lost reset here just makes the meter sticky one frame.
	if (peak > atomic_load_f32(&voice->intensity))
		atomic_store_f32(&voice->intensity, peak);

	return (ma_uint32)read;
}

///////////////////////////////////////////

// Pure point sources - mono, spatialized, zero spread, outside the head -
// all take the direct binaural path, so they can render 4-wide.
static bool voice_is_direct(const au_voice_t* voice, vec3 listener_pos) {
	if (atomic_load_i32(&au_force_bus))                        return false;
	if (voice->sound->channels != sound_channels_mono)         return false;
	if (atomic_load_i32(&voice->params.flags) & sound_flags_head_locked) return false;
	if (atomic_load_f32(&voice->params.spread) > 0)            return false;
	vec3 d = vec3{
		atomic_load_f32(&voice->params.pos_x),
		atomic_load_f32(&voice->params.pos_y),
		atomic_load_f32(&voice->params.pos_z) } - listener_pos;
	return vec3_magnitude_sq(d) > 0.0001f;
}

// Audio thread. Four pure-direct voices rendered as SIMD lanes across one
// block; only the fractional ear taps stay scalar. Lanes hold lockstep by
// zero-padding outside each voice's live span - filtered zeros are exact
// silence, so onsets and finishes match the scalar path.
static void voice_mix_direct4(au_voice_t** vs, pose_t head, const ma_uint32* offsets, const ma_uint32* blocks, ma_uint32* out_read, float* output, ma_uint32 frame_count) {
	quat qinv = quat_inverse(head.orientation);

	float        gains_l[4], gains_r[4], sends[4], lpws[4], alphas[4], wets_l[4], wets_r[4];
	float        dlys_l[4], dlys_r[4], stps_l[4], stps_r[4], tgts_l[4], tgts_r[4];
	au_biquad_t  vcs[4][AU_VOICING_STAGES];
	float        shg[4], shd[4], shstp[4], shtgt[4];
	au_er_taps_t ers[4];
	float*       srcs[4];
	int32_t      rats[4];

	for (int32_t v = 0; v < 4; v++) {
		au_voice_t* voice  = vs[v];
		float       volume = atomic_load_f32(&voice->params.volume);
		float       pitch  = atomic_load_f32(&voice->params.pitch);
		pitch = pitch <= 0 ? 1 : fminf(AU_PITCH_MAX, fmaxf(AU_PITCH_MIN, pitch));
		bool  loop = (atomic_load_i32(&voice->params.flags) & sound_flags_loop) != 0;
		float rate = pitch * ((float)voice->sound->sample_rate / (float)AU_SAMPLE_RATE);

		float decibels  = atomic_load_f32(&voice->sound->decibels);
		float trim_gain = voice->sound->norm_gain * volume
		                * atomic_load_f32(&au_bus_volumes[voice->bus])
		                * atomic_load_f32(&au_master_volume);

		vec3 position = {
			atomic_load_f32(&voice->params.pos_x),
			atomic_load_f32(&voice->params.pos_y),
			atomic_load_f32(&voice->params.pos_z) };
		vec3  dir   = position - head.position;
		float dist2 = vec3_magnitude_sq(dir);
		float dist  = fmaxf(AU_MIN_DISTANCE, sqrtf(dist2));
		// The classification can go stale for one block if main moves the
		// source into the head mid-frame - render it frontal, not NaN.
		vec3  u     = dist2 > 0.0001f ? dir / sqrtf(dist2) : vec3{0, 0, -1};
		float gain  = decibels_to_signal(decibels - 20.0f*log10f(dist)) * trim_gain;

		float override = atomic_load_f32(&voice->params.cutoff);
		float cutoff   = override > 0 ? override
			: fmaxf(1000, -4000.f * logf(fmaxf(1, dist-3.5f)) + 22200);
		float RC  = 0.43496f / (2.0f * 3.14159265359f * cutoff);
		float dt  = 1.0f / AU_SAMPLE_RATE;
		alphas[v] = dt / (RC + dt);
		lpws[v]   = fminf(1.0f, fmaxf(0.0f, (22200.0f - cutoff) * (1.0f / 2200.0f)));

		vec3  uh   = qinv * u;
		// Near-field per-ear distance gain, matching the scalar path.
		float d_l  = sqrtf(dist*dist + AU_HEAD_RADIUS*AU_HEAD_RADIUS + 2.0f*dist*AU_HEAD_RADIUS*uh.x);
		float d_r  = sqrtf(dist*dist + AU_HEAD_RADIUS*AU_HEAD_RADIUS - 2.0f*dist*AU_HEAD_RADIUS*uh.x);
		gains_l[v] = gain * (dist / d_l);
		gains_r[v] = gain * (dist / d_r);
		sends  [v] = au_env.on ? gain * fminf(dist, au_env.send_ref) : 0; // Direct voices have spread 0
		au_er_setup(&ers[v], dir, dist, decibels, trim_gain, 0, atomic_load_i32(&voice->er_grant));
		float tgt_l = au_ear_delay(-uh.x);
		float tgt_r = au_ear_delay( uh.x);
		float base  = fminf(tgt_l, tgt_r);
		tgt_l -= base;
		tgt_r -= base;
		if (voice->dir_delay[0] < 0) { voice->dir_delay[0] = tgt_l; voice->dir_delay[1] = tgt_r; }
		tgts_l[v] = tgt_l;                  tgts_r[v] = tgt_r;
		dlys_l[v] = voice->dir_delay[0];    dlys_r[v] = voice->dir_delay[1];
		stps_l[v] = (tgt_l - dlys_l[v]) / (float)frame_count;
		stps_r[v] = (tgt_r - dlys_r[v]) / (float)frame_count;
		wets_l[v] = AU_SHADOW_WET * fmaxf(0,  uh.x);
		wets_r[v] = AU_SHADOW_WET * fmaxf(0, -uh.x);

		au_voicing(uh, vcs[v]);
		shg  [v] = AU_SHOULDER_GAIN * fmaxf(0, uh.y);
		shtgt[v] = (2.0f * AU_SHOULDER_M / 343.0f) * AU_SAMPLE_RATE * fmaxf(0, uh.y);
		shd  [v] = voice->dir_shoulder < 0 ? shtgt[v] : voice->dir_shoulder;
		shstp[v] = (shtgt[v] - shd[v]) / (float)frame_count;
		rats[v]    = voice->dir_ring_at;

		// Read into this lane's slice at its onset offset, zeros elsewhere.
		float* src = au_mix_temp + (uint64_t)v * au_mix_temp_size;
		memset(src, 0, sizeof(float) * frame_count);
		out_read[v] = (ma_uint32)voice_read(voice, src + offsets[v], blocks[v], rate, loop);
		srcs[v] = src;
	}

	// Voice state gathered into lanes, [lane] = voice.
	XMVECTOR lp0 = XMVectorSet(vs[0]->lpf_state[0], vs[1]->lpf_state[0], vs[2]->lpf_state[0], vs[3]->lpf_state[0]);
	XMVECTOR lp1 = XMVectorSet(vs[0]->lpf_state[1], vs[1]->lpf_state[1], vs[2]->lpf_state[1], vs[3]->lpf_state[1]);
	XMVECTOR lp2 = XMVectorSet(vs[0]->lpf_state[2], vs[1]->lpf_state[2], vs[2]->lpf_state[2], vs[3]->lpf_state[2]);
	XMVECTOR lp3 = XMVectorSet(vs[0]->lpf_state[3], vs[1]->lpf_state[3], vs[2]->lpf_state[3], vs[3]->lpf_state[3]);
	XMVECTOR vx1[AU_VOICING_STAGES], vx2[AU_VOICING_STAGES];
	XMVECTOR vy1[AU_VOICING_STAGES], vy2[AU_VOICING_STAGES];
	for (int32_t f = 0; f < AU_VOICING_STAGES; f++) {
		vx1[f] = XMVectorSet(vs[0]->dir_filter[f][0], vs[1]->dir_filter[f][0], vs[2]->dir_filter[f][0], vs[3]->dir_filter[f][0]);
		vx2[f] = XMVectorSet(vs[0]->dir_filter[f][1], vs[1]->dir_filter[f][1], vs[2]->dir_filter[f][1], vs[3]->dir_filter[f][1]);
		vy1[f] = XMVectorSet(vs[0]->dir_filter[f][2], vs[1]->dir_filter[f][2], vs[2]->dir_filter[f][2], vs[3]->dir_filter[f][2]);
		vy2[f] = XMVectorSet(vs[0]->dir_filter[f][3], vs[1]->dir_filter[f][3], vs[2]->dir_filter[f][3], vs[3]->dir_filter[f][3]);
	}
	XMVECTOR shl = XMVectorSet(vs[0]->dir_shadow[0], vs[1]->dir_shadow[0], vs[2]->dir_shadow[0], vs[3]->dir_shadow[0]);
	XMVECTOR shr = XMVectorSet(vs[0]->dir_shadow[1], vs[1]->dir_shadow[1], vs[2]->dir_shadow[1], vs[3]->dir_shadow[1]);

	XMVECTOR alpha4 = XMVectorSet(alphas[0], alphas[1], alphas[2], alphas[3]);
	XMVECTOR lpw4   = XMVectorSet(lpws  [0], lpws  [1], lpws  [2], lpws  [3]);
	XMVECTOR gnl4   = XMVectorSet(gains_l[0], gains_l[1], gains_l[2], gains_l[3]);
	XMVECTOR gnr4   = XMVectorSet(gains_r[0], gains_r[1], gains_r[2], gains_r[3]);
	XMVECTOR sg4    = XMVectorSet(sends  [0], sends  [1], sends  [2], sends  [3]);
	XMVECTOR wetl4  = XMVectorSet(wets_l[0], wets_l[1], wets_l[2], wets_l[3]);
	XMVECTOR wetr4  = XMVectorSet(wets_r[0], wets_r[1], wets_r[2], wets_r[3]);
	XMVECTOR vb0[AU_VOICING_STAGES], vb1[AU_VOICING_STAGES], vb2[AU_VOICING_STAGES];
	XMVECTOR va1[AU_VOICING_STAGES], va2[AU_VOICING_STAGES];
	for (int32_t f = 0; f < AU_VOICING_STAGES; f++) {
		vb0[f] = XMVectorSet(vcs[0][f].b0, vcs[1][f].b0, vcs[2][f].b0, vcs[3][f].b0);
		vb1[f] = XMVectorSet(vcs[0][f].b1, vcs[1][f].b1, vcs[2][f].b1, vcs[3][f].b1);
		vb2[f] = XMVectorSet(vcs[0][f].b2, vcs[1][f].b2, vcs[2][f].b2, vcs[3][f].b2);
		va1[f] = XMVectorSet(vcs[0][f].a1, vcs[1][f].a1, vcs[2][f].a1, vcs[3][f].a1);
		va2[f] = XMVectorSet(vcs[0][f].a2, vcs[1][f].a2, vcs[2][f].a2, vcs[3][f].a2);
	}
	XMVECTOR shk   = XMVectorReplicate(0.178f);
	XMVECTOR peak4 = XMVectorZero();
	XMVECTOR ones  = XMVectorSplatOne();
	bool     env_on = au_env.on;

	for (ma_uint32 i = 0; i < frame_count; i++) {
		XMVECTOR s = XMVectorSet(srcs[0][i], srcs[1][i], srcs[2][i], srcs[3][i]);
		peak4 = XMVectorMax(peak4, XMVectorAbs(s));

		// Air absorption cascade, 4 voices per op.
		lp0 = XMVectorMultiplyAdd(alpha4, XMVectorSubtract(s,   lp0), lp0);
		lp1 = XMVectorMultiplyAdd(alpha4, XMVectorSubtract(lp0, lp1), lp1);
		lp2 = XMVectorMultiplyAdd(alpha4, XMVectorSubtract(lp1, lp2), lp2);
		lp3 = XMVectorMultiplyAdd(alpha4, XMVectorSubtract(lp2, lp3), lp3);
		s   = XMVectorMultiplyAdd(lpw4, XMVectorSubtract(lp3, s), s);
		if (env_on) {
			au_env.send[i] += XMVectorGetX(XMVector4Dot(XMVectorMultiply(s, sg4), ones));
			// The reflection post-pass reads the filtered mono back out of
			// the lane slices, scatter it as we go.
			XMFLOAT4A sfe;
			XMStoreFloat4A(&sfe, s);
			srcs[0][i] = sfe.x; srcs[1][i] = sfe.y;
			srcs[2][i] = sfe.z; srcs[3][i] = sfe.w;
		}

		// The voicing chain: N1/N2 notches, P1 peak, then the dull shelf.
		for (int32_t f = 0; f < AU_VOICING_STAGES; f++) {
			XMVECTOR y = XMVectorMultiply(vb0[f], s);
			y = XMVectorMultiplyAdd             (vb1[f], vx1[f], y);
			y = XMVectorMultiplyAdd             (vb2[f], vx2[f], y);
			y = XMVectorNegativeMultiplySubtract(va1[f], vy1[f], y);
			y = XMVectorNegativeMultiplySubtract(va2[f], vy2[f], y);
			vx2[f] = vx1[f]; vx1[f] = s; vy2[f] = vy1[f]; vy1[f] = y; s = y;
		}

		// Per-voice fractional ear taps, the one scalar stage.
		XMFLOAT4A sf, vlf, vrf;
		XMStoreFloat4A(&sf, s);
		float* sfp = (float*)&sf;
		float* vlp = (float*)&vlf;
		float* vrp = (float*)&vrf;
		for (int32_t v = 0; v < 4; v++) {
			float* ring = vs[v]->dir_ring;
			int32_t rat = rats[v];
			ring[rat] = sfp[v];
			// Offset by a full ring so the reads stay positive, wrap by
			// mask - branches here were the mixer's hottest instructions.
			float rp_l = (float)(rat + AU_ITD_MAX) - dlys_l[v];
			float rp_r = (float)(rat + AU_ITD_MAX) - dlys_r[v];
			int32_t l0 = (int32_t)rp_l, r0 = (int32_t)rp_r;
			vlp[v] = math_lerp(ring[l0 & (AU_ITD_MAX-1)], ring[(l0+1) & (AU_ITD_MAX-1)], rp_l - (float)l0);
			vrp[v] = math_lerp(ring[r0 & (AU_ITD_MAX-1)], ring[(r0+1) & (AU_ITD_MAX-1)], rp_r - (float)r0);
			if (shg[v] > 0.01f) {
				float   rp_s = (float)(rat + AU_ITD_MAX) - shd[v];
				int32_t s0   = (int32_t)rp_s;
				float   vb   = math_lerp(ring[s0 & (AU_ITD_MAX-1)], ring[(s0+1) & (AU_ITD_MAX-1)], rp_s - (float)s0);
				vlp[v] += shg[v] * vb;
				vrp[v] += shg[v] * vb;
				shd[v] += shstp[v];
			}
			dlys_l[v] += stps_l[v];
			dlys_r[v] += stps_r[v];
			rats[v] = (rat + 1) & (AU_ITD_MAX-1);
		}

		// Far-side shadow one-pole and gain, then one sum into the output.
		XMVECTOR vl = XMLoadFloat4A(&vlf);
		XMVECTOR vr = XMLoadFloat4A(&vrf);
		shl = XMVectorMultiplyAdd(shk, XMVectorSubtract(vl, shl), shl);
		shr = XMVectorMultiplyAdd(shk, XMVectorSubtract(vr, shr), shr);
		vl  = XMVectorMultiplyAdd(wetl4, XMVectorSubtract(shl, vl), vl);
		vr  = XMVectorMultiplyAdd(wetr4, XMVectorSubtract(shr, vr), vr);
		output[i*2  ] += XMVectorGetX(XMVector4Dot(XMVectorMultiply(vl, gnl4), ones));
		output[i*2+1] += XMVectorGetX(XMVector4Dot(XMVectorMultiply(vr, gnr4), ones));
	}
	if (env_on) {
		// Zero-padded lane regions write silence through the taps, which
		// keeps the filters exact and costs only the pad width.
		for (int32_t v = 0; v < 4; v++)
			au_er_write(vs[v], &ers[v], srcs[v], 0, frame_count);
		au_env.sent = true;
	}

	// Scatter the lane state back to the voices.
	XMFLOAT4A st;
	float*    stp = (float*)&st;
	XMStoreFloat4A(&st, lp0); for (int32_t v = 0; v < 4; v++) vs[v]->lpf_state[0]     = stp[v];
	XMStoreFloat4A(&st, lp1); for (int32_t v = 0; v < 4; v++) vs[v]->lpf_state[1]     = stp[v];
	XMStoreFloat4A(&st, lp2); for (int32_t v = 0; v < 4; v++) vs[v]->lpf_state[2]     = stp[v];
	XMStoreFloat4A(&st, lp3); for (int32_t v = 0; v < 4; v++) vs[v]->lpf_state[3]     = stp[v];
	for (int32_t f = 0; f < AU_VOICING_STAGES; f++) {
		XMStoreFloat4A(&st, vx1[f]); for (int32_t v = 0; v < 4; v++) vs[v]->dir_filter[f][0] = stp[v];
		XMStoreFloat4A(&st, vx2[f]); for (int32_t v = 0; v < 4; v++) vs[v]->dir_filter[f][1] = stp[v];
		XMStoreFloat4A(&st, vy1[f]); for (int32_t v = 0; v < 4; v++) vs[v]->dir_filter[f][2] = stp[v];
		XMStoreFloat4A(&st, vy2[f]); for (int32_t v = 0; v < 4; v++) vs[v]->dir_filter[f][3] = stp[v];
	}
	XMStoreFloat4A(&st, shl); for (int32_t v = 0; v < 4; v++) vs[v]->dir_shadow[0]    = stp[v];
	XMStoreFloat4A(&st, shr); for (int32_t v = 0; v < 4; v++) vs[v]->dir_shadow[1]    = stp[v];
	XMStoreFloat4A(&st, peak4);
	for (int32_t v = 0; v < 4; v++) {
		vs[v]->dir_delay[0]  = tgts_l[v];
		vs[v]->dir_delay[1]  = tgts_r[v];
		vs[v]->dir_shoulder  = shtgt[v];
		vs[v]->dir_ring_at   = rats[v];
		if (stp[v] > atomic_load_f32(&vs[v]->intensity))
			atomic_store_f32(&vs[v]->intensity, stp[v]);
	}
}

static void audio_mix_block(float* output, ma_uint32 frame_count) {
	audio_drain_commands();

	// One listener snapshot for the whole block, so classification, mixing,
	// and the FOA decode all agree even if main republishes mid-block.
	pose_t head = audio_listener_get();

	if (frame_count + AU_SAMPLE_BUFFER_SIZE*2 > au_mix_temp_size) {
		if (!au_mix_truncate_warned) {
			au_mix_truncate_warned = true;
			log_errf("Audio block of %u frames exceeds the mix buffer, truncating!", frame_count);
		}
		frame_count = (ma_uint32)(au_mix_temp_size - AU_SAMPLE_BUFFER_SIZE*2);
	}

	au_foa_touched = false;
	audio_env_begin(frame_count);

	// Pure point sources gather here during the sweep and render 4-wide
	// after it; everything else mixes inline.
	au_voice_t* direct       [AU_VOICE_COUNT];
	ma_uint32   direct_offset[AU_VOICE_COUNT];
	ma_uint32   direct_block [AU_VOICE_COUNT];
	int16_t     direct_slot  [AU_VOICE_COUNT];
	int32_t     direct_count = 0;

	for (int16_t i = 0; i < AU_VOICE_COUNT; i++) {
		au_voice_t* voice = &au_voices[i];
		if (atomic_load_i32_acq(&voice->state) != au_voice_playing)
			continue;

		if (atomic_load_i32(&voice->params.stop_request) != 0) {
			voice_finish(voice, i);
			continue;
		}

		// Seeks only make sense for in-memory sounds, streams read forward.
		uint64_t seek = atomic_exchange_u64(&voice->params.seek_request, AU_SEEK_NONE);
		if (seek != AU_SEEK_NONE && voice->sound->data_type == sound_data_pcm) {
			atomic_store_u64(&voice->cursor, mini(seek, voice->sound->pcm_count));
			voice->resample_frac = 0;
			voice->fade_gain     = 0; // A seek is a mid-waveform step, ramp in
			memset(voice->lpf_state, 0, sizeof(voice->lpf_state));
		}

		if (atomic_load_i32(&voice->params.paused) != 0)
			continue;

		// Onset delay is sample accurate: a partial delay starts the voice
		// mid-block at a frame offset. It ticks even for dormant voices, a
		// sound in flight arrives on time whether or not it won mix budget.
		ma_uint32 offset = 0;
		ma_uint32 block  = frame_count;
		if (voice->delay_left >= block) {
			voice->delay_left -= block;
			continue;
		}
		if (voice->delay_left > 0) {
			offset = (ma_uint32)voice->delay_left;
			block -= offset;
			voice->delay_left = 0;
		}

		// Dormant voices ranked out of the mix budget freeze in place:
		// no read, no cursor movement, ready to resume.
		if (atomic_load_i32(&voice->audible) == 0)
			continue;

		if (voice_is_direct(voice, head.position)) {
			direct       [direct_count] = voice;
			direct_offset[direct_count] = offset;
			direct_block [direct_count] = block;
			direct_slot  [direct_count] = i;
			direct_count += 1;
			continue;
		}

		ma_uint32 mixed = voice_mix(voice, head, output, offset, block);
		if (mixed < block && voice->sound->data_type != sound_data_ring)
			voice_finish(voice, i);
	}

	// Full groups of 4 render batched, the remainder takes the scalar path.
	int32_t b = 0;
	for (; b + 4 <= direct_count; b += 4) {
		ma_uint32 reads[4];
		voice_mix_direct4(&direct[b], head, &direct_offset[b], &direct_block[b], reads, output, frame_count);
		for (int32_t v = 0; v < 4; v++) {
			if (reads[v] < direct_block[b+v] && direct[b+v]->sound->data_type != sound_data_ring)
				voice_finish(direct[b+v], direct_slot[b+v]);
		}
	}
	for (; b < direct_count; b++) {
		ma_uint32 mixed = voice_mix(direct[b], head, output, direct_offset[b], direct_block[b]);
		if (mixed < direct_block[b] && direct[b]->sound->data_type != sound_data_ring)
			voice_finish(direct[b], direct_slot[b]);
	}

	audio_env_mix(frame_count);

	// An untouched bus still decodes briefly - filters ring down, reflections
	// may sit ahead of the head - then turns off until the next spatial voice.
	if (au_foa_touched) au_foa_tail = maxi(au_foa_tail, au_foa_future + AU_SAMPLE_RATE / 10);
	if (au_foa_tail > 0) {
		au_foa_tail -= mini((int32_t)frame_count, au_foa_tail);

		// Pull the consumed window into the linear scratch, zeroing behind -
		// the head only advances on consumption, idle blocks leave it alone.
		for (ma_uint32 i = 0; i < frame_count; i++) {
			uint32_t idx = ((au_foa_head + i) & au_foa_mask) * 4;
			memcpy(au_foa + i*4, au_foa_ring + idx, sizeof(float) * 4);
			memset(au_foa_ring + idx, 0, sizeof(float) * 4);
		}
		au_foa_head   = (au_foa_head + frame_count) & au_foa_mask;
		au_foa_future = au_foa_future > (int32_t)frame_count ? au_foa_future - (int32_t)frame_count : 0;
		audio_decode_foa(output, frame_count, head);
	}

	// The limiter replaces per-voice clamping: transparent until -1dBFS, then
	// a smooth squash instead of hard clipping. The meter reads post-limit.
	double sum = 0;
	for (ma_uint32 i = 0; i < frame_count*2; i++) {
		output[i] = au_limit(output[i]);
		sum += (double)output[i] * output[i];
	}
	float rms = frame_count > 0 ? sqrtf((float)(sum / (frame_count*2))) : 0;
	atomic_store_f32(&au_output_dbfs, rms <= 0.000001f ? -120.0f : 20.0f*log10f(rms));
}

///////////////////////////////////////////

void data_callback(ma_device*, void* output, const void*, ma_uint32 frame_count) {
	audio_mix_block((float*)output, frame_count);
}

void audio_render_block(float* out_stereo, int32_t frame_count) {
	memset(out_stereo, 0, (size_t)frame_count * 2 * sizeof(float));
	audio_mix_block(out_stereo, (ma_uint32)frame_count);
}

void audio_test_offline(bool32_t enable) {
	au_offline = enable;
}

void audio_test_advance(float seconds) {
	au_main_clock += seconds;
}

// The main-thread half of a frame for offline tests. The listener stays at
// identity unless steered via audio_set_listener; fixed dt for determinism,
// which also makes deferred-play catch-up timing exact in tests.
void audio_test_step() {
	au_main_clock += 0.016;
	pose_t pose = au_listener_has_override ? au_listener_override : pose_identity;
	audio_listener_publish(pose);
	audio_mix_drain_returns();
	sound_play_pending_step();
	audio_voice_prefetch();
	audio_voice_shapes_step(pose.position, 0.016f);
	audio_voice_rank();

	for (int32_t i = 0; i < AU_VOICE_COUNT; i++) {
		au_voices[i].intensity_frame = atomic_load_f32(&au_voices[i].intensity);
		atomic_store_f32(&au_voices[i].intensity, 0);
	}
}

///////////////////////////////////////////
// Shaped emitters. All main-thread: the shape lives in main-owned voice
// fields, gets evaluated once per frame, and the audio thread only sees
// the resolved position + spread params.

// Closest point on the shape's core to `from`. One point is a sphere
// center, more make a polyline.
static vec3 shape_closest(const vec3* points, int32_t count, vec3 from) {
	if (count == 1) return points[0];

	vec3  best      = points[0];
	float best_dist = 1e30f;
	for (int32_t i = 0; i < count - 1; i++) {
		vec3  seg   = points[i+1] - points[i];
		float len2  = vec3_magnitude_sq(seg);
		float t     = len2 > 0 ? vec3_dot(from - points[i], seg) / len2 : 0;
		t = fmaxf(0, fminf(1, t));
		vec3  at    = points[i] + seg * t;
		float dist2 = vec3_magnitude_sq(from - at);
		if (dist2 < best_dist) { best_dist = dist2; best = at; }
	}
	return best;
}

// Emit position is the closest point on the shape's surface, apparent size
// how much of the view it fills - fully diffuse inside. Both are smoothed so
// polyline corners and equidistant flips glide instead of popping.
static void voice_shape_eval(au_voice_t* voice, vec3 listener_pos, float dt) {
	vec3  closest = shape_closest(voice->shape_points, voice->shape_count, listener_pos);
	float dist    = vec3_magnitude(listener_pos - closest);

	// Subtended half angle of the tube, 90deg (asin(1)) once inside.
	float alpha  = asinf(fminf(1, voice->shape_radius / fmaxf(dist, 0.0001f)));
	float spread = alpha / (3.14159265f * 0.5f);

	// A polyline that stretches past both ends of view is wide no matter
	// how far its closest point is.
	if (voice->shape_count >= 2) {
		vec3 to_a = voice->shape_points[0]                      - listener_pos;
		vec3 to_b = voice->shape_points[voice->shape_count - 1] - listener_pos;
		float la = vec3_magnitude(to_a), lb = vec3_magnitude(to_b);
		if (la > 0.0001f && lb > 0.0001f) {
			float ends = acosf(fmaxf(-1, fminf(1, vec3_dot(to_a, to_b) / (la * lb)))) / 3.14159265f;
			spread = fmaxf(spread, ends);
		}
	}
	spread = fmaxf(spread, voice->base_spread);

	// Emit from the surface, not the core: attenuation then tracks distance
	// to the surface, so growing a shape's radius never reads as the sound
	// backing away. At the boundary the surface point reaches the listener,
	// meeting the fully diffuse inside state without a jump in volume,
	// position, or spread (the asin hits 1 exactly there).
	vec3 emit = closest;
	if (dist < voice->shape_radius) {
		emit   = listener_pos;
		spread = 1;
	} else if (voice->shape_radius > 0) {
		emit = closest + (listener_pos - closest) * (voice->shape_radius / dist);
	}

	// Smoothing runs on the listener-relative offset, not the world point:
	// shape-side flips still glide, but a listener jump (teleport) carries
	// the emitter along instantly instead of dragging it behind the head.
	vec3 offset = emit - listener_pos;
	if (!voice->smooth_init) {
		voice->smooth_init   = true;
		voice->smooth_offset = offset;
		voice->smooth_spread = spread;
	} else {
		float blend = 1.0f - expf(-dt / AU_SMOOTH_TIME);
		voice->smooth_offset = vec3_lerp(voice->smooth_offset, offset, blend);
		voice->smooth_spread = math_lerp(voice->smooth_spread, spread, blend);
	}

	vec3 pos = listener_pos + voice->smooth_offset;
	atomic_store_f32(&voice->params.pos_x,  pos.x);
	atomic_store_f32(&voice->params.pos_y,  pos.y);
	atomic_store_f32(&voice->params.pos_z,  pos.z);
	atomic_store_f32(&voice->params.spread, voice->smooth_spread);
}

void audio_voice_shape_set(au_voice_t* voice, const vec3* points, int32_t count, float radius, vec3 listener_pos) {
	if (count > AU_SHAPE_MAX_POINTS) {
		log_errf("Audio emitter shapes are limited to %d points, truncating %d!", AU_SHAPE_MAX_POINTS, count);
		count = AU_SHAPE_MAX_POINTS;
	}
	memcpy(voice->shape_points, points, sizeof(vec3) * count);
	voice->shape_count  = count;
	voice->shape_radius = radius;
	voice->smooth_init  = false;
	voice_shape_eval(voice, listener_pos, 0);
}

// Main thread, once per frame.
void audio_voice_shapes_step(vec3 listener_pos, float dt) {
	for (int16_t i = 0; i < AU_VOICE_COUNT; i++) {
		au_voice_t* voice = &au_voices[i];
		if (voice->shape_count == 0) continue;
		int32_t state = atomic_load_i32(&voice->state);
		if (state != au_voice_reserved && state != au_voice_playing) continue;
		voice_shape_eval(voice, listener_pos, dt);
	}
}

///////////////////////////////////////////

// Main thread, strictly after the return drain: top up streaming prefetch
// rings. The decoder is main-owned, audio only consumes the ring; a voice
// finishing concurrently isn't freed until the *next* frame's drain.
void audio_voice_prefetch() {
	for (int16_t i = 0; i < AU_VOICE_COUNT; i++) {
		au_voice_t* voice = &au_voices[i];
		if (atomic_load_i32_acq(&voice->state) != au_voice_playing ||
		    voice->stream_decoder == nullptr ||
		    atomic_load_i32(&voice->stream_eof) != 0)
			continue;

		while (ma_pcm_rb_available_write(voice->stream_ring) > AU_STREAM_PREFETCH / 4) {
			ma_uint32 request = ma_pcm_rb_available_write(voice->stream_ring);
			void*     into    = nullptr;
			if (ma_pcm_rb_acquire_write(voice->stream_ring, &request, &into) != MA_SUCCESS || request == 0)
				break;
			ma_uint64 decoded = 0;
			ma_result result  = ma_decoder_read_pcm_frames(voice->stream_decoder, into, request, &decoded);
			if (voice->sound->fuma)
				sound_fuma_to_ambix((float*)into, decoded);
			ma_pcm_rb_commit_write(voice->stream_ring, (ma_uint32)decoded);
			if (result != MA_SUCCESS || decoded < request) {
				// Looping streams rewind the decoder and keep filling, the
				// ring never sees an eof.
				if ((atomic_load_i32(&voice->params.flags) & sound_flags_loop) != 0 &&
				    ma_decoder_seek_to_pcm_frame(voice->stream_decoder, 0) == MA_SUCCESS)
					continue;
				atomic_store_i32_rel(&voice->stream_eof, 1);
				break;
			}
		}
	}
}

///////////////////////////////////////////

// Main thread, device already stopped: release everything inline. Queued
// plays activate first so each voice's resources free once, through one path.
void audio_mix_shutdown() {
	audio_drain_commands(); // No audio thread anymore, safe to run here
	audio_mix_drain_returns();
	for (int16_t i = 0; i < AU_VOICE_COUNT; i++) {
		au_voice_t* voice = &au_voices[i];
		// Covers playing voices, and freed slots still holding a displaced
		// play's resources after a steal's submit was refused.
		if (voice->sound != nullptr)
			voice_free_resources(voice->sound, voice->stream_decoder, voice->stream_ring, voice->stream_ring_data);
		// Plays still waiting on their sound's decode hold only a ref.
		if (voice->pending_sound != nullptr)
			sound_release(voice->pending_sound);
		memset(voice, 0, sizeof(au_voice_t));
	}
	au_cmd_head = au_cmd_tail = 0;
	au_ret_head = au_ret_tail = 0;
	sk_free(au_mix_temp);
	au_mix_temp      = nullptr;
	au_mix_temp_size = 0;
	sk_free(au_resample_temp);
	au_resample_temp      = nullptr;
	au_resample_temp_size = 0;
	sk_free(au_foa);
	au_foa = nullptr;
	sk_free(au_foa_ring);
	au_foa_ring   = nullptr;
	au_foa_mask   = 0;
	au_foa_head   = 0;
	au_foa_future = 0;
	sk_free(au_env.lines);
	sk_free(au_env.send);
	sk_free(au_env.er_temp);
	memset(&au_env,    0, sizeof(au_env));
	memset(&au_decode, 0, sizeof(au_decode));
	au_mix_truncate_warned = false;
	atomic_store_f32(&au_output_dbfs, -120);
}

} // namespace sk

#pragma once

#include "../stereokit.h"

namespace sk {

typedef enum xr_top_level_ {
	xr_top_level_none,
	xr_top_level_head,
	xr_top_level_hand_left,
	xr_top_level_hand_right,
	xr_top_level_gamepad,
	xr_top_level_eyes,
	xr_top_level_max
} xr_top_level_;

typedef enum xra_type_ {
	xra_type_pose,
	xra_type_float,
	xra_type_bool,
	xra_type_xy,
	xra_type_haptic,

	xra_type_max
} xr_action_type_;

typedef struct xr_interaction_bind_t {
	uint16_t    xra_type;
	uint16_t    xra_type_val;
	const char* path;
} xr_interaction_bind_t;

typedef struct xr_interaction_profile_t {
	const char*           name;
	const char*           top_level_path;
	xr_interaction_bind_t binding[32];
	int32_t               binding_ct;
	pose_t                palm_offset;
	bool                  is_hand;
} xr_interaction_profile_t;

void oxri_register_profile(xr_interaction_profile_t profile);

void oxri_register    ();
void oxri_update_poses();

input_haptic_caps_ oxri_haptic_caps         (input_haptic_ haptic_type);
float              oxri_haptic_preferred_rate(input_haptic_ haptic_type);
void               oxri_haptic_pulse        (input_haptic_ haptic_type, float frequency, float amplitude, double duration_seconds);
void               oxri_haptic_waveform     (input_haptic_ haptic_type, const float* samples,    int32_t sample_count, float sample_rate_hz, bool append, int32_t* out_prev_consumed);
void               oxri_haptic_curve        (input_haptic_ haptic_type, const float* amplitudes, int32_t sample_count, float sample_rate_hz);
void               oxri_haptic_stop         (input_haptic_ haptic_type);
void               oxri_haptic_step         ();

} // namespace sk

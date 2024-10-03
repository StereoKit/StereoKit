/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../stereokit.h"

namespace sk {

#define AU_SAMPLE_RATE        48000
#define AU_SAMPLE_BUFFER_SIZE 10
#define AU_SAMPLE_FORMAT      ma_format_f32

struct _sound_inst_t {
	sound_t  sound;
	uint16_t id;
	vec3     position;
	float    volume;
	float    prev_buffer[AU_SAMPLE_BUFFER_SIZE*2];
	int32_t  prev_buffer_ct;
	int32_t  prev_offset[2];
	float    intensity_max_frame;
	float    intensity_max_last_read;
};

bool           audio_init          ();
void           audio_step          ();
void           audio_shutdown      ();

void           audio_pause         ();
void           audio_resume        ();

_sound_inst_t* audio_get_inst_slot (int32_t idx);
_sound_inst_t* audio_get_inst_gen  (int32_t idx, uint16_t generation);
int32_t        audio_get_inst_count();

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
void audio_set_default_device_in (const wchar_t *id);
void audio_set_default_device_out(const wchar_t *id);
#endif

} // namespace sk
/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#pragma once

namespace sk {

enum input_render_mode_ {
	input_render_none,
	input_render_hand_fallback,
	input_render_hand_ext,
	input_render_controller_model,
};

bool input_render_init     ();
void input_render_shutdown ();
void input_render_step     ();
void input_render_step_late();

void input_set_input_render(input_render_mode_ mode);

} // namespace sk
// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#pragma once

#include "../stereokit.h"
#include "../libraries/array.h"

namespace sk {

struct render_stats_t {
	int swaps_mesh;
	int swaps_texture;
	int swaps_material;
	int draw_calls;
	int draw_instances;
};

bool          render_init                 ();
void          render_step                 ();
void          render_shutdown             ();

matrix        render_get_projection_matrix();
float         render_get_ortho_view_height();
pose_t        render_cam_final_transform  (pose_t local_space);
matrix        render_get_cam_final        ();
matrix        render_get_cam_final_inv    ();
color128      render_get_clear_color_ln   ();
vec2          render_get_clip             ();
void          render_clear                ();
vec3          render_unproject_pt         (vec3 normalized_screen_pt);
void          render_update_projection    ();
const char   *render_fmt_name             (tex_format_ format);
tex_format_   render_preferred_depth_fmt  ();
void          render_blit_to_bound        (material_t material);
void          render_set_sim_origin       (pose_t pose);
void          render_set_sim_head         (pose_t pose);
void          render_draw_queue           (render_list_t list, const matrix* views, const matrix* projections, int32_t eye_offset, int32_t view_count, int32_t inst_multiplier, render_layer_ filter, int32_t material_variant);
void          render_check_screenshots    ();
void          render_check_pending_skytex ();
void          render_global_buffer_internal (int32_t register_slot, material_buffer_t buffer);
void          render_global_texture_internal(int32_t register_slot, tex_t             texture);
void          render_action_list_execute  ();

void          render_list_destroy         (      render_list_t list);
void          render_list_execute         (      render_list_t list, render_layer_ filter, int32_t material_variant, uint32_t inst_multiplier, int32_t queue_start, int32_t queue_end);

} // namespace sk
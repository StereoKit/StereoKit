/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "../stereokit.h"
#include "input.h"
#include "input_render.h"
#include "../hands/input_hand.h"
#include "../libraries/array.h"
#include "../xr_backends/openxr.h"
#include "../xr_backends/extensions/hand_mesh.h"
#include "../xr_backends/extensions/ext_interaction_render_model.h"
#include "../systems/defaults.h"

namespace sk {

///////////////////////////////////////////

struct input_event_t {
	input_source_ source;
	button_state_ event;
	void (*event_callback)(input_source_ source, button_state_ evt, const pointer_t &pointer);
};

struct input_render_state_t {
	input_render_mode_ render_mode;
	hand_mesh_t        hand_fallback_mesh   [2];
	hand_mesh_t        hand_articulated_mesh[2];
	material_t         hand_material        [2];
	model_t            controller_model     [2];
};
static input_render_state_t local = {};

///////////////////////////////////////////

void input_hand_update_fallback_mesh(handed_ handed, hand_mesh_t* hand_mesh);

///////////////////////////////////////////

bool input_render_init() {
	local = {};
	local.render_mode = input_render_hand_fallback;

	material_t hand_mat = material_copy_id(default_id_material);
	material_set_id          (hand_mat, default_id_material_hand);
	material_set_transparency(hand_mat, transparency_blend);

	gradient_t color_grad = gradient_create();
	gradient_add(color_grad, color128{ .4f,.4f,.4f,0 }, 0.0f);
	gradient_add(color_grad, color128{ .6f,.6f,.6f,0 }, 0.55f);
	gradient_add(color_grad, color128{ .8f,.8f,.8f,1 }, 0.7f);
	gradient_add(color_grad, color128{ 1,  1,  1,  1 }, 1.0f);

	color32 gradient[16 * 16];
	for (int32_t y = 0; y < 16; y++) {
		color32 col = gradient_get32(color_grad, 1-y/15.f);
		for (int32_t x = 0; x < 16; x++) {
			gradient[x + y * 16] = col;
		}
	}
	gradient_destroy(color_grad);

	tex_t gradient_tex = tex_create(tex_type_image, tex_format_rgba32_linear);
	tex_set_colors (gradient_tex, 16, 16, gradient);
	tex_set_address(gradient_tex, tex_address_clamp);
	material_set_texture     (hand_mat, "diffuse", gradient_tex);
	material_set_queue_offset(hand_mat, 10);

	input_hand_material(handed_max, hand_mat);

	tex_release(gradient_tex);
	material_release(hand_mat);

	for (int32_t i = 0; i < handed_max; i++) {
		// Set up the hand mesh
		local.hand_fallback_mesh[i].root_transform = matrix_identity;
		local.hand_fallback_mesh[i].mesh           = mesh_create();
		mesh_set_keep_data(local.hand_fallback_mesh[i].mesh, false);
		mesh_set_id       (local.hand_fallback_mesh[i].mesh, i == handed_left
			? default_id_mesh_lefthand
			: default_id_mesh_righthand);
	}

	return true;
}

///////////////////////////////////////////

void input_render_shutdown() {
	for (int32_t i = 0; i < handed_max; i++) {
		model_release(local.controller_model[i]);
		material_release(local.hand_material[i]);
		mesh_release    (local.hand_fallback_mesh[i].mesh);
		sk_free(local.hand_fallback_mesh[i].inds);
		sk_free(local.hand_fallback_mesh[i].verts);
		mesh_release(local.hand_articulated_mesh[i].mesh);
		sk_free(local.hand_articulated_mesh[i].inds);
		sk_free(local.hand_articulated_mesh[i].verts);
	}
	local = {};
}

///////////////////////////////////////////

void input_render_step() {
}

///////////////////////////////////////////

void input_render_step_late() {
	// Don't draw the input if the app isn't in focus, this is a Quest store
	// requirement! It generally seems like the correct choice when an overlay
	// may be overtop the app, rendering input of its own.
	if (sk_app_focus() != app_focus_active) return;

	for (int32_t i = 0; i < handed_max; i++) {
		if (input_hand_get_visible((handed_)i) == false) continue;

		hand_source_ source = input_hand_source((handed_)i);
		if (source == hand_source_articulated && input_controller_is_hand((handed_)i)) {
			const hand_t* hand = input_hand((handed_)i);
			if ((hand->tracked_state & button_state_active) != 0 && local.hand_material[i] != nullptr) {
				hand_mesh_t* hand_active_mesh = &local.hand_fallback_mesh[i];
				if (xr_ext_msft_hand_mesh_available()) { xr_ext_msft_hand_mesh_update_mesh((handed_)i, &local.hand_articulated_mesh  [i]); hand_active_mesh = &local.hand_articulated_mesh[i]; }
				else                                   { input_hand_update_fallback_mesh  ((handed_)i, &local.hand_fallback_mesh     [i]); hand_active_mesh = &local.hand_fallback_mesh   [i]; }

				render_add_mesh(hand_active_mesh->mesh, local.hand_material[i], hand_active_mesh->root_transform, hand->pinch_state & button_state_active ? color128{ 1.5f, 1.5f, 1.5f, 1 } : color128{ 1,1,1,1 });
			} 
			/// Here we could draw the detached controller
			if (input_controller_is_detached((handed_)i)) {
				pose_t detached_pose = input_controller_detached((handed_)i);
				if (local.controller_model[i] != nullptr) {
					// If the user has explicitly assigned a model
					render_add_model(local.controller_model[i], matrix_trs(detached_pose.position, detached_pose.orientation));
				} else if (xr_ext_interaction_render_model_available()) {
					// If OpenXR has a model for the controller
					xr_ext_interaction_render_model_draw_controller((handed_)i);
				} else {
					// Otherwise, our built-in backup models
					render_add_model(i == handed_left ? sk_default_controller_l : sk_default_controller_r, matrix_trs(detached_pose.position, detached_pose.orientation));
				}			
			}
		} else if (source == hand_source_simulated || source == hand_source_articulated ) {
			const controller_t* control = input_controller((handed_)i);
			if ((control->tracked & button_state_active) != 0) {
				if (local.controller_model[i] != nullptr) {
					// If the user has explicitly assigned a model
					render_add_model(local.controller_model[i], matrix_trs(control->pose.position, control->pose.orientation));
				} else if (xr_ext_interaction_render_model_available()) {
					// If OpenXR has a model for the controller
					xr_ext_interaction_render_model_draw_controller((handed_)i);
				} else {
					// Otherwise, our built-in backup models
					render_add_model(i == handed_left ? sk_default_controller_l : sk_default_controller_r, matrix_trs(control->pose.position, control->pose.orientation));
				}
			} 
		} else if (source == hand_source_overridden) {
			const hand_t* hand = input_hand((handed_)i);
			if ((hand->tracked_state & button_state_active) != 0 && local.hand_material[i] != nullptr) {
				input_hand_update_fallback_mesh((handed_)i, &local.hand_fallback_mesh[i]);

				render_add_mesh(local.hand_fallback_mesh[i].mesh, local.hand_material[i], local.hand_fallback_mesh[i].root_transform, hand->pinch_state & button_state_active ? color128{ 1.5f, 1.5f, 1.5f, 1 } : color128{ 1,1,1,1 });
			}
		}
	}

	xr_ext_interaction_render_model_draw_others();
}

///////////////////////////////////////////

void input_hand_update_fallback_mesh(handed_ handed, hand_mesh_t* hand_mesh) {
	input_gen_fallback_mesh(input_hand(handed)->fingers, hand_mesh->mesh, &hand_mesh->verts, &hand_mesh->inds);
}

///////////////////////////////////////////

void input_hand_material(handed_ hand, material_t material) {
	if (hand == handed_max) {
		input_hand_material(handed_left,  material);
		input_hand_material(handed_right, material);
		return;
	}

	if (material != nullptr)
		material_addref(material);
	material_release(local.hand_material[hand]);

	local.hand_material[hand] = material;
}

///////////////////////////////////////////

void input_controller_model_set(handed_ hand, model_t model) {
	if (hand == handed_max) {
		input_controller_model_set(handed_left,  model);
		input_controller_model_set(handed_right, model);
		return;
	}

	if (model)
		model_addref(model);
	model_release(local.controller_model[hand]);

	local.controller_model[hand] = model;
}

///////////////////////////////////////////

model_t input_controller_model_get(handed_ hand) {
	model_t result = local.controller_model[hand];
	if (result == nullptr && xr_ext_interaction_render_model_available()) result = xr_ext_interaction_render_model_get(hand);
	if (result == nullptr)                                                result = hand == handed_left ? sk_default_controller_l : sk_default_controller_r;
	
	model_addref(result);
	return result;
}

} // namespace sk
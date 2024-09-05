/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "ui_layout.h"
#include "ui_theming.h"

#include "../libraries/array.h"
#include "../sk_math.h"

///////////////////////////////////////////

namespace sk {


array_t<ui_window_t> skui_windows     = {};
array_t<ui_layout_t> skui_layouts     = {};
array_t<ui_pad_>     skui_panel_stack = {};
ui_settings_t        skui_settings    = {};
bounds_t             skui_recent_layout;

///////////////////////////////////////////

void ui_layout_init() {
	skui_windows       = {};
	skui_layouts       = {};
	skui_panel_stack   = {};
	skui_settings      = {};
	skui_recent_layout = {};
}

///////////////////////////////////////////

void ui_layout_shutdown() {
	skui_windows    .free();
	skui_layouts    .free();
	skui_panel_stack.free();
}

///////////////////////////////////////////

inline bool ui_layout_is_auto_width (const ui_layout_t* layout) { return layout->size.x == 0; }
inline bool ui_layout_is_auto_height(const ui_layout_t* layout) { return layout->size.y == 0; }

///////////////////////////////////////////

float _ui_get_right_x(const ui_layout_t* layout) {
	float right = 0;
	if      (layout->size.x != 0)                                              right = layout->offset_initial.x - layout->size.x;
	else if (layout->parent != -1 && skui_layouts[layout->parent].size.x != 0) right = skui_layouts[layout->parent].offset_initial.x - skui_layouts[layout->parent].size.x;
	else if (layout->window != -1)                                             right = skui_windows[layout->window].layout_start  .x - (skui_windows[layout->window].prev_size.x - layout->margin);
	else if (layout->parent != -1)                                             right = _ui_get_right_x(&skui_layouts[layout->parent]);
	return right;
}

float _ui_get_bottom_y(const ui_layout_t* layout) {
	float bottom = 0;
	if      (layout->size.y != 0)                                              bottom = layout->offset_initial.y - layout->size.y;
	else if (layout->parent != -1 && skui_layouts[layout->parent].size.y != 0) bottom = skui_layouts[layout->parent].offset_initial.y - skui_layouts[layout->parent].size.y;
	else if (layout->window != -1)                                             bottom = skui_windows[layout->window].layout_start  .y - (skui_windows[layout->window].prev_size.y - layout->margin);
	else if (layout->parent != -1)                                             bottom = _ui_get_bottom_y(&skui_layouts[layout->parent]);
	return bottom;
}

vec2 ui_layout_remaining() {
	ui_layout_t* layout = &skui_layouts.last();
	vec2 max = {
		_ui_get_right_x (layout),
		_ui_get_bottom_y(layout)};

	return vec2 {
		fmaxf(0, layout->offset.x - max.x),
		fmaxf(0, layout->offset.y - max.y)
	};
}

///////////////////////////////////////////

vec3 ui_layout_at() {
	return skui_layouts.last().offset;
}

///////////////////////////////////////////

bounds_t ui_layout_last() {
	return skui_recent_layout;
}

///////////////////////////////////////////

void ui_override_recent_layout(vec3 start, vec2 size) {
	skui_recent_layout = {
		{start.x - size.x/2, start.y - size.y/2, 0},
		{size.x, size.y, 0}
	};
}

///////////////////////////////////////////

void ui_layout_reserve_vertical_sz(vec2 size, bool32_t add_padding, vec3* out_position, vec2* out_size) {
	if (size.x == 0) size.x = ui_line_height();
	if (size.y == 0) size.y = ui_layout_remaining().y - (add_padding ? skui_settings.padding * 2 : 0);
	ui_layout_reserve_sz(size, add_padding, out_position, out_size);
}

///////////////////////////////////////////

void ui_layout_reserve_sz(vec2 size, bool32_t add_padding, vec3 *out_position, vec2 *out_size) {
	if (size.x == 0) size.x = ui_layout_remaining().x - (add_padding ? skui_settings.padding*2 : 0);
	if (size.y == 0) size.y = ui_line_height();

	ui_layout_t *layout = &skui_layouts.last();

	vec3 final_pos  = layout->offset;
	vec2 final_size = add_padding
		? size + vec2{ skui_settings.padding, skui_settings.padding }*2
		: size;

	// If this is not the first element, and it goes outside the active window,
	// then we'll want to start this element on the next line down
	// 
	// Floating point rounding can cause this to occasionally round past the
	// end of the layout, so 0.0001f is meant to forgive a bit of rounding.
	if (final_pos.x    != layout->offset_initial.x &&
		layout->size.x != 0                        &&
		final_pos.x - final_size.x < layout->offset_initial.x - (layout->size.x + 0.0001f))
	{
		ui_nextline();
		final_pos = layout->offset;
	}

	// Track the sizes for this line, for ui_layout_remaining, as well as
	// window auto-sizing.
	layout->line.x += final_size.x;
	if (layout->line.y < final_size.y)
		layout->line.y = final_size.y;
	if (layout->furthest.x > final_pos.x - final_size.x)
		layout->furthest.x = final_pos.x - final_size.x;
	if (layout->furthest.y > final_pos.y - final_size.y)
		layout->furthest.y = final_pos.y - final_size.y;
	layout->line.x += skui_settings.gutter;

	// Advance the UI layout position
	layout->offset -= vec3{ final_size.x + skui_settings.gutter, 0, 0 };

	ui_nextline();

	// Store this as the most recent layout
	skui_recent_layout = {
		final_pos - vec3{final_size.x/2, final_size.y/2, 0},
		{final_size.x, final_size.y, 0}
	};

	if (out_position) *out_position = skui_recent_layout.center + skui_recent_layout.dimensions/2;
	if (out_size)     *out_size     = final_size;
}

///////////////////////////////////////////

bounds_t ui_layout_reserve(vec2 size, bool32_t add_padding, float depth) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, add_padding, &final_pos, &final_size);

	bounds_t result = skui_recent_layout;
	result.center.z -= depth/2;
	result.dimensions.z = depth;
	return result;
}

///////////////////////////////////////////

void ui_layout_area(vec3 start, vec2 dimensions, bool32_t add_margin) {
	vec3 margin_start = start;
	if (add_margin) {
		margin_start -= vec3{ skui_settings.margin, skui_settings.margin, 0 };
		if (dimensions.x != 0) dimensions.x -= skui_settings.margin * 2;
		if (dimensions.y != 0) dimensions.y -= skui_settings.margin * 2;
	}
	ui_layout_t* layout = &skui_layouts.last();
	layout->margin           = add_margin ? skui_settings.margin : 0;
	layout->offset_initial   = margin_start;
	layout->offset_prev      = margin_start;
	layout->offset           = margin_start;
	layout->furthest         = { margin_start.x, margin_start.y };
	layout->child_min = layout->child_max = layout->furthest;
	layout->size             = dimensions;
	layout->line             = {};
	layout->line_prev        = {};
}

///////////////////////////////////////////

void ui_layout_push(vec3 start, vec2 dimensions, bool32_t add_margin) {
	ui_layout_t layout = {};
	layout.parent = -1;
	layout.window = -1;
	skui_layouts.add(layout);
	ui_layout_area(start, dimensions, add_margin);
}

///////////////////////////////////////////

void ui_layout_push_cut(ui_cut_ cut_to, float size, bool32_t add_margin) {
	int32_t      curr_idx = skui_layouts.count - 1;
	ui_layout_t* curr     = &skui_layouts[curr_idx];
	if      (cut_to == ui_cut_bottom && ui_layout_is_auto_height(curr)) log_warn("Can't cut bottom for layout with a height of 0!");
	else if (cut_to == ui_cut_right  && ui_layout_is_auto_width (curr)) log_warn("Can't cut right for layout with an width of 0!");

	vec3 cut_start   = {};
	vec2 cut_size    = {};
	vec3 curr_offset = {};
	switch (cut_to) {
	case ui_cut_left:
		curr_offset   = {-(size+skui_settings.gutter), 0, 0 };
		cut_start     = curr->offset_initial;
		cut_size      = { size, curr->size.y };
		if (curr->size.x != 0)
			curr->size.x -= size+skui_settings.gutter;
		break;
	case ui_cut_right:
		curr_offset   = {0, 0, 0 };
		cut_start     = curr->offset_initial - vec3{curr->size.x-size, 0, 0};
		cut_size      = { size, curr->size.y };
		if (curr->size.x != 0)
			curr->size.x -= size+skui_settings.gutter;
		break;
	case ui_cut_top:
		curr_offset   = {0, -(size+skui_settings.gutter), 0 };
		cut_start     = curr->offset_initial;
		cut_size      = { curr->size.x, size };
		if (curr->size.y != 0)
			curr->size.y -= size+skui_settings.gutter;
		break;
	case ui_cut_bottom:
		curr_offset   = {0, 0, 0 };
		cut_start     = curr->offset_initial - vec3{0,curr->size.y-size,0};
		cut_size      = { curr->size.x, size };
		if (curr->size.y != 0)
			curr->size.y -= size+skui_settings.gutter;
		break;
	}
	curr->offset         += curr_offset;
	curr->offset_initial += curr_offset;
	curr->offset_prev    += curr_offset;
	ui_layout_push(cut_start, cut_size, add_margin);
	skui_layouts.last().parent = curr_idx;
}

///////////////////////////////////////////

void ui_layout_window(ui_window_id window_id, vec3 start, vec2 dimensions, bool32_t add_margin) {
	ui_layout_area(start, dimensions, add_margin);

	ui_window_t* win    = &skui_windows[window_id];
	ui_layout_t* layout = &skui_layouts.last();
	layout->window = window_id;
	layout->parent = -1;
	win->layout_start = start;
	win->layout_size  = dimensions;
	win->curr_size    = dimensions;
}

///////////////////////////////////////////
#include <stdio.h>
void _ui_visualize_layout(ui_layout_t* layout) {
	// Helpful debug visualization for working with layouts
	color32 colors[] = {
		{ 255,0,  0,  255 },
		{ 255,255,0,  255 },
		{ 255,0,  255,255 },
		{ 0,  255,0,  255 },
		{ 255,255,0,  255 },
		{ 0,  255,255,255 },
		{ 0,  0,  255,255 },
		{ 255,0,  255,255 },
		{ 0,  255,255,255 },
	};

	float z_offset = 0.001f;
	float weight   = 0.001f;
	color32 col = colors[(int32_t)(layout->offset_initial.x * 100 + layout->offset_initial.y * 1000) % (sizeof(colors)/sizeof(colors[0]))];

	vec3 start = layout->offset_initial;
	vec2 size  = vec2{ start.x, start.y } - layout->furthest;
	col.a = 32;
	line_add(start - vec3{ 0, 0, z_offset },      start - vec3{ size.x, 0, z_offset },      col, col, weight);
	line_add(start - vec3{ 0, 0, z_offset },      start - vec3{ 0, size.y, z_offset },      col, col, weight);
	line_add(start - vec3{ size.x, 0, z_offset }, start - vec3{ size.x, size.y, z_offset }, col, col, weight);
	line_add(start - vec3{ 0, size.y, z_offset }, start - vec3{ size.x, size.y, z_offset }, col, col, weight);

	start = layout->offset_initial;
	size  = vec2{ 
		layout->size.x == 0 ? start.x - layout->furthest.x : layout->size.x,
		layout->size.y == 0 ? start.y - layout->furthest.y : layout->size.y };
	col.a = 128;
	line_add(start - vec3{ 0, 0, z_offset },      start - vec3{ size.x, 0, z_offset },      col, col, weight);
	line_add(start - vec3{ 0, 0, z_offset },      start - vec3{ 0, size.y, z_offset },      col, col, weight);
	line_add(start - vec3{ size.x, 0, z_offset }, start - vec3{ size.x, size.y, z_offset }, col, col, weight);
	line_add(start - vec3{ 0, size.y, z_offset }, start - vec3{ size.x, size.y, z_offset }, col, col, weight);

	start = layout->offset_initial + vec3{ skui_settings.margin, skui_settings.margin };
	size  = size + vec2_one * skui_settings.margin * 2;
	col.a = 255;
	line_add(start - vec3{ 0, 0, z_offset },      start - vec3{ size.x, 0, z_offset },      col, col, weight);
	line_add(start - vec3{ 0, 0, z_offset },      start - vec3{ 0, size.y, z_offset },      col, col, weight);
	line_add(start - vec3{ size.x, 0, z_offset }, start - vec3{ size.x, size.y, z_offset }, col, col, weight);
	line_add(start - vec3{ 0, size.y, z_offset }, start - vec3{ size.x, size.y, z_offset }, col, col, weight);

	char text[256];
	snprintf(text, 256, "layout\n%.2gx%.2g", size.x * 100, size.y * 100);
	text_add_at(text, matrix_ts(start - vec3{ size.x, 0, z_offset }, vec3_one * 0.5f), 0, text_align_top_left, text_align_top_left, 0, 0, 0, color32_to_128(col));

	snprintf(text, 256, "%.2g,%.2g", start.x * 100, start.y * 100);
	text_add_at(text, matrix_ts(start - vec3{ 0, 0, z_offset }, vec3_one * 0.5f), 0, text_align_top_left, text_align_top_left, 0, 0, 0, color32_to_128(col));

	snprintf(text, 256, "%.2g,%.2g", (start.x - size.x) * 100, start.y * 100);
	text_add_at(text, matrix_ts(start - vec3{ size.x, 0, z_offset }, vec3_one * 0.5f), 0, text_align_top_right, text_align_top_left, 0, 0, 0, color32_to_128(col));

	if (layout->window != -1) {
		ui_window_t *win = &skui_windows[layout->window];
		start = win->layout_start;
		size  = skui_windows[layout->window].curr_size;
		col   = { 255,255,255,255 };
		z_offset = 0.002f;

		line_add(start - vec3{ 0, 0, z_offset },      start - vec3{ size.x, 0, z_offset },      col, col, weight);
		line_add(start - vec3{ 0, 0, z_offset },      start - vec3{ 0, size.y, z_offset },      col, col, weight);
		line_add(start - vec3{ size.x, 0, z_offset }, start - vec3{ size.x, size.y, z_offset }, col, col, weight);
		line_add(start - vec3{ 0, size.y, z_offset }, start - vec3{ size.x, size.y, z_offset }, col, col, weight);

		snprintf(text, 256, "win %.2gx%.2g", size.x * 100, size.y * 100);
		text_add_at(text, matrix_ts(start - vec3{ 0, 0, z_offset }, vec3_one * 0.5f), 0, text_align_bottom_left, text_align_top_left, 0, 0, 0, color32_to_128(col));
	}
}

///////////////////////////////////////////

void ui_layout_pop() {
	ui_layout_t* layout = &skui_layouts.last();

	// Some tools for debugging layouts
	/*static int32_t  idx = 0;
	static int32_t  idx_show = 0;
	static uint64_t last_frame = 0;
	if (last_frame != time_frame()) {
		idx = 0;
		last_frame = time_frame();

		if (input_key(key_left ) & button_state_just_active) idx_show--;
		if (input_key(key_right) & button_state_just_active) idx_show++;
	}
	else {
		idx++;
	}
	if (idx == idx_show)
		idx = idx_show;*/


	// Move to next line if we're still on a previous line
	if (layout->offset.x != layout->offset_initial.x)
		ui_nextline();

	float min_x = fminf(layout->child_min.x, layout->furthest.x      ) - layout->margin;
	float max_x = fmaxf(layout->child_max.x, layout->offset_initial.x) + layout->margin;
	float min_y = fminf(layout->child_min.y, layout->furthest.y      ) - layout->margin;
	float max_y = fmaxf(layout->child_max.y, layout->offset_initial.y) + layout->margin;
	if (layout->parent != -1) {
		ui_layout_t* parent = &skui_layouts[layout->parent];
		if (min_x < parent->child_min.x) parent->child_min.x = min_x;
		if (max_x > parent->child_max.x) parent->child_max.x = max_x;
		if (max_y > parent->child_max.y) parent->child_max.y = max_y;
		if (min_y < parent->child_min.y) parent->child_min.y = min_y;
	}

	if (layout->window != -1) {
		ui_window_t* win        = &skui_windows[layout->window];
		vec2         final_size = {
			layout->size.x == 0 ? layout->offset_initial.x - layout->furthest.x : layout->size.x,
			layout->size.y == 0 ? layout->offset_initial.y - layout->furthest.y : layout->size.y };
		if (win->layout_size.x == 0) {
			win->curr_size.x = fmaxf(win->curr_size.x, (max_x - min_x));
		}
		if (win->layout_size.y == 0) {
			win->curr_size.y = fmaxf(win->curr_size.y, (max_y - min_y));
		}
	}

	// Store this as the most recent layout
	ui_override_recent_layout(
		{ layout->offset_initial.x + layout->margin, layout->offset_initial.y + layout->margin, layout->offset_initial.z },
		{ max_x - min_x, max_y - min_y });

	// Other part of the tools for viewing layouts
	//if (idx == idx_show)
	//	_ui_visualize_layout(layout);

	skui_layouts.pop();
}

///////////////////////////////////////////

ui_window_id ui_layout_curr_window() {
	return skui_layouts.last().window;
}

///////////////////////////////////////////

ui_layout_t* ui_layout_curr() {
	return &skui_layouts.last();
}

///////////////////////////////////////////

float ui_line_height() {
	return skui_settings.padding * 2 + text_style_get_baseline(ui_get_text_style());
}

///////////////////////////////////////////

ui_settings_t ui_get_settings() {
	return skui_settings;
}

///////////////////////////////////////////

float ui_get_margin() {
	return skui_settings.margin;
}

///////////////////////////////////////////

float ui_get_padding() {
	return skui_settings.padding;
}

///////////////////////////////////////////

float ui_get_gutter() {
	return skui_settings.gutter;
}

///////////////////////////////////////////

void ui_nextline() {
	ui_layout_t *layout = &skui_layouts.last();
	layout->offset_prev = layout->offset;
	layout->line_prev   = layout->line;

	layout->offset.x    = layout->offset_initial.x - layout->line_pad;
	layout->offset.y   -= layout->line.y + skui_settings.gutter;
	layout->line        = {};
}

///////////////////////////////////////////

void ui_sameline() {
	ui_layout_t *layout = &skui_layouts.last();
	layout->offset      = layout->offset_prev;
	layout->line        = layout->line_prev;
}

///////////////////////////////////////////

void ui_hspace(float horizontal_space) {
	skui_layouts.last().offset.x -= horizontal_space;
}

///////////////////////////////////////////

void ui_vspace(float vertical_space) {
	skui_layouts.last().offset.y -= vertical_space;
}

///////////////////////////////////////////

void ui_panel_begin(ui_pad_ padding) {
	float pad   = padding == ui_pad_outside ? skui_settings.margin : 0;
	vec2  size  = ui_layout_remaining() + vec2{ pad * 2,0 };
	vec3  start = skui_layouts.last().offset - vec3{ -pad,-pad,skui_settings.depth * 0.1f };
	skui_panel_stack.add(padding);
	ui_layout_push(start, size, padding != ui_pad_none);
}

///////////////////////////////////////////

void ui_panel_end() {
	ui_pad_ padding = skui_panel_stack.last();
	
	ui_layout_t* panel_layout = &skui_layouts.last();
	float        pad          = padding == ui_pad_inside ? skui_settings.margin : 0;
	vec2         panel_size   = {
		panel_layout->offset_initial.x - panel_layout->furthest.x + pad * 2,
		panel_layout->offset_initial.y - panel_layout->furthest.y + pad * 2};
	ui_layout_pop();
	bounds_t bounds = ui_layout_reserve(panel_size, false);

	ui_panel_at(bounds.center + bounds.dimensions / 2, { bounds.dimensions.x, bounds.dimensions.y }, padding);
	skui_panel_stack.pop();
}

///////////////////////////////////////////

pose_t ui_popup_pose(vec3 shift) {
	pose_t result;
	if (ui_last_element_active() & (button_state_active | button_state_just_inactive) ) {
		// If there was a UI element focused, we'll use that
		vec3 at  = hierarchy_to_world_point   ( ui_layout_last().center );
		quat rot = hierarchy_to_world_rotation( quat_identity );

		// For an element attached popup, we can just offset the popup from the
		// element by a bit. Pretty straightforward!
		vec3 fwd   = rot * vec3_forward;
		vec3 up    = rot * vec3_up;
		vec3 right = rot * vec3_right;

		const float away =  0.1f;  // Away from the panel
		const float down = -0.08f; // Down from the element;
		result.position    = at + fwd * away + up * down;
		result.orientation = quat_from_angles(25, 0, 0) * rot;

		// If this pose is out-of-reach, we want to clamp it to be within arm's
		// reach, touchable by the user.
		const float max_dist = 0.6f;
		const float rot_dist = 0.1f;
		float dist_sq = vec3_distance_sq(input_head()->position, result.position);
		if (dist_sq > max_dist * max_dist) {
			result.position = input_head()->position + vec3_normalize(result.position - input_head()->position) * max_dist;

			// we don't want to rotate allll the way if the keyboard is pretty
			// much in the right spot. Instead, we should gradually face
			// towards the user as the pose gets further from its original
			// position.
			quat  dest_rot = quat_lookat(result.position, input_head()->position);
			float percent  = math_saturate((sqrtf(dist_sq) - max_dist) / rot_dist);
			result.orientation = quat_slerp(result.orientation, dest_rot, percent);
		}
	} else {
		// For an independant popup, we want to position it in front of the
		// user. We'll place it in arm's reach, but this may be a bit odd for
		// controller mode, or those using far hand rays.
		// TODO: consider far interaction placement

		const float popup_distance = 0.5f; // The XZ distance from the user
		const float height_blend   = 0.5f; // How much does the user's Y axis rotation affect the popup position
		const float rotation_blend = 0.4f; // How much does the popup position's Y offset affect the popup orientation

		pose_t head     = *input_head();
		vec3   head_fwd = head.orientation * vec3_forward;
		vec3   flat_fwd = vec3_normalize(vec3{ head_fwd.x, 0, head_fwd.z });
		flat_fwd.y = head_fwd.y * height_blend;
		result.position    = head.position + flat_fwd * popup_distance;

		vec3 face_dir = vec3_normalize(head.position - result.position);
		face_dir.y *= rotation_blend;
		result.orientation = quat_lookat(vec3_zero, face_dir);

		vec3 fwd   = result.orientation * vec3_forward;
		vec3 up    = result.orientation * vec3_up;
		vec3 right = result.orientation * vec3_right;
		result.position += shift.x * right + shift.y * up + shift.z * fwd;
	}
	return result;
}

///////////////////////////////////////////

ui_window_t* ui_window_get(ui_window_id window) {
	return &skui_windows[window];
}

///////////////////////////////////////////

ui_window_id ui_window_find_or_add(id_hash_t hash, vec2 size) {
	for (int32_t i = 0; i < skui_windows.count; i++) {
		if (skui_windows[i].hash == hash) {
			return i;
		}
	}

	ui_window_t window = {};
	window.hash      = hash;
	window.prev_size = size;
	window.curr_size = size;
	skui_windows.add(window);

	return skui_windows.count-1;
}

}
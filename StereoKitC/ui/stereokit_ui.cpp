/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#include "ui_layout.h"
#include "ui_theming.h"
#include "ui_core.h"
#include "interactors.h"
#include "../stereokit_ui.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../hierarchy.h"
#include "../systems/input_keyboard.h"
#include "../libraries/array.h"
#include "../libraries/unicode.h"
#include "../libraries/stref.h"
#include "../libraries/profiler.h"

#include <math.h>

///////////////////////////////////////////

namespace sk {

bool32_t skui_enable_far_interact;
ui_move_ skui_system_move_type;

id_hash_t skui_input_target;
bool32_t  skui_input_target_confirmed;
int32_t   skui_input_caret;
int32_t   skui_input_caret_end;
float     skui_input_blink;

// Button activation animations all use the same values
const float    skui_pressed_depth  = 0.4f;
const color128 skui_color_border   = { 1,1,1,1 };
const float    skui_aura_radius    = 0.02f;
const float    skui_img_text_gap   = 0.75f; // button image<->label gap, fraction of text size
const float    skui_img_optic_size = 0.2f;  // button image optical oversize, fraction of text size
const float    skui_caret_height   = 1.2f;  // input caret/selection height, fraction of the text line height

///////////////////////////////////////////

bool32_t ui_text_at(const char16_t* text, vec2* opt_ref_scroll, ui_scroll_ scroll_direction, text_fit_ fit, align_ text_align, vec3 window_relative_pos, vec2 size);

///////////////////////////////////////////

bool ui_init() {
	profiler_zone();

	skui_input_target        = 0;
	skui_input_caret         = 0;
	skui_input_caret_end     = 0;
	skui_input_blink         = 0;
	skui_system_move_type    = ui_move_face_user;
	skui_enable_far_interact = true;

	ui_layout_init();
	ui_theming_init();
	ui_core_init();

	ui_push_tint({ 1,1,1,1 });
	ui_push_enabled(true);
	ui_push_preserve_keyboard(false);
	return true;
}

///////////////////////////////////////////

void ui_shutdown() {
	ui_core_shutdown();
	ui_theming_shutdown();
	ui_layout_shutdown();
}

///////////////////////////////////////////

void ui_step() {
	profiler_zone();

	ui_core_update();
	ui_theming_update();

	ui_push_surface(pose_identity);

	skui_input_target_confirmed = false;
}

///////////////////////////////////////////

void ui_step_late() {
	profiler_zone();

	ui_pop_surface();

	// If the active input target was not confirmed to exist, we should drop
	// input focus.
	if (skui_input_target_confirmed == false && skui_input_target != 0)
		skui_input_target = 0;

	//if (skui_layers                 .count != 0) log_errf("ui: Mismatching number of %s calls!", "Begin/End");
	//if (skui_id_stack               .count != 1) log_errf("ui: Mismatching number of %s calls!", "id push/pop");
	//if (skui_tint_stack             .count != 1) log_errf("ui: Mismatching number of %s calls!", "tint push/pop");
	//if (skui_enabled_stack          .count != 1) log_errf("ui: Mismatching number of %s calls!", "enabled push/pop");
	//if (skui_preserve_keyboard_stack.count != 1) log_errf("ui: Mismatching number of %s calls!", "preserve keyboard push/pop");
}

///////////////////////////////////////////

void ui_enable_far_interact(bool32_t enable) {
	skui_enable_far_interact = enable;
}

///////////////////////////////////////////

bool32_t ui_far_interact_enabled() {
	return skui_enable_far_interact;
}

///////////////////////////////////////////

ui_move_ ui_system_get_move_type() {
	return skui_system_move_type;
}

///////////////////////////////////////////

void ui_system_set_move_type(ui_move_ move_type) {
	skui_system_move_type = move_type;
}

///////////////////////////////////////////

render_layer_ ui_get_render_layer() {
	return skui_render_layer;
}

///////////////////////////////////////////

void ui_set_render_layer(render_layer_ layer) {
	skui_render_layer = layer;
	text_style_set_render_layer(skui_font_style, layer);
}

///////////////////////////////////////////

bool32_t ui_has_keyboard_focus() {
	return skui_input_target != 0;
}

///////////////////////////////////////////
///////////   UI Components   /////////////
///////////////////////////////////////////

void ui_model_at(model_t model, vec3 start, vec3 size, color128 color) {
	matrix mx = matrix_trs(start, quat_identity, size);
	render_add_model(model, mx, color*skui_tint, skui_render_layer);
}

///////////////////////////////////////////

void ui_hseparator() {
	vec3 pos;
	vec2 size;
	ui_layout_reserve_sz({ 0, text_style_get_baseline(ui_get_text_style())*skui_settings.separator_scale }, false, &pos, &size);

	ui_draw_element(ui_vis_separator, pos, vec3{ size.x, size.y, size.y / 2.0f }, 0);
}

///////////////////////////////////////////

vec2 text_size_layout            (const char16_t* text_utf16, text_style_t style)                  { return text_size_layout_16(text_utf16, style); }
vec2 text_size_layout_constrained(const char16_t* text_utf16, text_style_t style, float max_width) { return text_size_layout_constrained_16(text_utf16, style, max_width); }

float ui_text_in  (const char*     text, pivot_ position, align_ align, text_fit_ fit, vec3 start, vec2 size, vec2 offset) { return text_add_in   (text, matrix_t(start), size, fit, ui_get_text_style(), position, align, offset.x,offset.y,0, ui_is_enabled() ? color128{ 1, 1, 1, 1 } : color128{ .5f, .5f, .5f, 1 }); }
float ui_text_in  (const char16_t* text, pivot_ position, align_ align, text_fit_ fit, vec3 start, vec2 size, vec2 offset) { return text_add_in_16(text, matrix_t(start), size, fit, ui_get_text_style(), position, align, offset.x,offset.y,0, ui_is_enabled() ? color128{ 1, 1, 1, 1 } : color128{ .5f, .5f, .5f, 1 }); }

///////////////////////////////////////////

template<typename C>
void ui_label_g(const C *text, vec2 size, bool32_t use_padding, align_ text_align) {
	// A {0,0} size means auto-size to fit the text plus its padding.
	if (size.x == 0 && size.y == 0)
		size = text_size_layout(text, ui_get_text_style()) + (use_padding ? vec2{ skui_settings.padding, skui_settings.padding }*2 : vec2{ 0, skui_settings.padding }*2);

	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	vec2 padding = use_padding
		? vec2{ skui_settings.padding, skui_settings.padding }
		: vec2{ 0, skui_settings.padding };
	// align_none falls back to the label's natural left-centered alignment.
	if (text_align == align_none) text_align = align_center_left;
	ui_text_in(text, pivot_top_left, text_align, text_fit_squeeze, final_pos - vec3{ padding.x, 0, skui_settings.depth/2}, vec2{final_size.x-padding.x*2, final_size.y}, vec2_zero);
}
void ui_label   (const char     *text, vec2 size, bool32_t use_padding, align_ text_align) { ui_label_g<char    >(text, size, use_padding, text_align); }
void ui_label_16(const char16_t *text, vec2 size, bool32_t use_padding, align_ text_align) { ui_label_g<char16_t>(text, size, use_padding, text_align); }

///////////////////////////////////////////

void ui_image(sprite_t image, vec2 size) {
	float aspect = sprite_get_aspect(image);
	size = vec2{
		size.x==0 ? size.y*aspect : size.x, 
		size.y==0 ? size.x/aspect : size.y };

	float scale = fminf(size.x / aspect, size.y);

	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);
	
	sprite_draw(image, matrix_ts(final_pos - vec3{size.x / 2, size.y / 2, 2 * mm2m }, vec3{ scale, scale, 1 }),
		pivot_center, ui_is_enabled() ? color32{255, 255, 255, 255} : color32{128, 128, 128, 255}, skui_render_layer);
}

///////////////////////////////////////////
template<typename C>
void _ui_button_img_surface(const C* text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, float finger_offset, color128 image_tint, align_ text_align_override) {
	const float  pad        = skui_settings.padding;
	const float  pad2       = pad * 2;
	const float  depth      = finger_offset + 2 * mm2m;
	const float  aspect     = image != nullptr ? sprite_get_aspect(image) : 1.0f;
	const float  font_size  = text_style_get_baseline(ui_get_text_style());
	const float  img_gap    = font_size * skui_img_text_gap;
	vec3         image_at   = {};
	float        image_size = 0;
	vec3         text_at    = {};
	vec2         text_size  = {};
	pivot_       text_pivot = pivot_center;
	align_       text_align = align_center;
	// image_size is the sprite's height; sprite_draw renders it aspect*image_size wide.
	switch (image_layout) {
	default:
	case ui_btn_layout_left: {
		// Image centered in a size.y square at the left edge; stays centered when tall.
		image_size = font_size;
		image_at   = window_relative_pos - vec3{ size.y/2, size.y/2, depth };
		const float image_w = aspect * image_size;

		text_pivot = pivot_center_left;
		text_align = align_center_left;
		text_at    = window_relative_pos - vec3{ size.y/2 + image_w/2 + img_gap, size.y/2, depth };
		text_size  = { size.x - pad - img_gap - size.y/2 - image_w/2, size.y - pad2 };
	} break;
	case ui_btn_layout_right: {
		// Mirror of 'left': image centered in a square column at the right.
		image_size = font_size;
		image_at   = window_relative_pos - vec3{ size.x - size.y/2, size.y/2, depth };
		const float image_w = aspect * image_size;

		text_pivot = pivot_center_right;
		text_align = align_center_right;
		text_at    = window_relative_pos - vec3{ size.x - size.y/2 - image_w/2 - img_gap, size.y/2, depth };
		text_size  = { size.x - pad - img_gap - size.y/2 - image_w/2, size.y - pad2 };
	} break;
	case ui_btn_layout_none:
		image_size = 0;

		text_pivot = pivot_top_left;
		text_at    = window_relative_pos - vec3{ pad, pad, depth };
		text_size  = vec2{ size.x - pad2, size.y - pad2 };
		break;
	case ui_btn_layout_center_no_text:
		// Image alone, sized to fit the padded area and centered.
		image_size = fminf(size.y - pad2, (size.x - pad2) / aspect);
		image_at   = window_relative_pos - vec3{ size.x/2, size.y/2, depth };
		break;
	case ui_btn_layout_center: {
		// Image stacked over a label, group-centered. When the button is too
		// short, image, gap and label scale down together so both stay visible.
		const float content_h = size.y - pad2;
		const float comfort_h = font_size*2 + img_gap;
		const float scale     = fminf(1.0f, content_h / comfort_h);
		const float gap       = img_gap   * scale;
		const float text_h    = font_size * scale;
		image_size      = fminf(content_h - gap - text_h, (size.x - pad2) / aspect);
		const float top = (size.y - (image_size + gap + text_h)) * 0.5f;
		image_at        = window_relative_pos - vec3{ size.x/2, top + image_size/2, depth };

		text_pivot = pivot_top_center;
		text_at    = window_relative_pos - vec3{ size.x/2, top + image_size + gap, depth };
		text_size  = { size.x - pad2, text_h };
	} break;
	}

	if (image_size>0 && image) {
		const color128 final_color = ui_is_enabled() ? image_tint : image_tint * color128{ .5f, .5f, .5f, 1 };

		// Drawn a touch larger than its layout slot, for optical weight.
		const float draw_size = image_size + font_size * skui_img_optic_size;
		sprite_draw(image, matrix_ts(image_at, { draw_size, draw_size, draw_size }), pivot_center, color_to_32( final_color ), skui_render_layer);
	}
	if (image_layout != ui_btn_layout_center_no_text) {
		// align_none means "use the layout's natural alignment".
		if (text_align_override != align_none) text_align = text_align_override;
		ui_text_in(text, text_pivot, text_align, text_fit_squeeze, text_at, text_size, vec2_zero);
	}
}

///////////////////////////////////////////

template<typename C>
vec2 _ui_button_img_size(const C* text, sprite_t image, ui_btn_layout_ image_layout) {
	text_style_t style   = ui_get_text_style();
	vec2         size    = {};
	float        text_sz = text_style_get_baseline(style);
	float        aspect  = image != nullptr ? sprite_get_aspect(image) : 1;
	float        img_gap = text_sz * skui_img_text_gap;
	if (image_layout == ui_btn_layout_center_no_text) {
		size = { text_sz, text_sz };
	} else if (image_layout == ui_btn_layout_none) {
		size = text_size_layout(text, style);
	} else if (image_layout == ui_btn_layout_center) {
		// Image stacked over text. Like everything else, auto-size reserves a
		// single line of height; ask for a taller button explicitly to give the
		// stacked image room. Width is the wider of the image or the text.
		vec2  txt_size   = text_size_layout(text, style);
		float image_size = text_sz * aspect;
		size = vec2{ fmaxf(txt_size.x, image_size), text_sz };
	} else {
		// left / right: image sized to the text line, laid out beside the text.
		vec2  txt_size   = text_size_layout(text, style);
		float image_size = text_sz * aspect;
		size = vec2{ txt_size.x + image_size + img_gap, text_sz };
	}
	return size;
}

///////////////////////////////////////////
template<typename C>
bool32_t ui_button_img_at_g(const C* text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, color128 image_tint, align_ text_align) {
	id_hash_t     id = ui_stack_hash(text);
	float         finger_offset;
	button_state_ state, focus;
	int32_t       interactor;
	ui_button_behavior(window_relative_pos, size, id, 0, 0, ui_btn_flag_none, finger_offset, state, focus, &interactor);

	float min_activation = 1 - (finger_offset / skui_settings.depth);
	ui_draw_element(ui_vis_button, window_relative_pos, vec3{ size.x,size.y,finger_offset }, fmaxf(min_activation, ui_get_anim_focus(id, focus, state)));
	_ui_button_img_surface(text, image, image_layout, window_relative_pos, size, finger_offset, image_tint, text_align);

	if (state & button_state_just_active)
		ui_play_sound_on_off(ui_vis_button, id, window_relative_pos - vec3{ size.x/2.f, size.y/2.f, 0 });

	return state & button_state_just_inactive;
}

bool32_t ui_button_img_at   (const char     *text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, color128 image_tint, align_ text_align) { return ui_button_img_at_g<char    >(text, image, image_layout, window_relative_pos, size, image_tint, text_align); }
bool32_t ui_button_img_at   (const char16_t *text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, color128 image_tint, align_ text_align) { return ui_button_img_at_g<char16_t>(text, image, image_layout, window_relative_pos, size, image_tint, text_align); }
bool32_t ui_button_img_at_16(const char16_t *text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, color128 image_tint, align_ text_align) { return ui_button_img_at_g<char16_t>(text, image, image_layout, window_relative_pos, size, image_tint, text_align); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_button_img_g(const C *text, sprite_t image, ui_btn_layout_ image_layout, vec2 size, color128 image_tint, align_ text_align) {
	vec3     final_pos;
	vec2     final_size;
	// A {0,0} size means auto-size to fit the content, which also pads the
	// reserved layout bounds. An explicit size is used as-is.
	bool32_t auto_size = size.x == 0 && size.y == 0;
	if (auto_size) size = _ui_button_img_size(text, image, image_layout);

	ui_layout_reserve_sz(size, auto_size, &final_pos, &final_size);
	return ui_button_img_at(text, image, image_layout, final_pos, final_size, image_tint, text_align);
}
bool32_t ui_button_img   (const char     *text, sprite_t image, ui_btn_layout_ image_layout, vec2 size, color128 image_tint, align_ text_align) { return ui_button_img_g<char    >(text, image, image_layout, size, image_tint, text_align); }
bool32_t ui_button_img_16(const char16_t *text, sprite_t image, ui_btn_layout_ image_layout, vec2 size, color128 image_tint, align_ text_align) { return ui_button_img_g<char16_t>(text, image, image_layout, size, image_tint, text_align); }

///////////////////////////////////////////

bool32_t ui_button_at   (const char     *text, vec3 window_relative_pos, vec2 size, align_ text_align) { return ui_button_img_at   (text, nullptr, ui_btn_layout_none, window_relative_pos, size, color128{1,1,1,1}, text_align); }
bool32_t ui_button_at   (const char16_t *text, vec3 window_relative_pos, vec2 size, align_ text_align) { return ui_button_img_at_16(text, nullptr, ui_btn_layout_none, window_relative_pos, size, color128{1,1,1,1}, text_align); }
bool32_t ui_button_at_16(const char16_t *text, vec3 window_relative_pos, vec2 size, align_ text_align) { return ui_button_img_at_16(text, nullptr, ui_btn_layout_none, window_relative_pos, size, color128{1,1,1,1}, text_align); }

///////////////////////////////////////////

bool32_t ui_button   (const char     *text, vec2 size, align_ text_align) { return ui_button_img   (text, nullptr, ui_btn_layout_none, size, color128{1,1,1,1}, text_align); }
bool32_t ui_button_16(const char16_t *text, vec2 size, align_ text_align) { return ui_button_img_16(text, nullptr, ui_btn_layout_none, size, color128{1,1,1,1}, text_align); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_toggle_img_at_g(const C* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, color128 image_tint, align_ text_align) {
	id_hash_t     id = ui_stack_hash(text);
	float         finger_offset;
	button_state_ state, focus;
	int32_t       interactor;
	ui_button_behavior(window_relative_pos, size, id, 0, 0, ui_btn_flag_none, finger_offset, state, focus, &interactor);

	if (state & button_state_just_inactive) {
		pressed = pressed ? false : true;
	}
	finger_offset = pressed ? fminf(skui_pressed_depth * skui_settings.depth, finger_offset) : finger_offset;

	float min_activation = 1 - (finger_offset / skui_settings.depth);
	ui_draw_element(ui_vis_toggle, window_relative_pos, vec3{ size.x,size.y,finger_offset }, fmaxf(min_activation, ui_get_anim_focus(id, focus, state)));
	_ui_button_img_surface(text, pressed?toggle_on:toggle_off, image_layout, window_relative_pos, size, finger_offset, image_tint, text_align);

	if (state & button_state_just_active)
		ui_play_sound_on_off(ui_vis_button, id, window_relative_pos - vec3{ size.x/2.f, size.y/2.f, 0 });

	return state & button_state_just_inactive;
}
bool32_t ui_toggle_img_at   (const char*     text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, color128 image_tint, align_ text_align) { return ui_toggle_img_at_g<char    >(text, pressed, toggle_off, toggle_on, image_layout, window_relative_pos, size, image_tint, text_align); }
bool32_t ui_toggle_img_at   (const char16_t* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, color128 image_tint, align_ text_align) { return ui_toggle_img_at_g<char16_t>(text, pressed, toggle_off, toggle_on, image_layout, window_relative_pos, size, image_tint, text_align); }
bool32_t ui_toggle_img_at_16(const char16_t* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, color128 image_tint, align_ text_align) { return ui_toggle_img_at_g<char16_t>(text, pressed, toggle_off, toggle_on, image_layout, window_relative_pos, size, image_tint, text_align); };

///////////////////////////////////////////

template<typename C>
bool32_t ui_toggle_img_g(const C* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec2 size, color128 image_tint, align_ text_align) {
	vec3     final_pos;
	vec2     final_size;
	bool32_t auto_size = size.x == 0 && size.y == 0;
	if (auto_size) size = _ui_button_img_size(text, toggle_off, image_layout);

	ui_layout_reserve_sz(size, auto_size, &final_pos, &final_size);
	return ui_toggle_img_at(text, pressed, toggle_off, toggle_on, image_layout, final_pos, final_size, image_tint, text_align);
}
bool32_t ui_toggle_img   (const char*     text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec2 size, color128 image_tint, align_ text_align) { return ui_toggle_img_g<char    >(text, pressed, toggle_off, toggle_on, image_layout, size, image_tint, text_align); }
bool32_t ui_toggle_img_16(const char16_t* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec2 size, color128 image_tint, align_ text_align) { return ui_toggle_img_g<char16_t>(text, pressed, toggle_off, toggle_on, image_layout, size, image_tint, text_align); }

///////////////////////////////////////////

bool32_t ui_toggle_at   (const char     *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size, align_ text_align) { return ui_toggle_img_at   (text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, window_relative_pos, size, color128{1,1,1,1}, text_align); }
bool32_t ui_toggle_at   (const char16_t *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size, align_ text_align) { return ui_toggle_img_at_16(text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, window_relative_pos, size, color128{1,1,1,1}, text_align); }
bool32_t ui_toggle_at_16(const char16_t *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size, align_ text_align) { return ui_toggle_img_at_16(text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, window_relative_pos, size, color128{1,1,1,1}, text_align); }

///////////////////////////////////////////

bool32_t ui_toggle   (const char     *text, bool32_t &pressed, vec2 size, align_ text_align) { return ui_toggle_img   (text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, size, color128{1,1,1,1}, text_align); }
bool32_t ui_toggle_16(const char16_t *text, bool32_t &pressed, vec2 size, align_ text_align) { return ui_toggle_img_16(text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, size, color128{1,1,1,1}, text_align); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_button_round_at_g(const C *text, sprite_t image, vec3 window_relative_pos, float diameter) {
	id_hash_t     id = ui_stack_hash(text);
	float         finger_offset;
	button_state_ state, focus;
	int32_t       interactor;
	ui_button_behavior(window_relative_pos, { diameter,diameter }, id, 0, 0, ui_btn_flag_none, finger_offset, state, focus, &interactor);

	float min_activation = 1 - (finger_offset / skui_settings.depth);
	ui_draw_element(ui_vis_button_round, window_relative_pos, { diameter, diameter, finger_offset }, fmaxf(min_activation, ui_get_anim_focus(id, focus, state)));

	float sprite_scale = fmaxf(1, sprite_get_aspect(image));
	float sprite_size  = (diameter * 0.6f) / sprite_scale;
	sprite_draw(image, matrix_ts(window_relative_pos + vec3{ -diameter/2, -diameter/2, -(finger_offset + 2*mm2m) }, vec3{ sprite_size, sprite_size, 1 }), pivot_center, {255,255,255,255}, skui_render_layer);

	if (state & button_state_just_active)
		ui_play_sound_on_off(ui_vis_button, id, window_relative_pos - vec3{ diameter/2.f, diameter/2.f, 0 });

	return state & button_state_just_inactive;
}
bool32_t ui_button_round_at   (const char     *text, sprite_t image, vec3 window_relative_pos, float diameter) { return ui_button_round_at_g<char    >(text, image, window_relative_pos, diameter); }
bool32_t ui_button_round_at   (const char16_t *text, sprite_t image, vec3 window_relative_pos, float diameter) { return ui_button_round_at_g<char16_t>(text, image, window_relative_pos, diameter); }
bool32_t ui_button_round_at_16(const char16_t *text, sprite_t image, vec3 window_relative_pos, float diameter) { return ui_button_round_at_g<char16_t>(text, image, window_relative_pos, diameter); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_button_round_g(const C *id, sprite_t image, float diameter) {
	if (diameter == 0)
		diameter = ui_line_height();
	vec2 size = vec2{diameter, diameter};
	size = vec2_one * fmaxf(size.x, size.y);

	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	return ui_button_round_at(id, image, final_pos, final_size.x);
}
bool32_t ui_button_round   (const char     *id, sprite_t image, float diameter) { return ui_button_round_g<char>(id, image, diameter); }
bool32_t ui_button_round_16(const char16_t *id, sprite_t image, float diameter) { return ui_button_round_g<char16_t>(id, image, diameter); }

///////////////////////////////////////////

void ui_model(model_t model, vec2 ui_size, float model_scale) {
	if (ui_size.x == 0) ui_size.x = ui_layout_remaining().x - (skui_settings.padding*2);
	if (ui_size.y == 0) {
		bounds_t bounds = model_get_bounds(model);
		ui_size.y = ui_size.x * (bounds.dimensions.x / bounds.dimensions.y);
	}
	if (model_scale == 0) {
		bounds_t bounds = model_get_bounds(model);
		model_scale = fminf(ui_size.x / bounds.dimensions.x, ui_size.y / bounds.dimensions.y);
	}
	vec2 size = ui_size + vec2{ skui_settings.padding, skui_settings.padding }*2;

	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	final_size = final_size / 2;
	ui_model_at(model, { final_pos.x - final_size.x, final_pos.y - final_size.y, final_pos.z }, vec3_one * model_scale, { 1,1,1,1 });
}

///////////////////////////////////////////

inline vec2 text_char_at_o(const char*     text, text_style_t style, int32_t char_index, vec2* opt_size, text_fit_ fit, pivot_ position, align_ align) { return text_char_at   (text, style, char_index, opt_size, fit, position, align); }
inline vec2 text_char_at_o(const char16_t* text, text_style_t style, int32_t char_index, vec2* opt_size, text_fit_ fit, pivot_ position, align_ align) { return text_char_at_16(text, style, char_index, opt_size, fit, position, align); }

// Removes the selected range, and returns true if there was one to remove.
template<typename C>
bool ui_input_delete_selection_g(C* buffer) {
	if (skui_input_caret == skui_input_caret_end) return false;

	int32_t start = mini(skui_input_caret, skui_input_caret_end);
	int32_t count = maxi(skui_input_caret, skui_input_caret_end) - start;
	utf_remove_chars(utf_advance_chars(buffer, start), count);
	skui_input_caret_end = skui_input_caret = start;
	return true;
}

///////////////////////////////////////////

// Inserts at the caret, replacing any selection. Returns true if the buffer
// changed, which a full buffer's failed insert doesn't undo.
template<typename C>
bool ui_input_insert_g(C* buffer, int32_t buffer_size, char32_t add) {
	bool changed = ui_input_delete_selection_g(buffer);
	if (utf_insert_char(buffer, buffer_size, utf_advance_chars(buffer, skui_input_caret), add)) {
		skui_input_caret_end = skui_input_caret = skui_input_caret + 1;
		changed = true;
	}
	return changed;
}

///////////////////////////////////////////

template<typename C>
bool32_t ui_input_at_g(const C* id, C* buffer, int32_t buffer_size, vec3 window_relative_pos, vec2 size, text_context_ type) {
	id_hash_t id_hash  = ui_stack_hash(id);
	bool      result   = false;
	vec3      box_size = vec3{ size.x, size.y, skui_settings.depth / 2 };

	// Find out if the user is trying to focus this UI element
	float         finger_offset;
	int32_t       interactor;
	button_state_ state, focus;
	ui_button_behavior(window_relative_pos, size, id_hash, 0, 0, ui_btn_flag_none, finger_offset, state, focus, &interactor);

	if (state & button_state_just_inactive) {
		platform_keyboard_show(true,type);
		skui_input_blink  = time_totalf_unscaled();
		skui_input_target = id_hash;
		skui_input_caret  = skui_input_caret_end = (int32_t)utf_charlen(buffer);
	}

	// Unfocus this if the user starts interacting with something else
	if (skui_input_target == id_hash && ui_keyboard_focus_lost(id_hash)) {
		skui_input_target = 0;
		platform_keyboard_show(false, type);
	}

	// If focused, acquire any input in the keyboard's queue
	if (skui_input_target == id_hash) {
		// Walking the event queue keeps text and editing in the order the user
		// produced them, and gives every auto-repeat its own action.
		keyboard_event_t evt = input_keyboard_consume();
		while (evt.type != keyboard_event_type_none) {
			bool shift = (evt.modifiers & key_mod_shift) != key_mod_none;

			if (evt.type == keyboard_event_type_text) {
				// The queue is insertable text by contract, this is a guard
				if (input_text_insertable(evt.character) && ui_input_insert_g(buffer, buffer_size, evt.character))
					result = true;
			} else if (evt.type == keyboard_event_type_key_press) {
				switch (evt.key) {
				case key_backspace: {
					if (ui_input_delete_selection_g(buffer)) {
						result = true;
					} else if (skui_input_caret > 0) {
						skui_input_caret_end = skui_input_caret = skui_input_caret - 1;
						utf_remove_chars(utf_advance_chars(buffer, skui_input_caret), 1);
						result = true;
					}
				} break;
				case key_del: {
					if (ui_input_delete_selection_g(buffer) ||
					    utf_remove_chars(utf_advance_chars(buffer, skui_input_caret), 1))
						result = true;
				} break;
				case key_tab: {
					if (ui_input_insert_g(buffer, buffer_size, '\t')) result = true;
				} break;
				case key_return: {
					if (shift) { if (ui_input_insert_g(buffer, buffer_size, '\n')) result = true; }
					else       { skui_input_target = 0; platform_keyboard_show(false, type); result = true; }
				} break;
				case key_esc: {
					skui_input_target = 0;
					platform_keyboard_show(false, type);
				} break;
				case key_left: {
					skui_input_blink = time_totalf_unscaled();
					if (shift) skui_input_caret = maxi(0, skui_input_caret - 1);
					else {
						if (skui_input_caret_end == skui_input_caret) skui_input_caret = maxi(0, skui_input_caret - 1);
						skui_input_caret_end = skui_input_caret;
					}
				} break;
				case key_right: {
					skui_input_blink = time_totalf_unscaled();
					if (shift) skui_input_caret = mini((int32_t)utf_charlen(buffer), skui_input_caret + 1);
					else {
						if (skui_input_caret_end == skui_input_caret) skui_input_caret = mini((int32_t)utf_charlen(buffer), skui_input_caret + 1);
						skui_input_caret_end = skui_input_caret;
					}
				} break;
				default: break;
				}
			}

			// Submit or escape closed the field, so leave the rest of the
			// frame's events for whoever reads next.
			if (skui_input_target != id_hash) break;

			evt = input_keyboard_consume();
		}
	}

	// Render the input UI
	vec2  text_bounds    = { size.x - skui_settings.padding * 2,size.y };
	float min_activation = 1 - (finger_offset / skui_settings.depth);
	ui_draw_element(ui_vis_input, window_relative_pos, vec3{ size.x, size.y, skui_settings.depth / 2 }, fmaxf(min_activation, ui_get_anim_focus(id_hash, focus, state)));

	// Swap out for a string of asterisks to hide any password
	const C* draw_text = buffer;
	if (type == text_context_password) {
		size_t len          = utf_charlen(buffer);
		C*     password_txt = sk_stack_alloc_t(C, len + 1);
		for (size_t i = 0; i < len; i++)
			password_txt[i] = '*';
		password_txt[len] = '\0';
		draw_text = password_txt;
	}

	float text_depth = skui_settings.depth / 2 + 2 * mm2m;

	// If the input is focused, display text selection information
	if (skui_input_target == id_hash) {
		// Confirm that the input target still exists
		skui_input_target_confirmed = true;

		// Advance the displayed text if it's off the right side of the input
		text_style_t style     = ui_get_text_style();
		float        baseline  = text_style_get_baseline (style);
		float        ascender  = text_style_get_ascender (style);
		float        descender = text_style_get_descender(style);
		float        line_h    = (ascender + descender) * skui_caret_height;
		float        caret_sz  = baseline * 0.1f;

		int32_t caret_at      = skui_input_caret;
		vec2    caret_pos     = text_char_at_o(draw_text, style, caret_at, &text_bounds, text_fit_clip, pivot_top_left, align_center_left);
		float   scroll_margin = text_bounds.x - baseline;
		while (caret_pos.x < -scroll_margin && *draw_text != '\0' && caret_at >= 0) {
			draw_text += 1;
			caret_at  -= 1;
			caret_pos = text_char_at_o(draw_text, style, caret_at, &text_bounds, text_fit_clip, pivot_top_left, align_center_left);
		}

		// Center the box on the glyph run (text_char_at_o reports a point above
		// the baseline), then lift to a top-edge anchor since ui_draw_* extends down.
		float center_y = caret_pos.y - baseline - descender + (ascender - descender) * 0.5f;
		float top_y    = center_y + line_h * 0.5f;

		// Display a selection box for highlighted text
		if (skui_input_caret != skui_input_caret_end) {
			int32_t end       = maxi(0, caret_at + (skui_input_caret_end - skui_input_caret));
			vec2    caret_end = text_char_at_o(draw_text, style, end, &text_bounds, text_fit_clip, pivot_top_left, align_center_left);
			float   left      =       fmaxf(caret_pos.x, caret_end.x);
			float   right     = fmaxf(fminf(caret_pos.x, caret_end.x), -text_bounds.x);

			vec3 sz  = vec3{ -(right - left), line_h, line_h * 0.01f };
			vec3 pos = (window_relative_pos + vec3{ left - skui_settings.padding, top_y, -(text_depth - caret_sz*0.5f) });
			ui_draw_cube(pos, sz, ui_color_complement, 0);
		}

		// Show a blinking text caret
		if ((int)((time_totalf_unscaled()-skui_input_blink)*2)%2==0) {
			ui_draw_element(ui_vis_caret, window_relative_pos + vec3{ caret_pos.x - skui_settings.padding, top_y, -(text_depth) }, vec3{ caret_sz, line_h, caret_sz }, 0);
		}
	}

	ui_text_in(draw_text, pivot_top_left, align_center_left, text_fit_clip, window_relative_pos - vec3{ skui_settings.padding, 0, text_depth }, text_bounds, vec2_zero);

	if (state & button_state_just_active)
		ui_play_sound_on_off(ui_vis_button, id_hash, window_relative_pos - vec3{ size.x/2.f, size.y/2.f, 0 });

	return result;
}

template<typename C>
bool32_t ui_input_g(const C* id, C* buffer, int32_t buffer_size, vec2 size, text_context_ type) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	return ui_input_at_g(id, buffer, buffer_size, final_pos, final_size, type);
}

bool32_t ui_input      (const char*     id, char*     buffer, int32_t buffer_size, vec2 size, text_context_ type) { return ui_input_g<char    >(id, buffer, buffer_size, size, type); }
bool32_t ui_input_16   (const char16_t* id, char16_t* buffer, int32_t buffer_size, vec2 size, text_context_ type) { return ui_input_g<char16_t>(id, buffer, buffer_size, size, type); }

bool32_t ui_input_at   (const char*     id, char*     buffer, int32_t buffer_size, vec3 window_relative_pos, vec2 size, text_context_ type) { return ui_input_at_g<char    >(id, buffer, buffer_size, window_relative_pos, size, type); }
bool32_t ui_input_at_16(const char16_t* id, char16_t* buffer, int32_t buffer_size, vec3 window_relative_pos, vec2 size, text_context_ type) { return ui_input_at_g<char16_t>(id, buffer, buffer_size, window_relative_pos, size, type); }

///////////////////////////////////////////

void ui_progress_bar_at_ex(float percent, vec3 start_pos, vec2 size, float focus, ui_dir_ bar_direction, bool flip_fill_dir) {
	// For a vertical progress bar, the easiest thing is to just rotate the
	// hierarchy 90, as this simplifies any issues with trying to rotate the
	// calls to draw the left and right line segments. Same with flipping the
	// fill direction.
	float rotation = 0;
	if (bar_direction == ui_dir_vertical) { rotation  = 90; }
	if (flip_fill_dir == true           ) { rotation += 180; }

	hierarchy_push(matrix_trs(start_pos - vec3{ size.x*0.5f, size.y*0.5f,0 }, quat_from_angles(0, 0, rotation)));

	if (bar_direction == ui_dir_vertical) { size = vec2{ size.y, size.x }; }

	// Find sizes of bar elements
	float bar_height = fmaxf(skui_settings.padding, size.y / 6.f);
	float bar_depth  = bar_height * skui_pressed_depth - mm2m;
	float bar_y      = bar_height * 0.5f;
	float half_w     = size.x     * 0.5f;

	// If the left or right side of the bar is too small, then we'll just draw
	// a single solid bar.
	float bar_length = math_saturate(percent) * size.x;
	vec2  min_size   = ui_get_mesh_minsize(ui_vis_slider_line_active);
	if (bar_length <= min_size.x) {
		ui_draw_element_color(ui_vis_slider_line, ui_vis_slider_line_inactive,
			vec3{ half_w, bar_y,      0 },
			vec3{ size.x, bar_height, bar_depth },
			focus);
		hierarchy_pop();
		return;
	} else if (bar_length >= size.x-min_size.x) {
		ui_draw_element_color(ui_vis_slider_line, ui_vis_slider_line_active,
			vec3{ half_w, bar_y,      0 },
			vec3{ size.x, bar_height, bar_depth },
			focus);
		hierarchy_pop();
		return;
	}

	// Slide line
	ui_draw_element(ui_vis_slider_line_active,
		vec3{ half_w,     bar_y,      0 },
		vec3{ bar_length, bar_height, bar_depth },
		focus);
	ui_draw_element(ui_vis_slider_line_inactive,
		vec3{ half_w - bar_length, bar_y,      0 },
		vec3{ size.x - bar_length, bar_height, bar_depth },
		focus);
	hierarchy_pop();
}

///////////////////////////////////////////

void ui_progress_bar_at(float percent, vec3 start_pos, vec2 size, ui_dir_ bar_direction, bool32_t flip_fill_dir) {
	ui_progress_bar_at_ex(percent, start_pos, size, 0, bar_direction, flip_fill_dir);
}

///////////////////////////////////////////

void ui_hprogress_bar(float percent, float width, bool32_t flip_fill_dir) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz({ width, 0 }, false, &final_pos, &final_size);

	return ui_progress_bar_at(percent, final_pos, final_size, ui_dir_horizontal, flip_fill_dir);
}

///////////////////////////////////////////

void ui_vprogress_bar(float percent, float height, bool32_t flip_fill_dir) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_vertical_sz({ 0, height }, false, &final_pos, &final_size);

	return ui_progress_bar_at(percent, final_pos, final_size, ui_dir_vertical, flip_fill_dir);
}

///////////////////////////////////////////

template<typename C>
bool32_t ui_slider_at_g(ui_dir_ bar_direction, const C *id_text, float &value, float min, float max, float step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) {
	id_hash_t id = ui_stack_hash(id_text);

	float size_min    = bar_direction == ui_dir_vertical ? size.x : size.y;
	float rule_size   = fmaxf(skui_settings.padding, size_min / 6.f);
	vec2  button_size = confirm_method == ui_confirm_push
		? vec2{ size_min * 0.55f, size_min * 0.55f }
		: (bar_direction == ui_dir_vertical
			? vec2{ size_min, size_min / 2 }
			: vec2{ size_min / 2, size_min});

	float old_value = value;
	ui_slider_data_t slider;

	vec2 vmin, vmax, vstep, vval;
	if (bar_direction == ui_dir_vertical) { vmin = { 0,min }; vmax = { 0,max }; vstep = { 0,step }; vval = { 0,value }; }
	else                                  { vmin = { min,0 }; vmax = { max,0 }; vstep = { step,0 }; vval = { value,0 }; }
	ui_slider_behavior(window_relative_pos, size, id, &vval, vmin, vmax, button_size, button_size + vec2{skui_settings.padding, skui_settings.padding}*2, confirm_method, &slider);
	if (vstep.x != 0) vval.x = vmin.x + ((int32_t)(((vval.x - vmin.x) / vstep.x) + (vmin.x <= vmax.x ? 0.5f : -0.5f))) * vstep.x;
	if (vstep.y != 0) vval.y = vmin.y + ((int32_t)(((vval.y - vmin.y) / vstep.y) + (vmin.y <= vmax.y ? 0.5f : -0.5f))) * vstep.y;
	value = bar_direction == ui_dir_vertical ? vval.y : vval.x;

	// Draw the UI
	float percent   = (value - min) / (max - min);
	float vis_focus = ui_get_anim_focus(id, slider.focus_state, slider.active_state);
	ui_progress_bar_at_ex(percent, window_relative_pos, size, vis_focus, bar_direction, false);
	ui_draw_element(confirm_method == ui_confirm_push ? ui_vis_slider_push : ui_vis_slider_pinch,
		vec3{ slider.button_center.x+button_size.x/2, slider.button_center.y+button_size.y/2, window_relative_pos.z },
		vec3{ button_size.x, button_size.y, fmaxf(slider.finger_offset,rule_size * skui_pressed_depth + mm2m) },
		vis_focus);
	
	if (slider.active_state & button_state_just_active)
		ui_play_sound_on_off(ui_vis_slider_pinch, id, { slider.button_center.x, slider.button_center.y, window_relative_pos.z });

	// Play tick sound as the value updates
	if (slider.active_state & button_state_active && old_value != value) {
		if (step != 0) {
			// Play on every change if there's a user specified step value
			ui_play_sound_on(ui_vis_slider_line, { slider.button_center.x, slider.button_center.y, window_relative_pos.z });
		} else {
			// If no user specified step, then we'll do a set number of
			// clicks across the whole bar.
			const int32_t click_steps = 10;

			float   old_percent  = (old_value - min) / (max - min);
			int32_t old_quantize = (int32_t)(old_percent * click_steps + 0.5f);
			int32_t new_quantize = (int32_t)(percent     * click_steps + 0.5f);

			if (old_quantize != new_quantize) {
				ui_play_sound_on(ui_vis_slider_line, { slider.button_center.x, slider.button_center.y, window_relative_pos.z });
			}
		}
	}

	if (notify_on == ui_notify_finalize) return slider.active_state & button_state_just_inactive;
	else                                 return old_value != value;
}
bool32_t ui_hslider_at       (const char     *id_text, float  &value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_at_g<char    >(ui_dir_horizontal, id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }
bool32_t ui_hslider_at       (const char16_t *id_text, float  &value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_at_g<char16_t>(ui_dir_horizontal, id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }
bool32_t ui_hslider_at_16    (const char16_t *id_text, float  &value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_at_g<char16_t>(ui_dir_horizontal, id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }


bool32_t ui_vslider_at       (const char     *id_text, float  &value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_at_g<char    >(ui_dir_vertical, id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }
bool32_t ui_vslider_at       (const char16_t *id_text, float  &value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_at_g<char16_t>(ui_dir_vertical, id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }
bool32_t ui_vslider_at_16    (const char16_t *id_text, float  &value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_at_g<char16_t>(ui_dir_vertical, id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }


///////////////////////////////////////////

template<typename C>
bool32_t ui_slider_g(ui_dir_ bar_direction, const C *name, float &value, float min, float max, float step, float width, ui_confirm_ confirm_method, ui_notify_ notify_on) {
	vec3 final_pos;
	vec2 final_size;
	if (bar_direction == ui_dir_vertical) ui_layout_reserve_vertical_sz({0, width}, false, &final_pos, &final_size);
	else                                  ui_layout_reserve_sz         ({width, 0}, false, &final_pos, &final_size);

	return ui_slider_at_g<C>(bar_direction, name, value, min, max, step, final_pos, final_size, confirm_method, notify_on);
}

bool32_t ui_hslider       (const char     *name, float  &value, float  min, float  max, float  step, float width,  ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_g<char    >(ui_dir_horizontal, name, value, min, max, step, width, confirm_method, notify_on); }
bool32_t ui_hslider_16    (const char16_t *name, float  &value, float  min, float  max, float  step, float width,  ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_g<char16_t>(ui_dir_horizontal, name, value, min, max, step, width, confirm_method, notify_on); }


bool32_t ui_vslider       (const char     *name, float  &value, float  min, float  max, float  step, float height, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_g<char    >(ui_dir_vertical,  name, value, min, max, step, height, confirm_method, notify_on); }
bool32_t ui_vslider_16    (const char16_t *name, float  &value, float  min, float  max, float  step, float height, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_g<char16_t>(ui_dir_vertical,  name, value, min, max, step, height, confirm_method, notify_on); }


///////////////////////////////////////////

template<typename C>
bool32_t ui_text_at_g(const C* text, vec2* opt_ref_scroll, ui_scroll_ scroll_direction, align_ text_align, text_fit_ fit, vec3 window_relative_pos, vec2 size) {
	vec2 txt_bounds = size;
	bool result     = false;

	if (scroll_direction != ui_scroll_none && opt_ref_scroll != nullptr) {
		float        scroll_size = ui_line_height();
		text_style_t style       = ui_get_text_style();

		vec2 txt_size = (fit & text_fit_wrap) > 0
			? text_size_layout_constrained(text, style, size.x - scroll_size)
			: text_size_layout            (text, style);

		bool show_vertical   = txt_size.y > size.y && (scroll_direction & ui_scroll_vertical)   > 0;
		bool show_horizontal = txt_size.x > size.x && (scroll_direction & ui_scroll_horizontal) > 0;

		// Do these separately first so the scroll bars create a little gap at the
		// corner when both are visible.
		if (show_vertical)   txt_bounds.x -= scroll_size;
		if (show_horizontal) txt_bounds.y -= scroll_size;

		ui_push_id(text);
		if (show_vertical)   result = result || ui_vslider_at("vertical",   opt_ref_scroll->y, 0, fmaxf(0, txt_size.y-scroll_size),  0, window_relative_pos - vec3{ txt_bounds.x, 0, 0 }, vec2{scroll_size, txt_bounds.y}, ui_confirm_push, ui_notify_change);
		if (show_horizontal) result = result || ui_hslider_at("horizontal", opt_ref_scroll->x, 0, fmaxf(0, txt_size.x-txt_bounds.x), 0, window_relative_pos - vec3{ 0, txt_bounds.y, 0 }, vec2{txt_bounds.x, scroll_size}, ui_confirm_push, ui_notify_change);
		ui_pop_id();

		// Scrolling texts always need to clip, but it can be a small perf
		// boost if we don't need to clip.
		if (show_vertical || show_horizontal)
			fit |= text_fit_clip;
	}

	ui_text_in(text, pivot_top_left, text_align, fit, window_relative_pos - vec3{ 0, 0, skui_settings.depth / 2 }, txt_bounds, opt_ref_scroll == nullptr ? vec2_zero : *opt_ref_scroll);

	return result;
}

bool32_t ui_text_at   (const char16_t* text, vec2* opt_ref_scroll, ui_scroll_ scroll_direction, align_ text_align, text_fit_ fit, vec3 window_relative_pos, vec2 size) { return ui_text_at_g<char16_t>(text, opt_ref_scroll, scroll_direction, text_align, fit, window_relative_pos, size); }
bool32_t ui_text_at   (const char*     text, vec2* opt_ref_scroll, ui_scroll_ scroll_direction, align_ text_align, text_fit_ fit, vec3 window_relative_pos, vec2 size) { return ui_text_at_g<char    >(text, opt_ref_scroll, scroll_direction, text_align, fit, window_relative_pos, size); }
bool32_t ui_text_at_16(const char16_t* text, vec2* opt_ref_scroll, ui_scroll_ scroll_direction, align_ text_align, text_fit_ fit, vec3 window_relative_pos, vec2 size) { return ui_text_at_g<char16_t>(text, opt_ref_scroll, scroll_direction, text_align, fit, window_relative_pos, size); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_text_g(const C* text, vec2* opt_ref_scroll, ui_scroll_ scroll_direction, vec2 size, align_ text_align, text_fit_ fit) {
	if (size.x == 0) size.x = ui_layout_remaining().x;
	if (size.y == 0) size.y = (fit & text_fit_wrap) > 0
		? text_size_layout_constrained(text, ui_get_text_style(), size.x).y
		: text_size_layout            (text, ui_get_text_style()).y;

	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	return ui_text_at_g<C>(text, opt_ref_scroll, scroll_direction, text_align, fit, final_pos, final_size);
}

bool32_t ui_text      (const char*     text, vec2* opt_ref_scroll, ui_scroll_ scroll_direction, vec2 size, align_ text_align, text_fit_ fit) { return ui_text_g<char    >(text, opt_ref_scroll, scroll_direction, size, text_align, fit); }
bool32_t ui_text_16   (const char16_t* text, vec2* opt_ref_scroll, ui_scroll_ scroll_direction, vec2 size, align_ text_align, text_fit_ fit) { return ui_text_g<char16_t>(text, opt_ref_scroll, scroll_direction, size, text_align, fit); }

///////////////////////////////////////////

template<typename C>
void ui_window_begin_g(const C *text, pose_t* opt_pose, vec2 window_size, ui_win_ window_type, ui_move_ move_type) {
	id_hash_t    hash   = ui_push_id(text);
	ui_window_id win_id = ui_window_find_or_add(hash, window_size, opt_pose);
	ui_window_t* win    = ui_window_get(win_id);
	win->age  = 0;
	win->type = window_type;
	win->move = move_type;
	pose_t* pose = opt_pose ? opt_pose : &win->pose;

	// do a quick strcpy of the title
	const C* src = (const C*)text;
	C*       dst = (C*)win->title;
	do {
		*dst = *src;
		src++;
		dst++;
	} while (*src != 0 && (uint8_t*)dst - (uint8_t*)win->title < sizeof(win->title));
	win->title_step = sizeof(C);
	
	// figure out the size of it, based on its window type
	vec3 box_start = {}, box_size = {};
	if (win->type & ui_win_head) {
		float line = ui_line_height();
		box_start = vec3{ 0, line/2, 0 };
		box_size  = vec3{ win->prev_size.x, line, skui_settings.depth*2 };
	}
	if (win->type & ui_win_body || win->type & ui_win_empty) {
		box_start.y -= win->prev_size.y / 2;
		box_size.x   = win->prev_size.x;
		box_size.y  += win->prev_size.y;
		box_size.z   = skui_settings.depth * 2;
	}
	// Expand the volume a bit if we're using a grab aura
	if (win->move != ui_move_none && ui_grab_aura_enabled()) {
		box_size .x += skui_aura_radius*2;
		box_size .y += skui_aura_radius*2;
	}
	// Add a little extra depth to the box, so that it's easier to grab
	box_size .z += 0.02f;
	box_start.z = box_size.z/2;

	// Set up window handle and layout area
	_ui_handle_begin(hash, pose, nullptr, { box_start, box_size }, false, move_type, ui_gesture_pinch);
	ui_layout_window(win_id, { win->prev_size.x / 2,0,0 }, window_size, true);

	// Ensure space for the header text
	if (win->type & ui_win_head) {
		float header_width = window_size.x == 0
			? text_size_layout(text, ui_get_text_style()).x + (skui_settings.padding + skui_settings.margin) * 2 
			: window_size.x - (skui_settings.margin * 2);
		if (win->curr_size.x < header_width)
			win->curr_size.x = header_width;
	}
	win->pose = *pose;
}
void ui_window_begin(const char *text, pose_t *opt_pose, vec2 window_size, ui_win_ window_type, ui_move_ move_type) {
	ui_window_begin_g<char>(text, opt_pose, window_size, window_type, move_type);
}
void ui_window_begin_16(const char16_t *text, pose_t * opt_pose, vec2 window_size, ui_win_ window_type, ui_move_ move_type) {
	ui_window_begin_g<char16_t>(text, opt_pose, window_size, window_type, move_type);
}

///////////////////////////////////////////

void ui_window_end() {
	ui_window_id win_id      = ui_layout_curr_window();
	ui_window_t* win         = ui_window_get(win_id);
	float        line_height = ui_line_height();

	matrix top = hierarchy_top();
	ui_handle_end();
	hierarchy_push(top, hierarchy_parent_ignore);

	win->prev_size.x = win->layout_size.x == 0 ? win->curr_size.x : win->layout_size.x;
	win->prev_size.y = win->layout_size.y == 0 ? win->curr_size.y : win->layout_size.y;

	vec3 start = win->layout_start + vec3{ 0,0,skui_settings.depth };
	vec3 size  = { win->prev_size.x, win->prev_size.y, skui_settings.depth };

	float focus = ui_get_anim_focus_inout(win->hash, ui_id_focus_state(win->hash));
	if (win->move != ui_move_none && ui_grab_aura_enabled()) {
		vec3 aura_start = vec3{ start.x+skui_aura_radius,  start.y+skui_aura_radius,  start.z };
		vec3 aura_size  = vec3{ size .x+skui_aura_radius*2,size .y+skui_aura_radius*2,size .z };
		if (win->type & ui_win_head) { aura_start.y += line_height; aura_size.y += line_height; }
		ui_draw_element(ui_vis_aura, aura_start, aura_size, focus);
	}

	if (win->type & ui_win_head) {
		// draw label
		vec2 label_size = vec2{ size.x - (skui_settings.margin + skui_settings.padding)* 2, ui_line_height() };
		vec3 at         = win->layout_start - vec3{ skui_settings.padding + skui_settings.margin, 0, 2 * mm2m };

		if (win->title_step == 1) ui_text_in((const char    *)win->title, pivot_bottom_left, align_center_left, text_fit_none, at, label_size, vec2_zero);
		else                      ui_text_in((const char16_t*)win->title, pivot_bottom_left, align_center_left, text_fit_none, at, label_size, vec2_zero);

		ui_draw_element(win->type == ui_win_head ? ui_vis_window_head_only : ui_vis_window_head, start + vec3{0,line_height,0}, { size.x, line_height, size.z }, focus);
	}
	if (win->type & ui_win_body) {
		ui_draw_element(win->type == ui_win_body ? ui_vis_window_body_only : ui_vis_window_body, start, size, 0);
	}
	hierarchy_pop();
	ui_pop_id();

	// Store this as the most recent layout
	if (win->type & ui_win_head) { start.y += line_height; size.y += line_height; }
	ui_override_recent_layout(start, { size.x, size.y });
}

///////////////////////////////////////////

void ui_panel_at(vec3 start, vec2 size, ui_pad_ padding) {
	vec3 start_offset = vec3_zero;
	vec3 size_offset  = vec3_zero;
	if (padding == ui_pad_outside) {
		float gutter  = skui_settings.gutter / 2;
		float gutter2 = skui_settings.gutter;
		start_offset = { gutter,  gutter,  0 };
		size_offset  = { gutter2, gutter2, 0 };
	}
	ui_draw_element(ui_vis_panel, start+start_offset, vec3{ size.x, size.y, skui_settings.depth* 0.1f }+size_offset, 0);
}

} // namespace sk

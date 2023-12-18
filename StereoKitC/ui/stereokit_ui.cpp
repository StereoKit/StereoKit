#include "ui_layout.h"
#include "ui_theming.h"
#include "ui_core.h"
#include "../stereokit_ui.h"
#include "../_stereokit_ui.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../systems/input.h"
#include "../systems/input_keyboard.h"
#include "../platforms/platform_utils.h"
#include "../hands/input_hand.h"
#include "../libraries/ferr_hash.h"
#include "../libraries/array.h"
#include "../libraries/unicode.h"

#include <math.h>
#include <float.h>

///////////////////////////////////////////

namespace sk {

bool32_t skui_enable_far_interact;
ui_move_ skui_system_move_type;

uint64_t skui_input_target;
int32_t  skui_input_carat;
int32_t  skui_input_carat_end;
float    skui_input_blink;

// Button activation animations all use the same values
const float    skui_anim_duration  = 0.2f;
const float    skui_anim_overshoot = 10;
const float    skui_anim_focus_duration = 0.1f;
const float    skui_pressed_depth  = 0.4f;
const color128 skui_color_border   = { 1,1,1,1 };
const float    skui_aura_radius    = 0.02f;

///////////////////////////////////////////

bool ui_init() {
	skui_input_target        = 0;
	skui_input_carat         = 0;
	skui_input_carat_end     = 0;
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
	ui_core_update();
	ui_theming_update();

	ui_push_surface(pose_identity);
}

///////////////////////////////////////////

void ui_step_late() {
	ui_pop_surface();

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

bool32_t ui_has_keyboard_focus() {
	return skui_input_target != 0;
}

///////////////////////////////////////////
///////////   UI Components   /////////////
///////////////////////////////////////////

void ui_model_at(model_t model, vec3 start, vec3 size, color128 color) {
	matrix mx = matrix_trs(start, quat_identity, size);
	render_add_model(model, mx, color*skui_tint);
}

///////////////////////////////////////////

void ui_hseparator() {
	vec3 pos;
	vec2 size;
	ui_layout_reserve_sz({ 0, text_style_get_char_height(ui_get_text_style())*0.4f }, false, &pos, &size);

	ui_draw_el(ui_vis_separator, pos, vec3{ size.x, size.y, size.y / 2.0f }, 0);
}

///////////////////////////////////////////

vec2 text_size(const char16_t* text_utf16, text_style_t style) { return text_size_16(text_utf16, style); }

///////////////////////////////////////////

template<typename C>
void ui_label_sz_g(const C *text, vec2 size, bool32_t use_padding) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	vec2 padding = use_padding
		? vec2{ skui_settings.padding, skui_settings.padding }
		: vec2{ 0, skui_settings.padding };
	ui_text_at(text, text_align_center_left, text_fit_squeeze, final_pos - vec3{ padding.x, 0, skui_settings.depth/2}, vec2{final_size.x-padding.x*2, final_size.y});
}
void ui_label_sz   (const char     *text, vec2 size, bool32_t use_padding) { ui_label_sz_g<char    >(text, size, use_padding); }
void ui_label_sz_16(const char16_t *text, vec2 size, bool32_t use_padding) { ui_label_sz_g<char16_t>(text, size, use_padding); }

///////////////////////////////////////////

template<typename C>
void ui_label_g(const C *text, bool32_t use_padding) {
	vec2 txt_size = text_size(text, ui_get_text_style());
	vec2 padding  = use_padding
		? vec2{skui_settings.padding, skui_settings.padding}*2
		: vec2{0, skui_settings.padding}*2;

	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(txt_size + padding, false, &final_pos, &final_size);

	ui_text_at(text, text_align_top_left, text_fit_squeeze, final_pos - vec3{padding.x,padding.y,skui_settings.depth}/2, txt_size);
}
void ui_label   (const char     *text, bool32_t use_padding) { ui_label_g<char    >(text, use_padding); }
void ui_label_16(const char16_t *text, bool32_t use_padding) { ui_label_g<char16_t>(text, use_padding); }

///////////////////////////////////////////

float ui_text_in  (const char*     text, text_align_ position, text_align_ align, text_fit_ fit, vec3 start, vec2 size) { return text_add_in   (text, matrix_identity, size, fit, ui_get_text_style(), position, align, start.x, start.y, start.z, ui_is_enabled() ? color128{ 1, 1, 1, 1 } : color128{ .5f, .5f, .5f, 1 }); }
float ui_text_in  (const char16_t* text, text_align_ position, text_align_ align, text_fit_ fit, vec3 start, vec2 size) { return text_add_in_16(text, matrix_identity, size, fit, ui_get_text_style(), position, align, start.x, start.y, start.z, ui_is_enabled() ? color128{ 1, 1, 1, 1 } : color128{ .5f, .5f, .5f, 1 }); }

///////////////////////////////////////////

void ui_text_at   (const char*     text, text_align_ align, text_fit_ fit, vec3 start, vec2 size) { text_add_in   (text, matrix_identity, size, fit, ui_get_text_style(), text_align_top_left, align, start.x, start.y, start.z, ui_is_enabled() ? color128{ 1, 1, 1, 1 } : color128{ .5f, .5f, .5f, 1 }); }
void ui_text_at   (const char16_t* text, text_align_ align, text_fit_ fit, vec3 start, vec2 size) { text_add_in_16(text, matrix_identity, size, fit, ui_get_text_style(), text_align_top_left, align, start.x, start.y, start.z, ui_is_enabled() ? color128{ 1, 1, 1, 1 } : color128{ .5f, .5f, .5f, 1 }); }
void ui_text_at_16(const char16_t* text, text_align_ align, text_fit_ fit, vec3 start, vec2 size) { text_add_in_16(text, matrix_identity, size, fit, ui_get_text_style(), text_align_top_left, align, start.x, start.y, start.z, ui_is_enabled() ? color128{ 1, 1, 1, 1 } : color128{ .5f, .5f, .5f, 1 }); }

///////////////////////////////////////////

template<typename C>
void ui_text_sz_g(const C *text, text_align_ text_align, text_fit_ fit, vec2 size) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	ui_text_at(text, text_align, fit, final_pos - vec3{ 0, 0, skui_settings.depth/2 }, final_size);
}
void ui_text_sz   (const char     *text, text_align_ text_align, text_fit_ fit, vec2 size) { ui_text_sz_g<char    >(text, text_align, fit, size); }
void ui_text_sz_16(const char16_t *text, text_align_ text_align, text_fit_ fit, vec2 size) { ui_text_sz_g<char16_t>(text, text_align, fit, size); }

///////////////////////////////////////////

template<typename C>
void ui_text_g(const C *text, text_align_ text_align) {
	vec3 offset = ui_layout_at();
	vec2 size   = { ui_layout_remaining().x, 0 };
	vec3 at     = offset - vec3{ 0, 0, skui_settings.depth/2 };
	size.y = ui_text_in(text, text_align_top_left, text_align, text_fit_wrap, at, size);

	ui_layout_reserve(size);
}
void ui_text   (const char     *text, text_align_ text_align) { ui_text_g<char    >(text, text_align); }
void ui_text_16(const char16_t *text, text_align_ text_align) { ui_text_g<char16_t>(text, text_align); }

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
	
	sprite_draw_at(image, matrix_ts(final_pos - vec3{size.x/2,size.y/2,2*mm2m }, vec3{ scale, scale, 1 }), text_align_center);
}

///////////////////////////////////////////
template<typename C>
void _ui_button_img_surface(const C* text, sprite_t image, ui_btn_layout_ image_layout, text_align_ text_layout, vec3 window_relative_pos, vec2 size, float finger_offset) {
	_ui_button_img_surface(text, image, image_layout, text_layout, window_relative_pos, size, finger_offset, skui_tint);
}

template<typename C>
void _ui_button_img_surface(const C* text, sprite_t image, ui_btn_layout_ image_layout, text_align_ text_layout, vec3 window_relative_pos, vec2 size, float finger_offset, color128 image_tint) {
	float pad2       = skui_settings.padding * 2;
	float pad2gutter = pad2 + skui_settings.gutter;
	float depth      = finger_offset + 2 * mm2m;
	vec3  image_at   = {};
	float image_size;
	text_align_ image_align = text_align_x_left;
	vec3  text_at;
	vec2  text_size;
	text_align_ text_align;
	float aspect = image != nullptr ? sprite_get_aspect(image) : 1.0f;
	float font_size = text_style_get_char_height(ui_get_text_style());
	switch (image_layout) {
	default:
	case ui_btn_layout_left:
		image_align = text_align_center;
		image_size  = fminf(size.y - pad2, font_size);
		image_at    = window_relative_pos - vec3{ size.y/2.0f, size.y/2.0f, depth };

		text_align = text_align_center_right;
		text_at    = window_relative_pos - vec3{ size.x-skui_settings.padding, size.y/2, depth };
		text_size  = { size.x - ((size.y+image_size)/2.0f + pad2), size.y - pad2 };
		break;
	case ui_btn_layout_right:
		image_align = text_align_center;
		image_at    = window_relative_pos - vec3{ size.x-(size.y/2), size.y / 2, depth };
		image_size  = fminf(size.y - pad2, font_size);

		text_align = text_align_center_left;
		text_at    = window_relative_pos - vec3{ skui_settings.padding, size.y / 2, depth };
		text_size  = { size.x - ((size.y+image_size)/2.0f + pad2), size.y - pad2 };
		break;
	case ui_btn_layout_none:
		image_size = 0;

		text_align = text_align_top_left;
		text_at    = window_relative_pos - vec3{ skui_settings.padding, skui_settings.padding, depth };
		text_size  = vec2{ size.x - pad2, size.y - pad2 };
		break;
	case ui_btn_layout_center_no_text:
	case ui_btn_layout_center:
		image_align = text_align_center;
		image_size  = fminf(size.y - pad2, (size.x - pad2) / aspect);
		image_at    = window_relative_pos - vec3{ size.x/2, size.y / 2, depth }; 

		text_align = text_align_top_center;
		float y = size.y / 2 + image_size / 2;
		text_at    = window_relative_pos - vec3{size.x/2, y, depth};
		text_size  = { size.x-pad2, (size.y-skui_settings.padding*0.25f)-y };
		break;
	}

	if (image_size>0 && image) {
		color128 final_color = image_tint;
		if (!ui_is_enabled()) final_color = final_color * color128{ .5f, .5f, .5f, 1 };
	
		sprite_draw_at(image, matrix_ts(image_at, { image_size, image_size, image_size }), image_align, color_to_32( final_color ));
	}
	if (image_layout != ui_btn_layout_center_no_text)
		ui_text_in(text, text_align, text_layout, text_fit_squeeze, text_at, text_size);
}

///////////////////////////////////////////

template<typename C>
vec2 _ui_button_img_size(const C* text, sprite_t image, ui_btn_layout_ image_layout) {
	vec2 size = {};
	float font_size = text_style_get_char_height(ui_get_text_style());
	if (image_layout == font_size || image_layout == ui_btn_layout_center_no_text) {
		size = { font_size, font_size };
	} else if (image_layout == ui_btn_layout_none) {
		size = text_size(text, ui_get_text_style());
	} else {
		vec2  txt_size   = text_size(text, ui_get_text_style());
		float aspect     = image != nullptr ? sprite_get_aspect(image) : 1;
		float image_size = font_size * aspect;
		size = vec2{ txt_size.x + image_size + skui_settings.gutter, font_size };
	}
	return size;
}

///////////////////////////////////////////
template<typename C>
bool32_t ui_button_img_at_g(const C* text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) {
	return ui_button_img_at_g(text, image, image_layout, window_relative_pos, size, skui_tint);
}

template<typename C>
bool32_t ui_button_img_at_g(const C* text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, color128 image_tint) {
	uint64_t      id = ui_stack_hash(text);
	float         finger_offset;
	button_state_ state, focus;
	ui_button_behavior(window_relative_pos, size, id, finger_offset, state, focus);

	if (state & button_state_just_active)
		ui_anim_start(id, 0);
	float color_blend = state & button_state_active ? 1.0f : 0.0f;
	if (ui_anim_has(id, 0, skui_anim_duration)) {
		float t     = ui_anim_elapsed(id, 0, skui_anim_duration);
		color_blend = math_ease_overshoot(0, 1, skui_anim_overshoot, t);
	}

	float activation = (1 - (finger_offset / skui_settings.depth)) * 0.5f + ((focus & button_state_active) > 0 ? 0.5f : 0);
	ui_draw_el(ui_vis_button, window_relative_pos, vec3{ size.x,size.y,finger_offset }, fmaxf(activation, color_blend));
	_ui_button_img_surface(text, image, image_layout, text_align_center, window_relative_pos, size, finger_offset, image_tint);

	return state & button_state_just_active;
}

bool32_t ui_button_img_at   (const char     *text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_button_img_at_g<char    >(text, image, image_layout, window_relative_pos, size); }
bool32_t ui_button_img_at   (const char16_t *text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_button_img_at_g<char16_t>(text, image, image_layout, window_relative_pos, size); }
bool32_t ui_button_img_at_16(const char16_t *text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_button_img_at_g<char16_t>(text, image, image_layout, window_relative_pos, size); }
bool32_t ui_button_img_at_color_16(const char16_t* text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, color128 image_tint) { return ui_button_img_at_g<char16_t>(text, image, image_layout, window_relative_pos, size, image_tint); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_button_img_g(const C *text, sprite_t image, ui_btn_layout_ image_layout) {
	vec3 final_pos;
	vec2 final_size;
	vec2 size = _ui_button_img_size(text, image, image_layout);

	ui_layout_reserve_sz(size, true, &final_pos, &final_size);
	return ui_button_img_at(text, image, image_layout, final_pos, final_size);
}
template<typename C>
bool32_t ui_button_img_color_g(const C* text, sprite_t image, ui_btn_layout_ image_layout, color128 image_tint) {
	vec3 final_pos;
	vec2 final_size;
	vec2 size = _ui_button_img_size(text, image, image_layout);

	ui_layout_reserve_sz(size, true, &final_pos, &final_size);
	return ui_button_img_at_color_16(text, image, image_layout, final_pos, final_size, image_tint);
}
bool32_t ui_button_img   (const char     *text, sprite_t image, ui_btn_layout_ image_layout) { return ui_button_img_g<char    >(text, image, image_layout); }
bool32_t ui_button_img_16(const char16_t *text, sprite_t image, ui_btn_layout_ image_layout) { return ui_button_img_g<char16_t>(text, image, image_layout); }
bool32_t ui_button_img_color_16(const char16_t* text, sprite_t image, ui_btn_layout_ image_layout, color128 image_tint) { return ui_button_img_color_g<char16_t>(text, image, image_layout, image_tint); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_button_img_sz_g(const C *text, sprite_t image, ui_btn_layout_ image_layout, vec2 size) {
	vec3 final_pos;
	vec2 final_size;

	ui_layout_reserve_sz(size, false, &final_pos, &final_size);
	return ui_button_img_at(text, image, image_layout, final_pos, final_size);
}
bool32_t ui_button_img_sz   (const char     *text, sprite_t image, ui_btn_layout_ image_layout, vec2 size) { return ui_button_img_sz_g<char    >(text, image, image_layout, size); }
bool32_t ui_button_img_sz_16(const char16_t *text, sprite_t image, ui_btn_layout_ image_layout, vec2 size) { return ui_button_img_sz_g<char16_t>(text, image, image_layout, size); }

///////////////////////////////////////////

bool32_t ui_button_at   (const char     *text, vec3 window_relative_pos, vec2 size) { return ui_button_img_at   (text, nullptr, ui_btn_layout_none, window_relative_pos, size); }
bool32_t ui_button_at   (const char16_t *text, vec3 window_relative_pos, vec2 size) { return ui_button_img_at_16(text, nullptr, ui_btn_layout_none, window_relative_pos, size); }
bool32_t ui_button_at_16(const char16_t *text, vec3 window_relative_pos, vec2 size) { return ui_button_img_at_16(text, nullptr, ui_btn_layout_none, window_relative_pos, size); }

///////////////////////////////////////////

bool32_t ui_button_sz   (const char     *text, vec2 size) { return ui_button_img_sz   (text, nullptr, ui_btn_layout_none, size); }
bool32_t ui_button_sz_16(const char16_t *text, vec2 size) { return ui_button_img_sz_16(text, nullptr, ui_btn_layout_none, size); }

///////////////////////////////////////////

bool32_t ui_button   (const char     *text) { return ui_button_img   (text, nullptr, ui_btn_layout_none); }
bool32_t ui_button_16(const char16_t *text) { return ui_button_img_16(text, nullptr, ui_btn_layout_none); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_toggle_img_at_g(const C* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) {
	uint64_t      id = ui_stack_hash(text);
	float         finger_offset;
	button_state_ state, focus;
	ui_button_behavior(window_relative_pos, size, id, finger_offset, state, focus);

	if (state & button_state_just_active)
		ui_anim_start(id, 0);
	float color_blend = state & button_state_active ? 1.0f : 0.0f;
	if (ui_anim_has(id, 0, skui_anim_duration)) {
		float t     = ui_anim_elapsed(id, 0, skui_anim_duration);
		color_blend = math_ease_overshoot(0, 1, skui_anim_overshoot, t);
	}

	if (state & button_state_just_active) {
		pressed = pressed ? false : true;
	}
	finger_offset = pressed ? fminf(skui_pressed_depth * skui_settings.depth, finger_offset) : finger_offset;

	float activation = (1 - (finger_offset / skui_settings.depth)) * 0.5f + ((focus & button_state_active) > 0 ? 0.5f : 0);
	ui_draw_el(ui_vis_toggle, window_relative_pos, vec3{ size.x,size.y,finger_offset }, fmaxf(activation, color_blend));
	_ui_button_img_surface(text, pressed?toggle_on:toggle_off, image_layout, text_align_center, window_relative_pos, size, finger_offset);

	return state & button_state_just_active;
}
bool32_t ui_toggle_img_at   (const char*     text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_toggle_img_at_g<char    >(text, pressed, toggle_off, toggle_on, image_layout, window_relative_pos, size); }
bool32_t ui_toggle_img_at   (const char16_t* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_toggle_img_at_g<char16_t>(text, pressed, toggle_off, toggle_on, image_layout, window_relative_pos, size); }
bool32_t ui_toggle_img_at_16(const char16_t* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_toggle_img_at_g<char16_t>(text, pressed, toggle_off, toggle_on, image_layout, window_relative_pos, size); };

///////////////////////////////////////////

template<typename C>
bool32_t ui_toggle_img_g(const C* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout) {
	vec3 final_pos;
	vec2 final_size;
	vec2 size = _ui_button_img_size(text, toggle_off, image_layout);

	ui_layout_reserve_sz(size, true, &final_pos, &final_size);
	return ui_toggle_img_at(text, pressed, toggle_off, toggle_on, image_layout, final_pos, final_size);
}
bool32_t ui_toggle_img   (const char*     text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout) { return ui_toggle_img_g<char    >(text, pressed, toggle_off, toggle_on, image_layout); }
bool32_t ui_toggle_img_16(const char16_t* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout) { return ui_toggle_img_g<char16_t>(text, pressed, toggle_off, toggle_on, image_layout); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_toggle_img_sz_g(const C* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec2 size) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	return ui_toggle_img_at(text, pressed, toggle_off, toggle_on, image_layout, final_pos, final_size);
}
bool32_t ui_toggle_img_sz   (const char*     text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec2 size) { return ui_toggle_img_sz_g<char    >(text, pressed, toggle_off, toggle_on, image_layout, size); }
bool32_t ui_toggle_img_sz_16(const char16_t* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec2 size) { return ui_toggle_img_sz_g<char16_t>(text, pressed, toggle_off, toggle_on, image_layout, size); }

///////////////////////////////////////////

bool32_t ui_toggle_at   (const char     *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size) { return ui_toggle_img_at   (text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, window_relative_pos, size); }
bool32_t ui_toggle_at   (const char16_t *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size) { return ui_toggle_img_at_16(text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, window_relative_pos, size); }
bool32_t ui_toggle_at_16(const char16_t *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size) { return ui_toggle_img_at_16(text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, window_relative_pos, size); }

///////////////////////////////////////////

bool32_t ui_toggle   (const char     *text, bool32_t &pressed) { return ui_toggle_img   (text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left); }
bool32_t ui_toggle_16(const char16_t *text, bool32_t &pressed) { return ui_toggle_img_16(text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left); }

///////////////////////////////////////////

bool32_t ui_toggle_sz   (const char     *text, bool32_t &pressed, vec2 size) { return ui_toggle_img_sz   (text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, size); }
bool32_t ui_toggle_sz_16(const char16_t* text, bool32_t& pressed, vec2 size) { return ui_toggle_img_sz_16(text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, size); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_button_round_at_g(const C *text, sprite_t image, vec3 window_relative_pos, float diameter) {
	uint64_t      id = ui_stack_hash(text);
	float         finger_offset;
	button_state_ state, focus;
	ui_button_behavior(window_relative_pos, { diameter,diameter }, id, finger_offset, state, focus);

	if (state & button_state_just_active)
		ui_anim_start(id, 0);
	float color_blend = state & button_state_active ? 1.0f : 0.0f;
	if (ui_anim_has(id, 0, skui_anim_duration)) {
		float t     = ui_anim_elapsed(id, 0, skui_anim_duration);
		color_blend = math_ease_overshoot(0, 1, skui_anim_overshoot, t);
	}

	float activation = (1 - (finger_offset / skui_settings.depth)) * 0.5f + ((focus & button_state_active) > 0 ? 0.5f : 0);
	ui_draw_el(ui_vis_button_round, window_relative_pos, { diameter, diameter, finger_offset }, fmaxf(activation, color_blend));

	float sprite_scale = fmaxf(1, sprite_get_aspect(image));
	float sprite_size  = (diameter * 0.7f) / sprite_scale;
	sprite_draw_at(image, matrix_ts(window_relative_pos + vec3{ -diameter/2, -diameter/2, -(finger_offset + 2*mm2m) }, vec3{ sprite_size, sprite_size, 1 }), text_align_center);

	return state & button_state_just_active;
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

inline vec2 text_char_at_o(const char*     text, text_style_t style, int32_t char_index, vec2* opt_size, text_fit_ fit, text_align_ position, text_align_ align) { return text_char_at   (text, style, char_index, opt_size, fit, position, align); }
inline vec2 text_char_at_o(const char16_t* text, text_style_t style, int32_t char_index, vec2* opt_size, text_fit_ fit, text_align_ position, text_align_ align) { return text_char_at_16(text, style, char_index, opt_size, fit, position, align); }

template<typename C>
bool32_t ui_input_g(const C *id, C *buffer, int32_t buffer_size, vec2 size, text_context_ type) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	uint64_t id_hash  = ui_stack_hash(id);
	bool     result   = false;
	vec3     box_size = vec3{ final_size.x, final_size.y, skui_settings.depth/2 };

	// Find out if the user is trying to focus this UI element
	float         finger_offset;
	int32_t       hand;
	button_state_ state, focus;
	ui_button_behavior(final_pos, final_size, id_hash, finger_offset, state, focus, &hand);

	if (state & button_state_just_active) {
		platform_keyboard_show(true,type);
		skui_input_blink  = time_totalf_unscaled();
		skui_input_target = id_hash;
		skui_input_carat  = skui_input_carat_end = (int32_t)utf_charlen(buffer);
	}

	if (state & button_state_just_active)
		ui_anim_start(id_hash, 0);
	float color_blend = skui_input_target == id_hash ? 1.0f : 0.0f;
	if (ui_anim_has(id_hash, 0, skui_anim_duration)) {
		float t     = ui_anim_elapsed(id_hash, 0, skui_anim_duration);
		color_blend = math_ease_overshoot(0, 1, skui_anim_overshoot, t);
	}

	// Unfocus this if the user starts interacting with something else
	if (skui_input_target == id_hash && ui_keyboard_focus_lost(id_hash)) {
		skui_input_target = 0;
		platform_keyboard_show(false, type);
	}

	// If focused, acquire any input in the keyboard's queue
	if (skui_input_target == id_hash) {
		uint32_t curr = input_text_consume();
		while (curr != 0) {
			uint32_t add = '\0';

			if (curr == key_backspace) {
				if (skui_input_carat != skui_input_carat_end) {
					int32_t start = mini(skui_input_carat, skui_input_carat_end);
					int32_t count = maxi(skui_input_carat, skui_input_carat_end) - start;
					utf_remove_chars(utf_advance_chars(buffer, start), count);
					skui_input_carat_end = skui_input_carat = start;
					result = true;
				} else if (skui_input_carat > 0) {
					skui_input_carat_end = skui_input_carat = skui_input_carat - 1;
					utf_remove_chars(utf_advance_chars(buffer, skui_input_carat), 1);
					result = true;
				}
			} else if (curr == 0x7f) {
				if (skui_input_carat != skui_input_carat_end) {
					int32_t start = mini(skui_input_carat, skui_input_carat_end);
					int32_t count = maxi(skui_input_carat, skui_input_carat_end) - start;
					utf_remove_chars(utf_advance_chars(buffer, start), count);
					skui_input_carat_end = skui_input_carat = start;
					result = true;
				} else if (skui_input_carat >= 0) {
					utf_remove_chars(utf_advance_chars(buffer, skui_input_carat), 1);
					result = true;
				}
			} else if (curr == 0x0D) { // Enter, carriage return
				skui_input_target = 0;
				platform_keyboard_show(false, type);
				result = true;
			} else if (curr == 0x0A) { // Shift+Enter, linefeed
				add = '\n';
			} else if (curr == 0x1B) { // Escape
				skui_input_target = 0;
				platform_keyboard_show(false, type);
			} else {
				add = curr;
			}

			if (add != '\0') {
				// Remove any selected
				if (skui_input_carat != skui_input_carat_end) {
					int32_t start = mini(skui_input_carat, skui_input_carat_end);
					int32_t count = maxi(skui_input_carat, skui_input_carat_end) - start;
					utf_remove_chars(utf_advance_chars(buffer, start), count);
					skui_input_carat_end = skui_input_carat = start;
				}
				utf_insert_char(buffer, buffer_size, utf_advance_chars(buffer, skui_input_carat), add);
				skui_input_carat += 1;
				skui_input_carat_end = skui_input_carat;
				result = true;
			}

			curr = input_text_consume();
		}
		if      (input_key(key_shift) & button_state_active && input_key(key_left ) & button_state_just_active) { skui_input_blink = time_totalf_unscaled(); skui_input_carat = maxi(0, skui_input_carat - 1); }
		else if (input_key(key_left ) & button_state_just_active)                                               { skui_input_blink = time_totalf_unscaled(); if (skui_input_carat_end == skui_input_carat) skui_input_carat = maxi(0, skui_input_carat - 1); skui_input_carat_end = skui_input_carat; }
		if      (input_key(key_shift) & button_state_active && input_key(key_right) & button_state_just_active) { skui_input_blink = time_totalf_unscaled(); skui_input_carat = mini((int32_t)utf_charlen(buffer), skui_input_carat + 1); }
		else if (input_key(key_right) & button_state_just_active)                                               { skui_input_blink = time_totalf_unscaled(); if (skui_input_carat_end == skui_input_carat) skui_input_carat = mini((int32_t)utf_charlen(buffer), skui_input_carat + 1); skui_input_carat_end = skui_input_carat; }
	}

	// Render the input UI
	vec2 text_bounds = { final_size.x - skui_settings.padding * 2,final_size.y };
	ui_draw_el(ui_vis_input, final_pos, vec3{ final_size.x, final_size.y, skui_settings.depth/2 }, color_blend);

	// Swap out for a string of asterisks to hide any password
	const C* draw_text = buffer;
	if (type == text_context_password) {
		size_t len          = utf_charlen(buffer);
		C*     password_txt = (C*)alloca(sizeof(C) * (len + 1));
		for (size_t i = 0; i < len; i++)
			password_txt[i] = '*';
		password_txt[len] = '\0';
		draw_text = password_txt;
	}

	// If the input is focused, display text selection information
	if (skui_input_target == id_hash) {
		// Advance the displayed text if it's off the right side of the input
		text_style_t style = ui_get_text_style();

		int32_t carat_at      = skui_input_carat;
		vec2    carat_pos     = text_char_at_o(draw_text, style, carat_at, &text_bounds, text_fit_clip, text_align_top_left, text_align_center_left);
		float   scroll_margin = text_bounds.x - text_style_get_char_height(ui_get_text_style());
		while (carat_pos.x < -scroll_margin && *draw_text != '\0' && carat_at >= 0) {
			draw_text += 1;
			carat_at  -= 1;
			carat_pos = text_char_at_o(draw_text, style, carat_at, &text_bounds, text_fit_clip, text_align_top_left, text_align_center_left);
		}

		// Display a selection box for highlighted text
		float line = ui_line_height() * 0.5f;
		if (skui_input_carat != skui_input_carat_end) {
			int32_t end       = maxi(0, carat_at + (skui_input_carat_end - skui_input_carat));
			vec2    carat_end = text_char_at_o(draw_text, style, end, &text_bounds, text_fit_clip, text_align_top_left, text_align_center_left);
			float   left      = fmaxf(carat_pos.x, carat_end.x);
			float   right     = fmaxf(fminf(carat_pos.x, carat_end.x), -text_bounds.x);

			vec3 sz  = vec3{ -(right - left), line, line * 0.01f };
			vec3 pos = (final_pos - vec3{ skui_settings.padding - left, -carat_pos.y, skui_settings.depth / 2 + 1 * mm2m }) - sz / 2;
			ui_draw_cube(pos, sz, ui_color_complement, 0);
		} 

		// Show a blinking text carat
		if ((int)((time_totalf_unscaled()-skui_input_blink)*2)%2==0) {
			ui_draw_el(ui_vis_carat, final_pos - vec3{ skui_settings.padding - carat_pos.x, -carat_pos.y, skui_settings.depth/2 }, vec3{ line * 0.1f, line, line * 0.1f }, 0);
		}
	}

	ui_text_at(draw_text, text_align_center_left, text_fit_clip, final_pos - vec3{ skui_settings.padding, 0, skui_settings.depth / 2 + 2 * mm2m }, text_bounds);

	return result;
}

bool32_t ui_input(const char *id, char *buffer, int32_t buffer_size, vec2 size, text_context_ type) {
	return ui_input_g<char>(id, buffer, buffer_size, size, type);
}
bool32_t ui_input_16(const char16_t *id, char16_t *buffer, int32_t buffer_size, vec2 size, text_context_ type) {
	return ui_input_g<char16_t>(id, buffer, buffer_size, size, type);
}

///////////////////////////////////////////

void ui_progress_bar_at_ex(float percent, vec3 start_pos, vec2 size, float focus, bool vertical) {
	// For a vertical progress bar, the easiest thing is to just rotate the
	// hierarchy 90, as this simplifies any issues with trying to rotate the
	// calls to draw the left and right line segments.
	hierarchy_push(vertical
		? matrix_trs(start_pos - vec3{size.x,0,0}, quat_from_angles(0, 0, 90))
		: matrix_t  (start_pos));
	if (vertical) size = { size.y, size.x };

	// Find sizes of bar elements
	float bar_height = fmaxf(skui_settings.padding, size.y / 6.f);
	float bar_depth  = bar_height * skui_pressed_depth - mm2m;
	float bar_y      = -size.y / 2.f + bar_height / 2.f;

	// If the left or right side of the bar is too small, then we'll just draw
	// a single solid bar.
	float bar_length = math_saturate(percent) * size.x;
	vec2  min_size   = ui_get_mesh_minsize(ui_vis_slider_line_active);
	if (bar_length <= min_size.x) {
		ui_draw_el_color(ui_vis_slider_line, ui_vis_slider_line_inactive,
			vec3{ 0,      bar_y,      0 },
			vec3{ size.x, bar_height, bar_depth },
			focus);
		hierarchy_pop();
		return;
	} else if (bar_length >= size.x-min_size.x) {
		ui_draw_el_color(ui_vis_slider_line, ui_vis_slider_line_active,
			vec3{ 0,      bar_y,      0 },
			vec3{ size.x, bar_height, bar_depth },
			focus);
		hierarchy_pop();
		return;
	}

	// Slide line
	ui_draw_el(ui_vis_slider_line_active,
		vec3{ 0,          bar_y,      0 },
		vec3{ bar_length, bar_height, bar_depth },
		focus);
	ui_draw_el(ui_vis_slider_line_inactive,
		vec3{        - bar_length, bar_y,      0 },
		vec3{ size.x - bar_length, bar_height, bar_depth },
		focus);
	hierarchy_pop();
}

///////////////////////////////////////////

void ui_progress_bar_at(float percent, vec3 start_pos, vec2 size) {
	ui_progress_bar_at_ex(percent, start_pos, size, 0, false);
}

///////////////////////////////////////////

void ui_progress_bar(float percent, float width) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz({ width, 0 }, false, &final_pos, &final_size);

	return ui_progress_bar_at(percent, final_pos, final_size);
}

///////////////////////////////////////////

template<typename C, typename N>
bool32_t ui_slider_at_g(bool vertical, const C *id_text, N &value, N min, N max, N step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) {
	uint64_t id     = ui_stack_hash(id_text);
	bool     result = false;

	const float snap_scale = 1;
	const float snap_dist  = 7*cm2m;

	float size_min = vertical ? size.x : size.y;

	// Find sizes of slider elements
	float percent      = (float)((value - min) / (max - min));
	float button_depth = confirm_method == ui_confirm_push ? skui_settings.depth : skui_settings.depth * 1.5f;
	float rule_size    = fmaxf(skui_settings.padding, size_min / 6.f);
	vec2  button_size  = confirm_method == ui_confirm_push
		? vec2{ size_min / 2, size_min / 2 }
		: (vertical
			? vec2{ size_min, size_min / 4 }
			: vec2{ size_min / 4, size_min } );

	// Activation bounds sizing
	float activation_plane = button_depth + skui_finger_radius;

	// Set up for getting the state of the sliders.
	button_state_ focus_state   = button_state_inactive;
	button_state_ button_state  = button_state_inactive;
	float         finger_offset = button_depth;
	float         finger_at     = 0;
	int32_t       hand          = -1;
	if (confirm_method == ui_confirm_push) {
		vec3  activation_start = vertical
			? window_relative_pos + vec3{ -(size.x / 2 - button_size.x / 2) + button_size.x / 2.0f, percent * -(size.y-button_size.y) + button_size.y/2.0f, -activation_plane }
			: window_relative_pos + vec3{ percent * -(size.x-button_size.x) + button_size.x/2.0f, -(size.y/2 - button_size.y/2) + button_size.y/2.0f, -activation_plane };
		vec3  activation_size = vec3{ button_size.x*2, button_size.y*2, 0.0001f };
		vec3  sustain_size    = vec3{ size.x + 2*skui_finger_radius, size.y + 2*skui_finger_radius, activation_plane + 6*skui_finger_radius  };
		vec3  sustain_start   = window_relative_pos + vec3{ skui_finger_radius, skui_finger_radius, -activation_plane + sustain_size.z };

		ui_box_interaction_1h_poke(id,
			activation_start, activation_size,
			sustain_start,    sustain_size,
			&focus_state, &hand);

		// Here, we allow for pressing or pinching of the button to activate
		// the slider!
		if (hand != -1) {
			const hand_t* h     = input_hand((handed_)hand);
			button_state_ pinch = h->pinch_state;

			finger_offset = -skui_hand[hand].finger.z - window_relative_pos.z;
			bool pressed  = finger_offset < button_depth / 2;
			finger_offset = fminf(fmaxf(2 * mm2m, finger_offset), button_depth);

			button_state = ui_active_set(hand, id, pinch & button_state_active || pressed);
			// Focus can get lost if the user is dragging outside the box, so set
			// it to focused if it's still active.
			focus_state = ui_focus_set(hand, id, pinch & button_state_active || focus_state & button_state_active, 0);

			finger_at = vertical ? skui_hand[hand].finger.y : skui_hand[hand].finger.x;
		}
	} else if (confirm_method == ui_confirm_pinch || confirm_method == ui_confirm_variable_pinch) {
		vec3 activation_start;
		vec3 activation_size;
		if (vertical) {
			activation_start = window_relative_pos + vec3{ -(size.x / 2 - button_size.x / 2), percent * -(size.y - button_size.y) + button_size.y, button_depth };
			activation_size  = vec3{ button_size.x, button_size.y * 3, button_depth * 2 };
		} else {
			activation_start = window_relative_pos + vec3{ percent * -(size.x - button_size.x) + button_size.x, -(size.y / 2 - button_size.y / 2), button_depth };
			activation_size  = vec3{ button_size.x * 3, button_size.y, button_depth * 2 };
		}

		ui_box_interaction_1h_pinch(id,
			activation_start, activation_size,
			activation_start, activation_size,
			&focus_state, &hand);

		// Pinch confirm uses a handle that the user must pinch, in order to
		// drag it around the slider.
		if (hand != -1) {
			const hand_t *h     = input_hand((handed_)hand);
			button_state_ pinch = h->pinch_state;
			button_state = ui_active_set(hand, id, pinch & button_state_active);
			// Focus can get lost if the user is dragging outside the box, so set
			// it to focused if it's still active.
			focus_state = ui_focus_set(hand, id, button_state & button_state_active || focus_state & button_state_active, 0);
			vec3    pinch_local = hierarchy_to_local_point(h->pinch_pt);
			int32_t scale_step  = (int32_t)((-pinch_local.z-activation_plane) / snap_dist);
			finger_at = vertical ? pinch_local.y : pinch_local.x;

			if (confirm_method == ui_confirm_variable_pinch && button_state & button_state_active && scale_step > 0) {
				finger_at = finger_at / (1 + scale_step * snap_scale);
			}
		}
	}

	if (button_state & button_state_active) {
		float pos_in_slider = vertical
			? (float)fmin(1, fmax(0, ((window_relative_pos.y-button_size.y/2)-finger_at) / (size.y-button_size.y)))
			: (float)fmin(1, fmax(0, ((window_relative_pos.x-button_size.x/2)-finger_at) / (size.x-button_size.x)));
		N new_val = (N)min + (N)pos_in_slider*(N)(max-min);
		if (step != 0) {
			new_val = min + ((int)(((new_val - min) / step) + (N)0.5)) * step;
		}
		result  = value != new_val;
		percent = (float)((new_val - min) / (max - min));

		// Play tick sound as the value updates
		if (result) {
			
			if (step != 0) {
				// Play on every change if there's a user specified step value
				ui_play_sound_on(ui_vis_slider_line, skui_hand[hand].finger_world);
			} else {
				// If no user specified step, then we'll do a set number of
				// clicks across the whole bar.
				const int32_t click_steps = 10;

				float   old_percent  = (float)((value - min) / (max - min));
				int32_t old_quantize = (int32_t)(old_percent * click_steps + 0.5f);
				int32_t new_quantize = (int32_t)(percent     * click_steps + 0.5f);

				if (old_quantize != new_quantize) {
					ui_play_sound_on(ui_vis_slider_line, skui_hand[hand].finger_world);
				}
			}
		}

		// Do this down here so we can calculate old_percent above
		value = new_val;
	}

	if (button_state & button_state_just_active)
		ui_anim_start(id, 0);
	float color_blend = focus_state & button_state_active ? 1.0f : 0.0f;
	if (ui_anim_has(id, 0, skui_anim_duration)) {
		float t     = ui_anim_elapsed(id, 0, skui_anim_duration);
		color_blend = math_ease_overshoot(0, 1, skui_anim_overshoot, t);
	}

	// Draw the UI
	float x           = window_relative_pos.x;
	float y           = window_relative_pos.y;
	float slide_x_rel = 0;
	float slide_y_rel = 0;
	if (vertical) {
		slide_x_rel = (size.x - button_size.x) / 2;
		slide_y_rel = (float)(percent * (size.y - button_size.y));
	} else {
		slide_x_rel = (float)(percent * (size.x - button_size.x));
		slide_y_rel = (size.y - button_size.y) / 2;
	}

	ui_progress_bar_at_ex(percent, window_relative_pos, size, color_blend, vertical);

	if (confirm_method == ui_confirm_push) {
		ui_draw_el(ui_vis_slider_push,
			vec3{x - slide_x_rel, y - slide_y_rel, window_relative_pos.z},
			vec3{button_size.x, button_size.y, fmaxf(finger_offset,rule_size*skui_pressed_depth +mm2m)},
			color_blend);
	} else if (confirm_method == ui_confirm_pinch || confirm_method == ui_confirm_variable_pinch) {
		ui_draw_el(ui_vis_slider_pinch,
			vec3{x - slide_x_rel, y - slide_y_rel, window_relative_pos.z},
			vec3{button_size.x, button_size.y, button_depth},
			color_blend);

		vec3 pinch_local = hand < 0
			? vec3_zero
			: hierarchy_to_local_point(input_hand((handed_)hand)->pinch_pt);
		int32_t scale_step  = (int32_t)((-pinch_local.z-activation_plane) / snap_dist);
		if (confirm_method == ui_confirm_variable_pinch && button_state & button_state_active && scale_step > 0) {
			float scale     = 1 + scale_step * snap_scale;
			float z         = -activation_plane - (scale_step * snap_dist) + button_depth/2;
			float scaled_at = vertical
				? y+size.y*(scale-1)*0.5f
				: x+size.x*(scale-1)*0.5f;
			
			if (vertical) {
				float connector_x = (x-slide_x_rel) - size_min * 0.5f;
				line_add({ connector_x, y,        window_relative_pos.z}, { connector_x, scaled_at,              window_relative_pos.z + z}, {255,255,255,0}, {255,255,255,255}, rule_size*0.5f);
				line_add({ connector_x, y-size.y, window_relative_pos.z}, { connector_x, scaled_at-size.y*scale, window_relative_pos.z + z}, {255,255,255,0}, {255,255,255,255}, rule_size*0.5f);
			} else {
				float connector_y = (y-slide_y_rel) - size_min * 0.5f;
				line_add({ x,        connector_y, window_relative_pos.z}, { scaled_at,              connector_y, window_relative_pos.z + z}, {255,255,255,0}, {255,255,255,255}, rule_size*0.5f);
				line_add({ x-size.x, connector_y, window_relative_pos.z}, { scaled_at-size.x*scale, connector_y, window_relative_pos.z + z}, {255,255,255,0}, {255,255,255,255}, rule_size*0.5f);
			}

			if (vertical) {
				ui_progress_bar_at_ex(percent,
					vec3{ x-slide_x_rel, scaled_at, window_relative_pos.z + z },
					vec2{ size.x, size.y*scale },
					color_blend, vertical);
			} else {
				ui_progress_bar_at_ex(percent,
					vec3{ scaled_at, y-slide_y_rel, window_relative_pos.z + z },
					vec2{ size.x*scale, size.y },
					color_blend, vertical);
			}
			ui_draw_el(ui_vis_slider_pinch,
				vertical
					? vec3{ x - slide_x_rel, scaled_at - slide_y_rel * scale, window_relative_pos.z + z }
					: vec3{ scaled_at - slide_x_rel * scale, y - slide_y_rel, window_relative_pos.z + z },
				vec3{ button_size.x, button_size.y, button_depth },
				color_blend);
		}
	}
	
	if (hand >= 0 && hand < 2) {
		if (button_state & button_state_just_active)
			ui_play_sound_on_off(ui_vis_slider_pinch, id, skui_hand[hand].finger_world);
	}

	if      (notify_on == ui_notify_change)   return result;
	else if (notify_on == ui_notify_finalize) return button_state & button_state_just_inactive;
	else                                      return result;
}
bool32_t ui_hslider_at       (const char     *id_text, float  &value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_at_g<char    , float>(false, id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }
bool32_t ui_hslider_at       (const char16_t *id_text, float  &value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_at_g<char16_t, float>(false, id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }
bool32_t ui_hslider_at_16    (const char16_t *id_text, float  &value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_at_g<char16_t, float>(false, id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }

bool32_t ui_hslider_at_f64   (const char     *id_text, double &value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { float tmp = (float)value; bool32_t result = ui_slider_at_g<char    , float>(false, id_text, tmp, (float)min, (float)max, (float)step, window_relative_pos, size, confirm_method, notify_on); value = tmp; return result;}
bool32_t ui_hslider_at_f64   (const char16_t *id_text, double &value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { float tmp = (float)value; bool32_t result = ui_slider_at_g<char16_t, float>(false, id_text, tmp, (float)min, (float)max, (float)step, window_relative_pos, size, confirm_method, notify_on); value = tmp; return result;}
bool32_t ui_hslider_at_f64_16(const char16_t* id_text, double& value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { float tmp = (float)value; bool32_t result = ui_slider_at_g<char16_t, float>(false, id_text, tmp, (float)min, (float)max, (float)step, window_relative_pos, size, confirm_method, notify_on); value = tmp; return result;}

bool32_t ui_vslider_at       (const char     *id_text, float  &value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_at_g<char    , float>(true, id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }
bool32_t ui_vslider_at       (const char16_t *id_text, float  &value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_at_g<char16_t, float>(true, id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }
bool32_t ui_vslider_at_16    (const char16_t *id_text, float  &value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_at_g<char16_t, float>(true, id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }

bool32_t ui_vslider_at_f64   (const char     *id_text, double &value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { float tmp = (float)value; bool32_t result = ui_slider_at_g<char    , float>(true, id_text, tmp, (float)min, (float)max, (float)step, window_relative_pos, size, confirm_method, notify_on); value = tmp; return result;}
bool32_t ui_vslider_at_f64   (const char16_t *id_text, double &value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { float tmp = (float)value; bool32_t result = ui_slider_at_g<char16_t, float>(true, id_text, tmp, (float)min, (float)max, (float)step, window_relative_pos, size, confirm_method, notify_on); value = tmp; return result;}
bool32_t ui_vslider_at_f64_16(const char16_t *id_text, double &value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { float tmp = (float)value; bool32_t result = ui_slider_at_g<char16_t, float>(true, id_text, tmp, (float)min, (float)max, (float)step, window_relative_pos, size, confirm_method, notify_on); value = tmp; return result;}

///////////////////////////////////////////

template<typename C, typename N>
bool32_t ui_slider_g(bool vertical, const C *name, N &value, N min, N max, N step, float width, ui_confirm_ confirm_method, ui_notify_ notify_on) {
	vec3 final_pos;
	vec2 final_size;
	if (vertical) ui_layout_reserve_vertical_sz({width, 0}, false, &final_pos, &final_size);
	else          ui_layout_reserve_sz         ({width, 0}, false, &final_pos, &final_size);

	return ui_slider_at_g<C, N>(vertical, name, value, min, max, step, final_pos, final_size, confirm_method, notify_on);
}

bool32_t ui_hslider       (const char     *name, float  &value, float  min, float  max, float  step, float width,  ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_g<char,     float >(false, name, value, min, max, step, width, confirm_method, notify_on); }
bool32_t ui_hslider_16    (const char16_t *name, float  &value, float  min, float  max, float  step, float width,  ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_g<char16_t, float >(false, name, value, min, max, step, width, confirm_method, notify_on); }

bool32_t ui_hslider_f64   (const char     *name, double &value, double min, double max, double step, float width,  ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_g<char,     double>(false, name, value, min, max, step, width, confirm_method, notify_on); }
bool32_t ui_hslider_f64_16(const char16_t *name, double &value, double min, double max, double step, float width,  ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_g<char16_t, double>(false, name, value, min, max, step, width, confirm_method, notify_on); }

bool32_t ui_vslider       (const char     *name, float  &value, float  min, float  max, float  step, float height, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_g<char,     float >(true,  name, value, min, max, step, height, confirm_method, notify_on); }
bool32_t ui_vslider_16    (const char16_t *name, float  &value, float  min, float  max, float  step, float height, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_g<char16_t, float >(true,  name, value, min, max, step, height, confirm_method, notify_on); }

bool32_t ui_vslider_f64   (const char     *name, double &value, double min, double max, double step, float height, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_g<char,     double>(true,  name, value, min, max, step, height, confirm_method, notify_on); }
bool32_t ui_vslider_f64_16(const char16_t *name, double &value, double min, double max, double step, float height, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_slider_g<char16_t, double>(true,  name, value, min, max, step, height, confirm_method, notify_on); }

///////////////////////////////////////////


///////////////////////////////////////////

template<typename C>
void ui_window_begin_g(const C *text, pose_t &pose, vec2 window_size, ui_win_ window_type, ui_move_ move_type) {
	uint64_t     hash   = ui_push_id(text);
	ui_window_id win_id = ui_window_find_or_add(hash, window_size);
	ui_window_t* win    = ui_window_get(win_id);
	win->age  = 0;
	win->type = window_type;
	win->move = move_type;
	
	// figure out the size of it, based on its window type
	vec3 box_start = {}, box_size = {};
	if (win->type & ui_win_head) {
		float line = ui_line_height();
		box_start = vec3{ 0, line/2, skui_settings.depth/2 };
		box_size  = vec3{ win->prev_size.x, line, skui_settings.depth*2 };
	}
	if (win->type & ui_win_body || win->type & ui_win_empty) {
		box_start.z  = skui_settings.depth/2;
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
	box_start.z += 0.01f;
	box_size .z += 0.02f;

	// Set up window handle and layout area
	_ui_handle_begin(hash, pose, { box_start, box_size }, false, move_type, ui_gesture_pinch);
	ui_layout_push_win(win_id, { win->prev_size.x / 2,0,0 }, window_size, true);

	// draw label
	if (win->type & ui_win_head) {
		ui_layout_t* layout = ui_layout_curr();

		vec2 txt_size = text_size(text, ui_get_text_style());
		vec2 size     = vec2{ window_size.x == 0 ? txt_size.x : window_size.x-(skui_settings.margin*2), ui_line_height() };
		vec3 at       = layout->offset - vec3{ skui_settings.padding, -(size.y+skui_settings.margin), 2*mm2m };

		ui_text_at(text, text_align_center_left, text_fit_squeeze, at, size);

		float header_width = window_size.x == 0 ? size.x + skui_settings.padding * 2 + skui_settings.margin * 2 : size.x;
		if (win->curr_size.x < header_width)
			win->curr_size.x = header_width;
	}
	win->pose = pose;
}
void ui_window_begin(const char *text, pose_t &pose, vec2 window_size, ui_win_ window_type, ui_move_ move_type) {
	ui_window_begin_g<char>(text, pose, window_size, window_type, move_type);
}
void ui_window_begin_16(const char16_t *text, pose_t &pose, vec2 window_size, ui_win_ window_type, ui_move_ move_type) {
	ui_window_begin_g<char16_t>(text, pose, window_size, window_type, move_type);
}

///////////////////////////////////////////

void ui_window_end() {
	ui_window_id win_id      = ui_layout_curr_window();
	ui_window_t* win         = ui_window_get(win_id);
	float        line_height = ui_line_height();
	ui_layout_pop();
	win->prev_size.x = win->layout_size.x == 0 ? win->curr_size.x : win->layout_size.x;
	win->prev_size.y = win->layout_size.y == 0 ? win->curr_size.y : win->layout_size.y;

	vec3 start = win->layout_start + vec3{ 0,0,skui_settings.depth };
	vec3 size  = { win->prev_size.x, win->prev_size.y, skui_settings.depth };

	// Focus animation
	if ((skui_hand[0].focused_prev == win->hash && skui_hand[0].focused_prev_prev != win->hash) ||
		(skui_hand[1].focused_prev == win->hash && skui_hand[1].focused_prev_prev != win->hash))
		ui_anim_start(win->hash, 0);
	if ((skui_hand[0].focused_prev != win->hash && skui_hand[0].focused_prev_prev == win->hash) ||
		(skui_hand[1].focused_prev != win->hash && skui_hand[1].focused_prev_prev == win->hash))
		ui_anim_start(win->hash, 1);

	float focus = ui_id_focused(win->hash) & button_state_active ? 1.0f : 0.0f;
	if (ui_anim_has(win->hash, 0, skui_anim_focus_duration)) {
		focus = math_ease_smooth(0, 1, ui_anim_elapsed(win->hash, 0, skui_anim_focus_duration));
	} else if (ui_anim_has(win->hash, 1, skui_anim_focus_duration)) {
		focus = math_ease_smooth(1, 0, ui_anim_elapsed(win->hash, 1, skui_anim_focus_duration));
	}

	if (win->move != ui_move_none && ui_grab_aura_enabled()) {
		vec3 aura_start = vec3{ start.x+skui_aura_radius,  start.y+skui_aura_radius,  start.z };
		vec3 aura_size  = vec3{ size .x+skui_aura_radius*2,size .y+skui_aura_radius*2,size .z };
		if (win->type & ui_win_head) { aura_start.y += line_height; aura_size.y += line_height; }
		ui_draw_el(ui_vis_aura, aura_start, aura_size, focus);
	}

	if (win->type & ui_win_head) {
		ui_draw_el(win->type == ui_win_head ? ui_vis_window_head_only : ui_vis_window_head, start + vec3{0,line_height,0}, { size.x, line_height, size.z }, focus);
	}
	if (win->type & ui_win_body) {
		ui_draw_el(win->type == ui_win_body ? ui_vis_window_body_only : ui_vis_window_body, start, size, 0);
	}
	ui_handle_end();
	ui_pop_id();
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
	ui_draw_el(ui_vis_panel, start+start_offset, vec3{ size.x, size.y, skui_settings.depth* 0.1f }+size_offset, 0);
}

} // namespace sk

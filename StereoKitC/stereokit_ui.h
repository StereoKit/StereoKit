#pragma once

#include "stereokit.h"

#ifdef __cplusplus
namespace sk {
#endif

typedef enum ui_move_ {
	ui_move_exact = 0,
	ui_move_face_user,
	ui_move_pos_only,
	ui_move_none,
} ui_move_;

typedef enum ui_win_ {
	ui_win_empty  = 1 << 0,
	ui_win_head   = 1 << 1,
	ui_win_body   = 1 << 2,
	ui_win_normal = ui_win_head | ui_win_body,
} ui_win_;
SK_MakeFlag(ui_win_)

typedef enum ui_confirm_ {
	ui_confirm_push,
	ui_confirm_pinch,
	ui_confirm_variable_pinch
} ui_confirm_;

typedef enum ui_gesture_ {
	ui_gesture_none       = 0,
	ui_gesture_pinch      = 1 << 0,
	ui_gesture_grip       = 1 << 1,
	ui_gesture_pinch_grip = ui_gesture_pinch | ui_gesture_grip,
} ui_gesture_;
SK_MakeFlag(ui_gesture_)

typedef enum ui_notify_ {
	ui_notify_change,
	ui_notify_finalize,
} ui_notify_;

typedef enum ui_vis_ {
	ui_vis_none = 0,
	ui_vis_default,
	ui_vis_button,
	ui_vis_toggle,
	ui_vis_input,
	ui_vis_handle,
	ui_vis_window_body,
	ui_vis_window_body_only,
	ui_vis_window_head,
	ui_vis_window_head_only,
	ui_vis_separator,
	ui_vis_slider_line,
	ui_vis_slider_line_active,
	ui_vis_slider_line_inactive,
	ui_vis_slider_push,
	ui_vis_slider_pinch,
	ui_vis_button_round,
	ui_vis_panel,
	ui_vis_carat,
	ui_vis_aura,
	ui_vis_max,
} ui_vis_;

typedef enum ui_color_ {
	ui_color_none = 0,
	ui_color_primary,
	ui_color_background,
	ui_color_common,
	ui_color_complement,
	ui_color_text,
	ui_color_max,
} ui_color_;

typedef enum ui_color_state_ {
	ui_color_state_normal,
	ui_color_state_active,
	ui_color_state_disabled,
} ui_color_state_;

typedef enum ui_pad_ {
	ui_pad_none,
	ui_pad_inside,
	ui_pad_outside,
} ui_pad_;

typedef enum ui_btn_layout_ {
	ui_btn_layout_none,
	ui_btn_layout_left,
	ui_btn_layout_right,
	ui_btn_layout_center,
	ui_btn_layout_center_no_text,
} ui_btn_layout_;

typedef enum ui_cut_ {
	ui_cut_left,
	ui_cut_right,
	ui_cut_top,
	ui_cut_bottom,
} ui_cut_;

typedef struct ui_settings_t {
	float margin;
	float padding;
	float gutter;
	float depth;
	float rounding;
	float backplate_depth;
	float backplate_border;
} ui_settings_t;

SK_API void     ui_quadrant_size_verts  (vert_t *ref_vertices, int32_t vertex_count, float overflow_percent);
SK_API void     ui_quadrant_size_mesh   (mesh_t ref_mesh, float overflow_percent);
SK_API void     ui_show_volumes         (bool32_t      show);
SK_API void     ui_enable_far_interact  (bool32_t      enable);
SK_API bool32_t ui_far_interact_enabled (void);
SK_API ui_move_ ui_system_get_move_type (void);
SK_API void     ui_system_set_move_type (ui_move_ move_type);
SK_API void     ui_settings             (ui_settings_t settings);
SK_API ui_settings_t ui_get_settings    (void);
SK_API float    ui_get_margin           (void);
SK_API float    ui_get_padding          (void);
SK_API float    ui_get_gutter           (void);
SK_API void     ui_set_color            (color128      color);
SK_API void     ui_set_theme_color      (ui_color_ color_type, color128 color_gamma);
SK_API color128 ui_get_theme_color      (ui_color_ color_type);
SK_API void     ui_set_theme_color_state(ui_color_ color_type, ui_color_state_ state, color128 color_gamma);
SK_API color128 ui_get_theme_color_state(ui_color_ color_type, ui_color_state_ state);
SK_API void     ui_set_element_visual   (ui_vis_ element_visual, mesh_t mesh, material_t material sk_default(nullptr), vec2 min_size sk_default(vec2_zero));
SK_API void     ui_set_element_color    (ui_vis_ element_visual, ui_color_ color_category);
SK_API void     ui_set_element_sound    (ui_vis_ element_visual, sound_t activate, sound_t deactivate);
SK_API bool32_t ui_has_keyboard_focus   (void);
SK_API pose_t   ui_popup_pose           (vec3 shift);

SK_API void     ui_push_grab_aura        (bool32_t enabled);
SK_API void     ui_pop_grab_aura         ();
SK_API bool32_t ui_grab_aura_enabled     ();
SK_API void     ui_push_text_style       (text_style_t  style);
SK_API void     ui_pop_text_style        (void);
SK_API text_style_t ui_get_text_style    (void);
SK_API void     ui_push_tint             (color128 tint_gamma);
SK_API void     ui_pop_tint              (void);
SK_API void     ui_push_enabled          (bool32_t enabled);
SK_API void     ui_pop_enabled           (void);
SK_API void     ui_push_preserve_keyboard(bool32_t preserve_keyboard);
SK_API void     ui_pop_preserve_keyboard (void);
SK_API void     ui_push_surface          (pose_t surface_pose, vec3 layout_start sk_default(vec3_zero), vec2 layout_dimensions sk_default(vec2_zero));
SK_API void     ui_pop_surface           (void);
SK_API uint64_t ui_push_id               (const char     *id);
SK_API uint64_t ui_push_id_16            (const char16_t *id);
SK_API uint64_t ui_push_idi              (int32_t id);
SK_API void     ui_pop_id                (void);
SK_API uint64_t ui_stack_hash            (const char     *string);
SK_API uint64_t ui_stack_hash_16         (const char16_t *string);

SK_API void     ui_layout_area     (vec3 start, vec2 dimensions, bool32_t add_margin sk_default(true));
SK_API vec2     ui_layout_remaining(void);
SK_API vec3     ui_layout_at       (void);
SK_API bounds_t ui_layout_last     (void);
SK_API bounds_t ui_layout_reserve  (vec2 size, bool32_t add_padding sk_default(false), float depth sk_default(0));
SK_API void     ui_layout_push     (vec3 start, vec2 dimensions, bool32_t add_margin);
SK_API void     ui_layout_push_cut (ui_cut_ cut_to, float size, bool32_t add_margin sk_default(false));
SK_API void     ui_layout_pop      (void);

SK_API button_state_ ui_last_element_hand_used   (handed_ hand); // TODO: remove in v0.4
SK_API button_state_ ui_last_element_hand_active (handed_ hand);
SK_API button_state_ ui_last_element_hand_focused(handed_ hand);
SK_API button_state_ ui_last_element_active   (void);
SK_API button_state_ ui_last_element_focused  (void);

SK_API vec2     ui_area_remaining(void); // TODO: remove in v0.4, prefer ui_layout_remaining
SK_API void     ui_nextline      (void);
SK_API void     ui_sameline      (void);
SK_API float    ui_line_height   (void);

SK_API bool32_t ui_is_interacting       (handed_ hand);
SK_API void     ui_button_behavior      (vec3 window_relative_pos, vec2 size, uint64_t id, sk_ref(float) out_finger_offset, sk_ref(button_state_) out_button_state, sk_ref(button_state_) out_focus_state, int32_t* out_opt_hand sk_default(nullptr));
SK_API void     ui_button_behavior_depth(vec3 window_relative_pos, vec2 size, uint64_t id, float button_depth, float button_activation_depth, sk_ref(float) out_finger_offset, sk_ref(button_state_) out_button_state, sk_ref(button_state_) out_focus_state, int32_t* out_opt_hand sk_default(nullptr));

SK_API button_state_ ui_volumei_at        (const char     *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand sk_default(nullptr), button_state_ *out_opt_focus_state sk_default(nullptr));
SK_API button_state_ ui_volumei_at_16     (const char16_t *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand sk_default(nullptr), button_state_ *out_opt_focus_state sk_default(nullptr));
SK_API bool32_t      ui_volume_at         (const char     *id, bounds_t bounds); // TODO: remove in v0.4
SK_API bool32_t      ui_volume_at_16      (const char16_t *id, bounds_t bounds); // TODO: remove in v0.4
SK_API button_state_ ui_interact_volume_at(bounds_t bounds, sk_ref(handed_) out_hand);  // TODO: remove in v0.4

SK_API void     ui_label             (const char*     text, bool32_t use_padding sk_default(true));
SK_API void     ui_label_16          (const char16_t* text, bool32_t use_padding sk_default(true));
SK_API void     ui_label_sz          (const char*     text, vec2 size, bool32_t use_padding sk_default(true));
SK_API void     ui_label_sz_16       (const char16_t* text, vec2 size, bool32_t use_padding sk_default(true));
SK_API void     ui_text              (const char*     text, text_align_ text_align sk_default(text_align_top_left));
SK_API void     ui_text_16           (const char16_t* text, text_align_ text_align sk_default(text_align_top_left));
SK_API void     ui_text_sz           (const char*     text, text_align_ text_align, text_fit_ fit, vec2 size);
SK_API void     ui_text_sz_16        (const char16_t* text, text_align_ text_align, text_fit_ fit, vec2 size);
SK_API void     ui_text_at           (const char*     text, text_align_ text_align, text_fit_ fit, vec3 window_relative_pos, vec2 size);
SK_API void     ui_text_at_16        (const char16_t* text, text_align_ text_align, text_fit_ fit, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_button            (const char*     text);
SK_API bool32_t ui_button_16         (const char16_t* text);
SK_API bool32_t ui_button_sz         (const char*     text, vec2 size);
SK_API bool32_t ui_button_sz_16      (const char16_t* text, vec2 size);
SK_API bool32_t ui_button_at         (const char*     text, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_button_at_16      (const char16_t* text, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_button_img        (const char*     text, sprite_t image, ui_btn_layout_ image_layout);
SK_API bool32_t ui_button_img_16     (const char16_t* text, sprite_t image, ui_btn_layout_ image_layout);
SK_API bool32_t ui_button_img_color_16(const char16_t* text, sprite_t image, ui_btn_layout_ image_layout, color128 img_tint);
SK_API bool32_t ui_button_img_sz     (const char*     text, sprite_t image, ui_btn_layout_ image_layout, vec2 size);
SK_API bool32_t ui_button_img_sz_16  (const char16_t* text, sprite_t image, ui_btn_layout_ image_layout, vec2 size);
SK_API bool32_t ui_button_img_sz_color_16(const char16_t* text, sprite_t image, ui_btn_layout_ image_layout, vec2 size, color128 img_tint);
SK_API bool32_t ui_button_img_at     (const char*     text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_button_img_at_color_16(const char16_t* text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, color128 img_tint);
SK_API bool32_t ui_button_img_at_16  (const char16_t* text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_button_round      (const char*     id,   sprite_t image, float diameter sk_default(0));
SK_API bool32_t ui_button_round_16   (const char16_t* id,   sprite_t image, float diameter sk_default(0));
SK_API bool32_t ui_button_round_at   (const char*     id,   sprite_t image, vec3 window_relative_pos, float diameter);
SK_API bool32_t ui_button_round_at_16(const char16_t* id,   sprite_t image, vec3 window_relative_pos, float diameter);
SK_API bool32_t ui_toggle            (const char*     text, sk_ref(bool32_t) pressed);
SK_API bool32_t ui_toggle_16         (const char16_t* text, sk_ref(bool32_t) pressed);
SK_API bool32_t ui_toggle_sz         (const char*     text, sk_ref(bool32_t) pressed, vec2 size);
SK_API bool32_t ui_toggle_sz_16      (const char16_t* text, sk_ref(bool32_t) pressed, vec2 size);
SK_API bool32_t ui_toggle_at         (const char     *text, sk_ref(bool32_t) pressed, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_toggle_at_16      (const char16_t *text, sk_ref(bool32_t) pressed, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_toggle_img        (const char*     text, sk_ref(bool32_t) pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout);
SK_API bool32_t ui_toggle_img_16     (const char16_t* text, sk_ref(bool32_t) pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout);
SK_API bool32_t ui_toggle_img_sz     (const char*     text, sk_ref(bool32_t) pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec2 size);
SK_API bool32_t ui_toggle_img_sz_16  (const char16_t* text, sk_ref(bool32_t) pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec2 size);
SK_API bool32_t ui_toggle_img_at     (const char*     text, sk_ref(bool32_t) pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_toggle_img_at_16  (const char16_t* text, sk_ref(bool32_t) pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_hslider           (const char*     id,   sk_ref(float)  value, float  min, float  max, float  step sk_default(0), float width  sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_16        (const char16_t* id,   sk_ref(float)  value, float  min, float  max, float  step sk_default(0), float width  sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_f64       (const char*     id,   sk_ref(double) value, double min, double max, double step sk_default(0), float width  sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_f64_16    (const char16_t* id,   sk_ref(double) value, double min, double max, double step sk_default(0), float width  sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_at        (const char*     id,   sk_ref(float)  value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_at_16     (const char16_t* id,   sk_ref(float)  value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_at_f64    (const char*     id,   sk_ref(double) value, double min, double max, double step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_at_f64_16 (const char16_t* id,   sk_ref(double) value, double min, double max, double step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider           (const char*     id,   sk_ref(float)  value, float  min, float  max, float  step sk_default(0), float height sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_16        (const char16_t* id,   sk_ref(float)  value, float  min, float  max, float  step sk_default(0), float height sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_f64       (const char*     id,   sk_ref(double) value, double min, double max, double step sk_default(0), float height sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_f64_16    (const char16_t* id,   sk_ref(double) value, double min, double max, double step sk_default(0), float height sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_at        (const char*     id,   sk_ref(float)  value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_at_16     (const char16_t* id,   sk_ref(float)  value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_at_f64    (const char*     id,   sk_ref(double) value, double min, double max, double step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_at_f64_16 (const char16_t* id,   sk_ref(double) value, double min, double max, double step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_input             (const char*     id, char     *buffer, int32_t buffer_size, vec2 size sk_default(vec2_zero), text_context_ type sk_default(text_context_::text_context_text));
SK_API bool32_t ui_input_16          (const char16_t* id, char16_t *buffer, int32_t buffer_size, vec2 size sk_default(vec2_zero), text_context_ type sk_default(text_context_::text_context_text));
SK_API void     ui_image             (sprite_t image, vec2 size);
SK_API void     ui_model             (model_t model, vec2 ui_size, float model_scale);
SK_API void     ui_model_at          (model_t model, vec3 start, vec3 size, color128 color);
SK_API void     ui_progress_bar      (float percent, float width sk_default(0));
SK_API void     ui_progress_bar_at   (float percent, vec3 window_relative_pos, vec2 size);
SK_API void     ui_hseparator        (void);
SK_API void     ui_space             (float space);
SK_API void     ui_hspace            (float horizontal_space);
SK_API void     ui_vspace            (float vertical_space);

SK_API bool32_t ui_handle_begin      (const char     *text, sk_ref(pose_t) movement, bounds_t handle, bool32_t draw, ui_move_ move_type sk_default(ui_move_exact), ui_gesture_ allowed_gestures sk_default(ui_gesture_pinch));
SK_API bool32_t ui_handle_begin_16   (const char16_t *text, sk_ref(pose_t) movement, bounds_t handle, bool32_t draw, ui_move_ move_type sk_default(ui_move_exact), ui_gesture_ allowed_gestures sk_default(ui_gesture_pinch));
SK_API void     ui_handle_end        (void);
SK_API void     ui_window_begin      (const char     *text, sk_ref(pose_t) pose, vec2 size sk_default({ 0,0 }), ui_win_ window_type sk_default(ui_win_normal), ui_move_ move_type sk_default(ui_move_face_user));
SK_API void     ui_window_begin_16   (const char16_t *text, sk_ref(pose_t) pose, vec2 size sk_default({ 0,0 }), ui_win_ window_type sk_default(ui_win_normal), ui_move_ move_type sk_default(ui_move_face_user));
SK_API void     ui_window_end        (void);
SK_API void     ui_panel_at          (vec3 start, vec2 size, ui_pad_ padding sk_default(ui_pad_outside));
SK_API void     ui_panel_begin       (ui_pad_ padding sk_default(ui_pad_outside));
SK_API void     ui_panel_end         (void);

///////////////////////////////////////////

SK_CONST char* ui_default_id_radio_on_spr   = "sk/ui/radio_on_spr";
SK_CONST char* ui_default_id_radio_off_spr  = "sk/ui/radio_off_spr";
SK_CONST char* ui_default_id_toggle_on_spr  = "sk/ui/toggle_on_spr";
SK_CONST char* ui_default_id_toggle_off_spr = "sk/ui/toggle_off_spr";

#ifdef __cplusplus
} // namespace sk
#endif
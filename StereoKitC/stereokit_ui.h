#pragma once

#include "stereokit.h"

#ifdef __cplusplus
namespace sk {
#endif

typedef enum ui_move_ {
	ui_move_exact = 0,
	ui_move_face_user,
	ui_move_pos_only,
} ui_move_;

typedef enum ui_win_ {
	ui_win_empty  = 0,
	ui_win_head   = 1 << 0,
	ui_win_body   = 1 << 1,
	ui_win_normal = ui_win_head | ui_win_body,
} ui_win_;

typedef enum ui_confirm_ {
	ui_confirm_push,
	ui_confirm_pinch,
	ui_confirm_variable_pinch
} ui_confirm_;

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
	ui_vis_max,
} ui_vis_;

typedef struct ui_settings_t {
	float padding;
	float gutter;
	float depth;
	float backplate_depth;
	float backplate_border;
} ui_settings_t;

SK_API void     ui_quadrant_size_verts (vert_t *ref_vertices, int32_t vertex_count, float overflow_percent);
SK_API void     ui_quadrant_size_mesh  (mesh_t ref_mesh, float overflow_percent);
SK_API void     ui_show_volumes        (bool32_t      show);
SK_API void     ui_enable_far_interact (bool32_t      enable);
SK_API bool32_t ui_far_interact_enabled();
SK_API void     ui_settings            (ui_settings_t settings);
SK_API void     ui_set_color           (color128      color);
SK_API void     ui_set_element_visual  (ui_vis_ element_visual, mesh_t mesh, material_t material sk_default(nullptr));
SK_API void     ui_push_text_style     (text_style_t  style);
SK_API void     ui_pop_text_style      ();

SK_API void     ui_layout_area     (vec3 start, vec2 dimensions);
SK_API vec2     ui_layout_remaining();
SK_API vec3     ui_layout_at       ();
SK_API bounds_t ui_layout_last     ();
SK_API bounds_t ui_layout_reserve  (vec2 size, bool32_t add_padding sk_default(false), float depth sk_default(0));

SK_API void     ui_push_surface  (pose_t surface_pose, vec3 layout_start sk_default(vec3_zero), vec2 layout_dimensions sk_default(vec2_zero));
SK_API void     ui_pop_surface   ();
SK_API vec2     ui_area_remaining(); // TODO: remove in v0.4, prefer ui_layout_remaining
SK_API void     ui_nextline      ();
SK_API void     ui_sameline      ();
SK_API float    ui_line_height   ();
SK_API void     ui_space         (float space);
SK_API uint64_t ui_push_id       (const char     *id);
SK_API uint64_t ui_push_id_16    (const char16_t *id);
SK_API uint64_t ui_push_idi      (int32_t id);
SK_API void     ui_pop_id        ();
SK_API uint64_t ui_stack_hash    (const char     *string);
SK_API uint64_t ui_stack_hash_16 (const char16_t *string);

SK_API bool32_t ui_is_interacting (handed_ hand);
SK_API void     ui_button_behavior(vec3 window_relative_pos, vec2 size, uint64_t id, sk_ref(float) finger_offset, sk_ref(button_state_) button_state, sk_ref(button_state_) focus_state);

SK_API void          ui_model_at          (model_t model, vec3 start, vec3 size, color128 color);
SK_API button_state_ ui_volumei_at        (const char     *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand sk_default(nullptr), button_state_ *out_opt_focus_state sk_default(nullptr));
SK_API button_state_ ui_volumei_at_16     (const char16_t *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand sk_default(nullptr), button_state_ *out_opt_focus_state sk_default(nullptr));
SK_API bool32_t      ui_volume_at         (const char     *id, bounds_t bounds); // TODO: remove in v0.4
SK_API bool32_t      ui_volume_at_16      (const char16_t *id, bounds_t bounds); // TODO: remove in v0.4
SK_API button_state_ ui_interact_volume_at(bounds_t bounds, sk_ref(handed_) out_hand);  // TODO: remove in v0.4
SK_API bool32_t      ui_button_at         (const char     *text, vec3 window_relative_pos, vec2 size);
SK_API bool32_t      ui_button_at_16      (const char16_t *text, vec3 window_relative_pos, vec2 size);
SK_API bool32_t      ui_button_round_at   (const char     *text, sprite_t image, vec3 window_relative_pos, float diameter);
SK_API bool32_t      ui_button_round_at_16(const char16_t *text, sprite_t image, vec3 window_relative_pos, float diameter);
SK_API bool32_t      ui_toggle_at         (const char     *text, sk_ref(bool32_t) pressed, vec3 window_relative_pos, vec2 size);
SK_API bool32_t      ui_toggle_at_16      (const char16_t *text, sk_ref(bool32_t) pressed, vec3 window_relative_pos, vec2 size);
SK_API bool32_t      ui_hslider_at        (const char     *id,   sk_ref(float) value, float min, float max, float step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method sk_default(ui_confirm_push));
SK_API bool32_t      ui_hslider_at_16     (const char16_t *id,   sk_ref(float) value, float min, float max, float step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method sk_default(ui_confirm_push));
SK_API bool32_t      ui_hslider_at_f64    (const char     *id,   sk_ref(double) value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method sk_default(ui_confirm_push));
SK_API bool32_t      ui_hslider_at_f64_16 (const char16_t *id,   sk_ref(double) value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method sk_default(ui_confirm_push));

SK_API void     ui_hseparator     ();
SK_API void     ui_label          (const char     *text, bool32_t use_padding sk_default(true));
SK_API void     ui_label_16       (const char16_t *text, bool32_t use_padding sk_default(true));
SK_API void     ui_label_sz       (const char     *text, vec2 size);
SK_API void     ui_label_sz_16    (const char16_t *text, vec2 size);
SK_API void     ui_text           (const char     *text, text_align_ text_align sk_default(text_align_top_left));
SK_API void     ui_text_16        (const char16_t *text, text_align_ text_align sk_default(text_align_top_left));
SK_API void     ui_image          (sprite_t image, vec2 size);
SK_API bool32_t ui_button         (const char     *text);
SK_API bool32_t ui_button_16      (const char16_t *text);
SK_API bool32_t ui_button_sz      (const char     *text, vec2 size);
SK_API bool32_t ui_button_sz_16   (const char16_t *text, vec2 size);
SK_API bool32_t ui_button_round   (const char     *id, sprite_t image, float diameter sk_default(0));
SK_API bool32_t ui_button_round_16(const char16_t *id, sprite_t image, float diameter sk_default(0));
SK_API bool32_t ui_toggle         (const char     *text, sk_ref(bool32_t) pressed);
SK_API bool32_t ui_toggle_16      (const char16_t *text, sk_ref(bool32_t) pressed);
SK_API bool32_t ui_toggle_sz      (const char     *text, sk_ref(bool32_t) pressed, vec2 size);
SK_API bool32_t ui_toggle_sz_16   (const char16_t *text, sk_ref(bool32_t) pressed, vec2 size);
//SK_API bool32_t ui_toggle_round(const char *text, bool32_t &pressed);
SK_API void     ui_model          (model_t model, vec2 ui_size, float model_scale);
SK_API bool32_t ui_input          (const char     *id, char     *buffer, int32_t buffer_size, vec2 size sk_default(vec2_zero));
SK_API bool32_t ui_input_16       (const char16_t *id, char16_t *buffer, int32_t buffer_size, vec2 size sk_default(vec2_zero));
SK_API bool32_t ui_hslider        (const char     *id, sk_ref(float) value, float min, float max, float step sk_default(0), float width sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push));
SK_API bool32_t ui_hslider_16     (const char16_t *id, sk_ref(float) value, float min, float max, float step sk_default(0), float width sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push));
SK_API bool32_t ui_hslider_f64    (const char     *id, sk_ref(double) value, double min, double max, double step sk_default(0), float width sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push));
SK_API bool32_t ui_hslider_f64_16 (const char16_t *id, sk_ref(double) value, double min, double max, double step sk_default(0), float width sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push));
SK_API bool32_t ui_handle_begin   (const char     *text, sk_ref(pose_t) movement, bounds_t handle, bool32_t draw, ui_move_ move_type sk_default(ui_move_exact));
SK_API bool32_t ui_handle_begin_16(const char16_t *text, sk_ref(pose_t) movement, bounds_t handle, bool32_t draw, ui_move_ move_type sk_default(ui_move_exact));
SK_API void     ui_handle_end     ();
SK_API void     ui_window_begin   (const char     *text, sk_ref(pose_t) pose, vec2 size sk_default({ 0,0 }), ui_win_ window_type sk_default(ui_win_normal), ui_move_ move_type sk_default(ui_move_face_user));
SK_API void     ui_window_begin_16(const char16_t *text, sk_ref(pose_t) pose, vec2 size sk_default({ 0,0 }), ui_win_ window_type sk_default(ui_win_normal), ui_move_ move_type sk_default(ui_move_face_user));
SK_API void     ui_window_end     ();

#ifdef __cplusplus
} // namespace sk
#endif
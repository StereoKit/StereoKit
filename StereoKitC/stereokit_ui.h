#pragma once

#include "stereokit.h"

#ifdef __cplusplus
namespace sk {
#endif

/*This describes how a UI element moves when being dragged around by a user!*/
typedef enum ui_move_ {
	/*The element follows the position and orientation of the user's hand
	  exactly.*/
	ui_move_exact = 0,
	/*The element follows the position of the user's hand, but orients to face
	  the user's head instead of just using the hand's rotation.*/
	ui_move_face_user,
	/*This element follows the hand's position only, completely discarding any
	  rotation information.*/
	ui_move_pos_only,
	/*Do not allow user input to change the element's pose at all! You may also
	  be interested in UI.Push/PopSurface.*/
	ui_move_none,
} ui_move_;

/*A description of what type of window to draw! This is a bit flag, so it can
  contain multiple elements.*/
typedef enum ui_win_ {
	/*No body, no head. Not really a flag, just set to this value. The Window
	  will still be grab/movable. To prevent it from being grabbable, combine
	  with the UIMove.None option, or switch to UI.Push/PopSurface.*/
	ui_win_empty  = 1 << 0,
	/*Flag to include a head on the window.*/
	ui_win_head   = 1 << 1,
	/*Flag to include a body on the window.*/
	ui_win_body   = 1 << 2,
	/*A normal window has a head and a body to it. Both can be grabbed.*/
	ui_win_normal = ui_win_head | ui_win_body,
} ui_win_;
SK_MakeFlag(ui_win_)

/*Used with StereoKit's UI, and determines the interaction confirmation
  behavior for certain elements, such as the UI.HSlider!*/
typedef enum ui_confirm_ {
	/*The user must push a button with their finger to confirm interaction with
	  this element. This is simpler to activate as it requires no learned
	  gestures, but may result in more false positives.*/
	ui_confirm_push,
	/*The user must use a pinch gesture to interact with this element. This is
	  much harder to activate by accident, but does require the user to make a
	  precise pinch gesture. You can pretty much be sure that's what the user
	  meant to do!*/
	ui_confirm_pinch,
	/*HSlider specific. Same as Pinch, but pulling out from the slider creates
	  a scaled slider that lets you adjust the slider at a more granular
	  resolution.*/
	ui_confirm_variable_pinch
} ui_confirm_;

/*This is a bit flag that describes different types and combinations of
  gestures used within the UI system.*/
typedef enum ui_gesture_ {
	/*Default zero state, no gesture at all.*/
	ui_gesture_none       = 0,
	/*A pinching action, calculated by taking the distance between the tip of
	  the thumb and the index finger.*/
	ui_gesture_pinch      = 1 << 0,
	/*A gripping or grasping motion meant to represent a full hand grab. This
	  is calculated using the distance between the root and the tip of the ring
	  finger.*/
	ui_gesture_grip       = 1 << 1,
	/*This is a bit flag combination of both Pinch and Grip.*/
	ui_gesture_pinch_grip = ui_gesture_pinch | ui_gesture_grip,
} ui_gesture_;
SK_MakeFlag(ui_gesture_)

/*Determines when this UI function returns true.*/
typedef enum ui_notify_ {
	/*This function returns true any time the values has changed!*/
	ui_notify_change,
	/*This function returns true when the user has finished interacting with
	  it. This does not guarantee the value has changed.*/
	ui_notify_finalize,
} ui_notify_;

/*Used with StereoKit's UI to indicate a particular type of UI element visual.*/
typedef enum ui_vis_ {
	/*Default state, no UI element at all.*/
	ui_vis_none = 0,
	/*A default root UI element. Not a particular element, but other elements
	  may refer to this if there is nothing more specific present.*/
	ui_vis_default,
	/*Refers to UI.Button elements.*/
	ui_vis_button,
	/*Refers to UI.Toggle elements.*/
	ui_vis_toggle,
	/*Refers to UI.Input elements.*/
	ui_vis_input,
	/*Refers to UI.Handle/HandleBegin elements.*/
	ui_vis_handle,
	/*Refers to UI.Window/WindowBegin body panel element, this element is used
	  when a Window head is also present.*/
	ui_vis_window_body,
	/*Refers to UI.Window/WindowBegin body element, this element is used when a
	  Window only has the body panel, without a head.*/
	ui_vis_window_body_only,
	/*Refers to UI.Window/WindowBegin head panel element, this element is used
	  when a Window body is also present.*/
	ui_vis_window_head,
	/*Refers to UI.Window/WindowBegin head element, this element is used when a
	  Window only has the head panel, without a body.*/
	ui_vis_window_head_only,
	/*Refers to UI Separator elements.*/
	ui_vis_separator,
	/*Refers to the back line component of the Slider elements for full lines.
	  */
	ui_vis_slider_line,
	/*Refers to the back line component of the Slider elements for the active
	  or "full" half of the line.*/
	ui_vis_slider_line_active,
	/*Refers to the back line component of the Slider elements for the inactive
	  or "empty" half of the line.*/
	ui_vis_slider_line_inactive,
	/*Refers to the push button component of the UI.HSlider element when using
	  UIConfirm.Push.*/
	ui_vis_slider_push,
	/*Refers to the pinch button component of the Slider elements when using
	  UIConfirm.Pinch.*/
	ui_vis_slider_pinch,
	/*Refers to UI.ButtonRound elements.*/
	ui_vis_button_round,
	/*Refers to UI.PanelBegin/End elements.*/
	ui_vis_panel,
	/*Refers to the text position indicator carat on text input elements.*/
	ui_vis_carat,
	/*Refers to the grabbable area indicator outside a window.*/
	ui_vis_aura,
	/*A maximum enum value to allow for iterating through enum values.*/
	ui_vis_max,
} ui_vis_;

/*Theme color categories to pair with `UI.SetThemeColor`.*/
typedef enum ui_color_ {
	/*The default category, used to indicate that no category has been
	  selected.*/
	ui_color_none = 0,
	/*This is the main accent color used by window headers, separators, etc.*/
	ui_color_primary,
	/*This is a background sort of color that should generally be dark. Used by
	  window bodies and backgrounds of certain elements.*/
	ui_color_background,
	/*A normal UI element color, for elements like buttons and sliders.*/
	ui_color_common,
	/*Not really used anywhere at the moment, maybe for the UI.Panel.*/
	ui_color_complement,
	/*Text color! This should generally be really bright, and at the very least
	  contrast-ey.*/
	ui_color_text,
	/*A maximum enum value to allow for iterating through enum values.*/
	ui_color_max,
} ui_color_;

/*Indicates the state of a UI theme color.*/
typedef enum ui_color_state_ {
	/*The UI element is in its normal resting state.*/
	ui_color_state_normal,
	/*The UI element has been activated fully by some type of interaction.*/
	ui_color_state_active,
	/*The UI element is currently disabled, and cannot be used.*/
	ui_color_state_disabled,
} ui_color_state_;

/*This specifies a particular padding mode for certain UI elements, such as the
  UI.Panel! This describes where padding is applied and how it affects the
  layout of elements.*/
typedef enum ui_pad_ {
	/*No padding, this matches the element's layout bounds exactly!*/
	ui_pad_none,
	/*This applies padding inside the element's layout bounds, and will inflate
	  the layout bounds to fit the extra padding.*/
	ui_pad_inside,
	/*This will apply the padding outside of the layout bounds! This will
	  maintain the size and position of the layout volume, but the visual
	  padding will go outside of the volume.*/
	ui_pad_outside,
} ui_pad_;

/*Describes the layout of a button with image/text contents! You can think of
  the naming here as being the location of the image, with the text filling the
  remaining space.*/
typedef enum ui_btn_layout_ {
	/*Hide the image, and only show text.*/
	ui_btn_layout_none,
	/*Image to the left, text to the right. Image will take up no more than
	  half the width.*/
	ui_btn_layout_left,
	/*Image to the right, text to the left. Image will take up no more than
	  half the width.*/
	ui_btn_layout_right,
	/*Image will be centered in the button, and fill up the button as though it
	  was the only element. Text will cram itself under the padding below the
	  image.*/
	ui_btn_layout_center,
	/*Same as `Center`, but omitting the text.*/
	ui_btn_layout_center_no_text,
} ui_btn_layout_;

/*This describes how a layout should be cut up! Used with `UI.LayoutPushCut`.*/
typedef enum ui_cut_ {
	/*This cuts a chunk from the left side of the current layout. This will
	  work for layouts that are auto-sizing, and fixed sized.*/
	ui_cut_left,
	/*This cuts a chunk from the right side of the current layout. This will
	  work for layouts that are fixed sized, but not layouts that auto-size on
	  the X axis!*/
	ui_cut_right,
	/*This cuts a chunk from the top side of the current layout. This will work
	  for layouts that are auto-sizing, and fixed sized.*/
	ui_cut_top,
	/*This cuts a chunk from the bottom side of the current layout. This will
	  work for layouts that are fixed sized, but not layouts that auto-size on
	  the Y axis!*/
	ui_cut_bottom,
} ui_cut_;

/*For UI elements that can be oriented horizontally or vertically, this
  specifies that orientation.*/
typedef enum ui_dir_ {
	/*The element should be layed out along the horizontal axis.*/
	ui_dir_horizontal,
	/*The element should be layed out along the vertical axis.*/
	ui_dir_vertical,
} ui_dir_;

/*For elements that contain corners, this bit flag allows you to specify which
  corners.*/
typedef enum ui_corner_ {
	/*No corners at all.*/
	ui_corner_none         = 0,
	/*The top left corner.*/
	ui_corner_top_left     = 1 << 0,
	/*The top right corner.*/
	ui_corner_top_right    = 1 << 1,
	/*The bottom right corner.*/
	ui_corner_bottom_right = 1 << 2,
	/*The bottom left corner.*/
	ui_corner_bottom_left  = 1 << 3,
	/*All corners.*/
	ui_corner_all          = ui_corner_top_left    | ui_corner_top_right | ui_corner_bottom_left | ui_corner_bottom_right,
	/*The top left and top right corners.*/
	ui_corner_top          = ui_corner_top_left    | ui_corner_top_right,
	/*The bottom left and bottom right corners.*/
	ui_corner_bottom       = ui_corner_bottom_left | ui_corner_bottom_right,
	/*The top left and bottom left corners.*/
	ui_corner_left         = ui_corner_top_left    | ui_corner_bottom_left,
	/*The top right and bottom right corners.*/
	ui_corner_right        = ui_corner_top_right   | ui_corner_bottom_right,
} ui_corner_;
SK_MakeFlag(ui_corner_)

/*This describes how UI elements with scrollable regions scroll around or use
  scroll bars! This allows you to enable or disable vertical and horizontal
  scrolling.*/
typedef enum ui_scroll_ {
	/*No scroll bars or scrolling.*/
	ui_scroll_none       = 0,
	/*This will enable vertical scroll bars or scrolling.*/
	ui_scroll_vertical   = 1 << 0,
	/*This will enable horizontal scroll bars or scrolling.*/
	ui_scroll_horizontal = 1 << 1,
	/*This will enable both vertical and horizontal scroll bars or scrolling.*/
	ui_scroll_both       = ui_scroll_vertical | ui_scroll_horizontal,
} ui_scroll_;
SK_MakeFlag(ui_scroll_)

typedef struct ui_lathe_pt_t {
	vec2     pt;
	vec2     normal;
	color32  color;
	bool32_t connect_next;
	bool32_t flip_face;
} ui_lathe_pt_t;

typedef struct ui_settings_t {
	float margin;
	float padding;
	float gutter;
	float depth;
	float rounding;
	float backplate_depth;
	float backplate_border;
	float separator_scale;
} ui_settings_t;

typedef struct ui_slider_data_t {
	vec2          button_center;
	float         finger_offset;
	button_state_ focus_state;
	button_state_ active_state;
	int32_t       interactor;
} ui_slider_data_t;

SK_API void     ui_quadrant_size_verts  (vert_t *ref_arr_vertices, int32_t vertex_count, float overflow_percent);
SK_API void     ui_quadrant_size_mesh   (mesh_t ref_mesh, float overflow_percent);
SK_API mesh_t   ui_gen_quadrant_mesh    (ui_corner_ rounded_corners, float corner_radius, uint32_t corner_resolution, bool32_t delete_flat_sides, bool32_t quadrantify, const ui_lathe_pt_t* arr_lathe_pts, int32_t lathe_pt_count);
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
SK_API void     ui_draw_element         (ui_vis_ element_visual,                        vec3 start, vec3 size, float focus);
SK_API void     ui_draw_element_color   (ui_vis_ element_visual, ui_vis_ element_color, vec3 start, vec3 size, float focus);
SK_API color128 ui_get_element_color    (ui_vis_ element_visual,                                               float focus);
SK_API float    ui_get_anim_focus       (id_hash_t id, button_state_ focus_state, button_state_ activation_state);
SK_API void     ui_play_sound_on_off    (ui_vis_ element_visual, id_hash_t element_id, vec3 at_local);
SK_API void     ui_play_sound_on        (ui_vis_ element_visual, vec3 at_local);
SK_API void     ui_play_sound_off       (ui_vis_ element_visual, vec3 at_local);

SK_API void     ui_push_grab_aura        (bool32_t enabled);
SK_API void     ui_pop_grab_aura         ();
SK_API bool32_t ui_grab_aura_enabled     ();
SK_API void     ui_push_text_style       (text_style_t  style);
SK_API void     ui_pop_text_style        (void);
SK_API text_style_t ui_get_text_style    (void);
SK_API void     ui_push_tint             (color128 tint_gamma);
SK_API void     ui_pop_tint              (void);
SK_API void     ui_push_enabled          (bool32_t enabled, hierarchy_parent_ parent_behavior sk_default(hierarchy_parent_inherit));
SK_API void     ui_pop_enabled           (void);
SK_API bool32_t ui_is_enabled            (void);
SK_API void     ui_push_preserve_keyboard(bool32_t preserve_keyboard);
SK_API void     ui_pop_preserve_keyboard (void);
SK_API void     ui_push_surface          (pose_t surface_pose, vec3 layout_start sk_default(vec3_zero), vec2 layout_dimensions sk_default(vec2_zero));
SK_API void     ui_pop_surface           (void);

SK_API id_hash_t ui_push_id              (const char*     id_utf8);
SK_API id_hash_t ui_push_id_16           (const char16_t* id_utf16);
SK_API id_hash_t ui_push_idi             (int32_t id);
SK_API void      ui_pop_id               (void);
SK_API id_hash_t ui_stack_hash           (const char*     string_id_utf8);
SK_API id_hash_t ui_stack_hash_16        (const char16_t* string_id_utf16);

SK_API void     ui_layout_area     (vec3 start, vec2 dimensions, bool32_t add_margin sk_default(true));
SK_API vec2     ui_layout_remaining(void);
SK_API vec3     ui_layout_at       (void);
SK_API bounds_t ui_layout_last     (void);
SK_API bounds_t ui_layout_reserve  (vec2 size, bool32_t add_padding sk_default(false), float depth sk_default(0));
SK_API void     ui_layout_push     (vec3 start, vec2 dimensions, bool32_t add_margin);
SK_API void     ui_layout_push_cut (ui_cut_ cut_to, float size, bool32_t add_margin sk_default(false));
SK_API void     ui_layout_pop      (void);

// TODO: v0.4 These functions use hands instead of interactors, they need replaced!
SK_API bool32_t      ui_is_interacting           (handed_ hand);
// TODO: v0.4 These functions use hands instead of interactors, they need replaced!
SK_API button_state_ ui_last_element_hand_active (handed_ hand);
// TODO: v0.4 These functions use hands instead of interactors, they need replaced!
SK_API button_state_ ui_last_element_hand_focused(handed_ hand);

SK_API button_state_ ui_last_element_active   (void);
SK_API button_state_ ui_last_element_focused  (void);

SK_API void     ui_nextline      (void);
SK_API void     ui_sameline      (void);
SK_API float    ui_line_height   (void);

SK_API void     ui_button_behavior      (vec3 window_relative_pos, vec2 size, id_hash_t id,                                                    sk_ref(float) out_finger_offset, sk_ref(button_state_) out_button_state, sk_ref(button_state_) out_focus_state, interactor_t* out_opt_interactor sk_default(nullptr));
SK_API void     ui_button_behavior_depth(vec3 window_relative_pos, vec2 size, id_hash_t id, float button_depth, float button_activation_depth, sk_ref(float) out_finger_offset, sk_ref(button_state_) out_button_state, sk_ref(button_state_) out_focus_state, interactor_t* out_opt_interactor sk_default(nullptr));
SK_API void     ui_slider_behavior      (vec3 window_relative_pos, vec2 size, id_hash_t id, vec2* ref_value, vec2 min, vec2 max, vec2 button_size_visual, vec2 button_size_interact, ui_confirm_ confirm_method, ui_slider_data_t* out_state);

SK_API button_state_ ui_volume_at        (const char     *id_utf8,  bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand sk_default(nullptr), button_state_ *out_opt_focus_state sk_default(nullptr));
SK_API button_state_ ui_volume_at_16     (const char16_t *id_utf16, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand sk_default(nullptr), button_state_ *out_opt_focus_state sk_default(nullptr));

SK_API void     ui_label             (const char*     text_utf8,  bool32_t use_padding sk_default(true));
SK_API void     ui_label_16          (const char16_t* text_utf16, bool32_t use_padding sk_default(true));
SK_API void     ui_label_sz          (const char*     text_utf8,  vec2 size, bool32_t use_padding sk_default(true));
SK_API void     ui_label_sz_16       (const char16_t* text_utf16, vec2 size, bool32_t use_padding sk_default(true));
SK_API bool32_t ui_text              (const char*     text_utf8,  vec2* opt_ref_scroll sk_default(nullptr), ui_scroll_ scroll_direction sk_default(ui_scroll_vertical), float height sk_default(0), align_ text_align sk_default(align_top_left), text_fit_ fit sk_default(text_fit_wrap));
SK_API bool32_t ui_text_16           (const char16_t* text_utf16, vec2* opt_ref_scroll sk_default(nullptr), ui_scroll_ scroll_direction sk_default(ui_scroll_vertical), float height sk_default(0), align_ text_align sk_default(align_top_left), text_fit_ fit sk_default(text_fit_wrap));
SK_API bool32_t ui_text_sz           (const char*     text_utf8,  vec2* opt_ref_scroll, ui_scroll_ scroll_direction, vec2 size, align_ text_align sk_default(align_top_left), text_fit_ fit sk_default(text_fit_wrap));
SK_API bool32_t ui_text_sz_16        (const char16_t* text_utf16, vec2* opt_ref_scroll, ui_scroll_ scroll_direction, vec2 size, align_ text_align sk_default(align_top_left), text_fit_ fit sk_default(text_fit_wrap));
SK_API bool32_t ui_text_at           (const char*     text_utf8,  vec2* opt_ref_scroll, ui_scroll_ scroll_direction,            align_ text_align, text_fit_ fit, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_text_at_16        (const char16_t* text_utf16, vec2* opt_ref_scroll, ui_scroll_ scroll_direction,            align_ text_align, text_fit_ fit, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_button            (const char*     text_utf8);
SK_API bool32_t ui_button_16         (const char16_t* text_utf16);
SK_API bool32_t ui_button_sz         (const char*     text_utf8,  vec2 size);
SK_API bool32_t ui_button_sz_16      (const char16_t* text_utf16, vec2 size);
SK_API bool32_t ui_button_at         (const char*     text_utf8,  vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_button_at_16      (const char16_t* text_utf16, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_button_img        (const char*     text_utf8,  sprite_t image, ui_btn_layout_ image_layout, color128 image_tint sk_default(color128{ 1,1,1,1 }));
SK_API bool32_t ui_button_img_16     (const char16_t* text_utf16, sprite_t image, ui_btn_layout_ image_layout, color128 image_tint sk_default(color128{ 1,1,1,1 }));
SK_API bool32_t ui_button_img_sz     (const char*     text_utf8,  sprite_t image, ui_btn_layout_ image_layout, vec2 size, color128 image_tint sk_default(color128{ 1,1,1,1 }));
SK_API bool32_t ui_button_img_sz_16  (const char16_t* text_utf16, sprite_t image, ui_btn_layout_ image_layout, vec2 size, color128 image_tint sk_default(color128{ 1,1,1,1 }));
SK_API bool32_t ui_button_img_at     (const char*     text_utf8,  sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, color128 image_tint sk_default(color128{ 1,1,1,1 }));
SK_API bool32_t ui_button_img_at_16  (const char16_t* text_utf16, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, color128 image_tint sk_default(color128{ 1,1,1,1 }));
SK_API bool32_t ui_button_round      (const char*     id_utf8,    sprite_t image, float diameter sk_default(0));
SK_API bool32_t ui_button_round_16   (const char16_t* id_utf16,   sprite_t image, float diameter sk_default(0));
SK_API bool32_t ui_button_round_at   (const char*     id_utf8,    sprite_t image, vec3 window_relative_pos, float diameter);
SK_API bool32_t ui_button_round_at_16(const char16_t* id_utf16,   sprite_t image, vec3 window_relative_pos, float diameter);
SK_API bool32_t ui_toggle            (const char*     text_utf8,  sk_ref(bool32_t) ref_pressed);
SK_API bool32_t ui_toggle_16         (const char16_t* text_utf16, sk_ref(bool32_t) ref_pressed);
SK_API bool32_t ui_toggle_sz         (const char*     text_utf8,  sk_ref(bool32_t) ref_pressed, vec2 size);
SK_API bool32_t ui_toggle_sz_16      (const char16_t* text_utf16, sk_ref(bool32_t) ref_pressed, vec2 size);
SK_API bool32_t ui_toggle_at         (const char     *text_utf8,  sk_ref(bool32_t) ref_pressed, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_toggle_at_16      (const char16_t *text_utf16, sk_ref(bool32_t) ref_pressed, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_toggle_img        (const char*     text_utf8,  sk_ref(bool32_t) ref_pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout);
SK_API bool32_t ui_toggle_img_16     (const char16_t* text_utf16, sk_ref(bool32_t) ref_pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout);
SK_API bool32_t ui_toggle_img_sz     (const char*     text_utf8,  sk_ref(bool32_t) ref_pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec2 size);
SK_API bool32_t ui_toggle_img_sz_16  (const char16_t* text_utf16, sk_ref(bool32_t) ref_pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec2 size);
SK_API bool32_t ui_toggle_img_at     (const char*     text_utf8,  sk_ref(bool32_t) ref_pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_toggle_img_at_16  (const char16_t* text_utf16, sk_ref(bool32_t) ref_pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size);
SK_API bool32_t ui_hslider           (const char*     id_utf8,    sk_ref(float)  ref_value, float  min, float  max, float  step sk_default(0), float width  sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_16        (const char16_t* id_utf16,   sk_ref(float)  ref_value, float  min, float  max, float  step sk_default(0), float width  sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_f64       (const char*     id_utf8,    sk_ref(double) ref_value, double min, double max, double step sk_default(0), float width  sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_f64_16    (const char16_t* id_utf16,   sk_ref(double) ref_value, double min, double max, double step sk_default(0), float width  sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_at        (const char*     id_utf8,    sk_ref(float)  ref_value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_at_16     (const char16_t* id_utf16,   sk_ref(float)  ref_value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_at_f64    (const char*     id_utf8,    sk_ref(double) ref_value, double min, double max, double step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_hslider_at_f64_16 (const char16_t* id_utf16,   sk_ref(double) ref_value, double min, double max, double step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider           (const char*     id_utf8,    sk_ref(float)  ref_value, float  min, float  max, float  step sk_default(0), float height sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_16        (const char16_t* id_utf16,   sk_ref(float)  ref_value, float  min, float  max, float  step sk_default(0), float height sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_f64       (const char*     id_utf8,    sk_ref(double) ref_value, double min, double max, double step sk_default(0), float height sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_f64_16    (const char16_t* id_utf16,   sk_ref(double) ref_value, double min, double max, double step sk_default(0), float height sk_default(0), ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_at        (const char*     id_utf8,    sk_ref(float)  ref_value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_at_16     (const char16_t* id_utf16,   sk_ref(float)  ref_value, float  min, float  max, float  step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_at_f64    (const char*     id_utf8,    sk_ref(double) ref_value, double min, double max, double step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_vslider_at_f64_16 (const char16_t* id_utf16,   sk_ref(double) ref_value, double min, double max, double step, vec3 window_relative_pos, vec2 size,      ui_confirm_ confirm_method sk_default(ui_confirm_push), ui_notify_ notify_on sk_default(ui_notify_change));
SK_API bool32_t ui_input             (const char*     id_utf8,  char     *buffer_utf8,  int32_t buffer_size, vec2 size sk_default(vec2_zero), text_context_ type sk_default(text_context_text));
SK_API bool32_t ui_input_16          (const char16_t* id_utf16, char16_t *buffer_utf16, int32_t buffer_size, vec2 size sk_default(vec2_zero), text_context_ type sk_default(text_context_text));
SK_API bool32_t ui_input_at          (const char*     id_utf8,  char     *buffer_utf8,  int32_t buffer_size, vec3 window_relative_pos, vec2 size, text_context_ type sk_default(text_context_text));
SK_API bool32_t ui_input_at_16       (const char16_t* id_utf16, char16_t *buffer_utf16, int32_t buffer_size, vec3 window_relative_pos, vec2 size, text_context_ type sk_default(text_context_text));

SK_API void     ui_image             (sprite_t image, vec2 size);
SK_API void     ui_model             (model_t model, vec2 ui_size, float model_scale);
SK_API void     ui_model_at          (model_t model, vec3 start, vec3 size, color128 color);
SK_API void     ui_hprogress_bar     (float percent, float width  sk_default(0), bool32_t flip_fill_dir sk_default(false));
SK_API void     ui_vprogress_bar     (float percent, float height sk_default(0), bool32_t flip_fill_dir sk_default(false));
SK_API void     ui_progress_bar_at   (float percent, vec3 window_relative_pos, vec2 size, ui_dir_ bar_direction sk_default(ui_dir_horizontal), bool32_t flip_fill_dir sk_default(false));
SK_API void     ui_hseparator        (void);
SK_API void     ui_hspace            (float horizontal_space);
SK_API void     ui_vspace            (float vertical_space);

SK_API bool32_t ui_handle_begin      (const char     *text_utf8,  sk_ref(pose_t) ref_movement, bounds_t handle, bool32_t draw, ui_move_ move_type sk_default(ui_move_exact), ui_gesture_ allowed_gestures sk_default(ui_gesture_pinch));
SK_API bool32_t ui_handle_begin_16   (const char16_t *text_utf16, sk_ref(pose_t) ref_movement, bounds_t handle, bool32_t draw, ui_move_ move_type sk_default(ui_move_exact), ui_gesture_ allowed_gestures sk_default(ui_gesture_pinch));
SK_API void     ui_handle_end        (void);
SK_API void     ui_window_begin      (const char     *text_utf8,  pose_t* opt_ref_pose, vec2 size sk_default({ 0,0 }), ui_win_ window_type sk_default(ui_win_normal), ui_move_ move_type sk_default(ui_move_face_user));
SK_API void     ui_window_begin_16   (const char16_t *text_utf16, pose_t* opt_ref_pose, vec2 size sk_default({ 0,0 }), ui_win_ window_type sk_default(ui_win_normal), ui_move_ move_type sk_default(ui_move_face_user));
SK_API void     ui_window_end        (void);
SK_API void     ui_panel_at          (vec3 start, vec2 size, ui_pad_ padding sk_default(ui_pad_outside));
SK_API void     ui_panel_begin       (ui_pad_ padding sk_default(ui_pad_outside));
SK_API void     ui_panel_end         (void);

///////////////////////////////////////////

SK_CONST char* ui_default_id_spr_radio_on    = "sk/ui/radio_on";
SK_CONST char* ui_default_id_spr_radio_off   = "sk/ui/radio_off";
SK_CONST char* ui_default_id_spr_toggle_on   = "sk/ui/toggle_on";
SK_CONST char* ui_default_id_spr_toggle_off  = "sk/ui/toggle_off";
SK_CONST char* ui_default_id_spr_arrow_left  = "sk/ui/arrow_left";
SK_CONST char* ui_default_id_spr_arrow_right = "sk/ui/arrow_right";
SK_CONST char* ui_default_id_spr_arrow_up    = "sk/ui/arrow_up";
SK_CONST char* ui_default_id_spr_arrow_down  = "sk/ui/arrow_down";
SK_CONST char* ui_default_id_spr_backspace   = "sk/ui/backspace";
SK_CONST char* ui_default_id_spr_shift       = "sk/ui/shift";
SK_CONST char* ui_default_id_spr_close       = "sk/ui/close";
SK_CONST char* ui_default_id_spr_list        = "sk/ui/list";
SK_CONST char* ui_default_id_spr_grid        = "sk/ui/grid";


#ifdef __cplusplus
} // namespace sk
#endif
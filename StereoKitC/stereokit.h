/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#pragma once

#define SK_VERSION_MAJOR 0
#define SK_VERSION_MINOR 3
#define SK_VERSION_PATCH 11
#define SK_VERSION_PRERELEASE 0

#if defined(__GNUC__) || defined(__clang__)
	#define SK_DEPRECATED __attribute__((deprecated))
	#define SK_EXIMPORT __attribute__((visibility("default")))
	#define SK_CONST static const
#elif defined(_MSC_VER)
	#define SK_DEPRECATED __declspec(deprecated)
	#if defined(SK_BUILD_SHARED)
		#define SK_EXIMPORT __declspec(dllexport)
	#else
		#define SK_EXIMPORT
	#endif
	#define SK_CONST extern const __declspec(selectany)
#endif

#ifdef __cplusplus
#define SK_EXTERN extern "C"
#define sk_default(...) = __VA_ARGS__
#define sk_ref(x) x&
#define sk_ref_arr(x) x*&
#define SK_MakeFlag(enumType) \
inline enumType  operator| (enumType  a, enumType b)        { return static_cast<enumType>(static_cast<int>(a) | static_cast<int>(b)); } \
inline enumType &operator|=(enumType& a, const enumType& b) { a = a | b; return a; } \
inline enumType  operator& (enumType  a, enumType b)        { return static_cast<enumType>(static_cast<int>(a) & static_cast<int>(b)); } \
inline enumType &operator&=(enumType& a, const enumType& b) { a = a & b; return a; } \
inline enumType  operator~ (const enumType& a)              { return static_cast<enumType>(~static_cast<int>(a)); }
#else
#define SK_EXTERN
#define sk_default(...)
#define sk_ref(x) x*
#define sk_ref_arr(x) x**
#define SK_MakeFlag(enumType)
#endif

#define SK_API SK_EXTERN SK_EXIMPORT
#define SK_API_S SK_EXTERN SK_EXIMPORT
#define SK_DeclarePrivateType(name) struct _ ## name; typedef struct _ ## name *name;

#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <uchar.h>

#ifdef __cplusplus
namespace sk {
#endif

typedef int32_t bool32_t;

typedef struct vec2 {
	float x;
	float y;
} vec2;
typedef struct vec3 {
	float x;
	float y;
	float z;
} vec3;
typedef struct vec4 {
	float x;
	float y;
	float z;
	float w;
} vec4;
typedef struct quat {
	float x;
	float y;
	float z; 
	float w;
} quat;
typedef union matrix {
	vec4 row[4];
	float m[16];
} matrix;
typedef struct rect_t {
	float x;
	float y;
	float w;
	float h;
} rect_t;
typedef struct ray_t {
	vec3 pos;
	vec3 dir;
} ray_t;
typedef struct bounds_t {
	vec3 center;
	vec3 dimensions;
} bounds_t;
typedef struct plane_t {
	vec3  normal;
	float d;
} plane_t;
typedef struct sphere_t {
	vec3  center;
	float radius;
} sphere_t;
typedef struct pose_t {
	vec3 position;
	quat orientation;
} pose_t;

///////////////////////////////////////////

/*Specifies a type of display mode StereoKit uses, like
  Mixed Reality headset display vs. a PC display, or even just
  rendering to an offscreen surface, or not rendering at all!*/
typedef enum display_mode_ {
	/*Creates an OpenXR instance, and drives display/input
	  through that.*/
	display_mode_mixedreality     = 0,
	/*Creates a flat, Win32 window, and simulates some MR
	  functionality. Great for debugging.*/
	display_mode_flatscreen       = 1,
	/*Not tested yet, but this is meant to run StereoKit
	  without rendering to any display at all. This would allow for
	  rendering to textures, running a server that can do MR related
	  tasks, etc.*/
	display_mode_none             = 2,
} display_mode_;

/*Specifies a type of display mode StereoKit uses, like
  Mixed Reality headset display vs. a PC display, or even just
  rendering to an offscreen surface, or not rendering at all!*/
typedef enum app_mode_ {
	/*No mode has been specified, default behavior will be used. StereoKit will
	  pick XR in this case.*/
	app_mode_none = 0,
	/*Creates an OpenXR or WebXR instance, and drives display/input through
	  that.*/
	app_mode_xr,
	/*Creates a flat window, and simulates some XR functionality. Great for
	  development and debugging.*/
	app_mode_simulator,
	/*Creates a flat window and displays to that, but doesn't simulate XR at
	  all. You will need to control your own camera here. This can be useful
	  if using StereoKit for non-XR 3D applications.*/
	app_mode_window,
	/*No display at all! StereoKit won't even render to a texture unless
	  requested to. This may be good for running tests on a server, or doing
	  graphics related tool or CLI work.*/
	app_mode_offscreen,
} app_mode_;

/*This is used to determine what kind of depth buffer
  StereoKit uses!*/
typedef enum depth_mode_ {
	/*Default mode, uses 16 bit on mobile devices like
	  HoloLens and Quest, and 32 bit on higher powered platforms like
	  PC. If you need a far view distance even on mobile devices,
	  prefer D32 or Stencil instead.*/
	depth_mode_balanced           = 0,
	/*16 bit depth buffer, this is fast and recommended for
	  devices like the HoloLens. This is especially important for fast
	  depth based reprojection. Far view distances will suffer here
	  though, so keep your clipping far plane as close as possible.*/
	depth_mode_d16,
	/*32 bit depth buffer, should look great at any distance! 
	  If you must have the best, then this is the best. If you're
	  interested in this one, Stencil may also be plenty for you, as 24
	  bit depth is also pretty peachy.*/
	depth_mode_d32,
	/*24 bit depth buffer with 8 bits of stencil data. 24 bits
	  is generally plenty for a depth buffer, so using the rest for 
	  stencil can open up some nice options! StereoKit has limited
	  stencil support right now though (v0.3).*/
	depth_mode_stencil,
} depth_mode_;

// TODO: remove this in v0.4
/*This describes the type of display tech used on a Mixed
  Reality device. This will be replaced by `DisplayBlend` in v0.4.*/
typedef enum display_ {
	/*Default value, when using this as a search type, it will
	  fall back to default behavior which defers to platform
	  preference.*/
	display_none                  = 0,
	/*This display is opaque, with no view into the real world!
	  This is equivalent to a VR headset, or a PC screen.*/
	display_opaque                = 1 << 0,
	/*This display is transparent, and adds light on top of
	  the real world. This is equivalent to a HoloLens type of device.*/
	display_additive              = 1 << 1,
	/*This is a physically opaque display, but with a camera
	  passthrough displaying the world behind it anyhow. This would be
	  like a Varjo XR-1, or phone-camera based AR.*/
	display_blend                 = 1 << 2,
	/*Use Display.Blend instead, to be removed in v0.4*/
	display_passthrough           = 1 << 2,
	/*This matches either transparent display type! Additive
	  or Blend. For use when you just want to see the world behind your
	  application.*/
	display_any_transparent       = display_additive | display_blend,
} display_;
SK_MakeFlag(display_);

/*This describes the way the display's content blends with
  whatever is behind it. VR headsets are normally Opaque, but some VR
  headsets provide passthrough video, and can support Opaque as well as
  Blend, like the Varjo. Transparent AR displays like the HoloLens
  would be Additive.*/
typedef enum display_blend_ {
	/*Default value, when using this as a search type, it will
	  fall back to default behavior which defers to platform
	  preference.*/
	display_blend_none            = 0,
	/*This display is opaque, with no view into the real world!
	  This is equivalent to a VR headset, or a PC screen.*/
	display_blend_opaque          = 1 << 0,
	/*This display is transparent, and adds light on top of
	  the real world. This is equivalent to a HoloLens type of device.*/
	display_blend_additive        = 1 << 1,
	/*This is a physically opaque display, but with a camera
	  passthrough displaying the world behind it anyhow. This would be
	  like a Varjo XR-1, or phone-camera based AR.*/
	display_blend_blend           = 1 << 2,
	/*This matches either transparent display type! Additive
	  or Blend. For use when you just want to see the world behind your
	  application.*/
	display_blend_any_transparent = display_blend_additive | display_blend_blend,
} display_blend_;
SK_MakeFlag(display_blend_);

/*This describes where the origin of the application should be. While these
  origins map closely to OpenXR features, not all runtimes support each
  feature. StereoKit will provide reasonable fallback behavior in the event the
  origin mode isn't directly supported.*/
typedef enum origin_mode_ {
	/*The origin will be at the location of the user's head when the
	  application starts, facing the same direction as the user. This mode
	  is available on all runtimes, and will never fall back to another mode!
	  However, due to variances in underlying behavior, StereoKit may introduce
	  an origin offset to ensure consistent behavior.*/
	origin_mode_local,
	/*The origin will be at the floor beneath where the user starts, facing the
	  direction of the user. If this mode is not natively supported, StereoKit
	  will use the stage mode with an offset. If stage mode is unavailable, it
	  will fall back to local mode with a -1.5 Y axis offset.*/
	origin_mode_floor,
	/*The origin will be at the center of a safe play area or stage that the
	  user or OS has defined, and will face one of the edges of the play
	  area. If this mode is not natively supported, StereoKit will use the 
	  floor origin mode. If floor mode is unavailable, it will fall back to
	  local mode with a -1.5 Y axis offset.*/
	origin_mode_stage,
} origin_mode_;

/*Severity of a log item.*/
typedef enum log_ {
	/*A default log level that indicates it has not yet been
	  set.*/
	log_none = 0,
	/*This is for diagnostic information, where you need to know
	  details about what -exactly- is going on in the system. This
	  info doesn't surface by default.*/
	log_diagnostic,
	/*This is non-critical information, just to let you know
	  what's going on.*/
	log_inform,
	/*Something bad has happened, but it's still within the
	  realm of what's expected.*/
	log_warning,
	/*Danger Will Robinson! Something really bad just happened
	  and needs fixing!*/
	log_error
} log_;

/*When rendering content, you can filter what you're rendering
  by the RenderLayer that they're on. This allows you to draw items that
  are visible in one render, but not another. For example, you may wish
  to draw a player's avatar in a 'mirror' rendertarget, but not in
  the primary display. See `Renderer.LayerFilter` for configuring what
  the primary display renders.
  
  Render layers can also be mixed and matched like bit-flags!*/
typedef enum render_layer_ {
	/*The default render layer. All Draw use this layer unless
	  otherwise specified.*/
	render_layer_0                = 1 << 0,
	/*Render layer 1.*/
	render_layer_1                = 1 << 1,
	/*Render layer 2.*/
	render_layer_2                = 1 << 2,
	/*Render layer 3.*/
	render_layer_3                = 1 << 3,
	/*Render layer 4.*/
	render_layer_4                = 1 << 4,
	/*Render layer 5.*/
	render_layer_5                = 1 << 5,
	/*Render layer 6.*/
	render_layer_6                = 1 << 6,
	/*Render layer 7.*/
	render_layer_7                = 1 << 7,
	/*Render layer 8.*/
	render_layer_8                = 1 << 8,
	/*Render layer 9.*/
	render_layer_9                = 1 << 9,
	/*The default VFX layer, StereoKit draws some non-standard
	  mesh content using this flag, such as lines.*/
	render_layer_vfx              = 1 << 10,
	/*For items that should only be drawn from the first person
	  perspective. By default, this is enabled for renders that
	  are from a 1st person viewpoint.*/
	render_layer_first_person     = 1 << 11,
	/*For items that should only be drawn from the third person
	  perspective. By default, this is enabled for renders that
	  are from a 3rd person viewpoint.*/
	render_layer_third_person     = 1 << 12,
	/*This is a flag that specifies all possible layers. If you
	  want to render all layers, then this is the layer filter
	  you would use. This is the default for render filtering.*/
	render_layer_all              = 0xFFFF,
	/*This is a combination of all layers that are not the VFX
	  layer.*/
	render_layer_all_regular      = render_layer_0 | render_layer_1 | render_layer_2 | render_layer_3 | render_layer_4 | render_layer_5 | render_layer_6 | render_layer_7 | render_layer_8 | render_layer_9,
	/*All layers except for the third person layer.*/
	render_layer_all_first_person = render_layer_all & ~render_layer_third_person,
	/*All layers except for the first person layer.*/
	render_layer_all_third_person = render_layer_all & ~render_layer_first_person,
} render_layer_;
SK_MakeFlag(render_layer_);

/*This tells about the app's current focus state, whether it's
  active and receiving input, or if it's backgrounded or hidden. This can
  be important since apps may still run and render when unfocused, as the
  app may still be visible behind the app that _does_ have focus.*/
typedef enum app_focus_ {
	/*This StereoKit app is active, focused, and receiving input
	from the user. Application should behave as normal.*/
	app_focus_active,
	/*This StereoKit app has been unfocused, something may be
	  compositing on top of the app such as an OS dashboard. The app is
	  still visible, but some other thing has focus and is receiving
	  input. You may wish to pause, disable input tracking, or other such
	  things.*/
	app_focus_background,
	/*This app is not rendering currently.*/
	app_focus_hidden,
} app_focus_;

/*StereoKit uses an asynchronous loading system to prevent assets from
  blocking execution! This means that asset loading systems will return
  an asset to you right away, even though it is still being processed
  in the background.
  
  This enum will tell you about what state the asset is currently in,
  so you know what sort of behavior to expect from it.*/
typedef enum asset_state_ {
	/*This asset encountered an issue when parsing the source data. Either
	  the format is unrecognized by StereoKit, or the data may be corrupt.
	  Check the logs for additional details.*/
	asset_state_error_unsupported = -3,
	/*The asset data was not found! This is most likely an issue with a
	  bad file path, or file permissions. Check the logs for additional
	  details.*/
	asset_state_error_not_found   = -2,
	/*An unknown error occurred when trying to load the asset! Check the
	  logs for additional details.*/
	asset_state_error             = -1,
	/*This asset is in its default state. It has not been told to load
	  anything, nor does it have any data!*/
	asset_state_none              =  0,
	/*This asset is currently queued for loading, but hasn't received any
	  data yet. Attempting to access metadata or asset data will result in
	  blocking the app's execution until that data is loaded!*/
	asset_state_loading,
	/*This asset is still loading, but some of the higher level data is
	  already available for inspection without blocking the app.
	  Attempting to access the core asset data will result in blocking the
	  app's execution until that data is loaded!*/
	asset_state_loaded_meta,
	/*This asset is completely loaded without issues, and is ready for
	  use!*/
	asset_state_loaded,
} asset_state_;

/*For performance sensitive areas, or places dealing with large chunks of
  memory, it can be faster to get a reference to that memory rather than
  copying it! However, if this isn't explicitly stated, it isn't necessarily
  clear what's happening. So this enum allows us to visibly specify what type
  of memory reference is occurring.*/
typedef enum memory_ {
	/*The chunk of memory involved here is a reference that is still managed or
	  used by StereoKit! You should _not_ free it, and be extremely cautious
	  about modifying it.*/
	memory_reference,
	/*This memory is now _yours_ and you must free it yourself! Memory has been
	  allocated, and the data has been copied over to it. Pricey! But safe.*/
	memory_copy,
} memory_;

/*When the device StereoKit is running on goes into standby mode, how should
  StereoKit react? Typically the app should pause, stop playing sound, and
  consume as little power as possible, but some scenarios such as multiplayer
  games may need the app to continue running.*/
typedef enum standby_mode_ {
	/*This will let StereoKit pick a mode based on its own preferences. On v0.3
	  and lower, this will be Slow, and on v0.4 and higher, this will be Pause.
	  */
	standby_mode_default = 0,
	/*The entire main thread will pause, and wait until the device has come out
	  of standby. This is the most power efficient mode for the device to take
	  when the device is in standby, and is recommended for the vast majority
	  of apps. This will also disable sound.*/
	standby_mode_pause = 1,
	/*The main thread will continue to execute, but with 100ms sleeps each
	  frame. This allows the app to continue polling and processing, but
	  reduces power consumption by throttling a bit. This will not disable
	  sound. In the Simulator, this will behave as Slow.*/
	standby_mode_slow = 2,
	/*The main thread will continue to execute, but with a very short sleep
	  each frame. This allows the app to continue polling and processing, but
	  without flooding the CPU with polling work while vsync is no longer the
	  throttle. This will not disable sound.*/
	standby_mode_none = 3
} standby_mode_;

typedef struct sk_settings_t {
	const char    *app_name;
	const char    *assets_folder;
	display_mode_  display_preference;
	app_mode_      mode;
	display_blend_ blend_preference;
	bool32_t       no_flatscreen_fallback;
	depth_mode_    depth_mode;
	log_           log_filter;
	bool32_t       overlay_app;
	uint32_t       overlay_priority;
	int32_t        flatscreen_pos_x;
	int32_t        flatscreen_pos_y;
	int32_t        flatscreen_width;
	int32_t        flatscreen_height;
	bool32_t       disable_flatscreen_mr_sim;
	bool32_t       disable_desktop_input_window;
	bool32_t       disable_unfocused_sleep;
	float          render_scaling;
	int32_t        render_multisample;
	origin_mode_   origin;
	bool32_t       omit_empty_frames;
	standby_mode_  standby_mode;

	void          *android_java_vm;  // JavaVM*
	void          *android_activity; // jobject
} sk_settings_t;

typedef struct system_info_t {
	display_       display_type;
	int32_t        display_width;
	int32_t        display_height;
	bool32_t       spatial_bridge_present;
	bool32_t       perception_bridge_present;
	bool32_t       eye_tracking_present;
	bool32_t       overlay_app;
	bool32_t       world_occlusion_present;
	bool32_t       world_raycast_present;
} system_info_t;

/* Provides a reason on why StereoKit has quit.*/
typedef enum quit_reason_ {
	/*Default state when SK has not quit.*/
	quit_reason_none,
	/*The user (or possibly the OS) has explicitly asked to exit the
	  application under normal circumstances.*/
	quit_reason_user,
	/*Some runtime error occurred, causing the application to quit
	  gracefully.*/
	quit_reason_error,
	/*If initialization failed, StereoKit won't run to begin with!*/
	quit_reason_initialization_failed,
	/*The runtime under StereoKit has encountered an issue and has been lost.*/
	quit_reason_session_lost,
} quit_reason_;

SK_API bool32_t      sk_init               (sk_settings_t settings);
SK_API void          sk_set_window         (void *window);
SK_API void          sk_set_window_xam     (void *window);
SK_API void          sk_shutdown           (void);
SK_API void          sk_shutdown_unsafe    (void);
SK_API void          sk_quit               (quit_reason_ quitReason = quit_reason_user);
SK_API bool32_t      sk_step               (void (*app_step)(void));
SK_API void          sk_run                (void (*app_step)(void), void (*app_shutdown)(void) sk_default(nullptr));
SK_API void          sk_run_data           (void (*app_step)(void *step_data), void *step_data, void (*app_shutdown)(void *shutdown_data), void *shutdown_data);
SK_API bool32_t      sk_is_stepping        (void);
SK_API display_mode_ sk_active_display_mode(void);
SK_API sk_settings_t sk_get_settings       (void);
SK_API system_info_t sk_system_info        (void);
SK_API const char   *sk_version_name       (void);
SK_API uint64_t      sk_version_id         (void);
SK_API app_focus_    sk_app_focus          (void);
SK_API quit_reason_  sk_get_quit_reason    (void);

///////////////////////////////////////////

/*What type of user motion is the device capable of tracking? For the normal
  fully capable XR headset, this should be 6dof (rotation and translation), but
  more limited headsets may be restricted to 3dof (rotation) and flatscreen
  computers with the simulator off would be none. */
typedef enum device_tracking_ {
	/*No tracking is available! This is likely a flatscreen application, not an
	  XR applicaion.*/
	device_tracking_none = 0,
	/*This tracks rotation only, this may be a limited device without tracking
	  cameras, or could be a more capable headset in a 3dof mode. DoF stands
	  for Degrees of Freedom.*/
	device_tracking_3dof,
	/*This is capable of tracking both the position and rotation of the device,
	  most fully featured XR headsets (such as a HoloLens 2) will have this.
	  DoF stands for Degrees of Freedom.*/
	device_tracking_6dof,
} device_tracking_;

/*This describes a type of display hardware!*/
typedef enum display_type_ {
	/*Not a display at all, or the variable hasn't been initialized properly
	  yet.*/
	display_type_none,
	/*This is a stereo display! It has 2 screens, or two sections that display
	  content in stereo, one for each eye. This could be a VR headset, or like
	  a 3D tv.*/
	display_type_stereo,
	/*This is a single flat screen, with no stereo depth. This could be
	  something like either a computer monitor, or a phone with passthrough AR.
	  */
	display_type_flatscreen,
} display_type_;

typedef struct fov_info_t {
	float left;
	float right;
	float top;
	float bottom;
} fov_info_t;

SK_API display_type_    device_display_get_type   (void);
SK_API display_blend_   device_display_get_blend  (void);
SK_API bool32_t         device_display_set_blend  (display_blend_ blend);
SK_API bool32_t         device_display_valid_blend(display_blend_ blend);
SK_API float            device_display_get_refresh_rate(void);
SK_API int32_t          device_display_get_width  (void);
SK_API int32_t          device_display_get_height (void);
SK_API fov_info_t       device_display_get_fov    (void);
SK_API device_tracking_ device_get_tracking       (void);
SK_API const char*      device_get_name           (void);
SK_API const char*      device_get_runtime        (void);
SK_API const char*      device_get_gpu            (void);
SK_API bool32_t         device_has_eye_gaze       (void);
SK_API bool32_t         device_has_hand_tracking  (void);

///////////////////////////////////////////

// TODO: remove `get` and `elapsed` in v0.4
SK_API SK_DEPRECATED double        time_get_raw          (void);
SK_API SK_DEPRECATED float         time_getf_unscaled    (void);
SK_API SK_DEPRECATED double        time_get_unscaled     (void);
SK_API SK_DEPRECATED float         time_getf             (void);
SK_API SK_DEPRECATED double        time_get              (void);
SK_API SK_DEPRECATED float         time_elapsedf_unscaled(void);
SK_API SK_DEPRECATED double        time_elapsed_unscaled (void);
SK_API SK_DEPRECATED float         time_elapsedf         (void);
SK_API SK_DEPRECATED double        time_elapsed          (void);

SK_API double        time_total_raw        (void);
SK_API float         time_totalf_unscaled  (void);
SK_API double        time_total_unscaled   (void);
SK_API float         time_totalf           (void);
SK_API double        time_total            (void);
SK_API float         time_stepf_unscaled   (void);
SK_API double        time_step_unscaled    (void);
SK_API float         time_stepf            (void);
SK_API double        time_step             (void);
SK_API void          time_scale            (double scale);
SK_API void          time_set_time         (double total_seconds, double frame_elapsed_seconds sk_default(0));
SK_API uint64_t      time_frame            (void);

///////////////////////////////////////////

SK_API vec3     vec3_cross                (const sk_ref(vec3) a, const sk_ref(vec3) b);

SK_API quat     quat_difference           (const sk_ref(quat) a, const sk_ref(quat) b);
SK_API quat     quat_lookat               (const sk_ref(vec3) from, const sk_ref(vec3) at);
SK_API quat     quat_lookat_up            (const sk_ref(vec3) from, const sk_ref(vec3) at, const sk_ref(vec3) up);
SK_API quat     quat_from_angles          (float pitch_x_deg, float yaw_y_deg, float roll_z_deg);
SK_API quat     quat_slerp                (const sk_ref(quat) a, const sk_ref(quat) b, float t);
SK_API quat     quat_normalize            (const sk_ref(quat) a);
SK_API quat     quat_inverse              (const sk_ref(quat) a);
SK_API quat     quat_mul                  (const sk_ref(quat) a, const sk_ref(quat) b);
SK_API vec3     quat_mul_vec              (const sk_ref(quat) a, const sk_ref(vec3) b);
SK_API void     quat_to_axis_angle        (quat a, vec3 *out_axis, float *out_rotation_deg);

SK_API matrix   pose_matrix               (const sk_ref(pose_t) pose, vec3 scale sk_default({1,1,1}));
SK_API void     pose_matrix_out           (const sk_ref(pose_t) pose, sk_ref(matrix) out_result, vec3 scale sk_default({1,1,1}));

SK_API void     matrix_inverse            (const sk_ref(matrix) a, sk_ref(matrix) out_matrix);
SK_API matrix   matrix_invert             (const sk_ref(matrix) a);
SK_API void     matrix_mul                (const sk_ref(matrix) a, const sk_ref(matrix) b, sk_ref(matrix) out_matrix);
// Deprecated, use matrix_transform_pt. Removing in v0.4
SK_API SK_DEPRECATED vec3     matrix_mul_point     (const sk_ref(matrix) transform, const sk_ref(vec3) point);
// Deprecated, use matrix_transform_pt4. Removing in v0.4
SK_API SK_DEPRECATED vec4     matrix_mul_point4    (const sk_ref(matrix) transform, const sk_ref(vec4) point);
// Deprecated, use matrix_transform_dir. Removing in v0.4
SK_API SK_DEPRECATED vec3     matrix_mul_direction (const sk_ref(matrix) transform, const sk_ref(vec3) direction);
// Deprecated, use matrix_transform_quat. Removing in v0.4
SK_API SK_DEPRECATED quat     matrix_mul_rotation  (const sk_ref(matrix) transform, const sk_ref(quat) orientation);
// Deprecated, use matrix_transform_pose. Removing in v0.4
SK_API SK_DEPRECATED pose_t   matrix_mul_pose      (const sk_ref(matrix) transform, const sk_ref(pose_t) pose);
SK_API vec3     matrix_transform_pt       (matrix transform, vec3 point);
SK_API vec4     matrix_transform_pt4      (matrix transform, vec4 point);
SK_API vec3     matrix_transform_dir      (matrix transform, vec3 direction);
SK_API ray_t    matrix_transform_ray      (matrix transform, ray_t ray);
SK_API quat     matrix_transform_quat     (matrix transform, quat rotation);
SK_API pose_t   matrix_transform_pose     (matrix transform, pose_t pose);
SK_API matrix   matrix_transpose          (matrix transform);
SK_API vec3     matrix_to_angles          (const sk_ref(matrix) transform);
SK_API matrix   matrix_trs                (const sk_ref(vec3) position, const sk_ref(quat) orientation sk_default({0,0,0,1}), const sk_ref(vec3) scale sk_default({1,1,1}));
SK_API matrix   matrix_t                  (vec3 position);
SK_API matrix   matrix_r                  (quat orientation);
SK_API matrix   matrix_s                  (vec3 scale);
SK_API matrix   matrix_ts                 (vec3 position, vec3 scale);
SK_API void     matrix_trs_out            (sk_ref(matrix) out_result, const sk_ref(vec3) position, const sk_ref(quat) orientation sk_default({0,0,0,1}), const sk_ref(vec3) scale sk_default({1,1,1}));
SK_API matrix   matrix_perspective        (float fov_degrees, float aspect_ratio, float near_clip, float far_clip);
SK_API matrix   matrix_orthographic       (float width, float height, float near_clip, float far_clip);
SK_API bool32_t matrix_decompose          (const sk_ref(matrix) transform, sk_ref(vec3) out_position, sk_ref(vec3) out_scale, sk_ref(quat) out_orientation);
SK_API vec3     matrix_extract_translation(const sk_ref(matrix) transform);
SK_API vec3     matrix_extract_scale      (const sk_ref(matrix) transform);
SK_API quat     matrix_extract_rotation   (const sk_ref(matrix) transform);
SK_API pose_t   matrix_extract_pose       (const sk_ref(matrix) transform);

SK_API bool32_t ray_intersect_plane       (ray_t ray, vec3 plane_pt, vec3 plane_normal, sk_ref(float) out_t);
SK_API bool32_t ray_from_mouse            (vec2 screen_pixel_pos, sk_ref(ray_t) out_ray);

SK_API plane_t  plane_from_points         (vec3 p1, vec3 p2, vec3 p3);
SK_API plane_t  plane_from_ray            (ray_t ray);

#ifdef __cplusplus
static inline vec2   operator* (vec2 a, float b) { return { a.x * b, a.y * b }; }
static inline vec2   operator* (float b, vec2 a) { return { a.x * b, a.y * b }; }
static inline vec2   operator/ (vec2 a, float b) { return { a.x / b, a.y / b }; }
static inline vec2   operator+ (vec2 a, vec2  b) { return { a.x + b.x, a.y + b.y }; }
static inline vec2   operator- (vec2 a, vec2  b) { return { a.x - b.x, a.y - b.y }; }
static inline vec2   operator+ (vec2 a, float b) { return { a.x + b,   a.y + b }; }
static inline vec2   operator- (vec2 a, float b) { return { a.x - b,   a.y - b }; }
static inline vec2   operator* (vec2 a, vec2  b) { return { a.x * b.x, a.y * b.y }; }
static inline vec2   operator/ (vec2 a, vec2  b) { return { a.x / b.x, a.y / b.y }; }
static inline vec2  &operator+=(vec2 &a, vec2  b) { a.x += b.x; a.y += b.y; return a; }
static inline vec2  &operator-=(vec2 &a, vec2  b) { a.x -= b.x; a.y -= b.y; return a; }
static inline vec2  &operator*=(vec2 &a, float b) { a.x *= b; a.y *= b; return a; }
static inline vec2  &operator/=(vec2 &a, float b) { a.x /= b; a.y /= b; return a; }

static inline vec3   operator* (vec3 a, float b) { return { a.x * b, a.y * b, a.z * b }; }
static inline vec3   operator* (float b, vec3 a) { return { a.x * b, a.y * b, a.z * b }; }
static inline vec3   operator/ (vec3 a, float b) { return { a.x / b, a.y / b, a.z / b }; }
static inline vec3   operator+ (vec3 a, vec3  b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline vec3   operator- (vec3 a, vec3  b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline vec3   operator- (vec3 a)          { return { -a.x, -a.y, -a.z }; }
static inline vec3   operator* (vec3 a, vec3  b) { return { a.x * b.x, a.y * b.y, a.z * b.z }; }
static inline vec3   operator/ (vec3 a, vec3  b) { return { a.x / b.x, a.y / b.y, a.z / b.z }; }
static inline vec3  &operator+=(vec3 &a, vec3  b) { a.x += b.x; a.y += b.y; a.z += b.z; return a; }
static inline vec3  &operator-=(vec3 &a, vec3  b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a; }
static inline vec3  &operator*=(vec3 &a, float b) { a.x *= b; a.y *= b; a.z *= b; return a; }
static inline vec3  &operator/=(vec3 &a, float b) { a.x /= b; a.y /= b; a.z /= b; return a; }

static inline vec4   operator* (vec4 a, float b) { return { a.x * b, a.y * b, a.z * b, a.w * b }; }
static inline vec4   operator* (float b, vec4 a) { return { a.x * b, a.y * b, a.z * b, a.w * b }; }
static inline vec4   operator+ (vec4 a, vec4  b) { return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }

static inline vec3   operator* (quat a, vec3 b) { return quat_mul_vec(a, b); }
static inline quat   operator* (quat a, quat b) { return quat_mul(a, b); }
static inline matrix operator* (matrix a, matrix b) { matrix result; matrix_mul(a, b, result); return result; }
static inline vec3   operator* (matrix a, vec3 b) { return matrix_transform_pt(a, b); }
static inline vec4   operator* (matrix a, vec4 b) { return matrix_transform_pt4(a, b); }
#endif

static inline float    vec4_magnitude   (vec4 a) { return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w); }
static inline float    vec3_magnitude   (vec3 a) { return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z); }
static inline float    vec2_magnitude   (vec2 a) { return sqrtf(a.x*a.x + a.y*a.y); }
static inline float    vec3_magnitude_sq(vec3 a) { return a.x*a.x + a.y*a.y + a.z*a.z; }
static inline float    vec2_magnitude_sq(vec2 a) { return a.x*a.x + a.y*a.y; }
static inline float    vec3_dot         (vec3 a, vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline float    vec2_dot         (vec2 a, vec2 b) { return a.x*b.x + a.y*b.y; }
static inline float    vec3_distance_sq (vec3 a, vec3 b) { vec3 v = {a.x-b.x, a.y-b.y, a.z-b.z}; return vec3_magnitude_sq(v); }
static inline float    vec2_distance_sq (vec2 a, vec2 b) { vec2 v = {a.x-b.x, a.y-b.y}; return vec2_magnitude_sq(v); }
static inline float    vec3_distance    (vec3 a, vec3 b) { vec3 v = {a.x-b.x, a.y-b.y, a.z-b.z}; return vec3_magnitude(v); }
static inline float    vec2_distance    (vec2 a, vec2 b) { vec2 v = {a.x-b.x, a.y-b.y}; return vec2_magnitude(v); }
static inline vec3     vec3_project     (vec3 a, vec3 onto_b) { float t = vec3_dot(a,onto_b)/vec3_magnitude_sq(onto_b); vec3 v = {onto_b.x*t, onto_b.y*t, onto_b.z*t}; return v; }
static inline vec4     vec4_normalize   (vec4 a) { float imag = 1.0f/vec4_magnitude(a); vec4 v = {a.x*imag, a.y*imag, a.z*imag, a.w*imag}; return v; }
static inline vec3     vec3_normalize   (vec3 a) { float imag = 1.0f/vec3_magnitude(a); vec3 v = {a.x*imag, a.y*imag, a.z*imag}; return v; }
static inline vec2     vec2_normalize   (vec2 a) { float imag = 1.0f/vec2_magnitude(a); vec2 v = {a.x*imag, a.y*imag}; return v; }
static inline vec3     vec3_abs         (vec3 a) { vec3 v = { fabsf(a.x), fabsf(a.y), fabsf(a.z) }; return v; }
static inline vec2     vec2_abs         (vec2 a) { vec2 v = { fabsf(a.x), fabsf(a.y) }; return v; }
static inline vec3     vec3_min         (vec3 a, vec3 b)          { vec3 v = { fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z) }; return v; }
static inline vec2     vec2_min         (vec2 a, vec2 b)          { vec2 v = { fminf(a.x, b.x), fminf(a.y, b.y) }; return v; }
static inline vec3     vec3_lerp        (vec3 a, vec3 b, float t) { vec3 v = { a.x + (b.x - a.x)*t, a.y + (b.y - a.y)*t, a.z + (b.z - a.z)*t }; return v; }
static inline vec2     vec2_lerp        (vec2 a, vec2 b, float t) { vec2 v = { a.x + (b.x - a.x)*t, a.y + (b.y - a.y)*t }; return v; }
static inline bool32_t vec3_in_radius   (vec3 pt, vec3 center, float radius) { return vec3_distance_sq(center, pt) < radius*radius; }
static inline bool32_t vec2_in_radius   (vec2 pt, vec2 center, float radius) { return vec2_distance_sq(center, pt) < radius*radius; }

SK_CONST float  deg2rad         = 0.01745329252f;
SK_CONST float  rad2deg         = 57.295779513f;
SK_CONST float  cm2m            = 0.01f;
SK_CONST float  mm2m            = 0.001f;
SK_CONST float  m2cm            = 100.f;
SK_CONST float  m2mm            = 1000.f;

SK_CONST vec2   vec2_zero       = { 0,0 };
SK_CONST vec2   vec2_one        = { 1,1 };
SK_CONST vec3   vec3_one        = { 1,1, 1 };
SK_CONST vec3   vec3_zero       = { 0,0, 0 };
SK_CONST vec3   vec3_up         = { 0,1, 0 };
SK_CONST vec3   vec3_forward    = { 0,0,-1 };
SK_CONST vec3   vec3_right      = { 1,0, 0 };
SK_CONST vec3   vec3_unit_x     = { 1,0, 0 };
SK_CONST vec3   vec3_unit_y     = { 0,1, 0 };
SK_CONST vec3   vec3_unit_z     = { 0,0, 1 };
SK_CONST quat   quat_identity   = { 0,0, 0,1 };
SK_CONST matrix matrix_identity = { { {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1} } };
SK_CONST pose_t pose_identity   = { {0,0,0}, {0,0,0,1} };

#define unit_cm(cm) ((cm)*0.01f)
#define unit_mm(mm) ((mm)*0.001f)
#define unit_dmm(dmm, distance) ((dmm)*(distance))

///////////////////////////////////////////

SK_API bool32_t plane_ray_intersect    (plane_t plane, ray_t ray, vec3 *out_pt);
SK_API bool32_t plane_line_intersect   (plane_t plane, vec3 p1, vec3 p2, vec3 *out_pt);
SK_API vec3     plane_point_closest    (plane_t plane, vec3 pt);
SK_API bool32_t sphere_ray_intersect   (sphere_t sphere, ray_t ray, vec3 *out_pt);
SK_API bool32_t sphere_point_contains  (sphere_t sphere, vec3 pt);
SK_API bool32_t bounds_ray_intersect   (bounds_t bounds, ray_t ray, vec3 *out_pt);
SK_API bool32_t bounds_point_contains  (bounds_t bounds, vec3 pt);
SK_API bool32_t bounds_line_contains   (bounds_t bounds, vec3 pt1, vec3 pt2);
SK_API bool32_t bounds_capsule_contains(bounds_t bounds, vec3 pt1, vec3 pt2, float radius);
SK_API bounds_t bounds_grow_to_fit_pt  (bounds_t bounds, vec3 pt);
SK_API bounds_t bounds_grow_to_fit_box (bounds_t bounds, bounds_t box, const matrix *opt_box_transform sk_default(nullptr));
SK_API bounds_t bounds_transform       (bounds_t bounds, matrix transform);
SK_API vec3     ray_point_closest      (ray_t ray, vec3 pt);

///////////////////////////////////////////

typedef struct color32 {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} color32;
typedef struct color128 {
	float r;
	float g;
	float b;
	float a;
} color128;

SK_API        color128 color_hsv      (float hue, float saturation, float value, float transparency);
SK_API        vec3     color_to_hsv   (const sk_ref(color128) color);
SK_API        color128 color_lab      (float l, float a, float b, float transparency);
SK_API        vec3     color_to_lab   (const sk_ref(color128) color);
SK_API        color128 color_to_linear(color128 srgb_gamma_correct);
SK_API        color128 color_to_gamma (color128 srgb_linear);

static inline color128 color_lerp     (color128 a, color128 b, float t) { color128 result = {a.r + (b.r - a.r)*t, a.g + (b.g - a.g)*t, a.b + (b.b - a.b)*t, a.a + (b.a - a.a)*t}; return result; }
static inline color32  color_to_32    (color128 a)                      { color32  result = {(uint8_t)(a.r * 255.f), (uint8_t)(a.g * 255.f), (uint8_t)(a.b * 255.f), (uint8_t)(a.a * 255.f)}; return result; }

static inline color128 color32_to_128 (color32 color) { color128 result = { color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a/255.0f }; return result; }
static inline color32  color32_hex    (uint32_t hex)  { color32  result = {(uint8_t)(hex>>24), (uint8_t)((hex>>16)&0x000000FF), (uint8_t)((hex>>8)&0x000000FF), (uint8_t)(hex&0x000000FF)}; return result; };
static inline color128 color_hex      (uint32_t hex)  { return color32_to_128(color32_hex(hex)); };

#ifdef __cplusplus
static inline color128  operator*(color128 a, float    b) { return { a.r*b,   a.g*b,   a.b*b,   a.a*b   }; }
static inline color128  operator*(color128 a, color128 b) { return { a.r*b.r, a.g*b.g, a.b*b.b, a.a*b.a }; }
#endif

///////////////////////////////////////////

SK_DeclarePrivateType(gradient_t);
SK_DeclarePrivateType(mesh_t);
SK_DeclarePrivateType(tex_t);
SK_DeclarePrivateType(font_t);
SK_DeclarePrivateType(shader_t);
SK_DeclarePrivateType(material_t);
SK_DeclarePrivateType(material_buffer_t);
SK_DeclarePrivateType(model_t);
SK_DeclarePrivateType(sprite_t);
SK_DeclarePrivateType(sound_t);
SK_DeclarePrivateType(solid_t);
SK_DeclarePrivateType(anchor_t);
SK_DeclarePrivateType(render_list_t);

///////////////////////////////////////////

typedef struct gradient_key_t {
	color128 color;
	float    position;
} gradient_key_t;

SK_API gradient_t gradient_create     (void);
SK_API gradient_t gradient_create_keys(const gradient_key_t *in_arr_keys, int32_t count);
SK_API void       gradient_add        (gradient_t gradient, color128 color_linear, float position);
SK_API void       gradient_set        (gradient_t gradient, int32_t index, color128 color_linear, float position);
SK_API void       gradient_remove     (gradient_t gradient, int32_t index);
SK_API int32_t    gradient_count      (gradient_t gradient);
SK_API color128   gradient_get        (gradient_t gradient, float at);
SK_API color32    gradient_get32      (gradient_t gradient, float at);
SK_API void       gradient_release    (gradient_t gradient); // TODO v0.4, consider renaming this to _destroy, no reference counting happens here!

///////////////////////////////////////////

typedef struct spherical_harmonics_t {
	vec3     coefficients[9];
} spherical_harmonics_t;

typedef struct sh_light_t {
	vec3     dir_to;
	color128 color;
} sh_light_t;

SK_API spherical_harmonics_t sh_create      (const sh_light_t* in_arr_lights, int32_t light_count);
SK_API void                  sh_brightness  (      sk_ref(spherical_harmonics_t) ref_harmonics, float scale);
SK_API void                  sh_add         (      sk_ref(spherical_harmonics_t) ref_harmonics, vec3 light_dir, vec3 light_color);
SK_API color128              sh_lookup      (const sk_ref(spherical_harmonics_t) harmonics, vec3 normal);
SK_API vec3                  sh_dominant_dir(const sk_ref(spherical_harmonics_t) harmonics);

///////////////////////////////////////////

typedef struct vert_t {
	vec3    pos;
	vec3    norm;
	vec2    uv;
	color32 col;
} vert_t;

static inline vert_t vert_create(vec3 position, vec3 normal sk_default({ 0,1,0 }), vec2 texture_coordinates sk_default({ 0,0 }), color32 vertex_color sk_default({ 255,255,255,255 })) { vert_t v = { position, normal, texture_coordinates, vertex_color }; return v;  }

typedef uint32_t vind_t;

/*Culling is discarding an object from the render pipeline!
  This enum describes how mesh faces get discarded on the graphics
  card. With culling set to none, you can double the number of pixels
  the GPU ends up drawing, which can have a big impact on performance.
  None can be appropriate in cases where the mesh is designed to be
  'double sided'. Front can also be helpful when you want to flip a
  mesh 'inside-out'!*/
typedef enum cull_ {
	/*Discard if the back of the triangle face is pointing
	  towards the camera. This is the default behavior.*/
	cull_back = 0,
	/*Discard if the front of the triangle face is pointing
	  towards the camera. This is opposite the default behavior.*/
	cull_front,
	/*No culling at all! Draw the triangle regardless of which
	  way it's pointing.*/
	cull_none,
} cull_;

SK_API mesh_t      mesh_find            (const char *name);
SK_API mesh_t      mesh_create          (void);
SK_API mesh_t      mesh_copy            (mesh_t mesh);
SK_API void        mesh_set_id          (mesh_t mesh, const char *id);
SK_API const char* mesh_get_id          (const mesh_t mesh);
SK_API void        mesh_addref          (mesh_t mesh);
SK_API void        mesh_release         (mesh_t mesh);
SK_API void        mesh_draw            (mesh_t mesh, material_t material, matrix transform, color128 color_linear sk_default({1,1,1,1}), render_layer_ layer sk_default(render_layer_0));
SK_API void        mesh_set_keep_data   (mesh_t mesh, bool32_t keep_data);
SK_API bool32_t    mesh_get_keep_data   (mesh_t mesh);
SK_API void        mesh_set_data        (mesh_t mesh, const vert_t *in_arr_vertices, int32_t vertex_count, const vind_t *in_arr_indices, int32_t index_count, bool32_t calculate_bounds sk_default(true));
SK_API void        mesh_set_verts       (mesh_t mesh, const vert_t *in_arr_vertices, int32_t vertex_count, bool32_t calculate_bounds sk_default(true));
SK_API void        mesh_get_verts       (mesh_t mesh, sk_ref_arr(vert_t) out_arr_vertices, sk_ref(int32_t) out_vertex_count, memory_ reference_mode);
SK_API int32_t     mesh_get_vert_count  (mesh_t mesh);
SK_API void        mesh_set_inds        (mesh_t mesh, const vind_t *in_arr_indices, int32_t index_count);
SK_API void        mesh_get_inds        (mesh_t mesh, sk_ref_arr(vind_t) out_arr_indices,  sk_ref(int32_t) out_index_count, memory_ reference_mode);
SK_API int32_t     mesh_get_ind_count   (mesh_t mesh);
SK_API void        mesh_set_draw_inds   (mesh_t mesh, int32_t index_count);
SK_API void        mesh_set_bounds      (mesh_t mesh, const sk_ref(bounds_t) bounds);
SK_API bounds_t    mesh_get_bounds      (mesh_t mesh);
SK_API bool32_t    mesh_has_skin        (mesh_t mesh);
SK_API void        mesh_set_skin        (mesh_t mesh, const uint16_t *in_arr_bone_ids_4, int32_t bone_id_4_count, const vec4 *in_arr_bone_weights, int32_t bone_weight_count, const matrix *bone_resting_transforms, int32_t bone_count);
SK_API void        mesh_update_skin     (mesh_t mesh, const matrix *in_arr_bone_transforms, int32_t bone_count);
// TODO: in 0.4 move cull_mode parameter up to directly after out_pt (both functions)
SK_API bool32_t    mesh_ray_intersect   (mesh_t mesh, ray_t model_space_ray, ray_t* out_pt, uint32_t* out_start_inds sk_default(nullptr), cull_ cull_mode sk_default(cull_back));
SK_API bool32_t    mesh_ray_intersect_bvh(mesh_t mesh, ray_t model_space_ray, ray_t* out_pt, uint32_t* out_start_inds sk_default(nullptr), cull_ cull_mode sk_default(cull_back));
SK_API bool32_t    mesh_get_triangle    (mesh_t mesh, uint32_t triangle_index, vert_t* out_a, vert_t* out_b, vert_t* out_c);

SK_API mesh_t      mesh_gen_plane       (vec2 dimensions, vec3 plane_normal, vec3 plane_top_direction, int32_t subdivisions sk_default(0), bool32_t double_sided sk_default(false));
SK_API mesh_t      mesh_gen_circle      (float diameter,  vec3 plane_normal, vec3 plane_top_direction, int32_t spokes sk_default(16), bool32_t double_sided sk_default(false));
SK_API mesh_t      mesh_gen_cube        (vec3 dimensions, int32_t subdivisions sk_default(0));
SK_API mesh_t      mesh_gen_sphere      (float diameter,  int32_t subdivisions sk_default(4));
SK_API mesh_t      mesh_gen_rounded_cube(vec3 dimensions, float edge_radius, int32_t subdivisions);
SK_API mesh_t      mesh_gen_cylinder    (float diameter,  float depth, vec3 direction, int32_t subdivisions sk_default(16));
SK_API mesh_t      mesh_gen_cone        (float diameter,  float depth, vec3 direction, int32_t subdivisions sk_default(16));

///////////////////////////////////////////

/*Textures come in various types and flavors! These are bit-flags
  that tell StereoKit what type of texture we want, and how the application
  might use it!*/
typedef enum tex_type_ {
	/*A standard color image, without any generated mip-maps.*/
	tex_type_image_nomips  = 1 << 0,
	/*A size sided texture that's used for things like skyboxes,
	  environment maps, and reflection probes. It behaves like a texture
	  array with 6 textures.*/
	tex_type_cubemap       = 1 << 1,
	/*This texture can be rendered to! This is great for textures
	  that might be passed in as a target to Renderer.Blit, or other
	  such situations.*/
	tex_type_rendertarget  = 1 << 2,
	/*This texture contains depth data, not color data!*/
	tex_type_depth         = 1 << 3,
	/*This texture will generate mip-maps any time the contents
	  change. Mip-maps are a list of textures that are each half the
	  size of the one before them! This is used to prevent textures from
	  'sparkling' or aliasing in the distance.*/
	tex_type_mips          = 1 << 4,
	/*This texture's data will be updated frequently from the
	  CPU (not renders)! This ensures the graphics card stores it
	  someplace where writes are easy to do quickly.*/
	tex_type_dynamic       = 1 << 5,
	/*A standard color image that also generates mip-maps
	  automatically.*/
	tex_type_image         = tex_type_image_nomips | tex_type_mips,
} tex_type_;
SK_MakeFlag(tex_type_);

/*What type of color information will the texture contain? A
  good default here is Rgba32.*/
typedef enum tex_format_ {
	/*A default zero value for TexFormat! Uninitialized formats
	  will get this value and **** **** up so you know to assign it
	  properly :)*/
	tex_format_none = 0,
	/*Red/Green/Blue/Transparency data channels, at 8 bits
	  per-channel in sRGB color space. This is what you'll want most of
	  the time you're dealing with color images! Matches well with the
	  Color32 struct! If you're storing normals, rough/metal, or
	  anything else, use Rgba32Linear.*/
	tex_format_rgba32 = 1,
	/*Red/Green/Blue/Transparency data channels, at 8 bits
	  per-channel in linear color space. This is what you'll want most
	  of the time you're dealing with color data! Matches well with the
	  Color32 struct.*/
	tex_format_rgba32_linear = 2,
	/*Blue/Green/Red/Transparency data channels, at 8 bits
	  per-channel in sRGB color space. This is a common swapchain format
	  on Windows.*/
	tex_format_bgra32 = 3,
	/*Blue/Green/Red/Transparency data channels, at 8 bits
	  per-channel in linear color space. This is a common swapchain
	  format on Windows.*/
	tex_format_bgra32_linear = 4,
	/*Red/Green/Blue data channels, with 11 bits for R and G,
	  and 10 bits for blue. This is a great presentation format for high
	  bit depth displays that still fits in 32 bits! This format has no
	  alpha channel.*/
	tex_format_rg11b10 = 5,
	/*Red/Green/Blue/Transparency data channels, with 10 
	  bits for R, G, and B, and 2 for alpha. This is a great presentation
	  format for high bit depth displays that still fits in 32 bits, and
	  also includes at least a bit of transparency!*/
	tex_format_rgb10a2 = 6,
	/*Red/Green/Blue/Transparency data channels, at 16 bits
	  per-channel! This is not common, but you might encounter it with
	  raw photos, or HDR images. TODO: remove during major version
	  update, prefer s, f, or u postfixed versions of this format.*/
	tex_format_rgba64 = 7,
	/*Red/Green/Blue/Transparency data channels, at 16 bits
	  per-channel! This is not common, but you might encounter it with
	  raw photos, or HDR images. The u postfix indicates that the raw
	  color data is stored as an unsigned 16 bit integer, which is then
	  normalized into the 0, 1 floating point range on the GPU.*/
	tex_format_rgba64u = tex_format_rgba64,
	/*Red/Green/Blue/Transparency data channels, at 16 bits
	  per-channel! This is not common, but you might encounter it with
	  raw photos, or HDR images. The s postfix indicates that the raw
	  color data is stored as a signed 16 bit integer, which is then
	  normalized into the -1, +1 floating point range on the GPU.*/
	tex_format_rgba64s = 8,
	/*Red/Green/Blue/Transparency data channels, at 16 bits
	  per-channel! This is not common, but you might encounter it with
	  raw photos, or HDR images. The f postfix indicates that the raw
	  color data is stored as 16 bit floats, which may be tricky to work
	  with in most languages.*/
	tex_format_rgba64f = 9,
	/*Red/Green/Blue/Transparency data channels at 32 bits
	  per-channel! Basically 4 floats per color, which is bonkers
	  expensive. Don't use this unless you know -exactly- what you're
	  doing.*/
	tex_format_rgba128 = 10,
	/*A single channel of data, with 8 bits per-pixel! This
	  can be great when you're only using one channel, and want to
	  reduce memory usage. Values in the shader are always 0.0-1.0.*/
	tex_format_r8 = 11,
	/*A single channel of data, with 16 bits per-pixel! This
	  is a good format for height maps, since it stores a fair bit of
	  information in it. Values in the shader are always 0.0-1.0.
	  TODO: remove during major version update, prefer s, f, or u
	  postfixed versions of this format, this item is the same as
	  r16u.*/
	tex_format_r16 = 12,
	/*A single channel of data, with 16 bits per-pixel! This
	  is a good format for height maps, since it stores a fair bit of
	  information in it. The u postfix indicates that the raw color data
	  is stored as an unsigned 16 bit integer, which is then normalized
	  into the 0, 1 floating point range on the GPU.*/
	tex_format_r16u = tex_format_r16,
	/*A single channel of data, with 16 bits per-pixel! This
	  is a good format for height maps, since it stores a fair bit of
	  information in it. The s postfix indicates that the raw color
	  data is stored as a signed 16 bit integer, which is then
	  normalized into the -1, +1 floating point range on the GPU.*/
	tex_format_r16s = 13,
	/*A single channel of data, with 16 bits per-pixel! This
	  is a good format for height maps, since it stores a fair bit of
	  information in it. The f postfix indicates that the raw color
	  data is stored as 16 bit floats, which may be tricky to work with
	  in most languages.*/
	tex_format_r16f = 14,
	/*A single channel of data, with 32 bits per-pixel! This
	  basically treats each pixel as a generic float, so you can do all
	  sorts of strange and interesting things with this.*/
	tex_format_r32 = 15,
	/*A depth data format, 24 bits for depth data, and 8 bits
	  to store stencil information! Stencil data can be used for things
	  like clipping effects, deferred rendering, or shadow effects.*/
	tex_format_depthstencil = 16,
	/*32 bits of data per depth value! This is pretty detailed,
	  and is excellent for experiences that have a very far view
	  distance.*/
	tex_format_depth32 = 17,
	/*16 bits of depth is not a lot, but it can be enough if
	  your far clipping plane is pretty close. If you're seeing lots of
	  flickering where two objects overlap, you either need to bring
	  your far clip in, or switch to 32/24 bit depth.*/
	tex_format_depth16 = 18,
	/*A double channel of data that supports 8 bits for the red
	  channel and 8 bits for the green channel.*/
	tex_format_r8g8 = 19,

	tex_format_bc1_rgb_srgb,
	tex_format_bc1_rgb,
	tex_format_bc3_rgba_srgb,
	tex_format_bc3_rgba,
	tex_format_bc4_r,
	tex_format_bc5_rg,
	tex_format_bc7_rgba_srgb,
	tex_format_bc7_rgba,

	tex_format_etc1_rgb,
	tex_format_etc2_rgba_srgb,
	tex_format_etc2_rgba,
	tex_format_etc2_r11,
	tex_format_etc2_rg11,
	tex_format_pvrtc1_rgb_srgb,
	tex_format_pvrtc1_rgb,
	tex_format_pvrtc1_rgba_srgb,
	tex_format_pvrtc1_rgba,
	tex_format_pvrtc2_rgba_srgb,
	tex_format_pvrtc2_rgba,
	tex_format_astc4x4_rgba_srgb,
	tex_format_astc4x4_rgba,
	tex_format_atc_rgb,
	tex_format_atc_rgba,
} tex_format_;

/*How does the shader grab pixels from the texture? Or more
  specifically, how does the shader grab colors between the provided
  pixels? If you'd like an in-depth explanation of these topics, check
  out [this exploration of texture filtering](https://bgolus.medium.com/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec)
  by graphics wizard Ben Golus.*/
typedef enum tex_sample_ {
	/*Use a linear blend between adjacent pixels, this creates
	  a smooth, blurry look when texture resolution is too low.*/
	tex_sample_linear = 0,
	/*Choose the nearest pixel's color! This makes your texture
	  look like pixel art if you're too close.*/
	tex_sample_point,
	/*This helps reduce texture blurriness when a surface is
	  viewed at an extreme angle!*/
	tex_sample_anisotropic
} tex_sample_;

/*What happens when the shader asks for a texture coordinate
  that's outside the texture?? Believe it or not, this happens plenty
  often!*/
typedef enum tex_address_ {
	/*Wrap the UV coordinate around to the other side of the
	  texture! This is basically like a looping texture, and is an
	  excellent default. If you can see weird bits of color at the edges
	  of your texture, this may be due to Wrap blending the color with
	  the other side of the texture, Clamp may be better in such cases.*/
	tex_address_wrap = 0,
	/*Clamp the UV coordinates to the edge of the texture!
	  This'll create color streaks that continue to forever. This is
	  actually really great for non-looping textures that you know will
	  always be accessed on the 0-1 range.*/
	tex_address_clamp,
	/*Like Wrap, but it reflects the image each time! Who needs
	  this? I'm not sure!! But the graphics card can do it, so now you
	  can too!*/
	tex_address_mirror,
} tex_address_;

SK_API tex_t        tex_find                (const char *id);
SK_API tex_t        tex_create              (tex_type_ type sk_default(tex_type_image), tex_format_ format sk_default(tex_format_rgba32));
SK_API tex_t        tex_create_rendertarget (int32_t width, int32_t height, int32_t msaa sk_default(1), tex_format_ color_format sk_default(tex_format_rgba32), tex_format_ depth_format sk_default(tex_format_depth16));
SK_API tex_t        tex_create_color32      (color32  *in_arr_data, int32_t width, int32_t height, bool32_t srgb_data sk_default(true));
SK_API tex_t        tex_create_color128     (color128 *in_arr_data, int32_t width, int32_t height, bool32_t srgb_data sk_default(true));
SK_API tex_t        tex_create_mem          (void *data, size_t data_size,                  bool32_t srgb_data sk_default(true), int32_t priority sk_default(10));
SK_API tex_t        tex_create_file         (const char *file_utf8,                         bool32_t srgb_data sk_default(true), int32_t priority sk_default(10));
SK_API tex_t        tex_create_file_arr     (const char **in_arr_files, int32_t file_count, bool32_t srgb_data sk_default(true), int32_t priority sk_default(10));
SK_API tex_t        tex_create_cubemap_file (const char *cubemap_file_utf8,                 bool32_t srgb_data sk_default(true), int32_t priority sk_default(10));
SK_API tex_t        tex_create_cubemap_files(const char **in_arr_cube_face_file_xxyyzz,     bool32_t srgb_data sk_default(true), int32_t priority sk_default(10));
SK_API tex_t        tex_copy                (const tex_t texture, tex_type_ type sk_default(tex_type_image), tex_format_ format sk_default(tex_format_none));
SK_API bool32_t     tex_gen_mips            (tex_t texture);
SK_API void         tex_set_id              (tex_t texture, const char *id);
SK_API const char*  tex_get_id              (const tex_t texture);
SK_API void         tex_set_fallback        (tex_t texture, tex_t fallback);
SK_API void         tex_set_surface         (tex_t texture, void *native_surface, tex_type_ type, int64_t native_fmt, int32_t width, int32_t height, int32_t surface_count, int32_t multisample sk_default(1), int32_t framebuffer_multisample sk_default(1), bool32_t owned sk_default(true));
SK_API void*        tex_get_surface         (tex_t texture);
SK_API void         tex_addref              (tex_t texture);
SK_API void         tex_release             (tex_t texture);
SK_API asset_state_ tex_asset_state         (const tex_t texture);
SK_API void         tex_on_load             (tex_t texture, void (*asset_on_load_callback)(tex_t texture, void *context), void *context);
SK_API void         tex_on_load_remove      (tex_t texture, void (*asset_on_load_callback)(tex_t texture, void *context));
SK_API void         tex_set_colors          (tex_t texture, int32_t width, int32_t height, void *data);
SK_API void         tex_set_color_arr       (tex_t texture, int32_t width, int32_t height, void** array_data, int32_t array_count, spherical_harmonics_t* out_sh_lighting_info sk_default(nullptr), int32_t multisample sk_default(1));
SK_API void         tex_set_color_arr_mips  (tex_t texture, int32_t width, int32_t height, void** array_data, int32_t array_count, int32_t mip_count, int32_t multisample sk_default(1), spherical_harmonics_t* sh_lighting_info sk_default(nullptr));
SK_API void         tex_set_mem             (tex_t texture, void* data, size_t data_size, bool32_t srgb_data sk_default(true), bool32_t blocking sk_default(false), int32_t priority sk_default(10));
// TODO: For v0.4, remove the return value here, since this needs to addref, and the texture may be ignored
SK_API tex_t        tex_add_zbuffer         (tex_t texture, tex_format_ format sk_default(tex_format_depthstencil));
SK_API void         tex_set_zbuffer         (tex_t texture, tex_t depth_texture);
// TODO: For v0.4, combine these two functions
SK_API void         tex_get_data            (tex_t texture, void *out_data, size_t out_data_size);
SK_API void         tex_get_data_mip        (tex_t texture, void *out_data, size_t out_data_size, int32_t mip_level);
SK_API tex_t        tex_gen_color           (color128 color, int32_t width, int32_t height, tex_type_ type sk_default(tex_type_image), tex_format_ format sk_default(tex_format_rgba32));
SK_API tex_t        tex_gen_particle        (int32_t width, int32_t height, float roundness sk_default(1), gradient_t gradient_linear sk_default(nullptr));
SK_API tex_t        tex_gen_cubemap         (const gradient_t gradient, vec3 gradient_dir, int32_t resolution, spherical_harmonics_t *out_sh_lighting_info sk_default(nullptr));
SK_API tex_t        tex_gen_cubemap_sh      (const sk_ref(spherical_harmonics_t) lookup, int32_t face_size, float light_spot_size_pct sk_default(0), float light_spot_intensity sk_default(6));
SK_API tex_format_  tex_get_format          (tex_t texture);
SK_API int32_t      tex_get_width           (tex_t texture);
SK_API int32_t      tex_get_height          (tex_t texture);
SK_API void         tex_set_sample          (tex_t texture, tex_sample_ sample sk_default(tex_sample_linear));
SK_API tex_sample_  tex_get_sample          (tex_t texture);
SK_API void         tex_set_address         (tex_t texture, tex_address_ address_mode sk_default(tex_address_wrap));
SK_API tex_address_ tex_get_address         (tex_t texture);
SK_API void         tex_set_anisotropy      (tex_t texture, int32_t anisotropy_level sk_default(4));
SK_API int32_t      tex_get_anisotropy      (tex_t texture);
SK_API int32_t      tex_get_mips            (tex_t texture);
SK_API void         tex_set_loading_fallback(tex_t loading_texture);
SK_API void         tex_set_error_fallback  (tex_t error_texture);
SK_API spherical_harmonics_t tex_get_cubemap_lighting(tex_t cubemap_texture);

///////////////////////////////////////////

SK_API font_t       font_find               (const char *id);
SK_API font_t       font_create             (const char *file_utf8);
SK_API font_t       font_create_files       (const char **in_arr_files, int32_t file_count);
SK_API void         font_set_id             (font_t font, const char* id);
SK_API const char*  font_get_id             (const font_t font);
SK_API void         font_addref             (font_t font);
SK_API void         font_release            (font_t font);
SK_API tex_t        font_get_tex            (font_t font);

///////////////////////////////////////////

SK_API shader_t     shader_find             (const char *id);
SK_API shader_t     shader_create_file      (const char *filename_utf8);
SK_API shader_t     shader_create_mem       (void *data, size_t data_size);
SK_API void         shader_set_id           (shader_t shader, const char *id);
SK_API const char*  shader_get_id           (const shader_t shader);
SK_API const char*  shader_get_name         (shader_t shader);
SK_API void         shader_addref           (shader_t shader);
SK_API void         shader_release          (shader_t shader);

///////////////////////////////////////////

/*Also known as 'alpha' for those in the know. But there's
  actually more than one type of transparency in rendering! The
  horrors. We're keepin' it fairly simple for now, so you get three
  options!*/
typedef enum transparency_ {
	/*Not actually transparent! This is opaque! Solid! It's
	  the default option, and it's the fastest option! Opaque objects
	  write to the z-buffer, the occlude pixels behind them, and they
	  can be used as input to important Mixed Reality features like
	  Late Stage Reprojection that'll make your view more stable!*/
	transparency_none = 1,
	/*Also known as Alpha To Coverage, this mode uses MSAA samples to
	  create transparency. This works with a z-buffer and therefore
	  functionally behaves more like an opaque material, but has a
	  quantized number of "transparent values" it supports rather than
	  a full range of  0-255 or 0-1. For 4x MSAA, this will give only
	  4 different transparent values, 8x MSAA only 8, etc.
	  From a performance perspective, MSAA usually is only costly
	  around triangle edges, but using this mode, MSAA is used for the
	  whole triangle.*/
	transparency_msaa = 2,
	/*This will blend with the pixels behind it. This is 
	  transparent! You may not want to write to the z-buffer, and it's
	  slower than opaque materials.*/
	transparency_blend = 3,
	/*This will straight up add the pixel color to the color
	  buffer! This usually looks -really- glowy, so it makes for good
	  particles or lighting effects.*/
	transparency_add = 4,
} transparency_;

/*Depth test describes how this material looks at and responds
  to depth information in the zbuffer! The default is Less, which means
  if the material pixel's depth is Less than the existing depth data,
  (basically, is this in front of some other object) it will draw that
  pixel. Similarly, Greater would only draw the material if it's
  'behind' the depth buffer. Always would just draw all the time, and
  not read from the depth buffer at all.*/
typedef enum depth_test_ {
	/*Default behavior, pixels behind the depth buffer will be
	  discarded, and pixels in front of it will be drawn.*/
	depth_test_less = 0,
	/*Pixels behind the depth buffer will be discarded, and
	  pixels in front of, or at the depth buffer's value it will be
	  drawn. This could be great for things that might be sitting
	  exactly on a floor or wall.*/
	depth_test_less_or_eq,
	/*Pixels in front of the zbuffer will be discarded! This
	  is opposite of how things normally work. Great for drawing
	  indicators that something is occluded by a wall or other
	  geometry.*/
	depth_test_greater,
	/*Pixels in front of (or exactly at) the zbuffer will be
	  discarded! This is opposite of how things normally work. Great
	  for drawing indicators that something is occluded by a wall or
	  other geometry.*/
	depth_test_greater_or_eq,
	/*Only draw pixels if they're at exactly the same depth as
	  the zbuffer!*/
	depth_test_equal,
	/*Draw any pixel that's not exactly at the value in the
	  zbuffer.*/
	depth_test_not_equal,
	/*Don't look at the zbuffer at all, just draw everything,
	  always, all the time! At this point, the order at which the mesh
	  gets drawn will be  super important, so don't forget about
	  `Material.QueueOffset`!*/
	depth_test_always,
	/*Never draw a pixel, regardless of what's in the zbuffer.
	  I can think of better ways to do this, but uhh, this is here for
	  completeness! Maybe you can find a use for it.*/
	depth_test_never,
} depth_test_;

// TODO: v0.4 This may need significant revision?
/*What type of data does this material parameter need? This is
  used to tell the shader how large the data is, and where to attach it
  to on the shader.*/
typedef enum material_param_ {
	/*This data type is not currently recognized. Please
	  report your case on GitHub Issues!*/
	material_param_unknown = 0,
	/*A single 32 bit float value.*/
	material_param_float = 1,
	/*A color value described by 4 floating point values. Memory-wise this is
	  the same as a Vector4, but in the shader this variable has a ':color'
	  tag applied to it using StereoKits's shader info syntax, indicating it's
	  a color value. Color values for shaders should be in linear space, not
	  gamma.*/
	material_param_color128 = 2,
	/*A 2 component vector composed of floating point values.*/
	material_param_vector2 = 3,
	/*A 3 component vector composed of floating point values.*/
	material_param_vector3 = 4,
	/*A 4 component vector composed of floating point values.*/
	material_param_vector4 = 5,
	/*obsolete: Replaced by MaterialParam.Vector4
	  A 4 component vector composed of floating point values.
	  TODO: Remove in v0.4*/
	material_param_vector = 5,
	/*A 4x4 matrix of floats.*/
	material_param_matrix = 6,
	/*Texture information!*/
	material_param_texture = 7,
	/*A 1 component vector composed of signed integers.*/
	material_param_int = 8,
	/*A 2 component vector composed of signed integers.*/
	material_param_int2 = 9,
	/*A 3 component vector composed of signed integers.*/
	material_param_int3 = 10,
	/*A 4 component vector composed of signed integers.*/
	material_param_int4 = 11,
	/*A 1 component vector composed of unsigned integers. This may also be a
	  boolean.*/
	material_param_uint = 12,
	/*A 2 component vector composed of unsigned integers.*/
	material_param_uint2 = 13,
	/*A 3 component vector composed of unsigned integers.*/
	material_param_uint3 = 14,
	/*A 4 component vector composed of unsigned integers.*/
	material_param_uint4 = 15,
} material_param_;

SK_API material_t        material_find            (const char *id);
SK_API material_t        material_create          (shader_t shader);
SK_API material_t        material_copy            (material_t material);
SK_API material_t        material_copy_id         (const char *id);
SK_API void              material_set_id          (material_t material, const char *id);
SK_API const char*       material_get_id          (const material_t material);
SK_API void              material_addref          (material_t material);
SK_API void              material_release         (material_t material);
SK_API void              material_set_transparency(material_t material, transparency_ mode);
SK_API void              material_set_cull        (material_t material, cull_ mode);
SK_API void              material_set_wireframe   (material_t material, bool32_t wireframe);
SK_API void              material_set_depth_test  (material_t material, depth_test_ depth_test_mode);
SK_API void              material_set_depth_write (material_t material, bool32_t write_enabled);
SK_API void              material_set_queue_offset(material_t material, int32_t offset);
SK_API void              material_set_chain       (material_t material, material_t chain_material);
SK_API transparency_     material_get_transparency(material_t material);
SK_API cull_             material_get_cull        (material_t material);
SK_API bool32_t          material_get_wireframe   (material_t material);
SK_API depth_test_       material_get_depth_test  (material_t material);
SK_API bool32_t          material_get_depth_write (material_t material);
SK_API int32_t           material_get_queue_offset(material_t material);
SK_API material_t        material_get_chain       (material_t material);
SK_API void              material_set_float       (material_t material, const char *name, float    value);
SK_API void              material_set_vector2     (material_t material, const char *name, vec2     value);
SK_API void              material_set_vector3     (material_t material, const char *name, vec3     value);
SK_API void              material_set_color       (material_t material, const char *name, color128 color_gamma);
SK_API void              material_set_vector4     (material_t material, const char *name, vec4     value);
SK_API void              material_set_vector      (material_t material, const char *name, vec4     value); // TODO: Remove in v0.4
SK_API void              material_set_int         (material_t material, const char *name, int32_t  value);
SK_API void              material_set_int2        (material_t material, const char *name, int32_t  value1, int32_t value2);
SK_API void              material_set_int3        (material_t material, const char *name, int32_t  value1, int32_t value2, int32_t value3);
SK_API void              material_set_int4        (material_t material, const char *name, int32_t  value1, int32_t value2, int32_t value3, int32_t value4);
SK_API void              material_set_bool        (material_t material, const char *name, bool32_t value);
SK_API void              material_set_uint        (material_t material, const char *name, uint32_t value);
SK_API void              material_set_uint2       (material_t material, const char *name, uint32_t value1, uint32_t value2);
SK_API void              material_set_uint3       (material_t material, const char *name, uint32_t value1, uint32_t value2, uint32_t value3);
SK_API void              material_set_uint4       (material_t material, const char *name, uint32_t value1, uint32_t value2, uint32_t value3, uint32_t value4);
SK_API void              material_set_matrix      (material_t material, const char *name, matrix   value);
SK_API bool32_t          material_set_texture     (material_t material, const char *name, tex_t    value);
SK_API bool32_t          material_set_texture_id  (material_t material, uint64_t    id,   tex_t    value);
SK_API float             material_get_float       (material_t material, const char *name);
SK_API vec2              material_get_vector2     (material_t material, const char *name);
SK_API vec3              material_get_vector3     (material_t material, const char *name);
SK_API color128          material_get_color       (material_t material, const char *name);
SK_API vec4              material_get_vector4     (material_t material, const char *name);
SK_API int32_t           material_get_int         (material_t material, const char *name);
SK_API bool32_t          material_get_bool        (material_t material, const char *name);
SK_API uint32_t          material_get_uint        (material_t material, const char *name);
SK_API matrix            material_get_matrix      (material_t material, const char *name);
SK_API tex_t             material_get_texture     (material_t material, const char *name);
SK_API bool32_t          material_has_param       (material_t material, const char *name, material_param_ type);
SK_API void              material_set_param       (material_t material, const char *name, material_param_ type, const void *value);
SK_API void              material_set_param_id    (material_t material, uint64_t    id,   material_param_ type, const void *value);
SK_API bool32_t          material_get_param       (material_t material, const char *name, material_param_ type, void *out_value);
SK_API bool32_t          material_get_param_id    (material_t material, uint64_t    id,   material_param_ type, void *out_value);
SK_API void              material_get_param_info  (material_t material, int32_t index, char **out_name, material_param_ *out_type);
SK_API int32_t           material_get_param_count (material_t material);
SK_API void              material_set_shader      (material_t material, shader_t shader);
SK_API shader_t          material_get_shader      (material_t material);

SK_API material_buffer_t material_buffer_create   (int32_t register_slot, int32_t size);
SK_API void              material_buffer_set_data (material_buffer_t buffer, const void *buffer_data);
SK_API void              material_buffer_release  (material_buffer_t buffer);

///////////////////////////////////////////

/*This enum describes how text layout behaves within the space
  it is given.*/
typedef enum text_fit_ {
	/*No particularly special behavior.*/
	text_fit_none           = 0,
	/*The text will wrap around to the next line down when it
	  reaches the end of the space on the X axis.*/
	text_fit_wrap           = 1 << 0,
	/*When the text reaches the end, it is simply truncated
	  and no longer visible.*/
	text_fit_clip           = 1 << 1,
	/*If the text is too large to fit in the space provided,
	  it will be scaled down to fit inside. This will not scale up.*/
	text_fit_squeeze        = 1 << 2,
	/*If the text is larger, or smaller than the space 
	  provided, it will scale down or up to fill the space.*/
	text_fit_exact          = 1 << 3,
	/*The text will ignore the containing space, and just keep
	  on going.*/
	text_fit_overflow       = 1 << 4
} text_fit_;
SK_MakeFlag(text_fit_)

/*A bit-flag enum for describing alignment or positioning.
  Items can be combined using the '|' operator, like so:
  
  `TextAlign alignment = TextAlign.YTop | TextAlign.XLeft;`
  
  Avoid combining multiple items of the same axis. There are also a
  complete list of valid bit flag combinations! These are the values
  without an axis listed in their names, 'TopLeft', 'BottomCenter',
  etc.*/
typedef enum text_align_ {
	/*On the x axis, this item should start on the left.*/
	text_align_x_left       = 1 << 0,
	/*On the y axis, this item should start at the top.*/
	text_align_y_top        = 1 << 1,
	/*On the x axis, the item should be centered.*/
	text_align_x_center     = 1 << 2,
	/*On the y axis, the item should be centered.*/
	text_align_y_center     = 1 << 3,
	/*On the x axis, this item should start on the right.*/
	text_align_x_right      = 1 << 4,
	/*On the y axis, this item should start on the bottom.*/
	text_align_y_bottom     = 1 << 5,
	/*Center on both X and Y axes. This is a combination of 
	  XCenter and YCenter.*/
	text_align_center       = text_align_x_center | text_align_y_center,
	/*Start on the left of the X axis, center on the Y axis. 
	  This is a combination of XLeft and YCenter.*/
	text_align_center_left  = text_align_x_left   | text_align_y_center,
	/*Start on the right of the X axis, center on the Y axis. 
	  This is a combination of XRight and YCenter.*/
	text_align_center_right = text_align_x_right  | text_align_y_center,
	/*Center on the X axis, and top on the Y axis. This is a
	  combination of XCenter and YTop.*/
	text_align_top_center   = text_align_x_center | text_align_y_top,
	/*Start on the left of the X axis, and top on the Y axis.
	  This is a combination of XLeft and YTop.*/
	text_align_top_left     = text_align_x_left   | text_align_y_top,
	/*Start on the right of the X axis, and top on the Y axis.
	  This is a combination of XRight and YTop.*/
	text_align_top_right    = text_align_x_right  | text_align_y_top,
	/*Center on the X axis, and bottom on the Y axis. This is
	  a combination of XCenter and YBottom.*/
	text_align_bottom_center= text_align_x_center | text_align_y_bottom,
	/*Start on the left of the X axis, and bottom on the Y
	  axis. This is a combination of XLeft and YBottom.*/
	text_align_bottom_left  = text_align_x_left   | text_align_y_bottom,
	/*Start on the right of the X axis, and bottom on the Y
	  axis.This is a combination of XRight and YBottom.*/
	text_align_bottom_right = text_align_x_right  | text_align_y_bottom,
} text_align_;
SK_MakeFlag(text_align_);

typedef uint32_t text_style_t;

SK_API text_style_t  text_make_style               (font_t font, float character_height,                      color128 color_gamma);
SK_API text_style_t  text_make_style_shader        (font_t font, float character_height, shader_t shader,     color128 color_gamma);
SK_API text_style_t  text_make_style_mat           (font_t font, float character_height, material_t material, color128 color_gamma);
SK_API void          text_add_at                   (const char*     text_utf8,  const sk_ref(matrix)  transform, text_style_t style sk_default(0), text_align_ position sk_default(text_align_center), text_align_ align sk_default(text_align_center), float off_x sk_default(0), float off_y sk_default(0), float off_z sk_default(0), color128 vertex_tint_linear sk_default({1,1,1,1}));
SK_API void          text_add_at_16                (const char16_t* text_utf16, const sk_ref(matrix)  transform, text_style_t style sk_default(0), text_align_ position sk_default(text_align_center), text_align_ align sk_default(text_align_center), float off_x sk_default(0), float off_y sk_default(0), float off_z sk_default(0), color128 vertex_tint_linear sk_default({1,1,1,1}));
SK_API float         text_add_in                   (const char*     text_utf8,  const sk_ref(matrix)  transform, vec2 size, text_fit_ fit, text_style_t style sk_default(0), text_align_ position sk_default(text_align_center), text_align_ align sk_default(text_align_center), float off_x sk_default(0), float off_y sk_default(0), float off_z sk_default(0), color128 vertex_tint_linear sk_default({1,1,1,1}));
SK_API float         text_add_in_16                (const char16_t* text_utf16, const sk_ref(matrix)  transform, vec2 size, text_fit_ fit, text_style_t style sk_default(0), text_align_ position sk_default(text_align_center), text_align_ align sk_default(text_align_center), float off_x sk_default(0), float off_y sk_default(0), float off_z sk_default(0), color128 vertex_tint_linear sk_default({1,1,1,1}));
SK_API vec2          text_size                     (const char*     text_utf8,  text_style_t style sk_default(0));
SK_API vec2          text_size_16                  (const char16_t* text_utf16, text_style_t style sk_default(0));
SK_API vec2          text_size_constrained         (const char*     text_utf8,  text_style_t style, float max_width);
SK_API vec2          text_size_constrained_16      (const char16_t* text_utf16, text_style_t style, float max_width);
SK_API vec2          text_char_at                  (const char*     text_utf8,  text_style_t style, int32_t char_index, vec2 *opt_size, text_fit_ fit, text_align_ position, text_align_ align);
SK_API vec2          text_char_at_16               (const char16_t* text_utf16, text_style_t style, int32_t char_index, vec2 *opt_size, text_fit_ fit, text_align_ position, text_align_ align);

SK_API material_t    text_style_get_material       (text_style_t style);
SK_API float         text_style_get_char_height    (text_style_t style);
SK_API void          text_style_set_char_height    (text_style_t style, float height_meters);

///////////////////////////////////////////

/*This describes the behavior of a 'Solid' physics object! The
  physics engine will apply forces differently based on this type.*/
typedef enum solid_type_ {
	/*This object behaves like a normal physical object, it'll
	  fall, get pushed around, and generally be susceptible to physical
	  forces! This is a 'Dynamic' body in physics simulation terms.*/
	solid_type_normal = 0,
	/*Immovable objects are always stationary! They have
	  infinite mass, zero velocity, and can't collide with Immovable of
	  Unaffected types.*/
	solid_type_immovable,
	/*Unaffected objects have infinite mass, but can have a
	  velocity! They'll move under their own forces, but nothing in the
	  simulation will affect them. They don't collide with Immovable or
	  Unaffected types.*/
	solid_type_unaffected,
} solid_type_;

SK_API solid_t       solid_create                  (const sk_ref(vec3) position, const sk_ref(quat) rotation, solid_type_ type sk_default(solid_type_normal));
SK_API void          solid_release                 (solid_t solid);
SK_API void          solid_set_id                  (const solid_t solid, const char *id);
SK_API const char*   solid_get_id                  (const solid_t solid);
SK_API void          solid_add_sphere              (solid_t solid, float diameter sk_default(1), float kilograms sk_default(1), const vec3 *offset sk_default(nullptr));
SK_API void          solid_add_box                 (solid_t solid, const sk_ref(vec3) dimensions,float kilograms sk_default(1), const vec3 *offset sk_default(nullptr));
SK_API void          solid_add_capsule             (solid_t solid, float diameter, float height, float kilograms sk_default(1), const vec3 *offset sk_default(nullptr));
SK_API void          solid_set_type                (solid_t solid, solid_type_ type);
SK_API void          solid_set_enabled             (solid_t solid, bool32_t enabled);
SK_API void          solid_move                    (solid_t solid, const sk_ref(vec3) position, const sk_ref(quat) rotation);
SK_API void          solid_teleport                (solid_t solid, const sk_ref(vec3) position, const sk_ref(quat) rotation);
SK_API void          solid_set_velocity            (solid_t solid, const sk_ref(vec3) meters_per_second);
SK_API void          solid_set_velocity_ang        (solid_t solid, const sk_ref(vec3) radians_per_second);
SK_API void          solid_get_pose                (const solid_t solid, sk_ref(pose_t) out_pose);

///////////////////////////////////////////

typedef int32_t model_node_id;

/*Describes how an animation is played back, and what to do when
  the animation hits the end.*/
typedef enum anim_mode_ {
	/*If the animation reaches the end, it will always loop
	  back around to the start again.*/
	anim_mode_loop,
	/*When the animation reaches the end, it will freeze
	  in-place.*/
	anim_mode_once,
	/*The animation will not progress on its own, and instead
	  must be driven by providing information to the model's AnimTime
	  or AnimCompletion properties.*/
	anim_mode_manual,
} anim_mode_;

SK_API model_t       model_find                    (const char *id);
SK_API model_t       model_copy                    (model_t model);
SK_API model_t       model_create                  (void);
SK_API model_t       model_create_mesh             (mesh_t mesh, material_t material);
SK_API model_t       model_create_mem              (const char *filename_utf8, const void *data, size_t data_size, shader_t shader sk_default(nullptr));
SK_API model_t       model_create_file             (const char *filename_utf8, shader_t shader sk_default(nullptr));
SK_API void          model_set_id                  (model_t model, const char *id);
SK_API const char*   model_get_id                  (const model_t model);
SK_API void          model_addref                  (model_t model);
SK_API void          model_release                 (model_t model);
SK_API void          model_draw                    (model_t model,                               matrix transform, color128 color_linear sk_default({1,1,1,1}), render_layer_ layer sk_default(render_layer_0));
SK_API void          model_draw_mat                (model_t model, material_t material_override, matrix transform, color128 color_linear sk_default({1,1,1,1}), render_layer_ layer sk_default(render_layer_0));
SK_API void          model_recalculate_bounds      (model_t model);
SK_API void          model_recalculate_bounds_exact(model_t model);
SK_API void          model_set_bounds              (model_t model, const sk_ref(bounds_t) bounds);
SK_API bounds_t      model_get_bounds              (model_t model);
SK_API bool32_t      model_ray_intersect           (model_t model, ray_t model_space_ray, ray_t* out_pt, cull_ cull_mode sk_default(cull_back));
SK_API bool32_t      model_ray_intersect_bvh       (model_t model, ray_t model_space_ray, ray_t *out_pt, cull_ cull_mode sk_default(cull_back));
// TODO: in 0.4 move cull_mode parameter up to directly after out_pt
SK_API bool32_t      model_ray_intersect_bvh_detailed(model_t model, ray_t model_space_ray, ray_t *out_pt, mesh_t *out_mesh sk_default(nullptr), matrix *out_matrix sk_default(nullptr), uint32_t* out_start_inds sk_default(nullptr), cull_ cull_mode sk_default(cull_back));

SK_API void          model_step_anim               (model_t model);
SK_API bool32_t      model_play_anim               (model_t model, const char *animation_name, anim_mode_ mode);
SK_API void          model_play_anim_idx           (model_t model, int32_t index,              anim_mode_ mode);
SK_API void          model_set_anim_time           (model_t model, float time);
SK_API void          model_set_anim_completion     (model_t model, float percent);
SK_API int32_t       model_anim_find               (model_t model, const char *animation_name);
SK_API int32_t       model_anim_count              (model_t model);
SK_API int32_t       model_anim_active             (model_t model);
SK_API anim_mode_    model_anim_active_mode        (model_t model);
SK_API float         model_anim_active_time        (model_t model);
SK_API float         model_anim_active_completion  (model_t model);
SK_API const char*   model_anim_get_name           (model_t model, int32_t index);
SK_API float         model_anim_get_duration       (model_t model, int32_t index);

// TODO: this whole section gets removed in v0.4, prefer the model_node api
SK_API const char*   model_get_name                (model_t model, int32_t subset);
SK_API material_t    model_get_material            (model_t model, int32_t subset);
SK_API mesh_t        model_get_mesh                (model_t model, int32_t subset);
SK_API matrix        model_get_transform           (model_t model, int32_t subset);
SK_API void          model_set_material            (model_t model, int32_t subset, material_t material);
SK_API void          model_set_mesh                (model_t model, int32_t subset, mesh_t mesh);
SK_API void          model_set_transform           (model_t model, int32_t subset, const sk_ref(matrix) transform);
SK_API void          model_remove_subset           (model_t model, int32_t subset);
SK_API int32_t       model_add_named_subset        (model_t model, const char *name, mesh_t mesh, material_t material, const sk_ref(matrix) transform);
SK_API int32_t       model_add_subset              (model_t model, mesh_t mesh, material_t material, const sk_ref(matrix) transform);
SK_API int32_t       model_subset_count            (model_t model);


SK_API model_node_id model_node_add                (model_t model,                       const char *name, matrix model_transform, mesh_t mesh sk_default(nullptr), material_t material sk_default(nullptr), bool32_t solid sk_default(true));
SK_API model_node_id model_node_add_child          (model_t model, model_node_id parent, const char *name, matrix local_transform, mesh_t mesh sk_default(nullptr), material_t material sk_default(nullptr), bool32_t solid sk_default(true));
SK_API model_node_id model_node_find               (model_t model, const char *name);
SK_API model_node_id model_node_sibling            (model_t model, model_node_id node);
SK_API model_node_id model_node_parent             (model_t model, model_node_id node);
SK_API model_node_id model_node_child              (model_t model, model_node_id node);
SK_API int32_t       model_node_count              (model_t model);
SK_API model_node_id model_node_index              (model_t model, int32_t index);
SK_API int32_t       model_node_visual_count       (model_t model);
SK_API model_node_id model_node_visual_index       (model_t model, int32_t index);
SK_API model_node_id model_node_iterate            (model_t model, model_node_id node);
SK_API model_node_id model_node_get_root           (model_t model);
SK_API const char*   model_node_get_name           (model_t model, model_node_id node);
SK_API bool32_t      model_node_get_solid          (model_t model, model_node_id node);
SK_API bool32_t      model_node_get_visible        (model_t model, model_node_id node);
SK_API material_t    model_node_get_material       (model_t model, model_node_id node);
SK_API mesh_t        model_node_get_mesh           (model_t model, model_node_id node);
SK_API matrix        model_node_get_transform_model(model_t model, model_node_id node);
SK_API matrix        model_node_get_transform_local(model_t model, model_node_id node);
SK_API void          model_node_set_name           (model_t model, model_node_id node, const char* name);
SK_API void          model_node_set_solid          (model_t model, model_node_id node, bool32_t    solid);
SK_API void          model_node_set_visible        (model_t model, model_node_id node, bool32_t    visible);
SK_API void          model_node_set_material       (model_t model, model_node_id node, material_t  material);
SK_API void          model_node_set_mesh           (model_t model, model_node_id node, mesh_t      mesh);
SK_API void          model_node_set_transform_model(model_t model, model_node_id node, matrix      transform_model_space);
SK_API void          model_node_set_transform_local(model_t model, model_node_id node, matrix      transform_local_space);
SK_API const char*   model_node_info_get           (model_t model, model_node_id node, const char* info_key_utf8);
SK_API void          model_node_info_set           (model_t model, model_node_id node, const char* info_key_utf8, const char* info_value_utf8);
SK_API bool32_t      model_node_info_remove        (model_t model, model_node_id node, const char* info_key_utf8);
SK_API void          model_node_info_clear         (model_t model, model_node_id node);
SK_API int32_t       model_node_info_count         (model_t model, model_node_id node);
SK_API bool32_t      model_node_info_iterate       (model_t model, model_node_id node, int32_t *ref_iterator, const char **out_key_utf8, const char **out_value_utf8);

///////////////////////////////////////////

/*The way the Sprite is stored on the backend! Does it get
  batched and atlased for draw efficiency, or is it a single image?*/
typedef enum sprite_type_ {
	/*The sprite will be batched onto an atlas texture so all
	  sprites can be drawn in a single pass. This is excellent for
	  performance! The only thing to watch out for here, adding a sprite
	  to an atlas will rebuild the atlas texture! This can be a bit
	  expensive, so it's recommended to add all sprites to an atlas at
	  start, rather than during runtime. Also, if an image is too large,
	  it may take up too much space on the atlas, and may be better as a
	  Single sprite type.*/
	sprite_type_atlased = 0,
	/*This sprite is on its own texture. This is best for large
	  images, items that get loaded and unloaded during runtime, or for
	  sprites that may have edge artifacts or severe 'bleed' from
	  adjacent atlased images.*/
	sprite_type_single
} sprite_type_;

SK_API sprite_t    sprite_find       (const char* id);
SK_API sprite_t    sprite_create     (tex_t    sprite,           sprite_type_ type sk_default(sprite_type_atlased), const char *atlas_id sk_default("default"));
SK_API sprite_t    sprite_create_file(const char *filename_utf8, sprite_type_ type sk_default(sprite_type_atlased), const char *atlas_id sk_default("default"));
SK_API void        sprite_set_id     (sprite_t sprite, const char *id);
SK_API const char* sprite_get_id     (const sprite_t sprite);
SK_API void        sprite_addref     (sprite_t sprite);
SK_API void        sprite_release    (sprite_t sprite);
SK_API float       sprite_get_aspect (sprite_t sprite);
SK_API int32_t     sprite_get_width  (sprite_t sprite);
SK_API int32_t     sprite_get_height (sprite_t sprite);
SK_API vec2        sprite_get_dimensions_normalized(sprite_t sprite);
SK_API void        sprite_draw       (sprite_t sprite, const sk_ref(matrix) transform, color32 color sk_default({255,255,255,255}));
SK_API void        sprite_draw_at    (sprite_t sprite, matrix transform, text_align_ anchor_position, color32 color sk_default({255,255,255,255}));

///////////////////////////////////////////

typedef struct line_point_t {
	vec3    pt;
	float   thickness;
	color32 color;
} line_point_t;

SK_API void line_add      (vec3 start, vec3 end, color32 color_start, color32 color_end, float thickness);
SK_API void line_addv     (line_point_t start, line_point_t end);
SK_API void line_add_axis (pose_t pose, float size);
SK_API void line_add_list (const vec3 *points, int32_t count, color32 color, float thickness);
SK_API void line_add_listv(const line_point_t *in_arr_points, int32_t count);

///////////////////////////////////////////

/*When rendering to a rendertarget, this tells if and what of the
  rendertarget gets cleared before rendering. For example, if you
  are assembling a sheet of images, you may want to clear
  everything on the first image draw, but not clear on subsequent
  draws.*/
typedef enum render_clear_ {
	/*Don't clear anything, leave it as it is.*/
	render_clear_none  = 0,
	/*Clear the rendertarget's color data.*/
	render_clear_color = 1 << 0,
	/*Clear the rendertarget's depth data, if present.*/
	render_clear_depth = 1 << 1,
	/*Clear both color and depth data.*/
	render_clear_all   = render_clear_color | render_clear_depth,
} render_clear_;
SK_MakeFlag(render_clear_)

/*The projection mode used by StereoKit for the main camera! You
  can use this with Renderer.Projection. These options are only
  available in flatscreen mode, as MR headsets provide very
  specific projection matrices.*/
typedef enum projection_ {
	/*This is the default projection mode, and the one you're most likely
	  to be familiar with! This is where parallel lines will converge as
	  they go into the distance.*/
	projection_perspective = 0,
	/*Orthographic projection mode is often used for tools, 2D rendering,
	  thumbnails of 3D objects, or other similar cases. In this mode,
	  parallel lines remain parallel regardless of how far they travel.*/
	projection_ortho = 1
} projection_;

//TODO: for v0.4, rename render_set_clip and render_set_fov to indicate they are only for perspective
SK_API void                  render_set_clip       (float near_plane sk_default(0.08f), float far_plane sk_default(50));
SK_API void                  render_set_fov        (float field_of_view_degrees sk_default(90.0f));
SK_API void                  render_set_ortho_clip (float near_plane sk_default(0.0f), float far_plane sk_default(50));
SK_API void                  render_set_ortho_size (float viewport_height_meters);
SK_API void                  render_set_projection (projection_ proj);
SK_API projection_           render_get_projection (void);
SK_API matrix                render_get_cam_root   (void);
SK_API void                  render_set_cam_root   (const sk_ref(matrix) cam_root);
SK_API void                  render_set_skytex     (tex_t sky_texture);
SK_API tex_t                 render_get_skytex     (void);
SK_API void                  render_set_skymaterial(material_t sky_material);
SK_API material_t            render_get_skymaterial(void);
SK_API void                  render_set_skylight   (const sk_ref(spherical_harmonics_t) light_info);
SK_API spherical_harmonics_t render_get_skylight   (void);
SK_API void                  render_set_filter     (render_layer_ layer_filter);
SK_API render_layer_         render_get_filter     (void);
SK_API void                  render_set_scaling    (float display_tex_scale);
SK_API float                 render_get_scaling    (void);
SK_API void                  render_set_viewport_scaling(float viewport_rect_scale);
SK_API float                 render_get_viewport_scaling(void);
SK_API void                  render_set_multisample(int32_t display_tex_multisample);
SK_API int32_t               render_get_multisample(void);
SK_API void                  render_override_capture_filter(bool32_t use_override_filter, render_layer_ layer_filter sk_default(render_layer_all));
SK_API render_layer_         render_get_capture_filter     (void);
SK_API bool32_t              render_has_capture_filter     (void);
SK_API void                  render_set_clear_color(color128 color_gamma);
SK_API color128              render_get_clear_color(void);
SK_API void                  render_enable_skytex  (bool32_t show_sky);
SK_API bool32_t              render_enabled_skytex (void);
SK_API void                  render_global_texture (int32_t register_slot, tex_t texture);
SK_API void                  render_add_mesh       (mesh_t  mesh,  material_t material,          const sk_ref(matrix) transform, color128 color_linear sk_default({1,1,1,1}), render_layer_ layer sk_default(render_layer_0));
SK_API void                  render_add_model      (model_t model,                               const sk_ref(matrix) transform, color128 color_linear sk_default({1,1,1,1}), render_layer_ layer sk_default(render_layer_0));
SK_API void                  render_add_model_mat  (model_t model, material_t material_override, const sk_ref(matrix) transform, color128 color_linear sk_default({1,1,1,1}), render_layer_ layer sk_default(render_layer_0));
SK_API void                  render_blit           (tex_t to_rendertarget, material_t material);
//TODO: for v0.4, replace render_screenshot with render_screenshot_pose
SK_API void                  render_screenshot     (const char *file_utf8, vec3 from_viewpt, vec3 at, int32_t width, int32_t height, float field_of_view_degrees);
SK_API void                  render_screenshot_pose(const char *file_utf8, int32_t file_quality_100, pose_t viewpoint, int32_t width, int32_t height, float field_of_view_degrees);
//TODO: for v0.4, reorder parameters, context in particular should be next to callback
SK_API void                  render_screenshot_capture  (void (*render_on_screenshot_callback)(color32* color_buffer, int32_t width, int32_t height, void* context), pose_t viewpoint, int32_t width, int32_t height, float field_of_view_degrees, tex_format_ tex_format sk_default(tex_format_rgba32), void *context sk_default(nullptr));
SK_API void                  render_screenshot_viewpoint(void (*render_on_screenshot_callback)(color32* color_buffer, int32_t width, int32_t height, void* context), matrix camera, matrix projection, int32_t width, int32_t height, render_layer_ layer_filter sk_default(render_layer_all), render_clear_ clear sk_default(render_clear_all), rect_t viewport sk_default(rect_t{}), tex_format_ tex_format sk_default(tex_format_rgba32), void* context sk_default(nullptr));
SK_API void                  render_to             (tex_t to_rendertarget, const sk_ref(matrix) camera, const sk_ref(matrix) projection, render_layer_ layer_filter sk_default(render_layer_all), render_clear_ clear sk_default(render_clear_all), rect_t viewport sk_default({}));
SK_API void                  render_material_to    (tex_t to_rendertarget, material_t override_material, const sk_ref(matrix) camera, const sk_ref(matrix) projection, render_layer_ layer_filter sk_default(render_layer_all), render_clear_ clear sk_default(render_clear_all), rect_t viewport sk_default({}));
SK_API void                  render_get_device     (void **device, void **context);
SK_API render_list_t         render_get_primary_list(void);

///////////////////////////////////////////


SK_API render_list_t         render_list_find         (const char* id);
SK_API render_list_t         render_list_create       (void);
SK_API void                  render_list_set_id       (      render_list_t list, const char* id);
SK_API const char*           render_list_get_id       (const render_list_t list);
SK_API void                  render_list_addref       (      render_list_t list);
SK_API void                  render_list_release      (      render_list_t list);
SK_API void                  render_list_clear        (      render_list_t list);
SK_API int32_t               render_list_item_count   (const render_list_t list);
SK_API int32_t               render_list_prev_count   (const render_list_t list);
SK_API void                  render_list_add_mesh     (      render_list_t list, mesh_t  mesh,  material_t material,          matrix world_transform, color128 color_linear, render_layer_ layer);
SK_API void                  render_list_add_model    (      render_list_t list, model_t model,                               matrix world_transform, color128 color_linear, render_layer_ layer);
SK_API void                  render_list_add_model_mat(      render_list_t list, model_t model, material_t material_override, matrix world_transform, color128 color_linear, render_layer_ layer);
SK_API void                  render_list_draw_now     (      render_list_t list, tex_t to_rendertarget, matrix camera, matrix projection, color128 clear_color sk_default({ 0,0,0,0 }), render_clear_ clear sk_default(render_clear_all), rect_t viewport_pct sk_default({}), render_layer_ layer_filter sk_default(render_layer_all));

SK_API void                  render_list_push         (      render_list_t list);
SK_API void                  render_list_pop          (void);

///////////////////////////////////////////

/*When used with a hierarchy modifying function that will push/pop items onto a
  stack, this can be used to change the behavior of how parent hierarchy items
  will affect the item being added to the top of the stack.*/
typedef enum hierarchy_parent_ {
	/*Inheriting is generally the default behavior of a hierarchy stack, the
	  current item will inherit the properties of the parent stack item in some
	  form or another.*/
	hierarchy_parent_inherit,
	/*Ignoring the parent hierarchy stack item will let you skip inheriting
	  anything from the parent item. The new item remains exactly as provided.*/
	hierarchy_parent_ignore,
} hierarchy_parent_;

SK_API void          hierarchy_push              (const sk_ref(matrix) transform, hierarchy_parent_ parent_behavior sk_default(hierarchy_parent_inherit));
SK_API void          hierarchy_pop               (void);
SK_API void          hierarchy_set_enabled       (bool32_t enabled);
SK_API bool32_t      hierarchy_is_enabled        (void);
SK_API const matrix* hierarchy_to_world          (void);
SK_API const matrix* hierarchy_to_local          (void);
SK_API vec3          hierarchy_to_local_point    (const sk_ref(vec3  ) world_pt);
SK_API vec3          hierarchy_to_local_direction(const sk_ref(vec3  ) world_dir);
SK_API quat          hierarchy_to_local_rotation (const sk_ref(quat  ) world_orientation);
SK_API pose_t        hierarchy_to_local_pose     (const sk_ref(pose_t) world_pose);
SK_API ray_t         hierarchy_to_local_ray      (ray_t world_ray);
SK_API vec3          hierarchy_to_world_point    (const sk_ref(vec3  ) local_pt);
SK_API vec3          hierarchy_to_world_direction(const sk_ref(vec3  ) local_dir);
SK_API quat          hierarchy_to_world_rotation (const sk_ref(quat  ) local_orientation);
SK_API pose_t        hierarchy_to_world_pose     (const sk_ref(pose_t) local_pose);
SK_API ray_t         hierarchy_to_world_ray      (ray_t local_ray);

///////////////////////////////////////////

typedef struct sound_inst_t {
	uint16_t _id;
	int16_t  _slot;
} sound_inst_t;

SK_API sound_t      sound_find           (const char *id);
SK_API void         sound_set_id         (sound_t sound, const char *id);
SK_API const char*  sound_get_id         (const sound_t sound);
SK_API sound_t      sound_create         (const char *filename_utf8);
SK_API sound_t      sound_create_stream  (float buffer_duration);
SK_API sound_t      sound_create_samples (const float *in_arr_samples_at_48000s, uint64_t sample_count);
SK_API sound_t      sound_generate       (float (*audio_generator)(float sample_time), float duration);
SK_API void         sound_write_samples  (sound_t sound, const float *in_arr_samples,  uint64_t sample_count);
SK_API uint64_t     sound_read_samples   (sound_t sound, float       *out_arr_samples, uint64_t sample_count);
SK_API uint64_t     sound_unread_samples (sound_t sound);
SK_API uint64_t     sound_total_samples  (sound_t sound);
SK_API uint64_t     sound_cursor_samples (sound_t sound);
SK_API sound_inst_t sound_play           (sound_t sound, vec3 at, float volume);
SK_API float        sound_duration       (sound_t sound);
SK_API void         sound_addref         (sound_t sound);
SK_API void         sound_release        (sound_t sound);

SK_API void         sound_inst_stop      (sound_inst_t sound_inst);
SK_API bool32_t     sound_inst_is_playing(sound_inst_t sound_inst);
SK_API void         sound_inst_set_pos   (sound_inst_t sound_inst, vec3 pos);
SK_API vec3         sound_inst_get_pos   (sound_inst_t sound_inst);
SK_API void         sound_inst_set_volume(sound_inst_t sound_inst, float volume);
SK_API float        sound_inst_get_volume(sound_inst_t sound_inst);

///////////////////////////////////////////

SK_API int32_t      mic_device_count     (void);
SK_API const char*  mic_device_name      (int32_t index);
SK_API bool32_t     mic_start            (const char *device_name sk_default(nullptr));
SK_API void         mic_stop             (void);
SK_API sound_t      mic_get_stream       (void);
SK_API bool32_t     mic_is_recording     (void);

///////////////////////////////////////////

typedef struct file_filter_t {
	char ext[32];
} file_filter_t;

/*When opening the Platform.FilePicker, this enum describes
  how the picker should look and behave.*/
typedef enum picker_mode_ {
	/*Allow opening a single file.*/
	picker_mode_open,
	/*Allow the user to enter or select the name of the
	  destination file.*/
	picker_mode_save,
} picker_mode_;

/*Soft keyboard layouts are often specific to the type of text that they're
  editing! This enum is a collection of common text contexts that SK can pass
  along to the OS's soft keyboard for a more optimal layout.*/
typedef enum text_context_ {
	/*General text editing, this is the most common type of text, and would
	  result in a 'standard' keyboard layout.*/
	text_context_text = 0,
	/*Numbers and numerical values.*/
	text_context_number = 1,
	/*This text specifically represents some kind of URL/URI address.*/
	text_context_uri = 2,
	/*This is a password, and should not be visible when typed!*/
	text_context_password = 3,

} text_context_;
SK_MakeFlag(text_context_);

SK_API void     platform_file_picker        (picker_mode_ mode, void *callback_data, void (*picker_callback   )(void *callback_data, bool32_t confirmed, const char *filename), const file_filter_t *filters, int32_t filter_count);
SK_API void     platform_file_picker_sz     (picker_mode_ mode, void *callback_data, void (*picker_callback_sz)(void *callback_data, bool32_t confirmed, const char *filename_ptr, int32_t filename_length), const file_filter_t *in_arr_filters, int32_t filter_count);
SK_API void     platform_file_picker_close  (void);
SK_API bool32_t platform_file_picker_visible(void);
SK_API bool32_t platform_read_file          (const char *filename_utf8,  void **out_data, size_t *out_size);
SK_API bool32_t platform_write_file         (const char *filename_utf8,  void *data, size_t size);
SK_API bool32_t platform_write_file_text    (const char *filename_utf8,  const char* text_utf8);

SK_API bool32_t platform_keyboard_get_force_fallback(void);
SK_API void     platform_keyboard_set_force_fallback(bool32_t force_fallback);
SK_API void     platform_keyboard_show              (bool32_t visible, text_context_ type);
SK_API bool32_t platform_keyboard_visible           (void);
SK_API bool32_t platform_keyboard_set_layout        (text_context_ type, char** keyboard_layout, int layouts_num);

///////////////////////////////////////////

/*What type of device is the source of the pointer? This is a
  bit-flag that can contain some input source family information.*/
typedef enum input_source_ {
	/*Matches with all input sources!*/
	input_source_any           = 0x7FFFFFFF,
	/*Matches with any hand input source.*/
	input_source_hand          = 1 << 0,
	/*Matches with left hand input sources.*/
	input_source_hand_left     = 1 << 1,
	/*Matches with right hand input sources.*/
	input_source_hand_right    = 1 << 2,
	/*Matches with Gaze category input sources.*/
	input_source_gaze          = 1 << 4,
	/*Matches with the head gaze input source.*/
	input_source_gaze_head     = 1 << 5,
	/*Matches with the eye gaze input source.*/
	input_source_gaze_eyes     = 1 << 6,
	/*Matches with mouse cursor simulated gaze as an input source.*/
	input_source_gaze_cursor   = 1 << 7,
	/*Matches with any input source that has an activation button!*/
	input_source_can_press     = 1 << 8,
} input_source_;
SK_MakeFlag(input_source_);

/*An enum for indicating which hand to use!*/
typedef enum handed_ {
	/*Left hand.*/
	handed_left                = 0,
	/*Right hand.*/
	handed_right               = 1,
	/*The number of hands one generally has, this is much nicer
	  than doing a for loop with '2' as the condition! It's much clearer
	  when you can loop Hand.Max times instead.*/
	handed_max                 = 2,
} handed_;

/*A bit-flag for the current state of a button input.*/
typedef enum button_state_ {
	/*Is the button currently up, unpressed?*/
	button_state_inactive      = 0,
	/*Is the button currently down, pressed?*/
	button_state_active        = 1 << 0,
	/*Has the button just been released? Only true for a single frame.*/
	button_state_just_inactive = 1 << 1,
	/*Has the button just been pressed? Only true for a single frame.*/
	button_state_just_active   = 1 << 2,
	/*Has the button just changed state this frame?*/
	button_state_changed       = button_state_just_inactive | button_state_just_active,
	/*Matches with all states!*/
	button_state_any           = 0x7FFFFFFF,
} button_state_;
SK_MakeFlag(button_state_);

/*This is the tracking state of a sensory input in the world,
  like a controller's position sensor, or a QR code identified by a
  tracking system.*/
typedef enum track_state_ {
	/*The system has no current knowledge about the state of
	  this input. It may be out of visibility, or possibly just
	  disconnected.*/
	track_state_lost           = 0,
	/*The system doesn't know for sure where this is, but it
	  has an educated guess that may be inferred from previous data at
	  a lower quality. For example, a controller may still have
	  accelerometer data after going out of view, which can still be
	  accurate for a short time after losing optical tracking.*/
	track_state_inferred       = 1,
	/*The system actively knows where this input is. Within
	  the constraints of the relevant hardware's capabilities, this is
	  as accurate as it gets!*/
	track_state_known          = 2,
} track_state_;

/*This enum provides information about StereoKit's hand tracking data source.
  It allows you to distinguish between true hand data such as that provided by
  a Leap Motion Controller, and simulated data that StereoKit provides when
  true hand data is not present.*/
typedef enum hand_source_ {
	/*There is currently no source of hand data! This means there are no
	  tracked controllers, or active hand tracking systems. This may happen if
	  the user has hand tracking disabled, and no active controllers.*/
	hand_source_none = 0,
	/*The current hand data is a simulation of hand data rather than true hand
	  data. It is backed by either a controller, or a mouse, and may have a
	  more limited range of motion. */
	hand_source_simulated,
	/*This is true hand data which exhibits the full range of motion of a
	  normal hand. It is backed by something like a Leap Motion Controller, or
	  some other optical (or maybe glove?) hand tracking system.*/
	hand_source_articulated,
	/*This hand data is provided by your use of SK's override functionality.
	  What properties it exhibits depends on what override data you're sending
	  to StereoKit!*/
	hand_source_overridden,
} hand_source_;

typedef struct pointer_t {
	input_source_ source;
	button_state_ tracked;
	button_state_ state;
	ray_t         ray;
	quat          orientation;
} pointer_t;

typedef struct hand_joint_t {
	vec3          position;
	quat          orientation;
	float         radius;
} hand_joint_t;

typedef struct hand_t {
	hand_joint_t  fingers[5][5];
	pose_t        wrist;
	pose_t        palm;
	vec3          pinch_pt;
	handed_       handedness;
	button_state_ tracked_state;
	button_state_ pinch_state;
	button_state_ grip_state;
	float         size;
	float         pinch_activation;
	float         grip_activation;
} hand_t;

typedef struct controller_t {
	pose_t        pose;
	pose_t        palm;
	pose_t        aim;
	button_state_ tracked;
	track_state_  tracked_pos;
	track_state_  tracked_rot;
	button_state_ stick_click;
	button_state_ x1;
	button_state_ x2;
	float         trigger;
	float         grip;
	vec2          stick;
} controller_t;

typedef struct mouse_t {
	bool32_t      available;
	vec2          pos;
	vec2          pos_change;
	float         scroll;
	float         scroll_change;
} mouse_t;

/*A collection of system key codes, representing keyboard
  characters and mouse buttons. Based on VK codes.*/
typedef enum key_ {
	/*Doesn't represent a key, generally means this item has not been set to
	  any particular value!*/
	key_none      = 0x00,
	/*Left mouse button.*/
	key_mouse_left = 0x01,
	/*Right mouse button.*/
	key_mouse_right = 0x02,
	/*Center mouse button.*/
	key_mouse_center = 0x04,
	/*Mouse forward button.*/
	key_mouse_forward = 0x05,
	/*Mouse back button.*/
	key_mouse_back = 0x06,

	/*Backspace*/
	key_backspace = 0x08,
	/*Tab*/
	key_tab       = 0x09,
	/*Return, or Enter.*/
	key_return    = 0x0D,
	/*Left or right Shift.*/
	key_shift     = 0x10,
	/*Left or right Control key.*/
	key_ctrl      = 0x11,
	/*Left or right Alt key.*/
	key_alt       = 0x12,
	/*This behaves a little differently! This tells the toggle
	  state of caps lock, rather than the key state itself.*/
	key_caps_lock = 0x14,
	/*Escape*/
	key_esc       = 0x1B,
	/*Space*/
	key_space     = 0x20,
	/*End*/
	key_end       = 0x23,
	/*Home*/
	key_home      = 0x24,
	/*Left arrow key.*/
	key_left      = 0x25,
	/*Right arrow key.*/
	key_right     = 0x27,
	/*Up arrow key.*/
	key_up        = 0x26,
	/*Down arrow key.*/
	key_down      = 0x28,
	/*Page up*/
	key_page_up   = 0x21,
	/*Page down*/
	key_page_down = 0x22,
	/*Printscreen*/
	key_printscreen=0x2A,
	/*Any Insert key.*/
	key_insert    = 0x2D,
	/*Any Delete key.*/
	key_del       = 0x2E,

	/*Keyboard top row 0, with shift is ')'.*/
	key_0 = 0x30,
	/*Keyboard top row 1, with shift is '!'.*/
	key_1 = 0x31,
	/*Keyboard top row 2, with shift is '@'.*/
	key_2 = 0x32,
	/*Keyboard top row 3, with shift is '#'.*/
	key_3 = 0x33,
	/*Keyboard top row 4, with shift is '$'.*/
	key_4 = 0x34,
	/*Keyboard top row 5, with shift is '%'.*/
	key_5 = 0x35,
	/*Keyboard top row 6, with shift is '^'.*/
	key_6 = 0x36,
	/*Keyboard top row 7, with shift is '&'.*/
	key_7 = 0x37,
	/*Keyboard top row 8, with shift is '*'.*/
	key_8 = 0x38,
	/*Keyboard top row 9, with shift is '('.*/
	key_9 = 0x39,

	/*a/A*/
	key_a = 0x41,
	/*b/B*/
	key_b = 0x42,
	/*c/C*/
	key_c = 0x43,
	/*d/D*/
	key_d = 0x44,
	/*e/E*/
	key_e = 0x45,
	/*f/F*/
	key_f = 0x46,
	/*g/G*/
	key_g = 0x47,
	/*h/H*/
	key_h = 0x48,
	/*i/I*/
	key_i = 0x49,
	/*j/J*/
	key_j = 0x4A,
	/*k/K*/
	key_k = 0x4B,
	/*l/L*/
	key_l = 0x4C,
	/*m/M*/
	key_m = 0x4D,
	/*n/N*/
	key_n = 0x4E,
	/*o/O*/
	key_o = 0x4F,
	/*p/P*/
	key_p = 0x50,
	/*q/Q*/
	key_q = 0x51,
	/*r/R*/
	key_r = 0x52,
	/*s/S*/
	key_s = 0x53,
	/*t/T*/
	key_t = 0x54,
	/*u/U*/
	key_u = 0x55,
	/*v/V*/
	key_v = 0x56,
	/*w/W*/
	key_w = 0x57,
	/*x/X*/
	key_x = 0x58,
	/*y/Y*/
	key_y = 0x59,
	/*z/Z*/
	key_z = 0x5A,

	/*0 on the numpad, when numlock is on.*/
	key_num0 = 0x60,
	/*1 on the numpad, when numlock is on.*/
	key_num1 = 0x61,
	/*2 on the numpad, when numlock is on.*/
	key_num2 = 0x62,
	/*3 on the numpad, when numlock is on.*/
	key_num3 = 0x63,
	/*4 on the numpad, when numlock is on.*/
	key_num4 = 0x64,
	/*5 on the numpad, when numlock is on.*/
	key_num5 = 0x65,
	/*6 on the numpad, when numlock is on.*/
	key_num6 = 0x66,
	/*7 on the numpad, when numlock is on.*/
	key_num7 = 0x67,
	/*8 on the numpad, when numlock is on.*/
	key_num8 = 0x68,
	/*9 on the numpad, when numlock is on.*/
	key_num9 = 0x69,

	/*Function key F1.*/
	key_f1  = 0x70,
	/*Function key F2.*/
	key_f2  = 0x71,
	/*Function key F3.*/
	key_f3  = 0x72,
	/*Function key F4.*/
	key_f4  = 0x73,
	/*Function key F5.*/
	key_f5  = 0x74,
	/*Function key F6.*/
	key_f6  = 0x75,
	/*Function key F7.*/
	key_f7  = 0x76,
	/*Function key F8.*/
	key_f8  = 0x77,
	/*Function key F9.*/
	key_f9  = 0x78,
	/*Function key F10.*/
	key_f10 = 0x79,
	/*Function key F11.*/
	key_f11 = 0x7A,
	/*Function key F12.*/
	key_f12 = 0x7B,

	/*,/&lt;*/
	key_comma = 0xBC,
	/*./&gt;*/
	key_period = 0xBE,
	/*/*/
	key_slash_fwd = 0xBF,
	/*\*/
	key_slash_back = 0xDC,
	/*;/:*/
	key_semicolon = 0xBA,
	/*'/"*/
	key_apostrophe = 0xDE,
	/*[/{*/
	key_bracket_open = 0xDB,
	/*]/}*/
	key_bracket_close = 0xDD,
	/*-/_*/
	key_minus = 0xBD,
	/*=/+*/
	key_equals = 0xBB,
	/*`/~*/
	key_backtick = 0xc0,
	/*The Windows/Mac Command button on the left side of the keyboard.*/
	key_lcmd = 0x5B,
	/*The Windows/Mac Command button on the right side of the keyboard.*/
	key_rcmd = 0x5C,
	/*Numpad '*', NOT the same as number row '*'.*/
	key_multiply = 0x6A,
	/*Numpad '+', NOT the same as number row '+'.*/
	key_add = 0x6B,
	/*Numpad '-', NOT the same as number row '-'.*/
	key_subtract = 0x6D,
	/*Numpad '.', NOT the same as character '.'.*/
	key_decimal = 0x6E,
	/*Numpad '/', NOT the same as character '/'.*/
	key_divide = 0x6F,
	/*Maximum value for key codes.*/
	key_MAX = 0xFF,
} key_;

/*Represents an input from an XR headset's controller!*/
typedef enum controller_key_ {
	/*Doesn't represent a key, generally means this item has not been set to
	  any particular value!*/
	controller_key_none = 0,
	/*The trigger button on the controller, where the user's index finger
	  typically sits.*/
	controller_key_trigger,
	/*The grip button on the controller, usually where the fingers that are not
	  the index finger sit.*/
	controller_key_grip,
	/*This is the lower of the two primary thumb buttons, sometimes labelled X,
	  and sometimes A. */
	controller_key_x1,
	/*This is the upper of the two primary thumb buttons, sometimes labelled Y,
	  and sometimes B. */
	controller_key_x2,
	/*This is when the thumbstick on the controller is actually pressed. This
	  has nothing to do with the horizontal or vertical movement of the stick.*/
	controller_key_stick,
	/*This is the menu, or settings button of the controller.*/
	controller_key_menu,
} controller_key_;

typedef int32_t hand_sim_id_t;

SK_API int32_t               input_pointer_count     (input_source_ filter sk_default(input_source_any));
SK_API pointer_t             input_pointer           (int32_t index, input_source_ filter sk_default(input_source_any));
SK_API const hand_t*         input_hand              (handed_ hand);
SK_API void                  input_hand_override     (handed_ hand, const hand_joint_t *in_arr_hand_joints);
SK_API hand_source_          input_hand_source       (handed_ hand);
SK_API const controller_t*   input_controller        (handed_ hand);
SK_API button_state_         input_controller_menu   (void);
SK_API const pose_t*         input_head              (void);
SK_API const pose_t*         input_eyes              (void);
SK_API button_state_         input_eyes_tracked      (void);
SK_API const mouse_t*        input_mouse             (void);
SK_API button_state_         input_key               (key_ key);
SK_API void                  input_key_inject_press  (key_ key);
SK_API void                  input_key_inject_release(key_ key);
SK_API char32_t              input_text_consume      (void);
SK_API void                  input_text_reset        (void);
SK_API void                  input_text_inject_char  (char32_t character);
SK_API void                  input_hand_visible      (handed_ hand, bool32_t visible);
SK_API void                  input_hand_solid        (handed_ hand, bool32_t solid);
SK_API void                  input_hand_material     (handed_ hand, material_t material);
SK_API bool32_t              input_get_finger_glow   (void);
SK_API void                  input_set_finger_glow   (bool32_t visible);

SK_API hand_sim_id_t         input_hand_sim_pose_add   (const pose_t* in_arr_palm_relative_hand_joints_25, controller_key_ button1, controller_key_ and_button2 sk_default(controller_key_none), key_ or_hotkey1 sk_default(key_none), key_ and_hotkey2 sk_default(key_none));
SK_API void                  input_hand_sim_pose_remove(hand_sim_id_t id);
SK_API void                  input_hand_sim_pose_clear (void);

SK_API void                  input_subscribe      (input_source_ source, button_state_ input_event, void (*input_event_callback)(input_source_ source, button_state_ input_event, const sk_ref(pointer_t) in_pointer));
SK_API void                  input_unsubscribe    (input_source_ source, button_state_ input_event, void (*input_event_callback)(input_source_ source, button_state_ input_event, const sk_ref(pointer_t) in_pointer));
SK_API void                  input_fire_event     (input_source_ source, button_state_ input_event, const sk_ref(pointer_t) pointer);

///////////////////////////////////////////

/*This is a bit flag that describes what an anchoring system is capable of
  doing.*/
typedef enum anchor_caps_ {
	/*This anchor system can store/persist anchors across sessions. Anchors
	  must still be explicitly marked as persistent.*/
	anchor_caps_storable  = 1 << 0,
	/*This anchor system will provide extra accuracy in locating the Anchor, so
	  if the SLAM/6dof tracking drifts over time or distance, the anchor may
	  remain fixed in the correct physical space, instead of drifting with the
	  virtual content.*/
	anchor_caps_stability = 1 << 1,
} anchor_caps_;
SK_MakeFlag(anchor_caps_);

SK_API anchor_t       anchor_find              (const char* asset_id_utf8);
SK_API anchor_t       anchor_create            (pose_t pose);
SK_API void           anchor_set_id            (      anchor_t anchor, const char* asset_id_utf8);
SK_API const char*    anchor_get_id            (const anchor_t anchor);
SK_API void           anchor_addref            (      anchor_t anchor);
SK_API void           anchor_release           (      anchor_t anchor);
SK_API bool32_t       anchor_try_set_persistent(      anchor_t anchor, bool32_t persistent);
SK_API bool32_t       anchor_get_persistent    (const anchor_t anchor);
SK_API pose_t         anchor_get_pose          (const anchor_t anchor);
SK_API bool32_t       anchor_get_changed       (const anchor_t anchor);
SK_API const char*    anchor_get_name          (const anchor_t anchor);
SK_API button_state_  anchor_get_tracked       (const anchor_t anchor);
SK_API bool32_t       anchor_get_perception_anchor(const anchor_t anchor, void** perception_spatial_anchor);

SK_API void           anchor_clear_stored      (void);
SK_API anchor_caps_   anchor_get_capabilities  (void);
SK_API int32_t        anchor_get_count         (void);
SK_API anchor_t       anchor_get_index         (int32_t index);
SK_API int32_t        anchor_get_new_count     (void);
SK_API anchor_t       anchor_get_new_index     (int32_t index);

///////////////////////////////////////////

/*A settings flag that lets you describe the behavior of how
  StereoKit will refresh data about the world mesh, if applicable. This
  is used with `World.RefreshType`.*/
typedef enum world_refresh_ {
	/*Refreshing occurs when the user leaves the area that was
	  most recently scanned. This area is a sphere that is 0.5 of the
	  World.RefreshRadius.*/
	world_refresh_area,
	/*Refreshing happens at timer intervals. If an update
	  doesn't happen in time, the next update will happen as soon as
	  possible. The timer interval is configurable via
	  `World.RefreshInterval`.*/
	world_refresh_timer,
} world_refresh_;

SK_API bool32_t       world_has_bounds                (void);
SK_API vec2           world_get_bounds_size           (void);
SK_API pose_t         world_get_bounds_pose           (void);
SK_API pose_t         world_from_spatial_graph        (uint8_t spatial_graph_node_id[16], bool32_t dynamic, int64_t qpc_time);
SK_API pose_t         world_from_perception_anchor    (void *perception_spatial_anchor);
SK_API bool32_t       world_try_from_spatial_graph    (uint8_t spatial_graph_node_id[16], bool32_t dynamic, int64_t qpc_time, pose_t *out_pose);
SK_API bool32_t       world_try_from_perception_anchor(void *perception_spatial_anchor,   pose_t *out_pose);
SK_API bool32_t       world_raycast                   (ray_t ray, ray_t *out_intersection);
SK_API void           world_set_occlusion_enabled     (bool32_t enabled);
SK_API bool32_t       world_get_occlusion_enabled     (void);
SK_API void           world_set_raycast_enabled       (bool32_t enabled);
SK_API bool32_t       world_get_raycast_enabled       (void);
SK_API void           world_set_occlusion_material    (material_t material);
SK_API material_t     world_get_occlusion_material    (void);
SK_API void           world_set_refresh_type          (world_refresh_ refresh_type);
SK_API world_refresh_ world_get_refresh_type          (void);
SK_API void           world_set_refresh_radius        (float radius_meters);
SK_API float          world_get_refresh_radius        (void);
SK_API void           world_set_refresh_interval      (float every_seconds);
SK_API float          world_get_refresh_interval      (void);
SK_API button_state_  world_get_tracked               (void);
SK_API origin_mode_   world_get_origin_mode           (void);
SK_API pose_t         world_get_origin_offset         (void);
SK_API void           world_set_origin_offset         (pose_t offset);

///////////////////////////////////////////

/*This describes what technology is being used to power StereoKit's
  XR backend.*/
typedef enum backend_xr_type_ {
	/*StereoKit is not using an XR backend of any sort. That means
	  the application is flatscreen and has the simulator disabled.*/
	backend_xr_type_none,
	/*StereoKit is using the flatscreen XR simulator. Inputs are
	  emulated, and some advanced XR functionality may not be
	  available.*/
	backend_xr_type_simulator,
	/*StereoKit is currently powered by OpenXR! This means we're
	  running on a real XR device. Not all OpenXR runtimes provide
	  the same functionality, but we will have access to more fun
	  stuff :)*/
	backend_xr_type_openxr,
	/*StereoKit is running in a browser, and is using WebXR!*/
	backend_xr_type_webxr,
} backend_xr_type_;

/*This describes the platform that StereoKit is running on.*/
typedef enum backend_platform_ {
	/*This is running as a Windows app using the Win32 APIs.*/
	backend_platform_win32,
	/*This is running as a Windows app using the UWP APIs.*/
	backend_platform_uwp,
	/*This is running as a Linux app.*/
	backend_platform_linux,
	/*This is running as an Android app.*/
	backend_platform_android,
	/*This is running in a browser.*/
	backend_platform_web,
} backend_platform_;

/*This describes the graphics API that StereoKit is using for rendering.*/
typedef enum backend_graphics_ {
	/*An invalid default value.*/
	backend_graphics_none,
	/*DirectX's Direct3D11 is used for rendering! This is used by default on
	  Windows.*/
	backend_graphics_d3d11,
	/*OpenGL is used for rendering, using GLX (OpenGL Extension to the X Window
	  System) for loading. This is used by default on Linux.*/
	backend_graphics_opengl_glx,
	/*OpenGL is used for rendering, using WGL (Windows Extensions to OpenGL)
	  for loading. Native developers can configure SK to use this on Windows.*/
	backend_graphics_opengl_wgl,
	/*OpenGL ES is used for rendering, using EGL (EGL Native Platform Graphics
	  Interface) for loading. This is used by default on Android, and native
	  developers can configure SK to use this on Linux.*/
	backend_graphics_opengles_egl,
	/*WebGL is used for rendering. This is used by default on Web.*/
	backend_graphics_webgl,
} backend_graphics_;

typedef uint64_t openxr_handle_t;

SK_API backend_xr_type_  backend_xr_get_type                (void);
SK_API openxr_handle_t   backend_openxr_get_instance        (void);
SK_API openxr_handle_t   backend_openxr_get_session         (void);
SK_API openxr_handle_t   backend_openxr_get_system_id       (void);
SK_API openxr_handle_t   backend_openxr_get_space           (void);
SK_API int64_t           backend_openxr_get_time            (void);
SK_API int64_t           backend_openxr_get_eyes_sample_time(void);
SK_API void*             backend_openxr_get_function        (const char *function_name);
SK_API bool32_t          backend_openxr_ext_enabled         (const char *extension_name);
SK_API void              backend_openxr_ext_request         (const char *extension_name);
SK_API void              backend_openxr_ext_exclude         (const char *extension_name);
SK_API void              backend_openxr_use_minimum_exts    (bool32_t use_minimum_exts);
SK_API void              backend_openxr_composition_layer   (void *XrCompositionLayerBaseHeader, int32_t data_size, int32_t sort_order);
SK_API void              backend_openxr_end_frame_chain     (void *XrBaseHeader, int32_t data_size);
SK_API void              backend_openxr_set_hand_joint_scale(float joint_scale_factor);

SK_API void              backend_openxr_add_callback_pre_session_create(void (*xr_pre_session_create_callback)(void* context), void* context);
SK_API void              backend_openxr_add_callback_poll_event        (void (*xr_poll_event_callback)(void* context, void* XrEventDataBuffer), void* context);
SK_API void              backend_openxr_remove_callback_poll_event     (void (*xr_poll_event_callback)(void* context, void* XrEventDataBuffer));

SK_API backend_platform_ backend_platform_get         (void);
SK_API void*             backend_android_get_java_vm  (void);
SK_API void*             backend_android_get_activity (void);
SK_API void*             backend_android_get_jni_env  (void);

SK_API backend_graphics_ backend_graphics_get                  (void);
SK_API void             *backend_d3d11_get_d3d_device          (void);
SK_API void             *backend_d3d11_get_d3d_context         (void);
SK_API void             *backend_d3d11_get_deferred_d3d_context(void);
SK_API void             *backend_d3d11_get_deferred_mtx        (void);
SK_API uint32_t          backend_d3d11_get_main_thread_id      (void);
SK_API void             *backend_opengl_wgl_get_hdc            (void);
SK_API void             *backend_opengl_wgl_get_hglrc          (void);
SK_API void             *backend_opengl_glx_get_context        (void);
SK_API void             *backend_opengl_glx_get_display        (void);
SK_API void             *backend_opengl_glx_get_drawable       (void);
SK_API void             *backend_opengl_egl_get_context        (void);
SK_API void             *backend_opengl_egl_get_config         (void);
SK_API void             *backend_opengl_egl_get_display        (void);

///////////////////////////////////////////

/*The log tool will write to the console with annotations for console
  colors, which helps with readability, but isn't always supported.
  These are the options available for configuring those colors.*/
typedef enum log_colors_ {
	/*Use console coloring annotations.*/
	log_colors_ansi = 0,
	/*Scrape out any color annotations, so logs are all completely
	  plain text.*/
	log_colors_none
} log_colors_;

SK_API void log_diag       (const char* text);
SK_API void log_diagf      (const char* text, ...);
SK_API void log_info       (const char* text);
SK_API void log_infof      (const char* text, ...);
SK_API void log_warn       (const char* text);
SK_API void log_warnf      (const char* text, ...);
SK_API void log_err        (const char* text);
SK_API void log_errf       (const char* text, ...);
SK_API void log_writef     (log_ level, const char *text, ...);
SK_API void log_write      (log_ level, const char* text);
SK_API void log_set_filter (log_ level);
SK_API void log_set_colors (log_colors_ colors);
// TODO: v0.4, replace these with the _data versions
SK_API void log_subscribe       (void (*log_callback)(log_ level, const char *text));
SK_API void log_unsubscribe     (void (*log_callback)(log_ level, const char *text));
SK_API void log_subscribe_data  (void (*log_callback)(void* context, log_ level, const char *text), void *context);
SK_API void log_unsubscribe_data(void (*log_callback)(void* context, log_ level, const char *text), void *context);

///////////////////////////////////////////

/*A flag for what 'type' an Asset may store.*/
typedef enum asset_type_ {
	/*No type, this may come from some kind of invalid Asset id.*/
	asset_type_none = 0,
	/*A Mesh.*/
	asset_type_mesh,
	/*A Tex.*/
	asset_type_tex,
	/*A Shader.*/
	asset_type_shader,
	/*A Material.*/
	asset_type_material,
	/*A Model.*/
	asset_type_model,
	/*A Font.*/
	asset_type_font,
	/*A Sprite.*/
	asset_type_sprite,
	/*A Sound.*/
	asset_type_sound,
	/*A Solid.*/
	asset_type_solid,
	/*An Anchor.*/
	asset_type_anchor,
	/*A RenderList*/
	asset_type_render_list,
} asset_type_;

typedef void* asset_t;

SK_API void        assets_releaseref_threadsafe(void *asset);
SK_API int32_t     assets_current_task         (void);
SK_API int32_t     assets_total_tasks          (void);
SK_API int32_t     assets_current_task_priority(void);
SK_API void        assets_block_for_priority   (int32_t priority);
SK_API int32_t     assets_count                (void);
SK_API asset_t     assets_get_index            (int32_t index);
SK_API asset_type_ assets_get_type             (int32_t index);

SK_API asset_type_ asset_get_type(asset_t asset);
SK_API void        asset_set_id  (asset_t asset, const char* id);
SK_API const char* asset_get_id  (const asset_t asset);
SK_API void        asset_addref  (asset_t asset);
SK_API void        asset_release (asset_t asset);

///////////////////////////////////////////

SK_CONST char *default_id_material             = "default/material";
SK_CONST char *default_id_material_pbr         = "default/material_pbr";
SK_CONST char *default_id_material_pbr_clip    = "default/material_pbr_clip";
SK_CONST char *default_id_material_unlit       = "default/material_unlit";
SK_CONST char *default_id_material_unlit_clip  = "default/material_unlit_clip";
SK_CONST char *default_id_material_equirect    = "default/equirect_convert";
SK_CONST char *default_id_material_font        = "default/material_font";
SK_CONST char *default_id_material_hand        = "default/material_hand";
SK_CONST char *default_id_material_ui          = "default/material_ui";
SK_CONST char *default_id_material_ui_box      = "default/material_ui_box";
SK_CONST char *default_id_material_ui_quadrant = "default/material_ui_quadrant";
SK_CONST char *default_id_material_ui_aura     = "default/material_ui_aura";
SK_CONST char *default_id_tex                  = "default/tex";
SK_CONST char *default_id_tex_black            = "default/tex_black";
SK_CONST char *default_id_tex_gray             = "default/tex_gray";
SK_CONST char *default_id_tex_flat             = "default/tex_flat";
SK_CONST char *default_id_tex_rough            = "default/tex_rough";
SK_CONST char *default_id_tex_devtex           = "default/tex_devtex";
SK_CONST char *default_id_tex_error            = "default/tex_error";
SK_CONST char *default_id_cubemap              = "default/cubemap";
SK_CONST char *default_id_font                 = "default/font";
SK_CONST char *default_id_mesh_quad            = "default/mesh_quad";
SK_CONST char *default_id_mesh_screen_quad     = "default/mesh_screen_quad";
SK_CONST char *default_id_mesh_cube            = "default/mesh_cube";
SK_CONST char *default_id_mesh_sphere          = "default/mesh_sphere";
SK_CONST char *default_id_mesh_lefthand        = "default/mesh_lefthand";
SK_CONST char *default_id_mesh_righthand       = "default/mesh_righthand";
SK_CONST char *default_id_mesh_ui_button       = "default/mesh_ui_button";
SK_CONST char *default_id_shader               = "default/shader";
SK_CONST char *default_id_shader_blit          = "default/shader_blit";
SK_CONST char *default_id_shader_pbr           = "default/shader_pbr";
SK_CONST char *default_id_shader_pbr_clip      = "default/shader_pbr_clip";
SK_CONST char *default_id_shader_unlit         = "default/shader_unlit";
SK_CONST char* default_id_shader_unlit_clip    = "default/shader_unlit_clip";
SK_CONST char *default_id_shader_lightmap      = "default/shader_lightmap";
SK_CONST char *default_id_shader_font          = "default/shader_font";
SK_CONST char *default_id_shader_equirect      = "default/shader_equirect";
SK_CONST char *default_id_shader_ui            = "default/shader_ui";
SK_CONST char *default_id_shader_ui_box        = "default/shader_ui_box";
SK_CONST char *default_id_shader_ui_quadrant   = "default/shader_ui_quadrant";
SK_CONST char *default_id_shader_ui_aura       = "default/shader_ui_aura";
SK_CONST char *default_id_shader_sky           = "default/shader_sky";
SK_CONST char *default_id_shader_lines         = "default/shader_lines";
SK_CONST char *default_id_sound_click          = "default/sound_click";
SK_CONST char *default_id_sound_unclick        = "default/sound_unclick";
SK_CONST char *default_id_sound_grab           = "default/sound_grab";
SK_CONST char *default_id_sound_ungrab         = "default/sound_ungrab";

#ifdef __cplusplus
} // namespace sk
#endif

///////////////////////////////////////////
// Tools for creating the version information

#define SK_STR2(x) #x
#define SK_STR(x) SK_STR2(x)

  // This will look like 'M.i.P-rcr', or 'M.i.P' if r is 0
#if SK_VERSION_PRERELEASE != 0
#define SK_VERSION SK_STR(SK_VERSION_MAJOR) "." SK_STR(SK_VERSION_MINOR) "." SK_STR(SK_VERSION_PATCH) "-preview." SK_STR(SK_VERSION_PRERELEASE)
#else
#define SK_VERSION SK_STR(SK_VERSION_MAJOR) "." SK_STR(SK_VERSION_MINOR) "." SK_STR(SK_VERSION_PATCH)
#endif

  // A version in hex looks like: 0xMMMMiiiiPPPPrrrr
  // In order of Major.mInor.Patch.pre-Release
#define SK_VERSION_ID ( ((uint64_t)SK_VERSION_MAJOR << 48) | ((uint64_t)SK_VERSION_MINOR << 32) | ((uint64_t)SK_VERSION_PATCH << 16) | (uint64_t)(SK_VERSION_PRERELEASE) )


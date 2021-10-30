#pragma once

#define SK_32BIT_INDICES

#define SK_VERSION_MAJOR 0
#define SK_VERSION_MINOR 3
#define SK_VERSION_PATCH 4
#define SK_VERSION_PRERELEASE 0

#if defined(__GNUC__) || defined(__clang__)
	#define SK_DEPRECATED __attribute__((deprecated))
	#define SK_EXIMPORT
#elif defined(_MSC_VER)
	#define SK_DEPRECATED __declspec(deprecated)
	#if defined(_DLL) || defined(BUILDING_DLL)
		#define SK_EXIMPORT __declspec(dllexport)
	#else
		#define SK_EXIMPORT __declspec(dllimport)
	#endif
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
#include <math.h>

#ifdef __cplusplus
namespace sk {
#endif

typedef int32_t bool32_t;

///////////////////////////////////////////

typedef enum display_mode_ {
	display_mode_mixedreality     = 0,
	display_mode_flatscreen       = 1,
	display_mode_none             = 2,
} display_mode_;

typedef enum depth_mode_ {
	depth_mode_balanced           = 0,
	depth_mode_d16,
	depth_mode_d32,
	depth_mode_stencil,
} depth_mode_;

// TODO: remove this in v0.4
typedef enum display_ {
	display_none                  = 0,
	display_opaque                = 1 << 0,
	display_additive              = 1 << 1,
	display_blend                 = 1 << 2,
	display_passthrough           = 1 << 2,
	display_any_transparent       = display_additive | display_blend,
} display_;

typedef enum display_blend_ {
	display_blend_none            = 0,
	display_blend_opaque          = 1 << 0,
	display_blend_additive        = 1 << 1,
	display_blend_blend           = 1 << 2,
	display_blend_any_transparent = display_blend_additive | display_blend_blend,
} display_blend_;

typedef enum log_ {
	log_none                      = 0,
	log_diagnostic,
	log_inform,
	log_warning,
	log_error
} log_;

typedef enum render_layer_ {
	render_layer_0                = 1 << 0,
	render_layer_1                = 1 << 1,
	render_layer_2                = 1 << 2,
	render_layer_3                = 1 << 3,
	render_layer_4                = 1 << 4,
	render_layer_5                = 1 << 5,
	render_layer_6                = 1 << 6,
	render_layer_7                = 1 << 7,
	render_layer_8                = 1 << 8,
	render_layer_9                = 1 << 9,
	render_layer_vfx              = 1 << 10,
	render_layer_all              = 0xFFFF,
	render_layer_all_regular      = render_layer_0 | render_layer_1 | render_layer_2 | render_layer_3 | render_layer_4 | render_layer_5 | render_layer_6 | render_layer_7 | render_layer_8 | render_layer_9,
} render_layer_;
SK_MakeFlag(render_layer_);

typedef struct sk_settings_t {
	const char    *app_name;
	const char    *assets_folder;
	display_mode_  display_preference;
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

SK_API bool32_t      sk_init               (sk_settings_t settings);
SK_API void          sk_set_window         (void *window);
SK_API void          sk_set_window_xam     (void *window);
SK_API void          sk_shutdown           ();
SK_API void          sk_quit               ();
SK_API bool32_t      sk_step               (void (*app_update)(void));
SK_API display_mode_ sk_active_display_mode();
SK_API sk_settings_t sk_get_settings       ();
SK_API system_info_t sk_system_info        ();
SK_API const char   *sk_version_name       ();
SK_API uint64_t      sk_version_id         ();

///////////////////////////////////////////

SK_API float         time_getf_unscaled    ();
SK_API double        time_get_unscaled     ();
SK_API float         time_getf             ();
SK_API double        time_get              ();
SK_API float         time_elapsedf_unscaled();
SK_API double        time_elapsed_unscaled ();
SK_API float         time_elapsedf         ();
SK_API double        time_elapsed          ();
SK_API void          time_scale            (double scale);
SK_API void          time_set_time         (double total_seconds, double frame_elapsed_seconds sk_default(0));

///////////////////////////////////////////

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
SK_API vec3     matrix_to_angles          (const sk_ref(matrix) transform);
SK_API matrix   matrix_trs                (const sk_ref(vec3) position, const sk_ref(quat) orientation sk_default({0,0,0,1}), const sk_ref(vec3) scale sk_default({1,1,1}));
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
static inline vec2   operator/ (vec2 a, float b) { return { a.x / b, a.y / b }; }
static inline vec2   operator+ (vec2 a, vec2  b) { return { a.x + b.x, a.y + b.y }; }
static inline vec2   operator- (vec2 a, vec2  b) { return { a.x - b.x, a.y - b.y }; }
static inline vec2   operator* (vec2 a, vec2  b) { return { a.x * b.x, a.y * b.y }; }
static inline vec2   operator/ (vec2 a, vec2  b) { return { a.x / b.x, a.y / b.y }; }
static inline vec2  &operator+=(vec2 &a, vec2  b) { a.x += b.x; a.y += b.y; return a; }
static inline vec2  &operator-=(vec2 &a, vec2  b) { a.x -= b.x; a.y -= b.y; return a; }
static inline vec2  &operator*=(vec2 &a, float b) { a.x *= b; a.y *= b; return a; }
static inline vec2  &operator/=(vec2 &a, float b) { a.x /= b; a.y /= b; return a; }

static inline vec3   operator* (vec3 a, float b) { return { a.x * b, a.y * b, a.z * b }; }
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

static inline vec3   operator* (quat a, vec3 b) { return quat_mul_vec(a, b); }
static inline quat   operator* (quat a, quat b) { return quat_mul(a, b); }
static inline matrix operator* (matrix a, matrix b) { matrix result; matrix_mul(a, b, result); return result; }
static inline vec3   operator* (matrix a, vec3 b) { return matrix_transform_pt(a, b); }
static inline vec4   operator* (matrix a, vec4 b) { return matrix_transform_pt4(a, b); }
#endif

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
static inline vec3     vec3_normalize   (vec3 a) { float imag = 1.0f/vec3_magnitude(a); vec3 v = {a.x*imag, a.y*imag, a.z*imag}; return v; }
static inline vec2     vec2_normalize   (vec2 a) { float imag = 1.0f/vec2_magnitude(a); vec2 v = {a.x*imag, a.y*imag}; return v; }
static inline vec3     vec3_abs         (vec3 a) { vec3 v = { fabsf(a.x), fabsf(a.y), fabsf(a.z) }; return v; }
static inline vec2     vec2_abs         (vec2 a) { vec2 v = { fabsf(a.x), fabsf(a.y) }; return v; }
static inline vec3     vec3_lerp        (vec3 a, vec3 b, float t) { vec3 v = { a.x + (b.x - a.x)*t, a.y + (b.y - a.y)*t, a.z + (b.z - a.z)*t }; return v; }
static inline vec2     vec2_lerp        (vec2 a, vec2 b, float t) { vec2 v = { a.x + (b.x - a.x)*t, a.y + (b.y - a.y)*t }; return v; }
static inline bool32_t vec3_in_radius   (vec3 pt, vec3 center, float radius) { return vec3_distance_sq(center, pt) < radius*radius; }
static inline bool32_t vec2_in_radius   (vec2 pt, vec2 center, float radius) { return vec2_distance_sq(center, pt) < radius*radius; }

static const float  deg2rad         = 0.01745329252f;
static const float  rad2deg         = 57.295779513f;
static const float  cm2m            = 0.01f;
static const float  mm2m            = 0.001f;
static const float  m2cm            = 100.f;
static const float  m2mm            = 1000.f;

static const vec2   vec2_zero       = { 0,0 };
static const vec2   vec2_one        = { 1,1 };
static const vec3   vec3_one        = { 1,1, 1 };
static const vec3   vec3_zero       = { 0,0, 0 };
static const vec3   vec3_up         = { 0,1, 0 };
static const vec3   vec3_forward    = { 0,0,-1 };
static const vec3   vec3_right      = { 1,0, 0 };
static const quat   quat_identity   = { 0,0, 0,1 };
static const matrix matrix_identity = { { {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1} } };
static const pose_t pose_identity   = { {0,0,0}, {0,0,0,1} };

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
static inline color128  operator*(const color128 &a, const float b) { return { a.r * b, a.g * b, a.b * b, a.a * b }; }
#endif

///////////////////////////////////////////

SK_DeclarePrivateType(gradient_t);
SK_DeclarePrivateType(mesh_t);
SK_DeclarePrivateType(tex_t);
SK_DeclarePrivateType(font_t);
SK_DeclarePrivateType(shader_t);
SK_DeclarePrivateType(material_t);
SK_DeclarePrivateType(model_t);
SK_DeclarePrivateType(sprite_t);
SK_DeclarePrivateType(sound_t);

///////////////////////////////////////////

typedef struct gradient_key_t {
	color128 color;
	float    position;
} gradient_key_t;

SK_API gradient_t gradient_create     ();
SK_API gradient_t gradient_create_keys(const gradient_key_t *keys, int32_t count);
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

SK_API spherical_harmonics_t sh_create      (const sh_light_t* lights, int32_t light_count);
SK_API void                  sh_brightness  (      sk_ref(spherical_harmonics_t) harmonics, float scale);
SK_API void                  sh_add         (      sk_ref(spherical_harmonics_t) harmonics, vec3 light_dir, vec3 light_color);
SK_API color128              sh_lookup      (const sk_ref(spherical_harmonics_t) harmonics, vec3 normal);
SK_API vec3                  sh_dominant_dir(const sk_ref(spherical_harmonics_t) harmonics);

///////////////////////////////////////////

typedef struct vert_t {
	vec3    pos;
	vec3    norm;
	vec2    uv;
	color32 col;
} vert_t;

#ifdef SK_32BIT_INDICES
typedef uint32_t vind_t;
#else
typedef uint16_t vind_t;
#endif

SK_API mesh_t   mesh_find            (const char *name);
SK_API mesh_t   mesh_create          ();
SK_API mesh_t   mesh_copy            (mesh_t mesh);
SK_API void     mesh_set_id          (mesh_t mesh, const char *id);
SK_API void     mesh_addref          (mesh_t mesh);
SK_API void     mesh_release         (mesh_t mesh);
SK_API void     mesh_draw            (mesh_t mesh, material_t material, matrix transform, color128 color_linear sk_default({1,1,1,1}), render_layer_ layer sk_default(render_layer_0));
SK_API void     mesh_set_keep_data   (mesh_t mesh, bool32_t keep_data);
SK_API bool32_t mesh_get_keep_data   (mesh_t mesh);
SK_API void     mesh_set_verts       (mesh_t mesh, const vert_t *vertices, int32_t vertex_count, bool32_t calculate_bounds sk_default(true));
SK_API void     mesh_get_verts       (mesh_t mesh, sk_ref_arr(vert_t) out_vertices, sk_ref(int32_t) out_vertex_count);
SK_API void     mesh_set_inds        (mesh_t mesh, const vind_t *indices, int32_t index_count);
SK_API void     mesh_get_inds        (mesh_t mesh, sk_ref_arr(vind_t) out_indices,  sk_ref(int32_t) out_index_count);
SK_API void     mesh_set_draw_inds   (mesh_t mesh, int32_t index_count);
SK_API void     mesh_set_bounds      (mesh_t mesh, const sk_ref(bounds_t) bounds);
SK_API bounds_t mesh_get_bounds      (mesh_t mesh);
SK_API bool32_t mesh_has_skin        (mesh_t mesh);
SK_API void     mesh_set_skin        (mesh_t mesh, const uint16_t *bone_ids_4, int32_t bone_id_4_count, const vec4 *bone_weights, int32_t bone_weight_count, const matrix *bone_resting_transforms, int32_t bone_count);
SK_API void     mesh_update_skin     (mesh_t mesh, const matrix *bone_transforms, int32_t bone_count);
SK_API bool32_t mesh_ray_intersect   (mesh_t mesh, ray_t model_space_ray, ray_t *out_pt);

SK_API mesh_t   mesh_gen_plane       (vec2 dimensions, vec3 plane_normal, vec3 plane_top_direction, int32_t subdivisions sk_default(0));
SK_API mesh_t   mesh_gen_cube        (vec3 dimensions, int32_t subdivisions sk_default(0));
SK_API mesh_t   mesh_gen_sphere      (float diameter,  int32_t subdivisions sk_default(4));
SK_API mesh_t   mesh_gen_rounded_cube(vec3 dimensions, float edge_radius, int32_t subdivisions);
SK_API mesh_t   mesh_gen_cylinder    (float diameter,  float depth, vec3 direction, int32_t subdivisions sk_default(16));

///////////////////////////////////////////

typedef enum tex_type_ {
	tex_type_image_nomips  = 1 << 0,
	tex_type_cubemap       = 1 << 1,
	tex_type_rendertarget  = 1 << 2,
	tex_type_depth         = 1 << 3,
	tex_type_mips          = 1 << 4,
	tex_type_dynamic       = 1 << 5,
	tex_type_image         = tex_type_image_nomips | tex_type_mips,
} tex_type_;
SK_MakeFlag(tex_type_);

typedef enum tex_format_ {
	tex_format_none = 0,
	tex_format_rgba32,
	tex_format_rgba32_linear,
	tex_format_bgra32,
	tex_format_bgra32_linear,
	tex_format_rg11b10,
	tex_format_rgb10a2,
	tex_format_rgba64, // TODO: remove during major version update
	tex_format_rgba64s,
	tex_format_rgba64f,
	tex_format_rgba128,
	tex_format_r8,
	tex_format_r16,
	tex_format_r32,
	tex_format_depthstencil,
	tex_format_depth32,
	tex_format_depth16,

	tex_format_rgba64u = tex_format_rgba64,
} tex_format_;

typedef enum tex_sample_ {
	tex_sample_linear = 0,
	tex_sample_point,
	tex_sample_anisotropic
} tex_sample_;

typedef enum tex_address_ {
	tex_address_wrap = 0,
	tex_address_clamp,
	tex_address_mirror,
} tex_address_;

SK_API tex_t        tex_find                (const char *id);
SK_API tex_t        tex_create              (tex_type_ type sk_default(tex_type_image), tex_format_ format sk_default(tex_format_rgba32));
SK_API tex_t        tex_create_mem          (void *file_data, size_t file_size, bool32_t srgb_data sk_default(true));
SK_API tex_t        tex_create_color32      (color32  *data, int32_t width, int32_t height, bool32_t srgb_data sk_default(true));
SK_API tex_t        tex_create_color128     (color128 *data, int32_t width, int32_t height, bool32_t srgb_data sk_default(true));
SK_API tex_t        tex_create_file         (const char *file,                       bool32_t srgb_data sk_default(true));
SK_API tex_t        tex_create_file_arr     (const char **files, int32_t file_count, bool32_t srgb_data sk_default(true));
SK_API tex_t        tex_create_cubemap_file (const char *equirectangular_file,       bool32_t srgb_data sk_default(true), spherical_harmonics_t *out_sh_lighting_info sk_default(nullptr));
SK_API tex_t        tex_create_cubemap_files(const char **cube_face_file_xxyyzz,     bool32_t srgb_data sk_default(true), spherical_harmonics_t *out_sh_lighting_info sk_default(nullptr));
SK_API void         tex_set_id              (tex_t texture, const char *id);
SK_API void         tex_set_surface         (tex_t texture, void *native_surface, tex_type_ type, int64_t native_fmt, int32_t width, int32_t height, int32_t surface_count);
SK_API void         tex_addref              (tex_t texture);
SK_API void         tex_release             (tex_t texture);
SK_API void         tex_set_colors          (tex_t texture, int32_t width, int32_t height, void *data);
SK_API void         tex_set_color_arr       (tex_t texture, int32_t width, int32_t height, void** data, int32_t data_count, spherical_harmonics_t *out_sh_lighting_info sk_default(nullptr), int32_t multisample sk_default(1));
SK_API tex_t        tex_add_zbuffer         (tex_t texture, tex_format_ format sk_default(tex_format_depthstencil));
SK_API void         tex_get_data            (tex_t texture, void *out_data, size_t out_data_size);
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

///////////////////////////////////////////

SK_API font_t       font_find               (const char *id);
SK_API font_t       font_create             (const char *file);
SK_API font_t       font_create_files       (const char **files, int32_t file_count);
SK_API void         font_set_id             (font_t font, const char* id);
SK_API void         font_addref             (font_t font);
SK_API void         font_release            (font_t font);
SK_API tex_t        font_get_tex            (font_t font);

///////////////////////////////////////////

SK_API shader_t     shader_find             (const char *id);
SK_API shader_t     shader_create_file      (const char *filename);
SK_API shader_t     shader_create_mem       (void *data, size_t data_size);
SK_API void         shader_set_id           (shader_t shader, const char *id);
SK_API const char  *shader_get_name         (shader_t shader);
SK_API void         shader_addref           (shader_t shader);
SK_API void         shader_release          (shader_t shader);

///////////////////////////////////////////

typedef enum transparency_ {
	transparency_none = 1,
	transparency_blend,
	transparency_add,
} transparency_;

typedef enum cull_ {
	cull_back = 0,
	cull_front,
	cull_none,
} cull_;

typedef enum depth_test_ {
	depth_test_less = 0,
	depth_test_less_or_eq,
	depth_test_greater,
	depth_test_greater_or_eq,
	depth_test_equal,
	depth_test_not_equal,
	depth_test_always,
	depth_test_never,
} depth_test_;

typedef enum material_param_ {
	material_param_float = 0,
	material_param_color128,
	material_param_vector,
	material_param_matrix,
	material_param_texture,
} material_param_;

SK_DeclarePrivateType(material_buffer_t);

SK_API material_t        material_find            (const char *id);
SK_API material_t        material_create          (shader_t shader);
SK_API material_t        material_copy            (material_t material);
SK_API material_t        material_copy_id         (const char *id);
SK_API void              material_set_id          (material_t material, const char *id);
SK_API void              material_addref          (material_t material);
SK_API void              material_release         (material_t material);
SK_API void              material_set_transparency(material_t material, transparency_ mode);
SK_API void              material_set_cull        (material_t material, cull_ mode);
SK_API void              material_set_wireframe   (material_t material, bool32_t wireframe);
SK_API void              material_set_depth_test  (material_t material, depth_test_ depth_test_mode);
SK_API void              material_set_depth_write (material_t material, bool32_t write_enabled);
SK_API void              material_set_queue_offset(material_t material, int32_t offset);
SK_API transparency_     material_get_transparency(material_t material);
SK_API cull_             material_get_cull        (material_t material);
SK_API bool32_t          material_get_wireframe   (material_t material);
SK_API depth_test_       material_get_depth_test  (material_t material);
SK_API bool32_t          material_get_depth_write (material_t material);
SK_API int32_t           material_get_queue_offset(material_t material);
SK_API void              material_set_float       (material_t material, const char *name, float    value);
SK_API void              material_set_color       (material_t material, const char *name, color128 color_gamma);
SK_API void              material_set_vector4     (material_t material, const char *name, vec4     value);
SK_API void              material_set_vector3     (material_t material, const char *name, vec3     value);
SK_API void              material_set_vector2     (material_t material, const char *name, vec2     value);
SK_API void              material_set_vector      (material_t material, const char *name, vec4     value); // TODO: Remove in v0.4
SK_API void              material_set_matrix      (material_t material, const char *name, matrix   value);
SK_API bool32_t          material_set_texture     (material_t material, const char *name, tex_t    value);
SK_API bool32_t          material_set_texture_id  (material_t material, uint64_t    id,   tex_t    value);
SK_API bool32_t          material_has_param       (material_t material, const char *name, material_param_ type);
SK_API void              material_set_param       (material_t material, const char *name, material_param_ type, const void *value);
SK_API void              material_set_param_id    (material_t material, uint64_t    id,   material_param_ type, const void *value);
SK_API bool32_t          material_get_param       (material_t material, const char *name, material_param_ type, void *out_value);
SK_API bool32_t          material_get_param_id    (material_t material, uint64_t    id,   material_param_ type, void *out_value);
SK_API void              material_get_param_info  (material_t material, int32_t index, char **out_name, material_param_ *out_type);
SK_API int               material_get_param_count (material_t material);
SK_API void              material_set_shader      (material_t material, shader_t shader);
SK_API shader_t          material_get_shader      (material_t material);

SK_API material_buffer_t material_buffer_create   (int32_t register_slot, int32_t size);
SK_API void              material_buffer_set_data (material_buffer_t buffer, const void *data);
SK_API void              material_buffer_release  (material_buffer_t buffer);

///////////////////////////////////////////

typedef enum text_fit_ {
	text_fit_wrap           = 1 << 0,
	text_fit_clip           = 1 << 1,
	text_fit_squeeze        = 1 << 2,
	text_fit_exact          = 1 << 3,
	text_fit_overflow       = 1 << 4
} text_fit_;

typedef enum text_align_ {
	text_align_x_left       = 1 << 0,
	text_align_y_top        = 1 << 1,
	text_align_x_center     = 1 << 2,
	text_align_y_center     = 1 << 3,
	text_align_x_right      = 1 << 4,
	text_align_y_bottom     = 1 << 5,
	text_align_center       = text_align_x_center | text_align_y_center,
	text_align_center_left  = text_align_x_left   | text_align_y_center,
	text_align_center_right = text_align_x_right  | text_align_y_center,
	text_align_top_center   = text_align_x_center | text_align_y_top,
	text_align_top_left     = text_align_x_left   | text_align_y_top,
	text_align_top_right    = text_align_x_right  | text_align_y_top,
	text_align_bottom_center= text_align_x_center | text_align_y_bottom,
	text_align_bottom_left  = text_align_x_left   | text_align_y_bottom,
	text_align_bottom_right = text_align_x_right  | text_align_y_bottom,
} text_align_;
SK_MakeFlag(text_align_);

typedef uint32_t text_style_t;

SK_API text_style_t  text_make_style               (font_t font, float character_height,                      color128 color_gamma);
SK_API text_style_t  text_make_style_shader        (font_t font, float character_height, shader_t shader,     color128 color_gamma);
SK_API text_style_t  text_make_style_mat           (font_t font, float character_height, material_t material, color128 color_gamma);
SK_API void          text_add_at                   (const char     *text_utf8,  const sk_ref(matrix)  transform, text_style_t style sk_default(0), text_align_ position sk_default(text_align_center), text_align_ align sk_default(text_align_center), float off_x sk_default(0), float off_y sk_default(0), float off_z sk_default(0));
SK_API void          text_add_at_16                (const char16_t *text_utf16, const sk_ref(matrix)  transform, text_style_t style sk_default(0), text_align_ position sk_default(text_align_center), text_align_ align sk_default(text_align_center), float off_x sk_default(0), float off_y sk_default(0), float off_z sk_default(0));
SK_API float         text_add_in                   (const char     *text_utf8,  const sk_ref(matrix)  transform, vec2 size, text_fit_ fit, text_style_t style sk_default(0), text_align_ position sk_default(text_align_center), text_align_ align sk_default(text_align_center), float off_x sk_default(0), float off_y sk_default(0), float off_z sk_default(0));
SK_API float         text_add_in_16                (const char16_t *text_utf16, const sk_ref(matrix)  transform, vec2 size, text_fit_ fit, text_style_t style sk_default(0), text_align_ position sk_default(text_align_center), text_align_ align sk_default(text_align_center), float off_x sk_default(0), float off_y sk_default(0), float off_z sk_default(0));
SK_API vec2          text_size                     (const char     *text_utf8,  text_style_t style sk_default(0));
SK_API vec2          text_size_16                  (const char16_t *text_utf16, text_style_t style sk_default(0));

SK_API material_t    text_style_get_material       (text_style_t style);
SK_API float         text_style_get_char_height    (text_style_t style);

///////////////////////////////////////////

typedef enum solid_type_ {
	solid_type_normal = 0,
	solid_type_immovable,
	solid_type_unaffected,
} solid_type_;

typedef void* solid_t;

SK_API solid_t       solid_create                  (const sk_ref(vec3) position, const sk_ref(quat) rotation, solid_type_ type sk_default(solid_type_normal));
SK_API void          solid_release                 (solid_t solid);
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

typedef enum anim_mode_ {
	anim_mode_loop,
	anim_mode_once,
	anim_mode_manual,
} anim_mode_;

SK_API model_t       model_find                    (const char *id);
SK_API model_t       model_copy                    (model_t model);
SK_API model_t       model_create                  ();
SK_API model_t       model_create_mesh             (mesh_t mesh, material_t material);
SK_API model_t       model_create_mem              (const char *filename, void *data, size_t data_size, shader_t shader sk_default(nullptr));
SK_API model_t       model_create_file             (const char *filename, shader_t shader sk_default(nullptr));
SK_API void          model_set_id                  (model_t model, const char *id);
SK_API void          model_addref                  (model_t model);
SK_API void          model_release                 (model_t model);
SK_API void          model_draw                    (model_t model, matrix transform, color128 color_linear sk_default({1,1,1,1}), render_layer_ layer sk_default(render_layer_0));
SK_API void          model_recalculate_bounds      (model_t model);
SK_API void          model_set_bounds              (model_t model, const sk_ref(bounds_t) bounds);
SK_API bounds_t      model_get_bounds              (model_t model);
SK_API bool32_t      model_ray_intersect           (model_t model, ray_t model_space_ray, ray_t *out_pt);

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
SK_API const char   *model_anim_get_name           (model_t model, int32_t index);
SK_API float         model_anim_get_duration       (model_t model, int32_t index);

// TODO: this whole section gets removed in v0.4, prefer the model_node api
SK_API const char   *model_get_name                (model_t model, int32_t subset);
SK_API material_t    model_get_material            (model_t model, int32_t subset);
SK_API mesh_t        model_get_mesh                (model_t model, int32_t subset);
SK_API matrix        model_get_transform           (model_t model, int32_t subset);
SK_API void          model_set_material            (model_t model, int32_t subset, material_t material);
SK_API void          model_set_mesh                (model_t model, int32_t subset, mesh_t mesh);
SK_API void          model_set_transform           (model_t model, int32_t subset, const sk_ref(matrix) transform);
SK_API void          model_remove_subset           (model_t model, int32_t subset);
SK_API int32_t       model_add_named_subset        (model_t model, const char *name, mesh_t mesh, material_t material, const sk_ref(matrix) transform);
SK_API int32_t       model_add_subset              (model_t model, mesh_t mesh, material_t material, const sk_ref(matrix) transform);
SK_API int32_t       model_add_subset_n            (model_t model, const char *name, mesh_t mesh, material_t material, const sk_ref(matrix) transform);
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
SK_API material_t    model_node_get_material       (model_t model, model_node_id node);
SK_API mesh_t        model_node_get_mesh           (model_t model, model_node_id node);
SK_API matrix        model_node_get_transform_model(model_t model, model_node_id node);
SK_API matrix        model_node_get_transform_local(model_t model, model_node_id node);
SK_API void          model_node_set_name           (model_t model, model_node_id node, const char* name);
SK_API void          model_node_set_solid          (model_t model, model_node_id node, bool32_t    solid);
SK_API void          model_node_set_material       (model_t model, model_node_id node, material_t  material);
SK_API void          model_node_set_mesh           (model_t model, model_node_id node, mesh_t      mesh);
SK_API void          model_node_set_transform_model(model_t model, model_node_id node, matrix      transform_model_space);
SK_API void          model_node_set_transform_local(model_t model, model_node_id node, matrix      transform_local_space);

///////////////////////////////////////////

typedef enum sprite_type_ {
	sprite_type_atlased = 0,
	sprite_type_single
} sprite_type_;

SK_API sprite_t sprite_create     (tex_t    sprite,      sprite_type_ type sk_default(sprite_type_atlased), const char *atlas_id sk_default("default"));
SK_API sprite_t sprite_create_file(const char *filename, sprite_type_ type sk_default(sprite_type_atlased), const char *atlas_id sk_default("default"));
SK_API void     sprite_set_id     (sprite_t sprite, const char *id);
SK_API void     sprite_addref     (sprite_t sprite);
SK_API void     sprite_release    (sprite_t sprite);
SK_API float    sprite_get_aspect (sprite_t sprite);
SK_API int32_t  sprite_get_width  (sprite_t sprite);
SK_API int32_t  sprite_get_height (sprite_t sprite);
SK_API vec2     sprite_get_dimensions_normalized(sprite_t sprite);
SK_API void     sprite_draw       (sprite_t sprite, const sk_ref(matrix) transform, color32 color sk_default({255,255,255,255}));

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
SK_API void line_add_listv(const line_point_t *points, int32_t count);

///////////////////////////////////////////

typedef enum render_clear_ {
	render_clear_none  = 0,
	render_clear_color = 1 << 0,
	render_clear_depth = 1 << 1,
	render_clear_all   = render_clear_color | render_clear_depth,
} render_clear_;

SK_API void                  render_set_clip       (float near_plane sk_default(0.08f), float far_plane sk_default(50));
SK_API void                  render_set_fov        (float field_of_view_degrees sk_default(90.0f));
SK_API matrix                render_get_cam_root   ();
SK_API void                  render_set_cam_root   (const sk_ref(matrix) cam_root);
SK_API void                  render_set_skytex     (tex_t sky_texture);
SK_API tex_t                 render_get_skytex     ();
SK_API void                  render_set_skylight   (const sk_ref(spherical_harmonics_t) light_info);
SK_API spherical_harmonics_t render_get_skylight   ();
SK_API void                  render_set_filter     (render_layer_ layer_filter);
SK_API render_layer_         render_get_filter     ();
SK_API void                  render_override_capture_filter(bool32_t use_override_filter, render_layer_ layer_filter sk_default(render_layer_all));
SK_API render_layer_         render_get_capture_filter     ();
SK_API bool32_t              render_has_capture_filter     ();
SK_API void                  render_set_clear_color(color128 color_gamma);
SK_API void                  render_enable_skytex  (bool32_t show_sky);
SK_API bool32_t              render_enabled_skytex ();
SK_API void                  render_add_mesh       (mesh_t mesh, material_t material, const sk_ref(matrix) transform, color128 color_linear sk_default({1,1,1,1}), render_layer_ layer sk_default(render_layer_0));
SK_API void                  render_add_model      (model_t model, const sk_ref(matrix) transform, color128 color_linear sk_default({1,1,1,1}), render_layer_ layer sk_default(render_layer_0));
SK_API void                  render_blit           (tex_t to_rendertarget, material_t material);
SK_API void                  render_screenshot     (const char *file, vec3 from_viewpt, vec3 at, int width, int height, float field_of_view_degrees);
SK_API void                  render_to             (tex_t to_rendertarget, const sk_ref(matrix) camera, const sk_ref(matrix) projection, render_layer_ layer_filter sk_default(render_layer_all), render_clear_ clear sk_default(render_clear_all), rect_t viewport sk_default({}));
SK_API void                  render_get_device     (void **device, void **context);

///////////////////////////////////////////

SK_API void          hierarchy_push              (const sk_ref(matrix) transform);
SK_API void          hierarchy_pop               ();
SK_API void          hierarchy_set_enabled       (bool32_t enabled);
SK_API bool32_t      hierarchy_is_enabled        ();
SK_API const matrix *hierarchy_to_world          ();
SK_API const matrix *hierarchy_to_local          ();
SK_API vec3          hierarchy_to_local_point    (const sk_ref(vec3  ) world_pt);
SK_API vec3          hierarchy_to_local_direction(const sk_ref(vec3  ) world_dir);
SK_API quat          hierarchy_to_local_rotation (const sk_ref(quat  ) world_orientation);
SK_API pose_t        hierarchy_to_local_pose     (const sk_ref(pose_t) world_pose);
SK_API vec3          hierarchy_to_world_point    (const sk_ref(vec3  ) local_pt);
SK_API vec3          hierarchy_to_world_direction(const sk_ref(vec3  ) local_dir);
SK_API quat          hierarchy_to_world_rotation (const sk_ref(quat  ) local_orientation);
SK_API pose_t        hierarchy_to_world_pose     (const sk_ref(pose_t) local_pose);

///////////////////////////////////////////

typedef struct sound_inst_t {
	uint16_t _id;
	int16_t  _slot;
} sound_inst_t;

SK_API sound_t      sound_find           (const char *id);
SK_API void         sound_set_id         (sound_t sound, const char *id);
SK_API sound_t      sound_create         (const char *filename);
SK_API sound_t      sound_create_stream  (float buffer_duration);
SK_API sound_t      sound_generate       (float (*function)(float sample_time), float duration);
SK_API void         sound_write_samples  (sound_t sound, const float *samples,     uint64_t sample_count);
SK_API uint64_t     sound_read_samples   (sound_t sound, float       *out_samples, uint64_t sample_count);
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

SK_API int32_t      mic_device_count     ();
SK_API const char  *mic_device_name      (int32_t index);
SK_API bool32_t     mic_start            (const char *device_name sk_default(nullptr));
SK_API void         mic_stop             ();
SK_API sound_t      mic_get_stream       ();
SK_API bool32_t     mic_is_recording     ();

///////////////////////////////////////////

typedef struct file_filter_t {
	char ext[32];
} file_filter_t;

typedef enum picker_mode_ {
	picker_mode_open,
	picker_mode_save,
} picker_mode_;

SK_API void     platform_file_picker        (picker_mode_ mode, void *callback_data, void (*on_confirm)(void *callback_data, bool32_t confirmed, const char *filename), const file_filter_t *filters, int32_t filter_count);
SK_API void     platform_file_picker_close  ();
SK_API bool32_t platform_file_picker_visible();
SK_API bool32_t platform_read_file          (const char *filename, void **out_data, size_t *out_size);
SK_API bool32_t platform_write_file         (const char *filename, void *data, size_t size);

///////////////////////////////////////////

typedef enum input_source_ {
	input_source_any           = 0x7FFFFFFF,
	input_source_hand          = 1 << 0,
	input_source_hand_left     = 1 << 1,
	input_source_hand_right    = 1 << 2,
	input_source_gaze          = 1 << 4,
	input_source_gaze_head     = 1 << 5,
	input_source_gaze_eyes     = 1 << 6,
	input_source_gaze_cursor   = 1 << 7,
	input_source_can_press     = 1 << 8,
} input_source_;
SK_MakeFlag(input_source_);

typedef enum handed_ {
	handed_left                = 0,
	handed_right               = 1,
	handed_max                 = 2,
} handed_;

typedef enum button_state_ {
	button_state_inactive      = 0,
	button_state_active        = 1 << 0,
	button_state_just_inactive = 1 << 1,
	button_state_just_active   = 1 << 2,
	button_state_changed       = button_state_just_inactive | button_state_just_active,
} button_state_;
SK_MakeFlag(button_state_);

typedef enum track_state_ {
	track_state_lost           = 0,
	track_state_inferred       = 1,
	track_state_known          = 2,
} track_state_;

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

// Based on VK codes
typedef enum key_ {
	key_none      = 0x00,
	key_mouse_left= 0x01, key_mouse_right = 0x02, key_mouse_center = 0x04, key_mouse_forward = 0x05, key_mouse_back = 0x06,
	key_backspace = 0x08, key_tab       = 0x09,
	key_return    = 0x0D, key_shift     = 0x10,
	key_ctrl      = 0x11, key_alt       = 0x12,
	key_caps_lock = 0x14,
	key_esc       = 0x1B, key_space     = 0x20,
	key_end       = 0x23, key_home      = 0x24,
	key_left      = 0x25, key_right     = 0x27,
	key_up        = 0x26, key_down      = 0x28,
	key_page_up   = 0x21, key_page_down = 0x22,
	key_printscreen=0x2A, key_insert    = 0x2D, key_del = 0x2E,
	key_0 = 0x30, key_1 = 0x31, key_2 = 0x32, key_3 = 0x33, key_4 = 0x34, key_5 = 0x35, key_6 = 0x36, key_7 = 0x37, key_8 = 0x38, key_9 = 0x39, 
	key_a = 0x41, key_b = 0x42, key_c = 0x43, key_d = 0x44, key_e = 0x45, key_f = 0x46, key_g = 0x47, key_h = 0x48, key_i = 0x49, key_j = 0x4A, key_k = 0x4B, key_l = 0x4C, key_m = 0x4D, key_n = 0x4E, key_o = 0x4F, key_p = 0x50, key_q = 0x51, key_r = 0x52, key_s = 0x53, key_t = 0x54, key_u = 0x55, key_v = 0x56, key_w = 0x57, key_x = 0x58, key_y = 0x59, key_z = 0x5A,
	key_comma = 0xBC, key_period = 0xBE, key_slash_fwd = 0xBF, key_slash_back = 0xDC, key_semicolon = 0xBA, key_apostrophe = 0xDE, key_bracket_open = 0xDB, key_bracket_close = 0xDD, key_minus = 0xBD, key_equals = 0xBB, key_backtick = 0xc0,
	key_lcmd = 0x5B, key_rcmd = 0x5C,
	key_num0 = 0x60, key_num1 = 0x61, key_num2 = 0x62, key_num3 = 0x63, key_num4 = 0x64, key_num5 = 0x65, key_num6 = 0x66, key_num7 = 0x67, key_num8 = 0x68, key_num9 = 0x69,
	key_multiply = 0x6A, key_add = 0x6B, key_subtract = 0x6D, key_decimal = 0x6E, key_divide = 0x6F,
	key_f1 = 0x70, key_f2 = 0x71, key_f3 = 0x72, key_f4 = 0x73, key_f5 = 0x74, key_f6 = 0x75, key_f7 = 0x76, key_f8 = 0x77, key_f9 = 0x78, key_f10 = 0x79, key_f11 = 0x7A, key_f12 = 0x7B,
	key_MAX = 0xFF,
} key_;

SK_API int                   input_pointer_count  (input_source_ filter sk_default(input_source_any));
SK_API pointer_t             input_pointer        (int32_t index, input_source_ filter sk_default(input_source_any));
SK_API const hand_t         *input_hand           (handed_ hand);
SK_API void                  input_hand_override  (handed_ hand, const hand_joint_t *hand_joints);
SK_API const controller_t   *input_controller     (handed_ hand);
SK_API button_state_         input_controller_menu();
SK_API const pose_t         *input_head           ();
SK_API const pose_t         *input_eyes           ();
SK_API button_state_         input_eyes_tracked   ();
SK_API const mouse_t        *input_mouse          ();
SK_API button_state_         input_key            (key_ key);
SK_API char32_t              input_text_consume   ();
SK_API void                  input_text_reset     ();
SK_API void                  input_hand_visible   (handed_ hand, bool32_t visible);
SK_API void                  input_hand_solid     (handed_ hand, bool32_t solid);
SK_API void                  input_hand_material  (handed_ hand, material_t material);

SK_API void                  input_subscribe      (input_source_ source, button_state_ event, void (*event_callback)(input_source_ source, button_state_ event, const sk_ref(pointer_t) pointer));
SK_API void                  input_unsubscribe    (input_source_ source, button_state_ event, void (*event_callback)(input_source_ source, button_state_ event, const sk_ref(pointer_t) pointer));
SK_API void                  input_fire_event     (input_source_ source, button_state_ event, const sk_ref(pointer_t) pointer);

///////////////////////////////////////////

SK_API bool32_t   world_has_bounds                ();
SK_API vec2       world_get_bounds_size           ();
SK_API pose_t     world_get_bounds_pose           ();
SK_API pose_t     world_from_spatial_graph        (uint8_t spatial_graph_node_id[16], bool32_t dynamic, int64_t qpc_time);
SK_API pose_t     world_from_perception_anchor    (void *perception_spatial_anchor);
SK_API bool32_t   world_try_from_spatial_graph    (uint8_t spatial_graph_node_id[16], bool32_t dynamic, int64_t qpc_time, pose_t *out_pose);
SK_API bool32_t   world_try_from_perception_anchor(void *perception_spatial_anchor,   pose_t *out_pose);
SK_API bool32_t   world_raycast                   (ray_t ray, ray_t *out_intersection);
SK_API void       world_set_occlusion_enabled     (bool32_t enabled);
SK_API bool32_t   world_get_occlusion_enabled     ();
SK_API void       world_set_raycast_enabled       (bool32_t enabled);
SK_API bool32_t   world_get_raycast_enabled       ();
SK_API void       world_set_occlusion_material    (material_t material);
SK_API material_t world_get_occlusion_material    ();

///////////////////////////////////////////

typedef enum log_colors_ {
	log_colors_ansi = 0,
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
SK_API void log_subscribe  (void (*on_log)(log_ level, const char *text));
SK_API void log_unsubscribe(void (*on_log)(log_ level, const char *text));

///////////////////////////////////////////

SK_API void assets_releaseref_threadsafe(void *asset);

///////////////////////////////////////////

static const char *default_id_material             = "default/material";
static const char *default_id_material_pbr         = "default/material_pbr";
static const char *default_id_material_pbr_clip    = "default/material_pbr_clip";
static const char *default_id_material_unlit       = "default/material_unlit";
static const char *default_id_material_unlit_clip  = "default/material_unlit_clip";
static const char *default_id_material_equirect    = "default/equirect_convert";
static const char *default_id_material_font        = "default/material_font";
static const char *default_id_material_hand        = "default/material_hand";
static const char *default_id_material_ui          = "default/material_ui";
static const char *default_id_material_ui_box      = "default/material_ui_box";
static const char *default_id_material_ui_quadrant = "default/material_ui_quadrant";
static const char *default_id_tex                  = "default/tex";
static const char *default_id_tex_black            = "default/tex_black";
static const char *default_id_tex_gray             = "default/tex_gray";
static const char *default_id_tex_flat             = "default/tex_flat";
static const char *default_id_tex_rough            = "default/tex_rough";
static const char *default_id_cubemap              = "default/cubemap";
static const char *default_id_font                 = "default/font";
static const char *default_id_mesh_quad            = "default/mesh_quad";
static const char *default_id_mesh_screen_quad     = "default/mesh_screen_quad";
static const char *default_id_mesh_cube            = "default/mesh_cube";
static const char *default_id_mesh_sphere          = "default/mesh_sphere";
static const char *default_id_mesh_lefthand        = "default/mesh_lefthand";
static const char *default_id_mesh_righthand       = "default/mesh_righthand";
static const char *default_id_mesh_ui_button       = "default/mesh_ui_button";
static const char *default_id_shader               = "default/shader";
static const char *default_id_shader_pbr           = "default/shader_pbr";
static const char *default_id_shader_pbr_clip      = "default/shader_pbr_clip";
static const char *default_id_shader_unlit         = "default/shader_unlit";
static const char *default_id_shader_unlit_clip    = "default/shader_unlit_clip";
static const char *default_id_shader_font          = "default/shader_font";
static const char *default_id_shader_equirect      = "default/shader_equirect";
static const char *default_id_shader_ui            = "default/shader_ui";
static const char *default_id_shader_ui_box        = "default/shader_ui_box";
static const char *default_id_shader_ui_quadrant   = "default/shader_ui_quadrant";
static const char *default_id_shader_sky           = "default/shader_sky";
static const char *default_id_shader_lines         = "default/shader_lines";
static const char *default_id_sound_click          = "default/sound_click";
static const char *default_id_sound_unclick        = "default/sound_unclick";
static const char *default_id_sound_grab           = "default/sound_grab";
static const char *default_id_sound_ungrab         = "default/sound_ungrab";

#ifdef __cplusplus
} // namespace sk
#endif

///////////////////////////////////////////
// Tools for creating the version information

#define SK_STR2(x) #x
#define SK_STR(x) SK_STR2(x)

  // This will look like 'M.i.P-rcr', or 'M.i.P' if r is 0
#if SK_VERSION_PRERELEASE != 0
#define SK_VERSION (SK_STR(SK_VERSION_MAJOR) "." SK_STR(SK_VERSION_MINOR) "." SK_STR(SK_VERSION_PATCH) "-preview" SK_STR(SK_VERSION_PRERELEASE))
#else
#define SK_VERSION (SK_STR(SK_VERSION_MAJOR) "." SK_STR(SK_VERSION_MINOR) "." SK_STR(SK_VERSION_PATCH))
#endif

  // A version in hex looks like: 0xMMMMiiiiPPPPrrrr
  // In order of Major.mInor.Patch.pre-Release
#define SK_VERSION_ID ( ((uint64_t)SK_VERSION_MAJOR << 48) | ((uint64_t)SK_VERSION_MINOR << 32) | ((uint64_t)SK_VERSION_PATCH << 16) | (uint64_t)(SK_VERSION_PRERELEASE) )

#pragma once

#if defined(_DLL)
#define SK_API extern "C" __declspec(dllexport)
#define SK_API_S __declspec(dllexport)
#else
#define SK_API extern "C" __declspec(dllimport)
#define SK_API_S __declspec(dllimport)
#endif

#define SK_DeclarePrivateType(name) struct _ ## name; typedef struct _ ## name *name;
#define SK_MakeFlag(enumType) \
inline enumType  operator| (enumType  a, enumType b) {return static_cast<enumType>(static_cast<int>(a) | static_cast<int>(b));} \
inline enumType &operator|=(enumType& a, const enumType& b) { a = a | b; return a; } \
inline enumType  operator& (enumType  a, enumType b) {return static_cast<enumType>(static_cast<int>(a) & static_cast<int>(b));} \
inline enumType &operator&=(enumType& a, const enumType& b) { a = a & b; return a; } \
inline enumType  operator~ (const enumType& a) { return static_cast<enumType>(~static_cast<int>(a)); }

#include <stdint.h>
#include <DirectXMath.h>

///////////////////////////////////////////

enum sk_runtime_ {
	sk_runtime_flatscreen   = 0,
	sk_runtime_mixedreality = 1
};

extern float  sk_timevf;
extern double sk_timev;
extern double sk_timev_elapsed;
extern float  sk_timev_elapsedf;
extern long long sk_timev_raw;

extern sk_runtime_ sk_runtime;
extern bool sk_focused;
extern bool sk_run;

SK_API bool sk_init      (const char *app_name, sk_runtime_ runtime);
SK_API void sk_shutdown  ();
SK_API bool sk_step      (void (*app_update)(void));

SK_API float  sk_timef();
SK_API double sk_time();
SK_API float  sk_time_elapsedf();
SK_API double sk_time_elapsed();

///////////////////////////////////////////

struct color32 {
	uint8_t r, g, b, a;
};
struct color128 {
	float r, g, b, a;
};
struct vec2 {
	float x, y;
};
struct vec3 {
	float x, y, z;
};
struct vec4 {
	float x, y, z, w;
};
struct quat {
	float i, j, k, a;
};
struct matrix {
	vec4 row[4];
};
struct ray {
	vec3 pos;
	vec3 dir;
};
struct pose_t {
	vec3 position;
	quat orientation;
};

vec3 operator*(const quat &a, const vec3 &b);
quat operator*(const quat &a, const quat &b);

static inline vec2 operator*(const vec2 &a, const float b) { return { a.x * b, a.y * b }; }
static inline vec2 operator/(const vec2 &a, const float b) { return { a.x / b, a.y / b }; }
static inline vec2 operator+(const vec2 &a, const vec2 &b) { return { a.x + b.x, a.y + b.y }; }
static inline vec2 operator-(const vec2 &a, const vec2 &b) { return { a.x - b.x, a.y - b.y }; }
static inline vec2 operator*(const vec2 &a, const vec2 &b) { return { a.x * b.x, a.y * b.y }; }
static inline vec2 operator/(const vec2 &a, const vec2 &b) { return { a.x / b.x, a.y / b.y }; }
static inline vec2& operator+=(vec2& a, const vec2& b)     { a.x += b.x; a.y += b.y; return a; }
static inline vec2& operator-=(vec2& a, const vec2& b)     { a.x -= b.x; a.y -= b.y; return a; }
static inline vec2& operator*=(vec2& a, const float b)     { a.x *= b; a.y *= b; return a; }
static inline vec2& operator/=(vec2& a, const float b)     { a.x /= b; a.y /= b; return a; }

static inline vec3 operator*(const vec3 &a, const float b) { return { a.x * b, a.y * b, a.z * b }; }
static inline vec3 operator/(const vec3 &a, const float b) { return { a.x / b, a.y / b, a.z / b }; }
static inline vec3 operator+(const vec3 &a, const vec3 &b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline vec3 operator-(const vec3 &a, const vec3 &b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline vec3 operator*(const vec3 &a, const vec3 &b) { return { a.x * b.x, a.y * b.y, a.z * b.z }; }
static inline vec3 operator/(const vec3 &a, const vec3 &b) { return { a.x / b.x, a.y / b.y, a.z / b.z }; }
static inline vec3& operator+=(vec3& a, const vec3& b)     { a.x += b.x; a.y += b.y; a.z += b.z; return a; }
static inline vec3& operator-=(vec3& a, const vec3& b)     { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a; }
static inline vec3& operator*=(vec3& a, const float b)     { a.x *= b; a.y *= b; a.z *= b; return a; }
static inline vec3& operator/=(vec3& a, const float b)     { a.x /= b; a.y /= b; a.z /= b; return a; }

static inline float vec3_magnitude_sq(const vec3 &a) { return a.x * a.x + a.y * a.y + a.z * a.z; }
static inline float vec3_magnitude   (const vec3 &a) { return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z); }
static inline vec3  vec3_normalize   (const vec3 &a) { return a / vec3_magnitude(a); }
static inline vec3  vec3_lerp        (const vec3 &a, const vec3 &b, float t) { return a + (b - a)*t; }

static inline vec2  vec2_lerp        (const vec2 &a, const vec2 &b, float t) { return a + (b - a)*t; }

quat quat_lookat(const vec3 &from, const vec3 &at);
quat quat_lerp(const quat &a, const quat &b, float t);

#define deg2rad 0.01745329252
#define rad2deg 57.295779513

///////////////////////////////////////////

struct vert_t {
	vec3    pos;
	vec3    norm;
	vec2    uv;
	color32 col;
};

SK_DeclarePrivateType(mesh_t);

SK_API mesh_t mesh_find     (const char *id);
SK_API mesh_t mesh_create   (const char *id);
SK_API void   mesh_release  (mesh_t mesh);
SK_API void   mesh_set_verts(mesh_t mesh, vert_t   *vertices, int vertex_count);
SK_API void   mesh_set_inds (mesh_t mesh, uint16_t *indices,  int index_count);

SK_API mesh_t mesh_gen_cube (const char *id, vec3 size, int subdivisions);

///////////////////////////////////////////

SK_DeclarePrivateType(tex2d_t);

SK_API tex2d_t tex2d_find       (const char *id);
SK_API tex2d_t tex2d_create     (const char *id);
SK_API tex2d_t tex2d_create_file(const char *file);
SK_API void    tex2d_release    (tex2d_t texture);
SK_API void    tex2d_set_colors (tex2d_t texture, int width, int height, uint8_t *data_rgba32);

///////////////////////////////////////////

SK_DeclarePrivateType(shader_t);

SK_API shader_t shader_find       (const char *id);
SK_API shader_t shader_create     (const char *id, const char *hlsl);
SK_API shader_t shader_create_file(const char *filename);
SK_API void     shader_release    (shader_t shader);

///////////////////////////////////////////

SK_DeclarePrivateType(material_t);

SK_API material_t material_find   (const char *id);
SK_API material_t material_create (const char *id, shader_t shader);
SK_API void       material_release(material_t material);
SK_API void       material_set_float  (material_t material, const char *name, float    value);
SK_API void       material_set_color32(material_t material, const char *name, color32  value);
SK_API void       material_set_color  (material_t material, const char *name, color128 value);
SK_API void       material_set_vector (material_t material, const char *name, vec4     value);
SK_API void       material_set_matrix (material_t material, const char *name, matrix   value);
SK_API void       material_set_texture(material_t material, const char *name, tex2d_t  value);

///////////////////////////////////////////

struct transform_t {
	vec3 _position;
	vec3 _scale;
	quat _rotation;

	bool _dirty;
	DirectX::XMMATRIX _transform;
};

SK_API void transform_initialize  (transform_t &transform);
SK_API void transform_set         (transform_t &transform, const vec3 &position, const vec3 &scale, const quat &rotation );
SK_API void transform_set_position(transform_t &transform, const vec3 &position);
SK_API void transform_set_scale   (transform_t &transform, const vec3 &scale);
SK_API void transform_set_rotation(transform_t &transform, const quat &rotation);
SK_API void transform_lookat      (transform_t &transform, const vec3 &at);
SK_API vec3 transform_forward     (transform_t &transform);

SK_API void transform_matrix(transform_t &transform, DirectX::XMMATRIX &result);

///////////////////////////////////////////

SK_DeclarePrivateType(model_t);

SK_API model_t model_find       (const char *id);
SK_API model_t model_create_mesh(const char *id, mesh_t mesh, material_t material);
SK_API model_t model_create_file(const char *filename);
SK_API void    model_release    (model_t model);

///////////////////////////////////////////

struct camera_t {
	float fov;
	float clip_near;
	float clip_far;
};

SK_API void camera_initialize(camera_t &cam, float fov, float clip_near, float clip_far);
SK_API void camera_view      (transform_t &cam_transform, DirectX::XMMATRIX &result);
SK_API void camera_proj      (camera_t    &cam, DirectX::XMMATRIX &result);

///////////////////////////////////////////

SK_API void render_set_camera (camera_t &cam, transform_t &cam_transform);
SK_API void render_set_light  (const vec3 &direction, float intensity, const color128 &color);
SK_API void render_add_mesh   (mesh_t mesh, material_t material, transform_t &transform);
SK_API void render_add_model  (model_t model, transform_t &transform);

///////////////////////////////////////////

enum input_source_ {
	input_source_any        = 0x7FFFFFFF,
	input_source_hand       = 1 << 0,
	input_source_hand_left  = 1 << 1,
	input_source_hand_right = 1 << 2,
	input_source_gaze       = 1 << 4,
	input_source_gaze_head  = 1 << 5,
	input_source_gaze_eyes  = 1 << 6,
	input_source_gaze_cursor= 1 << 7,
	input_source_can_press  = 1 << 8,
};
SK_MakeFlag(input_source_);

enum pointer_state_ {
	pointer_state_none      = 0,
	pointer_state_available = 1 << 0,
};
SK_MakeFlag(pointer_state_);

enum handed_ {
	handed_left  = 0,
	handed_right = 1,
	handed_max   = 2,
};

enum input_state_ {
	input_state_none        = 0,
	input_state_any         = 0x7FFFFFFF,
	input_state_tracked     = 1 << 0,
	input_state_justtracked = 1 << 1,
	input_state_untracked   = 1 << 2,
	input_state_pinch       = 1 << 3,
	input_state_justpinch   = 1 << 4,
	input_state_unpinch     = 1 << 5,
	input_state_grip        = 1 << 6,
	input_state_justgrip    = 1 << 7,
	input_state_ungrip      = 1 << 8,
};
SK_MakeFlag(input_state_);

struct pointer_t {
	input_source_  source;
	pointer_state_ state;
	ray            ray;
	quat           orientation;
};

struct hand_t {
	pose_t  fingers[5][5];
	pose_t  wrist;
	pose_t  root;
	handed_ handedness;
	input_state_ state;
};

SK_API int           input_pointer_count(input_source_ filter = input_source_any);
SK_API pointer_t     input_pointer      (int index, input_source_ filter = input_source_any);
SK_API const hand_t &input_hand         (handed_ hand);
SK_API void          input_hand_visible (handed_ hand, int visible);
SK_API void          input_hand_material(handed_ hand, material_t material);

SK_API void input_subscribe  (input_source_ source, input_state_ event, void (*event_callback)(input_source_ source, input_state_ event, const pointer_t &pointer));
SK_API void input_unsubscribe(input_source_ source, input_state_ event, void (*event_callback)(input_source_ source, input_state_ event, const pointer_t &pointer));
SK_API void input_fire_event (input_source_ source, input_state_ event, const pointer_t &pointer);
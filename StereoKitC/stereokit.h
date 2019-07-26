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
inline enumType &operator|=(enumType& a, const enumType& b) { a = a | b; return a; }

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

vec3 operator*(const quat &a, const vec3 &b);

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

quat quat_lookat(const vec3 &from, const vec3 &at);

#define deg2rad 0.01745329252
#define rad2deg 57.295779513

///////////////////////////////////////////

struct vert_t {
	vec3    pos;
	vec3    norm;
	vec2    uv;
	uint8_t col[4];
};

SK_DeclarePrivateType(mesh_t);

SK_API mesh_t mesh_create   (const char *name);
SK_API void   mesh_release  (mesh_t mesh);
SK_API void   mesh_set_verts(mesh_t mesh, vert_t   *vertices, int vertex_count);
SK_API void   mesh_set_inds (mesh_t mesh, uint16_t *indices,  int index_count);

///////////////////////////////////////////

SK_DeclarePrivateType(tex2d_t);

SK_API tex2d_t tex2d_create     (const char *name);
SK_API tex2d_t tex2d_create_file(const char *file);
SK_API void    tex2d_release    (tex2d_t texture);
SK_API void    tex2d_set_colors (tex2d_t texture, int width, int height, uint8_t *data_rgba32);

///////////////////////////////////////////

SK_DeclarePrivateType(shader_t);

SK_API shader_t shader_create     (const char *name, const char *hlsl);
SK_API shader_t shader_create_file(const char *filename);
SK_API void     shader_release    (shader_t shader);

///////////////////////////////////////////

SK_DeclarePrivateType(material_t);

SK_API material_t material_create (const char *name, shader_t shader);
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

SK_API model_t model_create_file(const char *filename);
SK_API void    model_release    (model_t model);

///////////////////////////////////////////

struct camera_t {
	float fov;
	float clip_near;
	float clip_far;
};

SK_API void camera_initialize(camera_t &cam, float fov, float clip_near, float clip_far);
SK_API void camera_viewproj  (camera_t &cam, transform_t &cam_transform, DirectX::XMMATRIX &result);

///////////////////////////////////////////

SK_API void render_set_camera (camera_t &cam, transform_t &cam_transform);
SK_API void render_add_mesh   (mesh_t mesh, material_t material, transform_t &transform);
SK_API void render_add_model  (model_t model, transform_t &transform);

///////////////////////////////////////////

enum pointer_source_ {
	pointer_source_any        = 0x7FFFFFFF,
	pointer_source_hand       = 1 << 0,
	pointer_source_hand_left  = 1 << 1,
	pointer_source_hand_right = 1 << 2,
	pointer_source_gaze       = 1 << 4,
	pointer_source_gaze_head  = 1 << 5,
	pointer_source_gaze_eyes  = 1 << 6,
	pointer_source_gaze_cursor= 1 << 7,
	pointer_source_can_press  = 1 << 8,
};
SK_MakeFlag(pointer_source_);

enum pointer_state_ {
	pointer_state_none      = 0,
	pointer_state_pressed   = 1 << 0,
	pointer_state_just      = 1 << 1,
	pointer_state_available = 1 << 2,
};
SK_MakeFlag(pointer_state_);

struct pointer_t {
	pointer_source_ source;
	pointer_state_  state;
	ray             ray;
	quat            orientation;
};

SK_API int       input_pointer_count(pointer_source_ filter = pointer_source_any);
SK_API pointer_t input_pointer      (int index, pointer_source_ filter = pointer_source_any);
#pragma once

#if defined(_DLL)
#define SK_API extern "C" __declspec(dllexport)
#define SK_API_S __declspec(dllexport)
#else
#define SK_API extern "C" __declspec(dllimport)
#define SK_API_S __declspec(dllimport)
#endif

#define SK_DeclarePrivateType(name) struct _ ## name; typedef struct _ ## name *name;

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

struct vec2 {
	float x, y;
};
struct vec3 {
	float x, y, z;
};
struct quat {
	float i, j, k, a;
};
struct ray {
	vec3 pos;
	vec3 dir;
};

vec3 operator*(const quat &a, const vec3 &b);

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

SK_API mesh_t mesh_create        ();
SK_API mesh_t mesh_create_file   (const char *file);
SK_API mesh_t mesh_create_filemem(uint8_t *file_data, uint64_t file_size);
SK_API void   mesh_release   (mesh_t mesh);
SK_API void   mesh_set_verts (mesh_t mesh, vert_t   *verts, int vert_count);
SK_API void   mesh_set_inds  (mesh_t mesh, uint16_t *inds,  int ind_count);

///////////////////////////////////////////

SK_DeclarePrivateType(tex2d_t);

SK_API tex2d_t tex2d_create     ();
SK_API tex2d_t tex2d_create_file(const char *file);
SK_API void    tex2d_release    (tex2d_t tex);
SK_API void    tex2d_set_colors (tex2d_t tex, int width, int height, uint8_t *data_rgba32);

///////////////////////////////////////////

SK_DeclarePrivateType(shader_t);

SK_API shader_t shader_create     (const char *hlsl);
SK_API shader_t shader_create_file(const char *filename);
SK_API void     shader_release    (shader_t shader);

///////////////////////////////////////////

SK_DeclarePrivateType(material_t);

SK_API material_t material_create (shader_t shader, tex2d_t tex);
SK_API void       material_release(material_t material);

///////////////////////////////////////////

struct transform_t {
	vec3 _position;
	vec3 _scale;
	quat _rotation;

	bool _dirty;
	DirectX::XMMATRIX _transform;
};

SK_API void transform_initialize(transform_t &transform);
SK_API void transform_set       (transform_t &transform, const vec3 &position, const vec3 &scale, const quat &rotation );
SK_API void transform_set_pos   (transform_t &transform, const vec3 &position);
SK_API void transform_set_scale (transform_t &transform, const vec3 &scale);
SK_API void transform_set_rot   (transform_t &transform, const quat &rotation);
SK_API void transform_lookat    (transform_t &transform, const vec3 &at);

SK_API void transform_matrix(transform_t &transform, DirectX::XMMATRIX &result);

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
SK_API void render_add        (mesh_t mesh, material_t material, transform_t &transform);

///////////////////////////////////////////

enum pointer_source_ {
	pointer_source_any        = 0xFFFFFFFF,
	pointer_source_hand       = 1 << 0,
	pointer_source_hand_left  = 1 << 1,
	pointer_source_hand_right = 1 << 2,
	pointer_source_gaze       = 1 << 4,
	pointer_source_gaze_head  = 1 << 5,
	pointer_source_gaze_eyes  = 1 << 6,
	pointer_source_gaze_cursor= 1 << 7,
	pointer_source_can_press  = 1 << 8,
};

struct pointer_t {
	pointer_source_ source;
	ray             ray;
	bool pressed;
	bool just;
};

SK_API int       input_pointer_count(pointer_source_ filter = pointer_source_any);
SK_API pointer_t input_pointer      (int index, pointer_source_ filter = pointer_source_any);
#pragma once

#if defined(_DLL)
#define SK_API extern "C" __declspec(dllexport)
#define SK_API_S __declspec(dllexport)
#else
#define SK_API extern "C" __declspec(dllimport)
#define SK_API_S __declspec(dllimport)
#endif

#include <stdint.h>
#include <d3d11.h>
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

#define deg2rad 0.01745329252
#define rad2deg 57.295779513

///////////////////////////////////////////

struct vert_t {
	vec3    pos;
	vec3    norm;
	vec2    uv;
	uint8_t col[4];
};

struct mesh_t {
	void         *verts;
	int           vert_count;
	ID3D11Buffer *vert_buffer;
	uint16_t     *inds;
	int           ind_count;
	ID3D11Buffer *ind_buffer;
};

SK_API void mesh_create        (mesh_t &mesh);
SK_API bool mesh_create_file   (mesh_t &mesh, const char *file);
SK_API bool mesh_create_filemem(mesh_t &mesh, uint8_t *file_data, uint64_t file_size);
SK_API void mesh_destroy   (mesh_t &mesh);
SK_API void mesh_set_verts (mesh_t &mesh, vert_t   *verts, int vert_count);
SK_API void mesh_set_inds  (mesh_t &mesh, uint16_t *inds,  int ind_count);
SK_API void mesh_set_active(mesh_t &mesh);
SK_API void mesh_draw      (mesh_t &mesh);

///////////////////////////////////////////

struct tex2d_t {
	int width;
	int height;
	ID3D11ShaderResourceView *resource;
	ID3D11Texture2D          *texture;
};

SK_API void tex2d_create     (tex2d_t &tex);
SK_API bool tex2d_create_file(tex2d_t &tex, const char *file);
SK_API void tex2d_destroy    (tex2d_t &tex);
SK_API void tex2d_set_colors (tex2d_t &tex, int width, int height, uint8_t *data_rgba32);
SK_API void tex2d_set_active (tex2d_t &tex, int slot);

///////////////////////////////////////////

struct shader_t {
	ID3D11VertexShader *vshader;
	ID3D11PixelShader  *pshader;
	ID3D11InputLayout  *vert_layout;
};
struct shaderargs_t {
	ID3D11Buffer *const_buffer;
	int buffer_slot;
	int buffer_size;
};

SK_API void shader_create     (shader_t &shader, const char *hlsl);
SK_API bool shader_create_file(shader_t &shader, const char *filename);
SK_API void shader_destroy    (shader_t &shader);
SK_API void shader_set_active (shader_t &shader);

SK_API void shaderargs_create    (shaderargs_t &args, size_t buffer_size, int buffer_slot);
SK_API void shaderargs_destroy   (shaderargs_t &args);
SK_API void shaderargs_set_data  (shaderargs_t &args, void *data);
SK_API void shaderargs_set_active(shaderargs_t &args);

///////////////////////////////////////////

struct material_t {
	shader_t *shader;
	tex2d_t *texture;
};

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
SK_API void render_add        (mesh_t &mesh, material_t &material, transform_t &transform);
SK_API void render_draw       ();
SK_API void render_draw_from  (camera_t &cam, transform_t &cam_transform);
SK_API void render_draw_matrix(const float *cam_matrix, transform_t &cam_transform);
SK_API void render_clear();

SK_API void render_initialize();
SK_API void render_shutdown();
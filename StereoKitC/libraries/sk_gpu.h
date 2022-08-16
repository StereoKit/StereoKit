/*Licensed under MIT or Public Domain. See bottom of file for details.
	Author - Nick Klingensmith - @koujaku - https://twitter.com/koujaku
	Repository - https://github.com/maluoi/sk_gpu

sk_gpu.h

	Docs are not yet here as the project is still somewhat in flight, but check
	out some of the example projects in the repository for reference! They're 
	pretty clean as is :)

	Note: this is currently an amalgamated file, so it's best to modify the 
	individual files in the repository if making modifications.
*/

#pragma once

// You can force sk_gpu to use a specific API, but if you don't, it'll pick
// an API appropriate for the platform it's being compiled for!
//
//#define SKG_FORCE_DIRECT3D11
//#define SKG_FORCE_OPENGL

#if   defined( SKG_FORCE_DIRECT3D11 )
#define SKG_DIRECT3D11
#elif defined( SKG_FORCE_OPENGL )
#define SKG_OPENGL
#elif defined( _WIN32 )
#define SKG_DIRECT3D11
#else
#define SKG_OPENGL
#endif

// If we're using OpenGL, set up some additional defines so we know what
// flavor of GL is being used, and how to load it.
#ifdef SKG_OPENGL
	#if   defined(__EMSCRIPTEN__)
		#define _SKG_GL_WEB
		#define _SKG_GL_LOAD_EMSCRIPTEN
	#elif defined(__ANDROID__)
		#define _SKG_GL_ES
		#define _SKG_GL_LOAD_EGL
		#define _SKG_GL_MAKE_FUNCTIONS
	#elif defined(__linux__)
		#if defined(SKG_LINUX_EGL)
			#define _SKG_GL_ES
			#define _SKG_GL_LOAD_EGL
			#define _SKG_GL_MAKE_FUNCTIONS
		#else
			#define _SKG_GL_DESKTOP
			#define _SKG_GL_LOAD_GLX
		#endif
	#elif defined(_WIN32)
		#define _SKG_GL_DESKTOP
		#define _SKG_GL_LOAD_WGL
		#define _SKG_GL_MAKE_FUNCTIONS
	#endif
#endif

// Add definitions for how/if we want the functions exported
#ifdef __GNUC__
	#define SKG_API
#else
	#if defined(SKG_LIB_EXPORT)
		#define SKG_API __declspec(dllexport)
	#elif defined(SKG_LIB_IMPORT)
		#define SKG_API __declspec(dllimport)
	#else
		#define SKG_API
	#endif
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

///////////////////////////////////////////

typedef enum skg_buffer_type_ {
	skg_buffer_type_vertex,
	skg_buffer_type_index,
	skg_buffer_type_constant,
	skg_buffer_type_compute,
} skg_buffer_type_;

typedef enum skg_tex_type_ {
	skg_tex_type_image,
	skg_tex_type_cubemap,
	skg_tex_type_rendertarget,
	skg_tex_type_depth,
} skg_tex_type_;

typedef enum skg_use_ {
	skg_use_static        = 1 << 0,
	skg_use_dynamic       = 1 << 2,
	skg_use_compute_read  = 1 << 3,
	skg_use_compute_write = 1 << 4,
	skg_use_compute_readwrite = skg_use_compute_read | skg_use_compute_write
} skg_use_;

typedef enum skg_read_ {
	skg_read_only,
	skg_read_write,
} skg_read_;

typedef enum skg_mip_ {
	skg_mip_generate,
	skg_mip_none,
} skg_mip_;

typedef enum skg_tex_address_ {
	skg_tex_address_repeat,
	skg_tex_address_clamp,
	skg_tex_address_mirror,
} skg_tex_address_;

typedef enum skg_tex_sample_ {
	skg_tex_sample_linear,
	skg_tex_sample_point,
	skg_tex_sample_anisotropic
} skg_tex_sample_;

typedef enum skg_tex_fmt_ {
	skg_tex_fmt_none = 0,
	skg_tex_fmt_rgba32,
	skg_tex_fmt_rgba32_linear,
	skg_tex_fmt_bgra32,
	skg_tex_fmt_bgra32_linear,
	skg_tex_fmt_rg11b10,
	skg_tex_fmt_rgb10a2,
	skg_tex_fmt_rgba64u,
	skg_tex_fmt_rgba64s,
	skg_tex_fmt_rgba64f,
	skg_tex_fmt_rgba128,
	skg_tex_fmt_r8,
	skg_tex_fmt_r16,
	skg_tex_fmt_r32,
	skg_tex_fmt_depthstencil,
	skg_tex_fmt_depth32,
	skg_tex_fmt_depth16,
} skg_tex_fmt_;

typedef enum skg_fmt_ {
	skg_fmt_none,
	skg_fmt_f32_1,    skg_fmt_f32_2,    skg_fmt_f32_3,    skg_fmt_f32_4,
	skg_fmt_f16_1,    skg_fmt_f16_2,                      skg_fmt_f16_4,
	skg_fmt_i32_1,    skg_fmt_i32_2,    skg_fmt_i32_3,    skg_fmt_i32_4,
	skg_fmt_i16_1,    skg_fmt_i16_2,                      skg_fmt_i16_4,
	skg_fmt_i8_1,     skg_fmt_i8_2,                       skg_fmt_i8_4,
	skg_fmt_ui32_1,   skg_fmt_ui32_2,   skg_fmt_ui32_3,   skg_fmt_ui32_4,
	skg_fmt_ui16_1,   skg_fmt_ui16_2,                     skg_fmt_ui16_4,
	skg_fmt_ui8_1,    skg_fmt_ui8_2,                      skg_fmt_ui8_4,
	skg_fmt_ui16_n_1, skg_fmt_ui16_n_2,                   skg_fmt_ui16_n_4,
	skg_fmt_ui8_n_1,  skg_fmt_ui8_n_2,                    skg_fmt_ui8_n_4,
} skg_fmt_;

typedef enum skg_el_semantic_ {
	skg_el_semantic_none,
	skg_el_semantic_position,
	skg_el_semantic_normal,
	skg_el_semantic_texcoord,
	skg_el_semantic_color,
	skg_el_semantic_target_index,
} skg_el_semantic_;

typedef enum skg_stage_ {
	skg_stage_vertex  = 1 << 0,
	skg_stage_pixel   = 1 << 1,
	skg_stage_compute = 1 << 2,
} skg_stage_;

typedef enum skg_register_ {
	skg_register_default,
	skg_register_vertex,
	skg_register_index,
	skg_register_constant,
	skg_register_resource,
	skg_register_readwrite,
} skg_register_;

typedef enum skg_shader_var_ {
	skg_shader_var_none,
	skg_shader_var_int,
	skg_shader_var_uint,
	skg_shader_var_uint8,
	skg_shader_var_float,
	skg_shader_var_double,
} skg_shader_var_;

typedef enum skg_transparency_ {
	skg_transparency_none = 1,
	skg_transparency_blend,
	skg_transparency_add,
} skg_transparency_;

typedef enum skg_cull_ {
	skg_cull_back = 0,
	skg_cull_front,
	skg_cull_none,
} skg_cull_;

typedef enum skg_depth_test_ {
	skg_depth_test_less = 0,
	skg_depth_test_less_or_eq,
	skg_depth_test_greater,
	skg_depth_test_greater_or_eq,
	skg_depth_test_equal,
	skg_depth_test_not_equal,
	skg_depth_test_always,
	skg_depth_test_never,
} skg_depth_test_;

typedef enum skg_log_ {
	skg_log_info,
	skg_log_warning,
	skg_log_critical,
} skg_log_;

typedef enum skg_cap_ {
	skg_cap_tex_layer_select = 1,
	skg_cap_wireframe,
} skg_cap_;

typedef struct {
	uint8_t r, g, b, a;
} skg_color32_t;

typedef struct {
	float r, g, b, a;
} skg_color128_t;

typedef struct skg_vert_t {
	float         pos [3];
	float         norm[3];
	float         uv  [2];
	skg_color32_t col;
} skg_vert_t;

typedef struct skg_bind_t {
	uint16_t slot;
	uint8_t  stage_bits;
	uint8_t  register_type;
} skg_bind_t;

typedef struct skg_shader_var_t {
	char     name [32];
	uint64_t name_hash;
	char     extra[64];
	uint32_t offset;
	uint32_t size;
	uint16_t type;
	uint16_t type_count;
} skg_shader_var_t;

typedef struct skg_shader_buffer_t {
	char              name[32];
	uint64_t          name_hash;
	skg_bind_t        bind;
	uint32_t          size;
	void             *defaults;
	uint32_t          var_count;
	skg_shader_var_t *vars;
} skg_shader_buffer_t;

typedef struct skg_shader_resource_t {
	char       name [32];
	uint64_t   name_hash;
	char       extra[64];
	skg_bind_t bind;
} skg_shader_resource_t;

typedef struct skg_shader_meta_t {
	char                   name[256];
	uint32_t               buffer_count;
	skg_shader_buffer_t   *buffers;
	uint32_t               resource_count;
	skg_shader_resource_t *resources;
	int32_t                references;
	int32_t                global_buffer_id;
} skg_shader_meta_t;

///////////////////////////////////////////

#if   defined(SKG_DIRECT3D11)


#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>
#include <dxgi1_6.h>

///////////////////////////////////////////

typedef struct skg_buffer_t {
	skg_use_         use;
	skg_buffer_type_ type;
	uint32_t         stride;
	ID3D11Buffer    *_buffer;
	ID3D11ShaderResourceView  *_resource;
	ID3D11UnorderedAccessView *_unordered;
} skg_buffer_t;

typedef struct skg_computebuffer_t {
	skg_read_                  read_write;
	skg_buffer_t               buffer;
	ID3D11ShaderResourceView  *_resource;
	ID3D11UnorderedAccessView *_unordered;
} skg_computebuffer_t;

typedef struct skg_mesh_t {
	ID3D11Buffer *_ind_buffer;
	ID3D11Buffer *_vert_buffer;
} skg_mesh_t;

typedef struct skg_shader_stage_t {
	skg_stage_         type;
	void              *_shader;
	ID3D11InputLayout *_layout;
} skg_shader_stage_t;

typedef struct skg_shader_t {
	skg_shader_meta_t   *meta;
	ID3D11VertexShader  *_vertex;
	ID3D11PixelShader   *_pixel;
	ID3D11ComputeShader *_compute;
	ID3D11InputLayout   *_layout;
} skg_shader_t;

typedef struct skg_pipeline_t {
	skg_transparency_        transparency;
	skg_cull_                cull;
	bool                     wireframe;
	bool                     depth_write;
	bool                     scissor;
	skg_depth_test_          depth_test;
	skg_shader_meta_t       *meta;
	ID3D11VertexShader      *_vertex;
	ID3D11PixelShader       *_pixel;
	ID3D11InputLayout       *_layout;
	ID3D11BlendState        *_blend;
	ID3D11RasterizerState   *_rasterize;
	ID3D11DepthStencilState *_depth;
} skg_pipeline_t;

typedef struct skg_tex_t {
	int32_t                    width;
	int32_t                    height;
	int32_t                    array_count;
	int32_t                    array_start;
	int32_t                    multisample;
	skg_use_                   use;
	skg_tex_type_              type;
	skg_tex_fmt_               format;
	skg_mip_                   mips;
	ID3D11Texture2D           *_texture;
	ID3D11SamplerState        *_sampler;
	ID3D11ShaderResourceView  *_resource;
	ID3D11UnorderedAccessView *_unordered;
	ID3D11RenderTargetView    *_target_view;
	ID3D11DepthStencilView    *_depth_view;
} skg_tex_t;

typedef struct skg_swapchain_t {
	int32_t          width;
	int32_t          height;
	skg_tex_t        _target;
	skg_tex_t        _depth;
	IDXGISwapChain1 *_swapchain;
} skg_swapchain_t;

typedef struct skg_platform_data_t {
	void *_d3d11_device;
} skg_platform_data_t;

#elif defined(SKG_DIRECT3D12)

#elif defined(SKG_VULKAN)

#elif defined(SKG_OPENGL)

#define SKG_MANUAL_SRGB

///////////////////////////////////////////

typedef struct skg_buffer_t {
	skg_use_         use;
	skg_buffer_type_ type;
	uint32_t         stride;
	uint32_t         _target;
	uint32_t         _buffer;
} skg_buffer_t;

typedef struct skg_mesh_t {
	uint32_t _ind_buffer;
	uint32_t _vert_buffer;
	uint32_t _layout;
} skg_mesh_t;

typedef struct skg_shader_stage_t {
	skg_stage_ type;
	uint32_t   _shader;
} skg_shader_stage_t;

typedef struct skg_shader_t {
	skg_shader_meta_t *meta;
	uint32_t           _vertex;
	uint32_t           _pixel;
	uint32_t           _program;
	uint32_t           _compute;
} skg_shader_t;

typedef struct skg_pipeline_t {
	skg_transparency_ transparency;
	skg_cull_         cull;
	bool              wireframe;
	bool              depth_write;
	bool              scissor;
	skg_depth_test_   depth_test;
	skg_shader_meta_t*meta;
	skg_shader_t      _shader;
} skg_pipeline_t;

typedef struct skg_tex_t {
	int32_t       width;
	int32_t       height;
	int32_t       array_count;
	int32_t       array_start;
	int32_t       multisample;
	skg_use_      use;
	skg_tex_type_ type;
	skg_tex_fmt_  format;
	skg_mip_      mips;
	uint32_t      _texture;
	uint32_t      _framebuffer;
	uint32_t      _target;
	uint32_t      _access;
	uint32_t      _format;
} skg_tex_t;

typedef struct skg_swapchain_t {
	int32_t  width;
	int32_t  height;

#if   defined(_SKG_GL_LOAD_WGL)
	void *_hdc;
	void *_hwnd;
#elif defined(_SKG_GL_LOAD_EGL)
	void *_egl_surface;
#elif defined(_SKG_GL_LOAD_GLX)
	void *_x_window;
#elif defined(_SKG_GL_LOAD_EMSCRIPTEN) && defined(SKG_MANUAL_SRGB)
	skg_tex_t      _surface;
	skg_tex_t      _surface_depth;
	skg_shader_t   _convert_shader;
	skg_pipeline_t _convert_pipe;
	skg_buffer_t   _quad_vbuff;
	skg_buffer_t   _quad_ibuff;
	skg_mesh_t     _quad_mesh;
#endif
} skg_swapchain_t;

typedef struct skg_platform_data_t {
#if   defined(_SKG_GL_LOAD_EGL)
	void *_egl_display;
	void *_egl_config;
	void *_egl_context;
#elif defined(_SKG_GL_LOAD_WGL)
	void *_gl_hdc;
	void *_gl_hrc;
#elif defined(_SKG_GL_LOAD_GLX)
	void *_x_display;
	void *_visual_id;
	void *_glx_fb_config;
	void *_glx_drawable;
	void *_glx_context;
#endif
} skg_platform_data_t;
#endif

///////////////////////////////////////////

SKG_API void                skg_setup_xlib               (void *dpy, void *vi, void *fbconfig, void *drawable);
SKG_API int32_t             skg_init                     (const char *app_name, void *adapter_id);
SKG_API void                skg_shutdown                 ();
SKG_API void                skg_callback_log             (void (*callback)(skg_log_ level, const char *text));
SKG_API void                skg_callback_file_read       (bool (*callback)(const char *filename, void **out_data, size_t *out_size));
SKG_API skg_platform_data_t skg_get_platform_data        ();
SKG_API bool                skg_capability               (skg_cap_ capability);

SKG_API void                skg_draw_begin               ();
SKG_API void                skg_draw                     (int32_t index_start, int32_t index_base, int32_t index_count, int32_t instance_count);
SKG_API void                skg_compute                  (uint32_t thread_count_x, uint32_t thread_count_y, uint32_t thread_count_z);
SKG_API void                skg_viewport                 (const int32_t *xywh);
SKG_API void                skg_viewport_get             (int32_t *out_xywh);
SKG_API void                skg_scissor                  (const int32_t *xywh);
SKG_API void                skg_target_clear             (bool depth, const float *clear_color_4);

SKG_API skg_buffer_t        skg_buffer_create            (const void *data, uint32_t size_count, uint32_t size_stride, skg_buffer_type_ type, skg_use_ use);
SKG_API bool                skg_buffer_is_valid          (const skg_buffer_t *buffer);
SKG_API void                skg_buffer_set_contents      (      skg_buffer_t *buffer, const void *data, uint32_t size_bytes);
SKG_API void                skg_buffer_get_contents      (const skg_buffer_t *buffer, void *ref_buffer, uint32_t buffer_size);
SKG_API void                skg_buffer_bind              (const skg_buffer_t *buffer, skg_bind_t slot_vc, uint32_t offset_vi);
SKG_API void                skg_buffer_clear             (skg_bind_t bind);
SKG_API void                skg_buffer_destroy           (      skg_buffer_t *buffer);

SKG_API skg_mesh_t          skg_mesh_create              (const skg_buffer_t *vert_buffer, const skg_buffer_t *ind_buffer);
SKG_API void                skg_mesh_set_verts           (      skg_mesh_t *mesh, const skg_buffer_t *vert_buffer);
SKG_API void                skg_mesh_set_inds            (      skg_mesh_t *mesh, const skg_buffer_t *ind_buffer);
SKG_API void                skg_mesh_bind                (const skg_mesh_t *mesh);
SKG_API void                skg_mesh_destroy             (      skg_mesh_t *mesh);

SKG_API skg_shader_stage_t  skg_shader_stage_create      (const void *shader_data, size_t shader_size, skg_stage_ type);
SKG_API void                skg_shader_stage_destroy     (skg_shader_stage_t *stage);

SKG_API skg_shader_t        skg_shader_create_file       (const char *sks_filename);
SKG_API skg_shader_t        skg_shader_create_memory     (const void *sks_memory, size_t sks_memory_size);
SKG_API skg_shader_t        skg_shader_create_manual     (skg_shader_meta_t *meta, skg_shader_stage_t v_shader, skg_shader_stage_t p_shader, skg_shader_stage_t c_shader);
SKG_API bool                skg_shader_is_valid          (const skg_shader_t *shader);
SKG_API void                skg_shader_compute_bind      (const skg_shader_t *shader);
SKG_API skg_bind_t          skg_shader_get_bind          (const skg_shader_t *shader, const char *name);
SKG_API int32_t             skg_shader_get_var_count     (const skg_shader_t *shader);
SKG_API int32_t             skg_shader_get_var_index     (const skg_shader_t *shader, const char *name);
SKG_API int32_t             skg_shader_get_var_index_h   (const skg_shader_t *shader, uint64_t name_hash);
SKG_API const skg_shader_var_t *skg_shader_get_var_info  (const skg_shader_t *shader, int32_t var_index);
SKG_API void                skg_shader_destroy           (      skg_shader_t *shader);

SKG_API skg_pipeline_t      skg_pipeline_create          (skg_shader_t *shader);
SKG_API void                skg_pipeline_bind            (const skg_pipeline_t *pipeline);
SKG_API void                skg_pipeline_set_transparency(      skg_pipeline_t *pipeline, skg_transparency_ transparency);
SKG_API skg_transparency_   skg_pipeline_get_transparency(const skg_pipeline_t *pipeline);
SKG_API void                skg_pipeline_set_cull        (      skg_pipeline_t *pipeline, skg_cull_ cull);
SKG_API skg_cull_           skg_pipeline_get_cull        (const skg_pipeline_t *pipeline);
SKG_API void                skg_pipeline_set_wireframe   (      skg_pipeline_t *pipeline, bool wireframe);
SKG_API bool                skg_pipeline_get_wireframe   (const skg_pipeline_t *pipeline);
SKG_API void                skg_pipeline_set_depth_write (      skg_pipeline_t *pipeline, bool write);
SKG_API bool                skg_pipeline_get_depth_write (const skg_pipeline_t *pipeline);
SKG_API void                skg_pipeline_set_depth_test  (      skg_pipeline_t *pipeline, skg_depth_test_ test);
SKG_API skg_depth_test_     skg_pipeline_get_depth_test  (const skg_pipeline_t *pipeline);
SKG_API void                skg_pipeline_set_scissor     (      skg_pipeline_t *pipeline, bool enable);
SKG_API bool                skg_pipeline_get_scissor     (const skg_pipeline_t *pipeline);
SKG_API void                skg_pipeline_destroy         (      skg_pipeline_t *pipeline);

SKG_API skg_swapchain_t     skg_swapchain_create         (void *hwnd, skg_tex_fmt_ format, skg_tex_fmt_ depth_format, int32_t requested_width, int32_t requested_height);
SKG_API void                skg_swapchain_resize         (      skg_swapchain_t *swapchain, int32_t width, int32_t height);
SKG_API void                skg_swapchain_present        (      skg_swapchain_t *swapchain);
SKG_API void                skg_swapchain_bind           (      skg_swapchain_t *swapchain);
SKG_API void                skg_swapchain_destroy        (      skg_swapchain_t *swapchain);

SKG_API skg_tex_t           skg_tex_create_from_existing (void *native_tex, skg_tex_type_ type, skg_tex_fmt_ format, int32_t width, int32_t height, int32_t array_count);
SKG_API skg_tex_t           skg_tex_create_from_layer    (void *native_tex, skg_tex_type_ type, skg_tex_fmt_ format, int32_t width, int32_t height, int32_t array_layer);
SKG_API skg_tex_t           skg_tex_create               (skg_tex_type_ type, skg_use_ use, skg_tex_fmt_ format, skg_mip_ mip_maps);
SKG_API bool                skg_tex_is_valid             (const skg_tex_t *tex);
SKG_API void                skg_tex_copy_to              (const skg_tex_t *tex, skg_tex_t *destination);
SKG_API void                skg_tex_copy_to_swapchain    (const skg_tex_t *tex, skg_swapchain_t *destination);
SKG_API void                skg_tex_attach_depth         (      skg_tex_t *tex, skg_tex_t *depth);
SKG_API void                skg_tex_settings             (      skg_tex_t *tex, skg_tex_address_ address, skg_tex_sample_ sample, int32_t anisotropy);
SKG_API void                skg_tex_set_contents         (      skg_tex_t *tex, const void *data, int32_t width, int32_t height);
SKG_API void                skg_tex_set_contents_arr     (      skg_tex_t *tex, const void **data_frames, int32_t data_frame_count, int32_t width, int32_t height, int32_t multisample);
SKG_API bool                skg_tex_get_contents         (      skg_tex_t *tex, void *ref_data, size_t data_size);
SKG_API bool                skg_tex_get_mip_contents     (      skg_tex_t *tex, int32_t mip_level, void *ref_data, size_t data_size);
SKG_API bool                skg_tex_get_mip_contents_arr (      skg_tex_t *tex, int32_t mip_level, int32_t arr_index, void *ref_data, size_t data_size);
SKG_API void                skg_tex_bind                 (const skg_tex_t *tex, skg_bind_t bind);
SKG_API void                skg_tex_clear                (skg_bind_t bind);
SKG_API void                skg_tex_target_bind          (      skg_tex_t *render_target);
SKG_API skg_tex_t          *skg_tex_target_get           ();
SKG_API void                skg_tex_destroy              (      skg_tex_t *tex);
SKG_API int64_t             skg_tex_fmt_to_native        (skg_tex_fmt_ format);
SKG_API skg_tex_fmt_        skg_tex_fmt_from_native      (int64_t      format);
SKG_API uint32_t            skg_tex_fmt_size             (skg_tex_fmt_ format);


///////////////////////////////////////////
// API independant functions             //
///////////////////////////////////////////

typedef enum {
	skg_shader_lang_hlsl,
	skg_shader_lang_spirv,
	skg_shader_lang_glsl,
	skg_shader_lang_glsl_es,
	skg_shader_lang_glsl_web,
} skg_shader_lang_;

typedef struct {
	skg_shader_lang_ language;
	skg_stage_       stage;
	uint32_t         code_size;
	void            *code;
} skg_shader_file_stage_t;

typedef struct {
	skg_shader_meta_t       *meta;
	uint32_t                 stage_count;
	skg_shader_file_stage_t *stages;
} skg_shader_file_t;

///////////////////////////////////////////

SKG_API void                    skg_log                        (skg_log_ level, const char *text);
SKG_API bool                    skg_read_file                  (const char *filename, void **out_data, size_t *out_size);
SKG_API uint64_t                skg_hash                       (const char *string);
SKG_API uint32_t                skg_mip_count                  (int32_t width, int32_t height);
SKG_API void                    skg_mip_dimensions             (int32_t width, int32_t height, int32_t mip_level, int32_t *out_width, int32_t *out_height);

SKG_API skg_color32_t           skg_col_hsv32                  (float hue, float saturation, float value, float alpha);
SKG_API skg_color128_t          skg_col_hsv128                 (float hue, float saturation, float value, float alpha);
SKG_API skg_color32_t           skg_col_hsl32                  (float hue, float saturation, float lightness, float alpha);
SKG_API skg_color128_t          skg_col_hsl128                 (float hue, float saturation, float lightness, float alpha);
SKG_API skg_color32_t           skg_col_hcy32                  (float hue, float chroma, float lightness, float alpha);
SKG_API skg_color128_t          skg_col_hcy128                 (float hue, float chroma, float lightness, float alpha);
SKG_API skg_color32_t           skg_col_lch32                  (float hue, float chroma, float lightness, float alpha);
SKG_API skg_color128_t          skg_col_lch128                 (float hue, float chroma, float lightness, float alpha);
SKG_API skg_color32_t           skg_col_helix32                (float hue, float saturation, float lightness, float alpha);
SKG_API skg_color128_t          skg_col_helix128               (float hue, float saturation, float lightness, float alpha);
SKG_API skg_color32_t           skg_col_jab32                  (float j, float a, float b, float alpha);
SKG_API skg_color128_t          skg_col_jab128                 (float j, float a, float b, float alpha);
SKG_API skg_color32_t           skg_col_jsl32                  (float hue, float saturation, float lightness, float alpha);
SKG_API skg_color128_t          skg_col_jsl128                 (float hue, float saturation, float lightness, float alpha);
SKG_API skg_color32_t           skg_col_lab32                  (float l, float a, float b, float alpha);
SKG_API skg_color128_t          skg_col_lab128                 (float l, float a, float b, float alpha);
SKG_API skg_color128_t          skg_col_rgb_to_lab128          (skg_color128_t rgb);
SKG_API skg_color128_t          skg_col_to_srgb                (skg_color128_t rgb_linear);
SKG_API skg_color128_t          skg_col_to_linear              (skg_color128_t srgb);

SKG_API bool                    skg_shader_file_verify         (const void *file_memory, size_t file_size, uint16_t *out_version, char *out_name, size_t out_name_size);
SKG_API bool                    skg_shader_file_load_memory    (const void *file_memory, size_t file_size, skg_shader_file_t *out_file);
SKG_API bool                    skg_shader_file_load           (const char *file, skg_shader_file_t *out_file);
SKG_API skg_shader_stage_t      skg_shader_file_create_stage   (const skg_shader_file_t *file, skg_stage_ stage);
SKG_API void                    skg_shader_file_destroy        (      skg_shader_file_t *file);

SKG_API skg_bind_t              skg_shader_meta_get_bind       (const skg_shader_meta_t *meta, const char *name);
SKG_API int32_t                 skg_shader_meta_get_var_count  (const skg_shader_meta_t *meta);
SKG_API int32_t                 skg_shader_meta_get_var_index  (const skg_shader_meta_t *meta, const char *name);
SKG_API int32_t                 skg_shader_meta_get_var_index_h(const skg_shader_meta_t *meta, uint64_t name_hash);
SKG_API const skg_shader_var_t *skg_shader_meta_get_var_info   (const skg_shader_meta_t *meta, int32_t var_index);
SKG_API void                    skg_shader_meta_reference      (skg_shader_meta_t *meta);
SKG_API void                    skg_shader_meta_release        (skg_shader_meta_t *meta);
///////////////////////////////////////////
// Implementations!                      //
///////////////////////////////////////////

#ifdef SKG_IMPL

#ifdef SKG_DIRECT3D11
///////////////////////////////////////////
// Direct3D11 Implementation             //
///////////////////////////////////////////

#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"Dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#include <d3d11.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <math.h>

#include <stdio.h>

///////////////////////////////////////////

ID3D11Device            *d3d_device      = nullptr;
ID3D11DeviceContext     *d3d_context     = nullptr;
ID3D11InfoQueue         *d3d_info        = nullptr;
ID3D11RasterizerState   *d3d_rasterstate = nullptr;
ID3D11DepthStencilState *d3d_depthstate  = nullptr;
skg_tex_t               *d3d_active_rendertarget = nullptr;

///////////////////////////////////////////

bool skg_tex_make_view(skg_tex_t *tex, uint32_t mip_count, uint32_t array_start, bool use_in_shader);

template <typename T>
void skg_downsample_1(T *data, int32_t width, int32_t height, T **out_data, int32_t *out_width, int32_t *out_height);
template <typename T>
void skg_downsample_4(T *data, int32_t width, int32_t height, T **out_data, int32_t *out_width, int32_t *out_height);

///////////////////////////////////////////

int32_t skg_init(const char *, void *adapter_id) {
	UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
	creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
	skg_log(skg_log_info, "Requesting debug Direct3D context.");
#endif

	// Find the right adapter to use:
	IDXGIAdapter1 *final_adapter = nullptr;
	IDXGIAdapter1 *curr_adapter  = nullptr;
	IDXGIFactory1 *dxgi_factory  = nullptr;
	int            curr          = 0;
	SIZE_T         video_mem     = 0;
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)(&dxgi_factory));
	while (dxgi_factory->EnumAdapters1(curr++, &curr_adapter) == S_OK) {
		DXGI_ADAPTER_DESC1 adapterDesc;
		curr_adapter->GetDesc1(&adapterDesc);

		// By default, we pick the adapter that has the most available memory
		if (adapterDesc.DedicatedVideoMemory > video_mem) {
			video_mem = adapterDesc.DedicatedVideoMemory;
			if (final_adapter != nullptr) final_adapter->Release();
			final_adapter = curr_adapter;
			final_adapter->AddRef();
		}

		// If the user asks for a specific device though, return it right away!
		if (adapter_id != nullptr && memcmp(&adapterDesc.AdapterLuid, adapter_id, sizeof(LUID)) == 0) {
			if (final_adapter != nullptr) final_adapter->Release();
			final_adapter = curr_adapter;
			final_adapter->AddRef();
			break;
		}
		curr_adapter->Release();
	}
	dxgi_factory->Release();

	// Create the interface to the graphics card
	D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	HRESULT           hr               = D3D11CreateDevice(final_adapter, final_adapter == nullptr ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN, 0, creation_flags, feature_levels, _countof(feature_levels), D3D11_SDK_VERSION, &d3d_device, nullptr, &d3d_context);
	if (FAILED(hr)) {

		// Message that we failed to initialize with the selected adapter.
		char d3d_info_txt[128];
		if (final_adapter != nullptr) {
			DXGI_ADAPTER_DESC1 final_adapter_info;
			final_adapter->GetDesc1(&final_adapter_info);
			snprintf(d3d_info_txt, sizeof(d3d_info_txt), "Failed starting Direct3D 11 adapter '%ls': 0x%08X", &final_adapter_info.Description, hr);
			final_adapter->Release();
		} else {
			snprintf(d3d_info_txt, sizeof(d3d_info_txt), "Failed starting Direct3D 11 adapter 'Default adapter': 0x%08X", hr);
		}
		skg_log(skg_log_critical, d3d_info_txt);

		// Get a human readable description of that error message.
		char *error_text = NULL;
		FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(char*)&error_text, 0,
			NULL);
		skg_log(skg_log_critical, error_text);
		LocalFree(error_text);

		return 0;
	}

	// Notify what device and API we're using
	if (final_adapter != nullptr) {
		DXGI_ADAPTER_DESC1 final_adapter_info;
		final_adapter->GetDesc1(&final_adapter_info);
		char d3d_info_txt[128];
		snprintf(d3d_info_txt, sizeof(d3d_info_txt), "Using Direct3D 11: %ls", &final_adapter_info.Description);
		skg_log(skg_log_info, d3d_info_txt);
		final_adapter->Release();
	} else {
		skg_log(skg_log_info, "Using Direct3D 11: default device");
	}

	// Hook into debug information
	ID3D11Debug *d3d_debug = nullptr;
	if (SUCCEEDED(d3d_device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3d_debug))) {
		d3d_info = nullptr;
		if (SUCCEEDED(d3d_debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3d_info))) {
			D3D11_MESSAGE_ID hide[] = {
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				(D3D11_MESSAGE_ID)351,
			};

			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3d_info->ClearStorageFilter();
			d3d_info->AddStorageFilterEntries(&filter);
		}
		d3d_debug->Release();
	}

	D3D11_RASTERIZER_DESC desc_rasterizer = {};
	desc_rasterizer.FillMode = D3D11_FILL_SOLID;
	desc_rasterizer.CullMode = D3D11_CULL_BACK;
	desc_rasterizer.FrontCounterClockwise = true;
	desc_rasterizer.DepthClipEnable       = true;
	desc_rasterizer.MultisampleEnable     = true;
	d3d_device->CreateRasterizerState(&desc_rasterizer, &d3d_rasterstate);
	
	D3D11_DEPTH_STENCIL_DESC desc_depthstate = {};
	desc_depthstate.DepthEnable    = true;
	desc_depthstate.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc_depthstate.DepthFunc      = D3D11_COMPARISON_LESS;
	desc_depthstate.StencilEnable    = false;
	desc_depthstate.StencilReadMask  = 0xFF;
	desc_depthstate.StencilWriteMask = 0xFF;
	desc_depthstate.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	desc_depthstate.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
	desc_depthstate.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	desc_depthstate.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
	d3d_device->CreateDepthStencilState(&desc_depthstate, &d3d_depthstate);

	// This sets the default rasterize, depth_stencil, topology mode, etc.
	skg_draw_begin();

	return 1;
}

///////////////////////////////////////////

void skg_shutdown() {
	if (d3d_rasterstate) { d3d_rasterstate->Release(); d3d_rasterstate = nullptr; }
	if (d3d_depthstate ) { d3d_depthstate ->Release(); d3d_depthstate  = nullptr; }
	if (d3d_info       ) { d3d_info       ->Release(); d3d_info        = nullptr; }
	if (d3d_context    ) { d3d_context    ->Release(); d3d_context     = nullptr; }
	if (d3d_device     ) { d3d_device     ->Release(); d3d_device      = nullptr; }
}

///////////////////////////////////////////

void skg_draw_begin() {
	d3d_context->RSSetState            (d3d_rasterstate);
	d3d_context->OMSetDepthStencilState(d3d_depthstate, 1);
	d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

///////////////////////////////////////////

skg_platform_data_t skg_get_platform_data() {
	skg_platform_data_t result = {};
	result._d3d11_device = d3d_device;

	return result;
}

///////////////////////////////////////////

bool skg_capability(skg_cap_ capability) {
	switch (capability) {
	case skg_cap_tex_layer_select: {
		D3D11_FEATURE_DATA_D3D11_OPTIONS3 options;
		d3d_device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS3, &options, sizeof(options));
		return options.VPAndRTArrayIndexFromAnyShaderFeedingRasterizer;
	} break;
	case skg_cap_wireframe: return true;
	default: return false;
	}
}

///////////////////////////////////////////

void skg_tex_target_bind(skg_tex_t *render_target) {
	d3d_active_rendertarget = render_target;

	if (render_target == nullptr) {
		d3d_context->OMSetRenderTargets(0, nullptr, nullptr);
		return;
	}
	if (render_target->type != skg_tex_type_rendertarget)
		return;

	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.f, 0.f, (float)render_target->width, (float)render_target->height);
	d3d_context->RSSetViewports(1, &viewport);
	d3d_context->OMSetRenderTargets(1, &render_target->_target_view, render_target->_depth_view);
}

///////////////////////////////////////////

void skg_target_clear(bool depth, const float *clear_color_4) {
	if (!d3d_active_rendertarget) return;
	if (clear_color_4)
		d3d_context->ClearRenderTargetView(d3d_active_rendertarget->_target_view, clear_color_4);
	if (depth && d3d_active_rendertarget->_depth_view) {
		UINT clear_flags = D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL;
		d3d_context->ClearDepthStencilView(d3d_active_rendertarget->_depth_view, clear_flags, 1.0f, 0);
	}
}

///////////////////////////////////////////

skg_tex_t *skg_tex_target_get() {
	return d3d_active_rendertarget;
}

///////////////////////////////////////////

void skg_draw(int32_t index_start, int32_t index_base, int32_t index_count, int32_t instance_count) {
	d3d_context->DrawIndexedInstanced(index_count, instance_count, index_start, index_base, 0);
}

///////////////////////////////////////////

void skg_compute(uint32_t thread_count_x, uint32_t thread_count_y, uint32_t thread_count_z) {
	d3d_context->Dispatch(thread_count_x, thread_count_y, thread_count_z);
}

///////////////////////////////////////////

void skg_viewport(const int32_t *xywh) {
	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT((float)xywh[0], (float)xywh[1], (float)xywh[2], (float)xywh[3]);
	d3d_context->RSSetViewports(1, &viewport);
}

///////////////////////////////////////////

void skg_viewport_get(int32_t *out_xywh) {
	uint32_t       count = 1;
	D3D11_VIEWPORT viewport;
	d3d_context->RSGetViewports(&count, &viewport);
	out_xywh[0] = (int32_t)viewport.TopLeftX;
	out_xywh[1] = (int32_t)viewport.TopLeftY;
	out_xywh[2] = (int32_t)viewport.Width;
	out_xywh[3] = (int32_t)viewport.Height;
}

///////////////////////////////////////////

void skg_scissor(const int32_t *xywh) {
	D3D11_RECT rect = {xywh[0], xywh[1], xywh[0]+xywh[2], xywh[1]+xywh[3]};
	d3d_context->RSSetScissorRects(1, &rect);
}

///////////////////////////////////////////

skg_buffer_t skg_buffer_create(const void *data, uint32_t size_count, uint32_t size_stride, skg_buffer_type_ type, skg_use_ use) {
	skg_buffer_t result = {};
	result.use    = use;
	result.type   = type;
	result.stride = size_stride;

	D3D11_SUBRESOURCE_DATA buffer_data = { data };
	D3D11_BUFFER_DESC      buffer_desc = {};
	buffer_desc.ByteWidth           = size_count * size_stride;
	buffer_desc.StructureByteStride = size_stride;
	buffer_desc.Usage               = D3D11_USAGE_DEFAULT;

	if (use & skg_use_dynamic) {
		buffer_desc.Usage          = D3D11_USAGE_DYNAMIC;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	if (use & skg_use_compute_write || use & skg_use_compute_read) {
		buffer_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE; 
		buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	}

	switch (type) {
	case skg_buffer_type_vertex:   buffer_desc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;   break;
	case skg_buffer_type_index:    buffer_desc.BindFlags |= D3D11_BIND_INDEX_BUFFER;    break;
	case skg_buffer_type_constant: buffer_desc.BindFlags |= D3D11_BIND_CONSTANT_BUFFER; break;
	case skg_buffer_type_compute:  break;
	}
	if (FAILED(d3d_device->CreateBuffer(&buffer_desc, data == nullptr ? nullptr : &buffer_data, &result._buffer))) {
		skg_log(skg_log_critical, "CreateBuffer failed!");
		return {};
	}

	if (use & skg_use_compute_write) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC view = {};
		view.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		view.Format        = DXGI_FORMAT_UNKNOWN;
		view.Buffer.FirstElement = 0;
		view.Buffer.NumElements  = size_count; 

		if(FAILED(d3d_device->CreateUnorderedAccessView( result._buffer, &view, &result._unordered ))) {
			skg_log(skg_log_critical, "CreateUnorderedAccessView failed!");
			skg_buffer_destroy(&result);
			return {};
		}
	} 
	if (use & skg_use_compute_read) {
		D3D11_SHADER_RESOURCE_VIEW_DESC view = {};
		view.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		view.Format        = DXGI_FORMAT_UNKNOWN;
		view.BufferEx.FirstElement = 0;
		view.BufferEx.NumElements  = size_count;

		if (FAILED(d3d_device->CreateShaderResourceView(result._buffer, &view, &result._resource))) {
			skg_log(skg_log_critical, "CreateShaderResourceView failed!");
			skg_buffer_destroy(&result);
			return {};
		}
	} 
	return result;
}

///////////////////////////////////////////

bool skg_buffer_is_valid(const skg_buffer_t *buffer) {
	return buffer->_buffer != nullptr;
}

///////////////////////////////////////////

void skg_buffer_set_contents(skg_buffer_t *buffer, const void *data, uint32_t size_bytes) {
	if (buffer->use != skg_use_dynamic) {
		skg_log(skg_log_warning, "Attempting to dynamically set contents of a static buffer!");
		return;
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	if (SUCCEEDED(d3d_context->Map(buffer->_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource))) {
		memcpy(resource.pData, data, size_bytes);
		d3d_context->Unmap(buffer->_buffer, 0);
	} else {
		skg_log(skg_log_critical, "Failed to set contents of buffer, may not be using a writeable buffer type?");
	}
}

///////////////////////////////////////////

void skg_buffer_get_contents(const skg_buffer_t *buffer, void *ref_buffer, uint32_t buffer_size) {
	ID3D11Buffer* cpu_buff = nullptr;

	D3D11_BUFFER_DESC desc = {};
	buffer->_buffer->GetDesc( &desc );
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage          = D3D11_USAGE_STAGING;
	desc.BindFlags      = 0;
	desc.MiscFlags      = 0;
	if (FAILED(d3d_device->CreateBuffer(&desc, nullptr, &cpu_buff)) ) {
		skg_log(skg_log_critical, "Couldn't create a tep buffer for copy!");
		return;
	}
	d3d_context->CopyResource( cpu_buff, buffer->_buffer );

	D3D11_MAPPED_SUBRESOURCE resource;
	if (SUCCEEDED(d3d_context->Map(cpu_buff, 0, D3D11_MAP_READ, 0, &resource))) {
		memcpy(ref_buffer, resource.pData, buffer_size);
		d3d_context->Unmap(cpu_buff, 0);
	} else {
		memset(ref_buffer, 0, buffer_size);
		skg_log(skg_log_critical, "Failed to get contents of buffer!");
	}
	cpu_buff->Release();
}

///////////////////////////////////////////

void skg_buffer_clear(skg_bind_t bind) {
	if (bind.register_type == skg_register_readwrite) {
		ID3D11UnorderedAccessView *null_uav = nullptr;
		d3d_context->CSSetUnorderedAccessViews(bind.slot, 1, &null_uav, nullptr);
	}
}

///////////////////////////////////////////
void skg_buffer_bind(const skg_buffer_t *buffer, skg_bind_t bind, uint32_t offset) {
	switch (bind.register_type) {
	case skg_register_index:  d3d_context->IASetIndexBuffer(buffer->_buffer, DXGI_FORMAT_R32_UINT, offset); break;
	case skg_register_vertex: d3d_context->IASetVertexBuffers(bind.slot, 1, &buffer->_buffer, &buffer->stride, &offset); break;
	case skg_register_constant: {
#if !defined(NDEBUG)
		if (buffer->type != skg_buffer_type_constant) skg_log(skg_log_critical, "Attempting to bind the wrong buffer type to a constant register! Use skg_buffer_type_constant");
#endif
		if (bind.stage_bits & skg_stage_vertex ) d3d_context->VSSetConstantBuffers(bind.slot, 1, &buffer->_buffer);
		if (bind.stage_bits & skg_stage_pixel  ) d3d_context->PSSetConstantBuffers(bind.slot, 1, &buffer->_buffer);
		if (bind.stage_bits & skg_stage_compute) d3d_context->CSSetConstantBuffers(bind.slot, 1, &buffer->_buffer);
	} break;
	case skg_register_resource: {
#if !defined(NDEBUG)
		if (buffer->type != skg_buffer_type_compute) skg_log(skg_log_critical, "Attempting to bind the wrong buffer type to a resource register! Use skg_buffer_type_compute");
#endif
		if (bind.stage_bits & skg_stage_vertex ) d3d_context->VSSetShaderResources(bind.slot, 1, &buffer->_resource);
		if (bind.stage_bits & skg_stage_pixel  ) d3d_context->PSSetShaderResources(bind.slot, 1, &buffer->_resource);
		if (bind.stage_bits & skg_stage_compute) d3d_context->CSSetShaderResources(bind.slot, 1, &buffer->_resource);
	} break;
	case skg_register_readwrite: {
#if !defined(NDEBUG)
		if (buffer->type != skg_buffer_type_compute) skg_log(skg_log_critical, "Attempting to bind the wrong buffer type to a UAV register! Use skg_buffer_type_compute");
#endif
		if (bind.stage_bits & skg_stage_compute) d3d_context->CSSetUnorderedAccessViews(bind.slot, 1, &buffer->_unordered, nullptr);
	} break;
	}
}

///////////////////////////////////////////

void skg_buffer_destroy(skg_buffer_t *buffer) {
	if (buffer->_buffer) buffer->_buffer->Release();
	*buffer = {};
}

///////////////////////////////////////////

skg_mesh_t skg_mesh_create(const skg_buffer_t *vert_buffer, const skg_buffer_t *ind_buffer) {
	skg_mesh_t result = {};
	result._ind_buffer  = ind_buffer  ? ind_buffer ->_buffer : nullptr;
	result._vert_buffer = vert_buffer ? vert_buffer->_buffer : nullptr;
	if (result._ind_buffer ) result._ind_buffer ->AddRef();
	if (result._vert_buffer) result._vert_buffer->AddRef();

	return result;
}

///////////////////////////////////////////

void skg_mesh_set_verts(skg_mesh_t *mesh, const skg_buffer_t *vert_buffer) {
	if (mesh->_vert_buffer) mesh->_vert_buffer->Release();
	mesh->_vert_buffer = vert_buffer->_buffer;
	if (mesh->_vert_buffer) mesh->_vert_buffer->AddRef();
}

///////////////////////////////////////////

void skg_mesh_set_inds(skg_mesh_t *mesh, const skg_buffer_t *ind_buffer) {
	if (mesh->_ind_buffer) mesh->_ind_buffer->Release();
	mesh->_ind_buffer = ind_buffer->_buffer;
	if (mesh->_ind_buffer) mesh->_ind_buffer->AddRef();
}

///////////////////////////////////////////

void skg_mesh_bind(const skg_mesh_t *mesh) {
	UINT strides[] = { sizeof(skg_vert_t) };
	UINT offsets[] = { 0 };
	d3d_context->IASetVertexBuffers(0, 1, &mesh->_vert_buffer, strides, offsets);
	d3d_context->IASetIndexBuffer  (mesh->_ind_buffer, DXGI_FORMAT_R32_UINT, 0);
}

///////////////////////////////////////////

void skg_mesh_destroy(skg_mesh_t *mesh) {
	if (mesh->_ind_buffer ) mesh->_ind_buffer ->Release();
	if (mesh->_vert_buffer) mesh->_vert_buffer->Release();
	*mesh = {};
}

///////////////////////////////////////////

#include <stdio.h>
skg_shader_stage_t skg_shader_stage_create(const void *file_data, size_t shader_size, skg_stage_ type) {
	skg_shader_stage_t result = {};
	result.type = type;

	ID3DBlob   *compiled = nullptr;
	const void *buffer;
	size_t      buffer_size;
	HRESULT     hr = E_FAIL;
	if (shader_size >= 4 && memcmp(file_data, "DXBC", 4) == 0) {
		buffer      = file_data;
		buffer_size = shader_size;
	} else {
		DWORD flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#if !defined(NDEBUG)
		flags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		// Compile a text HLSL shader file to bytecode
		ID3DBlob *errors;
		const char *entrypoint = "", *target = "";
		switch (type) {
			case skg_stage_vertex:  entrypoint = "vs"; target = "vs_5_0"; break;
			case skg_stage_pixel:   entrypoint = "ps"; target = "ps_5_0"; break;
			case skg_stage_compute: entrypoint = "cs"; target = "cs_5_0"; break; }
		hr = D3DCompile(file_data, shader_size, nullptr, nullptr, nullptr, entrypoint, target, flags, 0, &compiled, &errors);
		if (errors) {
			skg_log(skg_log_warning, "D3DCompile errors:");
			skg_log(skg_log_warning, (char*)errors->GetBufferPointer());
			errors->Release();
		}
		if (FAILED(hr)) {
			char text[128];
			snprintf(text, sizeof(text), "D3DCompile failed: 0x%X", hr);
			skg_log(skg_log_warning, text);

			if (compiled) compiled->Release();
			return {};
		}

		buffer      = compiled->GetBufferPointer();
		buffer_size = compiled->GetBufferSize();
	}

	// Create a shader from HLSL bytecode
	hr = E_FAIL;
	switch (type) {
	case skg_stage_vertex  : hr = d3d_device->CreateVertexShader (buffer, buffer_size, nullptr, (ID3D11VertexShader **)&result._shader); break;
	case skg_stage_pixel   : hr = d3d_device->CreatePixelShader  (buffer, buffer_size, nullptr, (ID3D11PixelShader  **)&result._shader); break;
	case skg_stage_compute : hr = d3d_device->CreateComputeShader(buffer, buffer_size, nullptr, (ID3D11ComputeShader**)&result._shader); break;
	}
	if (FAILED(hr)) {
		char text[128];
		snprintf(text, sizeof(text), "CreateXShader failed: 0x%X", hr);
		skg_log(skg_log_warning, text);

		if (compiled) compiled->Release();
		if (result._shader) {
			switch (type) {
			case skg_stage_vertex:  ((ID3D11VertexShader *)result._shader)->Release(); break;
			case skg_stage_pixel:   ((ID3D11PixelShader  *)result._shader)->Release(); break;
			case skg_stage_compute: ((ID3D11ComputeShader*)result._shader)->Release(); break;
			}
		}
		return {};
	}

	if (type == skg_stage_vertex) {
		// Describe how our mesh is laid out in memory
		D3D11_INPUT_ELEMENT_DESC vert_desc[] = {
			{"SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR" ,      0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0} };
		d3d_device->CreateInputLayout(vert_desc, (UINT)_countof(vert_desc), buffer, buffer_size, &result._layout);
	}
	if (compiled) compiled->Release();

	return result;
}

///////////////////////////////////////////

void skg_shader_stage_destroy(skg_shader_stage_t *shader) {
	switch(shader->type) {
	case skg_stage_vertex  : ((ID3D11VertexShader *)shader->_shader)->Release(); shader->_layout->Release(); break;
	case skg_stage_pixel   : ((ID3D11PixelShader  *)shader->_shader)->Release(); break;
	case skg_stage_compute : ((ID3D11ComputeShader*)shader->_shader)->Release(); break;
	}
}

///////////////////////////////////////////
// skg_shader_t                          //
///////////////////////////////////////////

skg_shader_t skg_shader_create_manual(skg_shader_meta_t *meta, skg_shader_stage_t v_shader, skg_shader_stage_t p_shader, skg_shader_stage_t c_shader) {
	if (v_shader._shader == nullptr && p_shader._shader == nullptr && c_shader._shader == nullptr) {
		char text[290];
		snprintf(text, sizeof(text), "Shader '%s' has no valid stages!", meta->name);
		skg_log(skg_log_warning, text);
		return {};
	}

	skg_shader_t result = {};
	result.meta    = meta;
	if (v_shader._shader) result._vertex  = (ID3D11VertexShader *)v_shader._shader;
	if (v_shader._layout) result._layout  = v_shader._layout;
	if (p_shader._shader) result._pixel   = (ID3D11PixelShader  *)p_shader._shader;
	if (c_shader._shader) result._compute = (ID3D11ComputeShader*)c_shader._shader;
	skg_shader_meta_reference(result.meta);
	if (result._vertex ) result._vertex ->AddRef();
	if (result._layout ) result._layout ->AddRef();
	if (result._pixel  ) result._pixel  ->AddRef();
	if (result._compute) result._compute->AddRef();

	return result;
}

///////////////////////////////////////////

bool skg_shader_is_valid(const skg_shader_t *shader) {
	return shader->meta
		&& (shader->_vertex && shader->_pixel) || shader->_compute;
}

///////////////////////////////////////////

void skg_shader_compute_bind(const skg_shader_t *shader) {
	if (shader) d3d_context->CSSetShader(shader->_compute, nullptr, 0);
	else        d3d_context->CSSetShader(nullptr, nullptr, 0);
}

///////////////////////////////////////////

void skg_shader_destroy(skg_shader_t *shader) {
	skg_shader_meta_release(shader->meta);
	if (shader->_vertex ) shader->_vertex ->Release();
	if (shader->_layout ) shader->_layout ->Release();
	if (shader->_pixel  ) shader->_pixel  ->Release();
	if (shader->_compute) shader->_compute->Release();
	*shader = {};
}

///////////////////////////////////////////
// skg_pipeline                          //
///////////////////////////////////////////

void skg_pipeline_update_blend(skg_pipeline_t *pipeline) {
	if (pipeline->_blend) pipeline->_blend->Release();

	D3D11_BLEND_DESC desc_blend = {};
	desc_blend.AlphaToCoverageEnable  = false;
	desc_blend.IndependentBlendEnable = false;
	desc_blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	switch (pipeline->transparency) {
	case skg_transparency_blend:
		desc_blend.RenderTarget[0].BlendEnable           = true;
		desc_blend.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
		desc_blend.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
		desc_blend.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
		desc_blend.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
		desc_blend.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ONE;
		desc_blend.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_MAX;
		break;
	case skg_transparency_add:
		desc_blend.RenderTarget[0].BlendEnable           = true;
		desc_blend.RenderTarget[0].SrcBlend              = D3D11_BLEND_ONE;
		desc_blend.RenderTarget[0].DestBlend             = D3D11_BLEND_ONE;
		desc_blend.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
		desc_blend.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
		desc_blend.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ONE;
		desc_blend.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
		break;
	}

	d3d_device->CreateBlendState(&desc_blend, &pipeline->_blend);
}

///////////////////////////////////////////

void skg_pipeline_update_rasterizer(skg_pipeline_t *pipeline) {
	if (pipeline->_rasterize) pipeline->_rasterize->Release();

	D3D11_RASTERIZER_DESC desc_rasterizer = {};
	desc_rasterizer.FillMode              = pipeline->wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	desc_rasterizer.FrontCounterClockwise = true;
	desc_rasterizer.ScissorEnable         = pipeline->scissor;
	desc_rasterizer.DepthClipEnable       = true;
	switch (pipeline->cull) {
	case skg_cull_none:  desc_rasterizer.CullMode = D3D11_CULL_NONE;  break;
	case skg_cull_front: desc_rasterizer.CullMode = D3D11_CULL_FRONT; break;
	case skg_cull_back:  desc_rasterizer.CullMode = D3D11_CULL_BACK;  break;
	}
	d3d_device->CreateRasterizerState(&desc_rasterizer, &pipeline->_rasterize);
}

///////////////////////////////////////////

void skg_pipeline_update_depth(skg_pipeline_t *pipeline) {
	if (pipeline->_depth) pipeline->_depth->Release();

	D3D11_COMPARISON_FUNC comparison = D3D11_COMPARISON_LESS;
	switch (pipeline->depth_test) {
	case skg_depth_test_always:        comparison = D3D11_COMPARISON_ALWAYS;        break;
	case skg_depth_test_equal:         comparison = D3D11_COMPARISON_EQUAL;         break;
	case skg_depth_test_greater:       comparison = D3D11_COMPARISON_GREATER;       break;
	case skg_depth_test_greater_or_eq: comparison = D3D11_COMPARISON_GREATER_EQUAL; break;
	case skg_depth_test_less:          comparison = D3D11_COMPARISON_LESS;          break;
	case skg_depth_test_less_or_eq:    comparison = D3D11_COMPARISON_LESS_EQUAL;    break;
	case skg_depth_test_never:         comparison = D3D11_COMPARISON_NEVER;         break;
	case skg_depth_test_not_equal:     comparison = D3D11_COMPARISON_NOT_EQUAL;     break;
	}

	D3D11_DEPTH_STENCIL_DESC desc_depthstate = {};
	desc_depthstate.DepthEnable    = pipeline->depth_write != false || pipeline->depth_test != skg_depth_test_always;
	desc_depthstate.DepthWriteMask = pipeline->depth_write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	desc_depthstate.DepthFunc      = comparison;
	desc_depthstate.StencilEnable    = false;
	desc_depthstate.StencilReadMask  = 0xFF;
	desc_depthstate.StencilWriteMask = 0xFF;
	desc_depthstate.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	desc_depthstate.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
	desc_depthstate.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	desc_depthstate.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
	d3d_device->CreateDepthStencilState(&desc_depthstate, &pipeline->_depth);
}

///////////////////////////////////////////

skg_pipeline_t skg_pipeline_create(skg_shader_t *shader) {
	skg_pipeline_t result = {};
	result.transparency = skg_transparency_none;
	result.cull         = skg_cull_back;
	result.wireframe    = false;
	result.depth_write  = true;
	result.depth_test   = skg_depth_test_less;
	result.meta         = shader->meta;
	result._vertex      = shader->_vertex;
	result._pixel       = shader->_pixel;
	result._layout      = shader->_layout;
	if (result._vertex) result._vertex->AddRef();
	if (result._layout) result._layout->AddRef();
	if (result._pixel ) result._pixel ->AddRef();
	skg_shader_meta_reference(shader->meta);

	skg_pipeline_update_blend     (&result);
	skg_pipeline_update_rasterizer(&result);
	skg_pipeline_update_depth     (&result);
	return result;
}

///////////////////////////////////////////

void skg_pipeline_bind(const skg_pipeline_t *pipeline) {
	d3d_context->OMSetBlendState       (pipeline->_blend,  nullptr, 0xFFFFFFFF);
	d3d_context->OMSetDepthStencilState(pipeline->_depth,  0);
	d3d_context->RSSetState            (pipeline->_rasterize);
	d3d_context->VSSetShader           (pipeline->_vertex, nullptr, 0);
	d3d_context->PSSetShader           (pipeline->_pixel,  nullptr, 0);
	d3d_context->IASetInputLayout      (pipeline->_layout);
}

///////////////////////////////////////////

void skg_pipeline_set_transparency(skg_pipeline_t *pipeline, skg_transparency_ transparency) {
	if (pipeline->transparency != transparency) {
		pipeline->transparency  = transparency;
		skg_pipeline_update_blend(pipeline);
	}
}

///////////////////////////////////////////

void skg_pipeline_set_cull(skg_pipeline_t *pipeline, skg_cull_ cull) {
	if (pipeline->cull != cull) {
		pipeline->cull  = cull;
		skg_pipeline_update_rasterizer(pipeline);
	}
}

///////////////////////////////////////////

void skg_pipeline_set_depth_write(skg_pipeline_t *pipeline, bool write) {
	if (pipeline->depth_write != write) {
		pipeline->depth_write = write;
		skg_pipeline_update_depth(pipeline);
	}
}

///////////////////////////////////////////

void skg_pipeline_set_depth_test (skg_pipeline_t *pipeline, skg_depth_test_ test) {
	if (pipeline->depth_test != test) {
		pipeline->depth_test = test;
		skg_pipeline_update_depth(pipeline);
	}
}

///////////////////////////////////////////

void skg_pipeline_set_wireframe(skg_pipeline_t *pipeline, bool wireframe) {
	if (pipeline->wireframe != wireframe) {
		pipeline->wireframe  = wireframe;
		skg_pipeline_update_rasterizer(pipeline);
	}
}

///////////////////////////////////////////

void skg_pipeline_set_scissor(skg_pipeline_t *pipeline, bool enable) {
	if (pipeline->scissor != enable) {
		pipeline->scissor  = enable;
		skg_pipeline_update_rasterizer(pipeline);
	}
}

///////////////////////////////////////////

skg_transparency_ skg_pipeline_get_transparency(const skg_pipeline_t *pipeline) {
	return pipeline->transparency;
}

///////////////////////////////////////////

skg_cull_ skg_pipeline_get_cull(const skg_pipeline_t *pipeline) {
	return pipeline->cull;
}

///////////////////////////////////////////

bool skg_pipeline_get_wireframe(const skg_pipeline_t *pipeline) {
	return pipeline->wireframe;
}

///////////////////////////////////////////

bool skg_pipeline_get_depth_write(const skg_pipeline_t *pipeline) {
	return pipeline->depth_write;
}

///////////////////////////////////////////

skg_depth_test_ skg_pipeline_get_depth_test(const skg_pipeline_t *pipeline) {
	return pipeline->depth_test;
}

///////////////////////////////////////////

bool skg_pipeline_get_scissor(const skg_pipeline_t *pipeline) {
	return pipeline->scissor;
}

///////////////////////////////////////////

void skg_pipeline_destroy(skg_pipeline_t *pipeline) {
	skg_shader_meta_release(pipeline->meta);
	if (pipeline->_blend    ) pipeline->_blend    ->Release();
	if (pipeline->_rasterize) pipeline->_rasterize->Release();
	if (pipeline->_depth    ) pipeline->_depth    ->Release();
	if (pipeline->_vertex   ) pipeline->_vertex   ->Release();
	if (pipeline->_layout   ) pipeline->_layout   ->Release();
	if (pipeline->_pixel    ) pipeline->_pixel    ->Release();
	*pipeline = {};
}

///////////////////////////////////////////
// skg_swapchain                         //
///////////////////////////////////////////

skg_swapchain_t skg_swapchain_create(void *hwnd, skg_tex_fmt_ format, skg_tex_fmt_ depth_format, int32_t requested_width, int32_t requested_height) {
	skg_swapchain_t result = {};
	result.width  = requested_width;
	result.height = requested_height;

	DXGI_SWAP_CHAIN_DESC1 swapchain_desc = { };
	swapchain_desc.BufferCount = 2;
	swapchain_desc.Width       = result.width;
	swapchain_desc.Height      = result.height;
	swapchain_desc.Format      = (DXGI_FORMAT)skg_tex_fmt_to_native(format);
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchain_desc.AlphaMode   = DXGI_ALPHA_MODE_IGNORE;
	swapchain_desc.SampleDesc.Count = 1;

	IDXGIDevice2  *dxgi_device;  d3d_device  ->QueryInterface(__uuidof(IDXGIDevice2),  (void **)&dxgi_device);
	IDXGIAdapter  *dxgi_adapter; dxgi_device ->GetParent     (__uuidof(IDXGIAdapter),  (void **)&dxgi_adapter);
	IDXGIFactory2 *dxgi_factory; dxgi_adapter->GetParent     (__uuidof(IDXGIFactory2), (void **)&dxgi_factory);

	if (FAILED(dxgi_factory->CreateSwapChainForHwnd(d3d_device, (HWND)hwnd, &swapchain_desc, nullptr, nullptr, &result._swapchain))) {
		skg_log(skg_log_critical, "sk_gpu couldn't create swapchain!");
		result = {};
		return result;
	}

	// Set the target view to an sRGB format for proper presentation of 
	// linear color data.
	skg_tex_fmt_ target_fmt = format;
	switch (format) {
	case skg_tex_fmt_bgra32_linear: target_fmt = skg_tex_fmt_bgra32; break;
	case skg_tex_fmt_rgba32_linear: target_fmt = skg_tex_fmt_rgba32; break;
	}

	ID3D11Texture2D *back_buffer;
	result._swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
	result._target = skg_tex_create_from_existing(back_buffer, skg_tex_type_rendertarget, target_fmt, result.width, result.height, 1);
	if (depth_format != skg_tex_fmt_none) {
		result._depth = skg_tex_create(skg_tex_type_depth, skg_use_static, depth_format, skg_mip_none);
		skg_tex_set_contents(&result._depth, nullptr, result.width, result.height);
		skg_tex_attach_depth(&result._target, &result._depth);
	}
	back_buffer->Release();

	dxgi_factory->Release();
	dxgi_adapter->Release();
	dxgi_device ->Release();

	return result;
}

///////////////////////////////////////////

void skg_swapchain_resize(skg_swapchain_t *swapchain, int32_t width, int32_t height) {
	if (swapchain->_swapchain == nullptr || (width == swapchain->width && height == swapchain->height))
		return;

	skg_tex_fmt_ target_fmt = swapchain->_target.format;
	skg_tex_fmt_ depth_fmt  = swapchain->_depth .format;
	skg_tex_destroy(&swapchain->_target);
	skg_tex_destroy(&swapchain->_depth);

	swapchain->width  = width;
	swapchain->height = height;
	swapchain->_swapchain->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, 0);

	ID3D11Texture2D *back_buffer;
	swapchain->_swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
	swapchain->_target = skg_tex_create_from_existing(back_buffer, skg_tex_type_rendertarget, target_fmt, width, height, 1);
	if (depth_fmt != skg_tex_fmt_none) {
		swapchain->_depth = skg_tex_create(skg_tex_type_depth, skg_use_static, depth_fmt, skg_mip_none);
		skg_tex_set_contents(&swapchain->_depth, nullptr, width, height);
		skg_tex_attach_depth(&swapchain->_target, &swapchain->_depth);
	}
	back_buffer->Release();
}

///////////////////////////////////////////

void skg_swapchain_present(skg_swapchain_t *swapchain) {
	swapchain->_swapchain->Present(1, 0);
}

///////////////////////////////////////////

void skg_swapchain_bind(skg_swapchain_t *swapchain) {
	skg_tex_target_bind(swapchain->_target.format != 0 ? &swapchain->_target : nullptr);
}

///////////////////////////////////////////

void skg_swapchain_destroy(skg_swapchain_t *swapchain) {
	skg_tex_destroy(&swapchain->_target);
	skg_tex_destroy(&swapchain->_depth);
	swapchain->_swapchain->Release();
	*swapchain = {};
}

///////////////////////////////////////////

skg_tex_t skg_tex_create_from_existing(void *native_tex, skg_tex_type_ type, skg_tex_fmt_ override_format, int32_t width, int32_t height, int32_t array_count) {
	skg_tex_t result = {};
	result.type     = type;
	result.use      = skg_use_static;
	result._texture = (ID3D11Texture2D *)native_tex;
	result._texture->AddRef();

	// Get information about the image!
	D3D11_TEXTURE2D_DESC color_desc;
	result._texture->GetDesc(&color_desc);
	result.width       = color_desc.Width;     (void)width;
	result.height      = color_desc.Height;    (void)height;
	result.array_count = color_desc.ArraySize; (void)array_count;
	result.multisample = color_desc.SampleDesc.Count;
	result.format      = override_format != 0 ? override_format : skg_tex_fmt_from_native(color_desc.Format);
	skg_tex_make_view(&result, color_desc.MipLevels, 0, color_desc.BindFlags & D3D11_BIND_SHADER_RESOURCE);

	return result;
}

///////////////////////////////////////////

skg_tex_t skg_tex_create_from_layer(void *native_tex, skg_tex_type_ type, skg_tex_fmt_ override_format, int32_t width, int32_t height, int32_t array_layer) {
	skg_tex_t result = {};
	result.type     = type;
	result.use      = skg_use_static;
	result._texture = (ID3D11Texture2D *)native_tex;
	result._texture->AddRef();

	// Get information about the image!
	D3D11_TEXTURE2D_DESC color_desc;
	result._texture->GetDesc(&color_desc);
	result.width       = color_desc.Width;  (void)width;
	result.height      = color_desc.Height; (void)height;
	result.array_count = 1;
	result.multisample = color_desc.SampleDesc.Count;
	result.format      = override_format != 0 ? override_format : skg_tex_fmt_from_native(color_desc.Format);
	skg_tex_make_view(&result, color_desc.MipLevels, array_layer, color_desc.BindFlags & D3D11_BIND_SHADER_RESOURCE);

	return result;
}

///////////////////////////////////////////

skg_tex_t skg_tex_create(skg_tex_type_ type, skg_use_ use, skg_tex_fmt_ format, skg_mip_ mip_maps) {
	skg_tex_t result = {};
	result.type   = type;
	result.use    = use;
	result.format = format;
	result.mips   = mip_maps;

	if (use == skg_use_dynamic && mip_maps == skg_mip_generate)
		skg_log(skg_log_warning, "Dynamic textures don't support mip-maps!");

	return result;
}

///////////////////////////////////////////

void skg_tex_copy_to(const skg_tex_t *tex, skg_tex_t *destination) {
	if (destination->width != tex->width || destination->height != tex->height) {
		skg_tex_set_contents_arr(destination, nullptr, tex->array_count, tex->width, tex->height, tex->multisample);
	}

	if (tex->multisample > 1) {
		d3d_context->ResolveSubresource(destination->_texture, 0, tex->_texture, 0, (DXGI_FORMAT)skg_tex_fmt_to_native(tex->format));
	} else {
		d3d_context->CopyResource(destination->_texture, tex->_texture);
	}
}

///////////////////////////////////////////

void skg_tex_copy_to_swapchain(const skg_tex_t *tex, skg_swapchain_t *destination) {
	skg_tex_copy_to(tex, &destination->_target);
}

///////////////////////////////////////////

bool skg_tex_is_valid(const skg_tex_t *tex) {
	return tex->_texture != nullptr;
}

///////////////////////////////////////////

void skg_tex_attach_depth(skg_tex_t *tex, skg_tex_t *depth) {
	if (depth->type == skg_tex_type_depth) {
		if (tex->_depth_view) tex->_depth_view->Release();
		tex->_depth_view = depth->_depth_view;
		tex->_depth_view->AddRef();
	} else {
		skg_log(skg_log_warning, "Can't bind a depth texture to a non-rendertarget");
	}
}

///////////////////////////////////////////

void skg_tex_settings(skg_tex_t *tex, skg_tex_address_ address, skg_tex_sample_ sample, int32_t anisotropy) {
	if (tex->_sampler)
		tex->_sampler->Release();

	D3D11_TEXTURE_ADDRESS_MODE mode;
	switch (address) {
	case skg_tex_address_clamp:  mode = D3D11_TEXTURE_ADDRESS_CLAMP;  break;
	case skg_tex_address_repeat: mode = D3D11_TEXTURE_ADDRESS_WRAP;   break;
	case skg_tex_address_mirror: mode = D3D11_TEXTURE_ADDRESS_MIRROR; break;
	default: mode = D3D11_TEXTURE_ADDRESS_WRAP;
	}

	D3D11_FILTER filter;
	switch (sample) {
	case skg_tex_sample_linear:     filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break; // Technically trilinear
	case skg_tex_sample_point:      filter = D3D11_FILTER_MIN_MAG_MIP_POINT;  break;
	case skg_tex_sample_anisotropic:filter = D3D11_FILTER_ANISOTROPIC;        break;
	default: filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}

	D3D11_SAMPLER_DESC desc_sampler = {};
	desc_sampler.AddressU = mode;
	desc_sampler.AddressV = mode;
	desc_sampler.AddressW = mode;
	desc_sampler.Filter   = filter;
	desc_sampler.MaxAnisotropy  = anisotropy;
	desc_sampler.MaxLOD         = D3D11_FLOAT32_MAX;
	desc_sampler.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

	// D3D will already return the same sampler when provided the same 
	// settings, so we can just lean on that to prevent sampler duplicates :)
	if (FAILED(d3d_device->CreateSamplerState(&desc_sampler, &tex->_sampler)))
		skg_log(skg_log_critical, "Failed to create sampler state!");
}

///////////////////////////////////////////

bool skg_can_make_mips(skg_tex_fmt_ format) {
	switch (format) {
	case skg_tex_fmt_bgra32:
	case skg_tex_fmt_bgra32_linear:
	case skg_tex_fmt_rgba32:
	case skg_tex_fmt_rgba32_linear: 
	case skg_tex_fmt_rgba64u:
	case skg_tex_fmt_rgba64s:
	case skg_tex_fmt_rgba128:
	case skg_tex_fmt_depth32:
	case skg_tex_fmt_r32:
	case skg_tex_fmt_depth16:
	case skg_tex_fmt_r16:
	case skg_tex_fmt_r8: return true;
	default: return false;
	}
}

///////////////////////////////////////////

void skg_make_mips(D3D11_SUBRESOURCE_DATA *tex_mem, const void *curr_data, skg_tex_fmt_ format, int32_t width, int32_t height, uint32_t mip_levels) {
	const void *mip_data = curr_data;
	int32_t     mip_w    = width;
	int32_t     mip_h    = height;
	for (uint32_t m = 1; m < mip_levels; m++) {
		tex_mem[m] = {};
		switch (format) { // When adding a new format here, also add it to skg_can_make_mips
		case skg_tex_fmt_bgra32:
		case skg_tex_fmt_bgra32_linear:
		case skg_tex_fmt_rgba32:
		case skg_tex_fmt_rgba32_linear: 
			skg_downsample_4((uint8_t  *)mip_data, mip_w, mip_h, (uint8_t  **)&tex_mem[m].pSysMem, &mip_w, &mip_h); 
			break;
		case skg_tex_fmt_rgba64u:
			skg_downsample_4((uint16_t *)mip_data, mip_w, mip_h, (uint16_t **)&tex_mem[m].pSysMem, &mip_w, &mip_h);
			break;
		case skg_tex_fmt_rgba64s:
			skg_downsample_4((int16_t  *)mip_data, mip_w, mip_h, (int16_t  **)&tex_mem[m].pSysMem, &mip_w, &mip_h);
			break;
		case skg_tex_fmt_rgba128:
			skg_downsample_4((float    *)mip_data, mip_w, mip_h, (float    **)&tex_mem[m].pSysMem, &mip_w, &mip_h);
			break;
		case skg_tex_fmt_depth32:
		case skg_tex_fmt_r32:
			skg_downsample_1((float    *)mip_data, mip_w, mip_h, (float    **)&tex_mem[m].pSysMem, &mip_w, &mip_h); 
			break;
		case skg_tex_fmt_depth16:
		case skg_tex_fmt_r16:
			skg_downsample_1((uint16_t *)mip_data, mip_w, mip_h, (uint16_t **)&tex_mem[m].pSysMem, &mip_w, &mip_h); 
			break;
		case skg_tex_fmt_r8:
			skg_downsample_1((uint8_t  *)mip_data, mip_w, mip_h, (uint8_t  **)&tex_mem[m].pSysMem, &mip_w, &mip_h); 
			break;
		default: skg_log(skg_log_warning, "Unsupported texture format for mip maps!"); break;
		}
		mip_data = (void*)tex_mem[m].pSysMem;
		tex_mem[m].SysMemPitch = (UINT)(skg_tex_fmt_size(format) * mip_w);
	}
}

///////////////////////////////////////////

bool skg_tex_make_view(skg_tex_t *tex, uint32_t mip_count, uint32_t array_start, bool use_in_shader) {
	DXGI_FORMAT format = (DXGI_FORMAT)skg_tex_fmt_to_native(tex->format);

	if (tex->type != skg_tex_type_depth) {
		D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {};
		res_desc.Format = format;
		// This struct is a union, but all elements follow the same order in
		// the struct. Texture2DArray is representative of the union with the
		// most data in it, so if we fill it properly, all others should also
		// be filled correctly. *Fingers crossed it stays that way*
		res_desc.Texture2DArray.FirstArraySlice = array_start;
		res_desc.Texture2DArray.ArraySize       = tex->array_count;
		res_desc.Texture2DArray.MipLevels       = mip_count;

		if (tex->type == skg_tex_type_cubemap) {
			res_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		} else if (tex->array_count > 1) {
			res_desc.ViewDimension = tex->multisample > 1
				? D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY
				: D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		} else {
			res_desc.ViewDimension = tex->multisample > 1 
				? D3D11_SRV_DIMENSION_TEXTURE2DMS
				: D3D11_SRV_DIMENSION_TEXTURE2D;
		}

		if (use_in_shader && FAILED(d3d_device->CreateShaderResourceView(tex->_texture, &res_desc, &tex->_resource))) {
			skg_log(skg_log_critical, "Create Shader Resource View error!");
			return false;
		}
	} else {
		D3D11_DEPTH_STENCIL_VIEW_DESC stencil_desc = {};
		stencil_desc.Format = format;
		stencil_desc.Texture2DArray.FirstArraySlice = array_start;
		stencil_desc.Texture2DArray.ArraySize       = tex->array_count;
		if (tex->type == skg_tex_type_cubemap || tex->array_count > 1) {
			stencil_desc.ViewDimension = tex->multisample > 1
				? D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY
				: D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		} else {
			stencil_desc.ViewDimension = tex->multisample > 1 
				? D3D11_DSV_DIMENSION_TEXTURE2DMS
				: D3D11_DSV_DIMENSION_TEXTURE2D;
		}
		if (FAILED(d3d_device->CreateDepthStencilView(tex->_texture, &stencil_desc, &tex->_depth_view))) {
			skg_log(skg_log_critical, "Create Depth Stencil View error!");
			return false;
		}
	}

	if (tex->type == skg_tex_type_rendertarget) {
		D3D11_RENDER_TARGET_VIEW_DESC target_desc = {};
		target_desc.Format = format;
		target_desc.Texture2DArray.FirstArraySlice = array_start;
		target_desc.Texture2DArray.ArraySize       = tex->array_count;
		if (tex->type == skg_tex_type_cubemap || tex->array_count > 1) {
			target_desc.ViewDimension = tex->multisample > 1
				? D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY
				: D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		} else {
			target_desc.ViewDimension = tex->multisample > 1
				? D3D11_RTV_DIMENSION_TEXTURE2DMS
				: D3D11_RTV_DIMENSION_TEXTURE2D;
		}

		if (FAILED(d3d_device->CreateRenderTargetView(tex->_texture, &target_desc, &tex->_target_view))) {
			skg_log(skg_log_critical, "Create Render Target View error!");
			return false;
		}
	}

	if (tex->use & skg_use_compute_write) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC view = {};
		view.Format = DXGI_FORMAT_UNKNOWN;
		if (tex->type == skg_tex_type_cubemap || tex->array_count > 1) {
			view.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			view.Texture2DArray.FirstArraySlice = array_start;
			view.Texture2DArray.ArraySize       = tex->array_count;
		} else {
			view.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		}

		if(FAILED(d3d_device->CreateUnorderedAccessView( tex->_texture, &view, &tex->_unordered ))) {
			skg_log(skg_log_critical, "CreateUnorderedAccessView failed!");
			return {};
		}
	} 
	return true;
}

///////////////////////////////////////////

void skg_tex_set_contents(skg_tex_t *tex, const void *data, int32_t width, int32_t height) {
	const void *data_arr[1] = { data };
	return skg_tex_set_contents_arr(tex, data_arr, 1, width, height, 1);
}

///////////////////////////////////////////

void skg_tex_set_contents_arr(skg_tex_t *tex, const void **data_frames, int32_t data_frame_count, int32_t width, int32_t height, int32_t multisample) {
	// Some warning messages
	if (tex->use != skg_use_dynamic && tex->_texture) {
		skg_log(skg_log_warning, "Only dynamic textures can be updated!");
		return;
	}
	if (tex->use == skg_use_dynamic && (tex->mips == skg_mip_generate || data_frame_count > 1)) {
		skg_log(skg_log_warning, "Dynamic textures don't support mip-maps or texture arrays!");
		return;
	}

	tex->width       = width;
	tex->height      = height;
	tex->array_count = data_frame_count;
	tex->multisample = multisample;
	bool mips = 
		   tex->mips == skg_mip_generate
		&& skg_can_make_mips(tex->format)
		&& (width  & (width  - 1)) == 0
		&& (height & (height - 1)) == 0;

	uint32_t mip_levels = (mips ? skg_mip_count(width, height) : 1);
	uint32_t px_size    = skg_tex_fmt_size(tex->format);

	if (tex->_texture == nullptr) {
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width            = width;
		desc.Height           = height;
		desc.MipLevels        = mip_levels;
		desc.ArraySize        = data_frame_count;
		desc.SampleDesc.Count = multisample;
		desc.Format           = (DXGI_FORMAT)skg_tex_fmt_to_native(tex->format);
		desc.BindFlags        = tex->type == skg_tex_type_depth ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_SHADER_RESOURCE;
		desc.Usage            = tex->use  == skg_use_dynamic    ? D3D11_USAGE_DYNAMIC      : tex->type == skg_tex_type_rendertarget || tex->type == skg_tex_type_depth || data_frames != nullptr || data_frames[0] != nullptr ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags   = tex->use  == skg_use_dynamic    ? D3D11_CPU_ACCESS_WRITE   : 0;
		if (tex->type == skg_tex_type_rendertarget) desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		if (tex->type == skg_tex_type_cubemap     ) desc.MiscFlags  = D3D11_RESOURCE_MISC_TEXTURECUBE;
		if (tex->use  &  skg_use_compute_write    ) desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

		D3D11_SUBRESOURCE_DATA *tex_mem = nullptr;
		if (data_frames != nullptr && data_frames[0] != nullptr) {
			tex_mem = (D3D11_SUBRESOURCE_DATA *)malloc((int64_t)data_frame_count * mip_levels * sizeof(D3D11_SUBRESOURCE_DATA));
			if (!tex_mem) { skg_log(skg_log_critical, "Out of memory"); return;  }

			for (int32_t i = 0; i < data_frame_count; i++) {
				tex_mem[i*mip_levels] = {};
				tex_mem[i*mip_levels].pSysMem     = data_frames[i];
				tex_mem[i*mip_levels].SysMemPitch = (UINT)(px_size * width);

				if (mips) {
					skg_make_mips(&tex_mem[i*mip_levels], data_frames[i], tex->format, width, height, mip_levels);
				}
			}
		}

		if (FAILED(d3d_device->CreateTexture2D(&desc, tex_mem, &tex->_texture))) {
			skg_log(skg_log_critical, "Create texture error!");
		}

		if (tex_mem != nullptr) {
			for (int32_t i = 0; i < data_frame_count; i++) {
				for (uint32_t m = 1; m < mip_levels; m++) {
					free((void*)tex_mem[i*mip_levels + m].pSysMem);
				} 
			}
			free(tex_mem);
		}

		skg_tex_make_view(tex, mip_levels, 0, true);
	} else {
		// For dynamic textures, just upload the new value into the texture!
		D3D11_MAPPED_SUBRESOURCE tex_mem = {};
		if (FAILED(d3d_context->Map(tex->_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &tex_mem))) {
			skg_log(skg_log_critical, "Failed mapping a texture");
			return;
		}

		uint8_t *dest_line  = (uint8_t *)tex_mem.pData;
		uint8_t *src_line   = (uint8_t *)data_frames[0];
		for (int i = 0; i < height; i++) {
			memcpy(dest_line, src_line, (size_t)width * px_size);
			dest_line += tex_mem.RowPitch;
			src_line  += px_size * (uint64_t)width;
		}
		d3d_context->Unmap(tex->_texture, 0);
	}

	// If the sampler has not been set up yet, we'll make a default one real quick.
	if (tex->_sampler == nullptr) {
		skg_tex_settings(tex, skg_tex_address_repeat, skg_tex_sample_linear, 0);
	}
}

///////////////////////////////////////////

bool skg_tex_get_contents(skg_tex_t *tex, void *ref_data, size_t data_size) {
	return skg_tex_get_mip_contents_arr(tex, 0, 0, ref_data, data_size);
}

///////////////////////////////////////////

bool skg_tex_get_mip_contents(skg_tex_t *tex, int32_t mip_level, void *ref_data, size_t data_size) {
	return skg_tex_get_mip_contents_arr(tex, mip_level, 0, ref_data, data_size);
}

///////////////////////////////////////////

bool skg_tex_get_mip_contents_arr(skg_tex_t *tex, int32_t mip_level, int32_t arr_index, void *ref_data, size_t data_size) {
	// Double check on mips first
	int32_t mip_levels = tex->mips == skg_mip_generate ? (int32_t)skg_mip_count(tex->width, tex->height) : 1;
	if (mip_level != 0) {
		if (tex->mips != skg_mip_generate) {
			skg_log(skg_log_critical, "Can't get mip data from a texture with no mips!");
			return false;
		}
		if (mip_level >= mip_levels) {
			skg_log(skg_log_critical, "This texture doesn't have quite as many mip levels as you think.");
			return false;
		}
	}

	// Make sure we've been provided enough memory to hold this texture
	int32_t width       = 0;
	int32_t height      = 0;
	size_t  format_size = skg_tex_fmt_size(tex->format);
	skg_mip_dimensions(tex->width, tex->height, mip_level, &width, &height);

	if (data_size != (size_t)width * (size_t)height * format_size) {
		skg_log(skg_log_critical, "Insufficient buffer size for skg_tex_get_mip_contents_arr");
		return false;
	}

	D3D11_TEXTURE2D_DESC desc             = {};
	ID3D11Texture2D     *copy_tex         = nullptr;
	bool                 copy_tex_release = true;
	UINT                 subresource      = mip_level + (arr_index * mip_levels);
	tex->_texture->GetDesc(&desc);
	desc.Width     = width;
	desc.Height    = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.MiscFlags = 0;

	// Make sure copy_tex is a texture that we can read from!
	if (desc.SampleDesc.Count > 1) {
		// Not gonna bother with MSAA stuff
		skg_log(skg_log_warning, "skg_tex_get_mip_contents_arr MSAA surfaces not implemented");
		return false;
	} else if ((desc.Usage == D3D11_USAGE_STAGING) && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ)) {
		// Handle case where the source is already a staging texture we can use directly
		copy_tex         = tex->_texture;
		copy_tex_release = false;
	} else {
		// Otherwise, create a staging texture from the non-MSAA source
		desc.BindFlags      = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage          = D3D11_USAGE_STAGING;

		if (FAILED(d3d_device->CreateTexture2D(&desc, nullptr, &copy_tex))) {
			skg_log(skg_log_critical, "CreateTexture2D failed!");
			return false;
		}

		D3D11_BOX box = {};
		box.right  = width;
		box.bottom = height;
		box.back   = 1;
		d3d_context->CopySubresourceRegion(copy_tex, 0, 0, 0, 0, tex->_texture, subresource, &box);
		subresource = 0;
	}

	// Load the data into CPU RAM
	D3D11_MAPPED_SUBRESOURCE data;
	if (FAILED(d3d_context->Map(copy_tex, subresource, D3D11_MAP_READ, 0, &data))) {
		skg_log(skg_log_critical, "Texture Map failed!");
		return false;
	}

	// Copy it into our waiting buffer
	uint8_t *srcPtr  = (uint8_t*)data.pData;
	uint8_t *destPtr = (uint8_t*)ref_data;
	size_t   msize   = width*format_size;
	for (size_t h = 0; h < desc.Height; ++h) {
		memcpy(destPtr, srcPtr, msize);
		srcPtr  += data.RowPitch;
		destPtr += msize;
	}

	// And cleanup
	d3d_context->Unmap(copy_tex, 0);
	if (copy_tex_release)
		copy_tex->Release();

	return true;
}

///////////////////////////////////////////

void skg_tex_clear(skg_bind_t bind) {
	switch (bind.register_type) {
	case skg_register_resource: {
		ID3D11SamplerState       *null_state = nullptr;
		ID3D11ShaderResourceView *null_view  = nullptr;
		if (bind.stage_bits & skg_stage_pixel ){
			d3d_context->PSSetSamplers       (bind.slot, 1, &null_state);
			d3d_context->PSSetShaderResources(bind.slot, 1, &null_view);
		}
		if (bind.stage_bits & skg_stage_vertex) {
			d3d_context->VSSetSamplers       (bind.slot, 1, &null_state);
			d3d_context->VSSetShaderResources(bind.slot, 1, &null_view);
		}
		if (bind.stage_bits & skg_stage_compute) {
			d3d_context->CSSetSamplers       (bind.slot, 1, &null_state);
			d3d_context->CSSetShaderResources(bind.slot, 1, &null_view);
		}
	} break;
	case skg_register_readwrite: {
		if (bind.stage_bits & skg_stage_compute) {
			ID3D11UnorderedAccessView *null_view = nullptr;
			d3d_context->CSSetUnorderedAccessViews(bind.slot, 1, &null_view, nullptr);
		}
	} break;
	default: skg_log(skg_log_critical, "You can only bind/clear a texture to skg_register_resource, or skg_register_readwrite!"); break;
	}
}

///////////////////////////////////////////

void skg_tex_bind(const skg_tex_t *texture, skg_bind_t bind) {
	switch (bind.register_type) {
	case skg_register_resource: {
		if (bind.stage_bits & skg_stage_pixel ){
			d3d_context->PSSetSamplers       (bind.slot, 1, &texture->_sampler);
			d3d_context->PSSetShaderResources(bind.slot, 1, &texture->_resource);
		}
		if (bind.stage_bits & skg_stage_vertex) {
			d3d_context->VSSetSamplers       (bind.slot, 1, &texture->_sampler);
			d3d_context->VSSetShaderResources(bind.slot, 1, &texture->_resource);
		}
		if (bind.stage_bits & skg_stage_compute) {
			d3d_context->CSSetSamplers       (bind.slot, 1, &texture->_sampler);
			d3d_context->CSSetShaderResources(bind.slot, 1, &texture->_resource);
		}
	} break;
	case skg_register_readwrite: {
		if (bind.stage_bits & skg_stage_compute) d3d_context->CSSetUnorderedAccessViews(bind.slot, 1, &texture->_unordered, nullptr);
	} break;
	default: skg_log(skg_log_critical, "You can only bind/clear a texture to skg_register_resource, or skg_register_readwrite!"); break;
	}
}

///////////////////////////////////////////

void skg_tex_destroy(skg_tex_t *tex) {
	if (tex->_target_view) tex->_target_view->Release();
	if (tex->_depth_view ) tex->_depth_view ->Release();
	if (tex->_resource   ) tex->_resource   ->Release();
	if (tex->_sampler    ) tex->_sampler    ->Release();
	if (tex->_texture    ) tex->_texture    ->Release();
	*tex = {};
}

///////////////////////////////////////////

template <typename T>
void skg_downsample_4(T *data, int32_t width, int32_t height, T **out_data, int32_t *out_width, int32_t *out_height) {
	int w = (int32_t)log2f((float)width);
	int h = (int32_t)log2f((float)height);
	*out_width  = w = max(1, (1 << w) >> 1);
	*out_height = h = max(1, (1 << h) >> 1);

	*out_data = (T*)malloc((int64_t)w * h * sizeof(T) * 4);
	if (*out_data == nullptr) { skg_log(skg_log_critical, "Out of memory"); return; }
	memset(*out_data, 0, (int64_t)w * h * sizeof(T) * 4);
	T *result = *out_data;

	for (int32_t y = 0; y < (*out_height); y++) {
		int32_t src_row_start  = y * 2 * width;
		int32_t dest_row_start = y * (*out_width);
		for (int32_t x = 0; x < (*out_width);  x++) {
			int src   = (x*2 + src_row_start )*4;
			int dest  = (x   + dest_row_start)*4;
			int src_n = src + width*4;
			T *cD = &result[dest];

			cD[0] = (data[src] + data[src+4] + data[src_n] + data[src_n+4])/4; src++; src_n++;
			cD[1] = (data[src] + data[src+4] + data[src_n] + data[src_n+4])/4; src++; src_n++;
			cD[2] = (data[src] + data[src+4] + data[src_n] + data[src_n+4])/4; src++; src_n++;
			cD[3] = (data[src] + data[src+4] + data[src_n] + data[src_n+4])/4; src++; src_n++;
		}
	}
}

///////////////////////////////////////////

template <typename T>
void skg_downsample_1(T *data, int32_t width, int32_t height, T **out_data, int32_t *out_width, int32_t *out_height) {
	int w = (int32_t)log2f((float)width);
	int h = (int32_t)log2f((float)height);
	*out_width  = w = (1 << w) >> 1;
	*out_height = h = (1 << h) >> 1;

	*out_data = (T*)malloc((int64_t)w * h * sizeof(T));
	if (*out_data == nullptr) { skg_log(skg_log_critical, "Out of memory"); return; }
	memset(*out_data, 0, (int64_t)w * h * sizeof(T));
	T *result = *out_data;

	for (int32_t y = 0; y < (*out_height); y++) {
		int32_t src_row_start  = y * 2 * width;
		int32_t dest_row_start = y * (*out_width);
		for (int32_t x = 0; x < (*out_width);  x++) {
			int src   = x*2 + src_row_start;
			int dest  = x   + dest_row_start;
			int src_n = src + width;
			result[dest] = (data[src] + data[src+1] + data[src_n] + data[src_n+1]) / 4;
		}
	}
}

///////////////////////////////////////////

int64_t skg_tex_fmt_to_native(skg_tex_fmt_ format){
	switch (format) {
	case skg_tex_fmt_rgba32:        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case skg_tex_fmt_rgba32_linear: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case skg_tex_fmt_bgra32:        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	case skg_tex_fmt_bgra32_linear: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case skg_tex_fmt_rg11b10:       return DXGI_FORMAT_R11G11B10_FLOAT;
	case skg_tex_fmt_rgb10a2:       return DXGI_FORMAT_R10G10B10A2_UNORM;
	case skg_tex_fmt_rgba64u:       return DXGI_FORMAT_R16G16B16A16_UNORM;
	case skg_tex_fmt_rgba64s:       return DXGI_FORMAT_R16G16B16A16_SNORM;
	case skg_tex_fmt_rgba64f:       return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case skg_tex_fmt_rgba128:       return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case skg_tex_fmt_depth16:       return DXGI_FORMAT_D16_UNORM;
	case skg_tex_fmt_depth32:       return DXGI_FORMAT_D32_FLOAT;
	case skg_tex_fmt_depthstencil:  return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case skg_tex_fmt_r8:            return DXGI_FORMAT_R8_UNORM;
	case skg_tex_fmt_r16:           return DXGI_FORMAT_R16_UNORM;
	case skg_tex_fmt_r32:           return DXGI_FORMAT_R32_FLOAT;
	default: return DXGI_FORMAT_UNKNOWN;
	}
}

///////////////////////////////////////////

skg_tex_fmt_ skg_tex_fmt_from_native(int64_t format) {
	switch (format) {
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return skg_tex_fmt_rgba32;
	case DXGI_FORMAT_R8G8B8A8_UNORM:      return skg_tex_fmt_rgba32_linear;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return skg_tex_fmt_bgra32;
	case DXGI_FORMAT_B8G8R8A8_UNORM:      return skg_tex_fmt_bgra32_linear;
	case DXGI_FORMAT_R11G11B10_FLOAT:     return skg_tex_fmt_rg11b10;
	case DXGI_FORMAT_R10G10B10A2_UNORM:   return skg_tex_fmt_rgb10a2;
	case DXGI_FORMAT_R16G16B16A16_UNORM:  return skg_tex_fmt_rgba64u;
	case DXGI_FORMAT_R16G16B16A16_SNORM:  return skg_tex_fmt_rgba64s;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:  return skg_tex_fmt_rgba64f;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:  return skg_tex_fmt_rgba128;
	case DXGI_FORMAT_D16_UNORM:           return skg_tex_fmt_depth16;
	case DXGI_FORMAT_D32_FLOAT:           return skg_tex_fmt_depth32;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:   return skg_tex_fmt_depthstencil;
	case DXGI_FORMAT_R8_UNORM:            return skg_tex_fmt_r8;
	case DXGI_FORMAT_R16_UNORM:           return skg_tex_fmt_r16;
	case DXGI_FORMAT_R32_FLOAT:           return skg_tex_fmt_r32;
	default: return skg_tex_fmt_none;
	}
}

///////////////////////////////////////////

const char *skg_semantic_to_d3d(skg_el_semantic_ semantic) {
	switch (semantic) {
	case skg_el_semantic_none:         return "";
	case skg_el_semantic_position:     return "SV_POSITION";
	case skg_el_semantic_normal:       return "NORMAL";
	case skg_el_semantic_texcoord:     return "TEXCOORD";
	case skg_el_semantic_color:        return "COLOR";
	case skg_el_semantic_target_index: return "SV_RenderTargetArrayIndex";
	default: return "";
	}
}

#endif

#ifdef SKG_OPENGL
///////////////////////////////////////////
// OpenGL Implementation                 //
///////////////////////////////////////////

#include <malloc.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////

#if   defined(_SKG_GL_LOAD_EMSCRIPTEN)
	#include <emscripten.h>
	#include <emscripten/html5.h>
	#include <GLES3/gl32.h>
#elif defined(_SKG_GL_LOAD_EGL)
	#include <EGL/egl.h>
	#include <EGL/eglext.h>

	EGLDisplay egl_display;
	EGLContext egl_context;
	EGLConfig  egl_config;
#elif defined(_SKG_GL_LOAD_GLX)
	#include <GL/glxew.h>

	Display     *xDisplay;
	XVisualInfo *visualInfo;
	GLXFBConfig  glxFBConfig;
	GLXDrawable  glxDrawable;
	GLXContext   glxContext;
#elif defined(_SKG_GL_LOAD_WGL)
	#pragma comment(lib, "opengl32.lib")
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

	HWND  gl_hwnd;
	HDC   gl_hdc;
	HGLRC gl_hrc;
#endif

///////////////////////////////////////////
// GL loader                             //
///////////////////////////////////////////

#ifdef _SKG_GL_LOAD_WGL
#define WGL_DRAW_TO_WINDOW_ARB            0x2001
#define WGL_SUPPORT_OPENGL_ARB            0x2010
#define WGL_DOUBLE_BUFFER_ARB             0x2011
#define WGL_ACCELERATION_ARB              0x2003
#define WGL_FULL_ACCELERATION_ARB         0x2027
#define WGL_PIXEL_TYPE_ARB                0x2013
#define WGL_COLOR_BITS_ARB                0x2014
#define WGL_DEPTH_BITS_ARB                0x2022
#define WGL_STENCIL_BITS_ARB              0x2023
#define WGL_TYPE_RGBA_ARB                 0x202B
#define WGL_SAMPLE_BUFFERS_ARB            0x2041
#define WGL_SAMPLES_ARB                   0x2042
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
#define WGL_CONTEXT_DEBUG_BIT_ARB         0x0001

typedef BOOL  (*wglChoosePixelFormatARB_proc)    (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC (*wglCreateContextAttribsARB_proc) (HDC hDC, HGLRC hShareContext, const int *attribList);
wglChoosePixelFormatARB_proc    wglChoosePixelFormatARB;
wglCreateContextAttribsARB_proc wglCreateContextAttribsARB;
#endif

#ifdef _SKG_GL_MAKE_FUNCTIONS

#define GL_BLEND 0x0BE2
#define GL_ZERO 0
#define GL_ONE  1
#define GL_SRC_COLOR                0x0300
#define GL_ONE_MINUS_SRC_COLOR      0x0301
#define GL_SRC_ALPHA                0x0302
#define GL_ONE_MINUS_SRC_ALPHA      0x0303
#define GL_DST_ALPHA                0x0304
#define GL_ONE_MINUS_DST_ALPHA      0x0305
#define GL_DST_COLOR                0x0306
#define GL_ONE_MINUS_DST_COLOR      0x0307
#define GL_CONSTANT_COLOR           0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#define GL_CONSTANT_ALPHA           0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#define GL_FUNC_ADD                 0x8006
#define GL_MAX                      0x8008

#define GL_NEVER                    0x0200 
#define GL_LESS                     0x0201
#define GL_EQUAL                    0x0202
#define GL_LEQUAL                   0x0203
#define GL_GREATER                  0x0204
#define GL_NOTEQUAL                 0x0205
#define GL_GEQUAL                   0x0206
#define GL_ALWAYS                   0x0207

#define GL_INVALID_INDEX 0xFFFFFFFFu
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#define GL_VIEWPORT         0x0BA2
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_STENCIL_BUFFER_BIT 0x400
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_UNIFORM_BUFFER 0x8A11
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_TRIANGLES 0x0004
#define GL_VERSION 0x1F02
#define GL_CULL_FACE 0x0B44
#define GL_BACK 0x0405
#define GL_FRONT 0x0404
#define GL_FRONT_AND_BACK 0x0408
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02
#define GL_DEPTH_TEST 0x0B71
#define GL_SCISSOR_TEST 0x0C11
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_2D_ARRAY 0x8C1A
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
#define GL_TEXTURE_CUBE_MAP_ARRAY 0x9009
#define GL_TEXTURE_BINDING_CUBE_MAP 0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_MIN_LOD 0x813A
#define GL_TEXTURE_MAX_LOD 0x813B
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_TEXTURE_WIDTH 0x1000
#define GL_TEXTURE_HEIGHT 0x1001
#define GL_TEXTURE_INTERNAL_FORMAT 0x1003
#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_MIRRORED_REPEAT 0x8370
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_TEXTURE0 0x84C0
#define GL_FRAMEBUFFER 0x8D40
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A

#define GL_RED 0x1903
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_SRGB_ALPHA 0x8C42
#define GL_DEPTH_COMPONENT 0x1902
#define GL_DEPTH_STENCIL 0x84F9
#define GL_R8_SNORM 0x8F94
#define GL_RG8_SNORM 0x8F95
#define GL_RGB8_SNORM 0x8F96
#define GL_RGBA8_SNORM 0x8F97
#define GL_R16_SNORM 0x8F98
#define GL_RG16_SNORM 0x8F99
#define GL_RGB16_SNORM 0x8F9A
#define GL_RGBA16_SNORM 0x8F9B
#define GL_RG 0x8227
#define GL_RG_INTEGER 0x8228
#define GL_R8 0x8229
#define GL_RG8 0x822B
#define GL_RG16 0x822C
#define GL_R16F 0x822D
#define GL_R32F 0x822E
#define GL_RG16F 0x822F
#define GL_RG32F 0x8230
#define GL_R8I 0x8231
#define GL_R8UI 0x8232
#define GL_R16I 0x8233
#define GL_R16UI 0x8234
#define GL_R32I 0x8235
#define GL_R32UI 0x8236
#define GL_RG8I 0x8237
#define GL_RG8UI 0x8238
#define GL_RG16I 0x8239
#define GL_RG16UI 0x823A
#define GL_RG32I 0x823B
#define GL_RG32UI 0x823C
#define GL_RGBA8 0x8058
#define GL_RGBA16 0x805B
#define GL_BGRA 0x80E1
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_R11F_G11F_B10F 0x8C3A
#define GL_RGB10_A2 0x8059
#define GL_RGBA32F 0x8814
#define GL_RGBA16F 0x881A
#define GL_RGBA16I 0x8D88
#define GL_RGBA16UI 0x8D76
#define GL_COMPRESSED_RGB8_ETC2 0x9274
#define GL_COMPRESSED_SRGB8_ETC2 0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
#define GL_COMPRESSED_R11_EAC 0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC 0x9271
#define GL_COMPRESSED_RG11_EAC 0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC 0x9273
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_DEPTH_COMPONENT32F 0x8CAC
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_UNSIGNED_INT_24_8 0x84FA;
#define GL_FLOAT 0x1406
#define GL_DOUBLE 0x140A
#define GL_UNSIGNED_INT_8_8_8_8 0x8035
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPUTE_SHADER 0x91B9
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_NUM_EXTENSIONS 0x821D
#define GL_EXTENSIONS 0x1F03

#define GL_DEBUG_OUTPUT                0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS    0x8242
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B
#define GL_DEBUG_SEVERITY_HIGH         0x9146
#define GL_DEBUG_SEVERITY_MEDIUM       0x9147
#define GL_DEBUG_SEVERITY_LOW          0x9148

// Reference from here:
// https://github.com/ApoorvaJ/Papaya/blob/3808e39b0f45d4ca4972621c847586e4060c042a/src/libs/gl_lite.h

#ifdef _WIN32
#define GLDECL WINAPI
#else
#define GLDECL
#endif

typedef void (GLDECL *GLDEBUGPROC)(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length, const char* message, const void* userParam);

#define GL_API \
GLE(void,     glLinkProgram,             uint32_t program) \
GLE(void,     glClearColor,              float r, float g, float b, float a) \
GLE(void,     glClear,                   uint32_t mask) \
GLE(void,     glEnable,                  uint32_t cap) \
GLE(void,     glDisable,                 uint32_t cap) \
GLE(void,     glPolygonMode,             uint32_t face, uint32_t mode) \
GLE(void,     glDepthMask,               uint8_t flag) \
GLE(void,     glDepthFunc,               uint32_t func) \
GLE(uint32_t, glGetError,                ) \
GLE(void,     glGetProgramiv,            uint32_t program, uint32_t pname, int32_t *params) \
GLE(uint32_t, glCreateShader,            uint32_t type) \
GLE(void,     glShaderSource,            uint32_t shader, int32_t count, const char* const *string, const int32_t *length) \
GLE(void,     glCompileShader,           uint32_t shader) \
GLE(void,     glGetShaderiv,             uint32_t shader, uint32_t pname, int32_t *params) \
GLE(void,     glGetIntegerv,             uint32_t pname, int32_t *params) \
GLE(void,     glGetShaderInfoLog,        uint32_t shader, int32_t bufSize, int32_t *length, char *infoLog) \
GLE(void,     glGetProgramInfoLog,       uint32_t program, int32_t maxLength, int32_t *length, char *infoLog) \
GLE(void,     glDeleteShader,            uint32_t shader) \
GLE(uint32_t, glCreateProgram,           void) \
GLE(void,     glAttachShader,            uint32_t program, uint32_t shader) \
GLE(void,     glDetachShader,            uint32_t program, uint32_t shader) \
GLE(void,     glUseProgram,              uint32_t program) \
GLE(uint32_t, glGetUniformBlockIndex,    uint32_t program, const char *uniformBlockName) \
GLE(void,     glDeleteProgram,           uint32_t program) \
GLE(void,     glGenVertexArrays,         int32_t n, uint32_t *arrays) \
GLE(void,     glBindVertexArray,         uint32_t array) \
GLE(void,     glBufferData,              uint32_t target, int32_t size, const void *data, uint32_t usage) \
GLE(void,     glGenBuffers,              int32_t n, uint32_t *buffers) \
GLE(void,     glBindBuffer,              uint32_t target, uint32_t buffer) \
GLE(void,     glBindVertexBuffer,        uint32_t bindingindex, uint32_t buffer, size_t offset, uint32_t stride) \
GLE(void,     glDeleteBuffers,           int32_t n, const uint32_t *buffers) \
GLE(void,     glGenTextures,             int32_t n, uint32_t *textures) \
GLE(void,     glGenFramebuffers,         int32_t n, uint32_t *ids) \
GLE(void,     glDeleteFramebuffers,      int32_t n, uint32_t *ids) \
GLE(void,     glBindFramebuffer,         uint32_t target, uint32_t framebuffer) \
GLE(void,     glFramebufferTexture,      uint32_t target, uint32_t attachment, uint32_t texture, int32_t level) \
GLE(void,     glFramebufferTexture2D,    uint32_t target, uint32_t attachment, uint32_t textarget, uint32_t texture, int32_t level) \
GLE(void,     glFramebufferTextureLayer, uint32_t target, uint32_t attachment, uint32_t texture, int32_t level, int32_t layer) \
GLE(void,     glBlitFramebuffer,         int32_t srcX0, int32_t srcY0, int32_t srcX1, int32_t srcY1, int32_t dstX0, int32_t dstY0, int32_t dstX1, int32_t dstY1, uint32_t mask, uint32_t filter) \
GLE(void,     glDeleteTextures,          int32_t n, const uint32_t *textures) \
GLE(void,     glBindTexture,             uint32_t target, uint32_t texture) \
GLE(void,     glBindImageTexture,        uint32_t unit, uint32_t texture, int32_t level, unsigned char layered, int32_t layer, uint32_t access, uint32_t format) \
GLE(void,     glTexParameteri,           uint32_t target, uint32_t pname, int32_t param) \
GLE(void,     glGetInternalformativ,     uint32_t target, uint32_t internalformat, uint32_t pname, int32_t bufSize, int32_t *params)\
GLE(void,     glGetTexLevelParameteriv,  uint32_t target, int32_t level, uint32_t pname, int32_t *params) \
GLE(void,     glTexParameterf,           uint32_t target, uint32_t pname, float param) \
GLE(void,     glTexImage2D,              uint32_t target, int32_t level, int32_t internalformat, int32_t width, int32_t height, int32_t border, uint32_t format, uint32_t type, const void *data) \
GLE(void,     glCopyTexSubImage2D,       uint32_t target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t x, int32_t y, uint32_t width, uint32_t height) \
GLE(void,     glGetTexImage,             uint32_t target, int32_t level, uint32_t format, uint32_t type, void *img) \
GLE(void,     glReadPixels,              int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t format, uint32_t type, void *data) \
GLE(void,     glActiveTexture,           uint32_t texture) \
GLE(void,     glGenerateMipmap,          uint32_t target) \
GLE(void,     glBindAttribLocation,      uint32_t program, uint32_t index, const char *name) \
GLE(int32_t,  glGetUniformLocation,      uint32_t program, const char *name) \
GLE(void,     glUniform4f,               int32_t location, float v0, float v1, float v2, float v3) \
GLE(void,     glUniform4fv,              int32_t location, int32_t count, const float *value) \
GLE(void,     glDeleteVertexArrays,      int32_t n, const uint32_t *arrays) \
GLE(void,     glEnableVertexAttribArray, uint32_t index) \
GLE(void,     glVertexAttribPointer,     uint32_t index, int32_t size, uint32_t type, uint8_t normalized, int32_t stride, const void *pointer) \
GLE(void,     glUniform1i,               int32_t location, int32_t v0) \
GLE(void,     glDrawElementsInstanced,   uint32_t mode, int32_t count, uint32_t type, const void *indices, int32_t primcount) \
GLE(void,     glDrawElementsInstancedBaseVertex,   uint32_t mode, int32_t count, uint32_t type, const void *indices, int32_t instancecount, int32_t basevertex) \
GLE(void,     glDrawElements,            uint32_t mode, int32_t count, uint32_t type, const void *indices) \
GLE(void,     glDebugMessageCallback,    GLDEBUGPROC callback, const void *userParam) \
GLE(void,     glBindBufferBase,          uint32_t target, uint32_t index, uint32_t buffer) \
GLE(void,     glBufferSubData,           uint32_t target, int64_t offset, int32_t size, const void *data) \
GLE(void,     glViewport,                int32_t x, int32_t y, uint32_t width, uint32_t height) \
GLE(void,     glScissor,                 int32_t x, int32_t y, uint32_t width, uint32_t height) \
GLE(void,     glCullFace,                uint32_t mode) \
GLE(void,     glBlendFunc,               uint32_t sfactor, uint32_t dfactor) \
GLE(void,     glBlendFuncSeparate,       uint32_t srcRGB, uint32_t dstRGB, uint32_t srcAlpha, uint32_t dstAlpha) \
GLE(void,     glBlendEquationSeparate,   uint32_t modeRGB, uint32_t modeAlpha) \
GLE(void,     glDispatchCompute,         uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) \
GLE(const char *, glGetString,           uint32_t name) \
GLE(const char *, glGetStringi,          uint32_t name, uint32_t index)

#if defined(_SKG_GL_LOAD_WGL)
	// from https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions
	// Some GL functions can only be loaded with wglGetProcAddress, and others
	// can only be loaded by GetProcAddress.
	void *gl_get_function(const char *name) {
		static HMODULE dll = LoadLibraryA("opengl32.dll");
		void *f = (void *)wglGetProcAddress(name);
		if (f == 0 || (f == (void*)0x1) || (f == (void*)0x2) || (f == (void*)0x3) || (f == (void*)-1) ) {
			f = (void *)GetProcAddress(dll, name);
		}
		return f;
	}
#elif defined(_SKG_GL_LOAD_EGL)
	#define gl_get_function(x) eglGetProcAddress(x)
#endif

#define GLE(ret, name, ...) typedef ret GLDECL name##_proc(__VA_ARGS__); static name##_proc * name;
GL_API
#undef GLE

static void gl_load_extensions( ) {
#define GLE(ret, name, ...) name = (name##_proc *) gl_get_function(#name); if (name == nullptr) skg_log(skg_log_info, "Couldn't load gl function " #name);
	GL_API
#undef GLE
}

#endif // _SKG_GL_MAKE_FUNCTIONS

///////////////////////////////////////////

int32_t    gl_active_width        = 0;
int32_t    gl_active_height       = 0;
skg_tex_t *gl_active_rendertarget = nullptr;
uint32_t   gl_current_framebuffer = 0;

///////////////////////////////////////////

uint32_t skg_buffer_type_to_gl   (skg_buffer_type_ type);
uint32_t skg_tex_fmt_to_gl_type  (skg_tex_fmt_ format);
uint32_t skg_tex_fmt_to_gl_layout(skg_tex_fmt_ format);

///////////////////////////////////////////

// Some nice reference: https://gist.github.com/nickrolfe/1127313ed1dbf80254b614a721b3ee9c
int32_t gl_init_wgl() {
#ifdef _SKG_GL_LOAD_WGL
	///////////////////////////////////////////
	// Dummy initialization for pixel format //
	///////////////////////////////////////////

	WNDCLASSA dummy_class = { 0 };
	dummy_class.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	dummy_class.lpfnWndProc   = DefWindowProcA;
	dummy_class.hInstance     = GetModuleHandle(0);
	dummy_class.lpszClassName = "DummyGLWindow";
	if (!RegisterClassA(&dummy_class))
		return false;

	HWND dummy_window = CreateWindowExA(0, dummy_class.lpszClassName, "Dummy GL Window", 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, dummy_class.hInstance, 0);
	if (!dummy_window)
		return false;
	HDC dummy_dc = GetDC(dummy_window);

	PIXELFORMATDESCRIPTOR format_desc = { sizeof(PIXELFORMATDESCRIPTOR) };
	format_desc.nVersion     = 1;
	format_desc.iPixelType   = PFD_TYPE_RGBA;
	format_desc.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	format_desc.cColorBits   = 32;
	format_desc.cAlphaBits   = 8;
	format_desc.iLayerType   = PFD_MAIN_PLANE;
	format_desc.cDepthBits   = 24;
	format_desc.cStencilBits = 8;

	int pixel_format = ChoosePixelFormat(dummy_dc, &format_desc);
	if (!pixel_format) {
		skg_log(skg_log_critical, "Failed to find a suitable pixel format.");
		return false;
	}
	if (!SetPixelFormat(dummy_dc, pixel_format, &format_desc)) {
		skg_log(skg_log_critical, "Failed to set the pixel format.");
		return false;
	}
	HGLRC dummy_context = wglCreateContext(dummy_dc);
	if (!dummy_context) {
		skg_log(skg_log_critical, "Failed to create a dummy OpenGL rendering context.");
		return false;
	}
	if (!wglMakeCurrent(dummy_dc, dummy_context)) {
		skg_log(skg_log_critical, "Failed to activate dummy OpenGL rendering context.");
		return false;
	}

	// Load the function pointers we need to actually initialize OpenGL
	// Function pointers we need to actually initialize OpenGL
	wglChoosePixelFormatARB    = (wglChoosePixelFormatARB_proc   )wglGetProcAddress("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB = (wglCreateContextAttribsARB_proc)wglGetProcAddress("wglCreateContextAttribsARB");

	// Shut down the dummy so we can set up OpenGL for real
	wglMakeCurrent  (dummy_dc, 0);
	wglDeleteContext(dummy_context);
	ReleaseDC       (dummy_window, dummy_dc);
	DestroyWindow   (dummy_window);

	///////////////////////////////////////////
	// Real OpenGL initialization            //
	///////////////////////////////////////////

	WNDCLASSA win_class = { 0 };
	win_class.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	win_class.lpfnWndProc   = DefWindowProcA;
	win_class.hInstance     = GetModuleHandle(0);
	win_class.lpszClassName = "SKGPUWindow";
	if (!RegisterClassA(&win_class))
		return false;

	void *app_hwnd = CreateWindowExA(0, win_class.lpszClassName, "sk_gpu Window", 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, win_class.hInstance, 0);
	if (!app_hwnd)
		return false;

	gl_hwnd = (HWND)app_hwnd;
	gl_hdc  = GetDC(gl_hwnd);

	// Find a pixel format
	const int format_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, true,
		WGL_SUPPORT_OPENGL_ARB, true,
		WGL_DOUBLE_BUFFER_ARB,  true,
		WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,     32,
		WGL_DEPTH_BITS_ARB,     24,
		WGL_STENCIL_BITS_ARB,   8,
		WGL_SAMPLE_BUFFERS_ARB, 1,
		WGL_SAMPLES_ARB,        4,
		0 };

	pixel_format = 0;
	UINT num_formats = 0;
	if (!wglChoosePixelFormatARB(gl_hdc, format_attribs, nullptr, 1, &pixel_format, &num_formats)) {
		skg_log(skg_log_critical, "Couldn't find pixel format!");
		return false;
	}

	memset(&format_desc, 0, sizeof(format_desc));
	DescribePixelFormat(gl_hdc, pixel_format, sizeof(format_desc), &format_desc);
	if (!SetPixelFormat(gl_hdc, pixel_format, &format_desc)) {
		skg_log(skg_log_critical, "Couldn't set pixel format!");
		return false;
	}

	// Create an OpenGL context
	int attributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3, 
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
#if !defined(NDEBUG)
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0 };
	gl_hrc = wglCreateContextAttribsARB( gl_hdc, 0, attributes );
	if (!gl_hrc) {
		skg_log(skg_log_critical, "Couldn't create GL context!");
		return false;
	}
	if (!wglMakeCurrent(gl_hdc, gl_hrc)) {
		skg_log(skg_log_critical, "Couldn't activate GL context!");
		return false;
	}
#endif // _SKG_GL_LOAD_WGL
	return 1;
}

///////////////////////////////////////////

int32_t gl_init_emscripten() {
	// Some reference code:
	// https://github.com/emscripten-core/emscripten/blob/master/tests/glbook/Common/esUtil.c
	// https://github.com/emscripten-core/emscripten/tree/master/tests/minimal_webgl
#ifdef _SKG_GL_LOAD_EMSCRIPTEN
	EmscriptenWebGLContextAttributes attrs;
	emscripten_webgl_init_context_attributes(&attrs);
	attrs.alpha                     = false;
	attrs.depth                     = true;
	attrs.enableExtensionsByDefault = true;
	attrs.majorVersion              = 2;
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("canvas", &attrs);
	emscripten_webgl_make_context_current(ctx);
#endif // _SKG_GL_LOAD_EMSCRIPTEN
	return 1;
}

///////////////////////////////////////////

int32_t gl_init_egl() {
#ifdef _SKG_GL_LOAD_EGL
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_CONFORMANT,   EGL_OPENGL_ES3_BIT_KHR,
		EGL_BLUE_SIZE,  8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE,   8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_NONE
	};
	EGLint context_attribs[] = { 
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE };
	EGLint format;
	EGLint numConfigs;

	egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (eglGetError() != EGL_SUCCESS) { skg_log(skg_log_critical, "Err eglGetDisplay"); return 0; }

	int32_t major=0, minor=0;
	eglInitialize(egl_display, &major, &minor);
	if (eglGetError() != EGL_SUCCESS) { skg_log(skg_log_critical, "Err eglInitialize"); return 0; }
	char version[128];
	snprintf(version, sizeof(version), "EGL version %d.%d", major, minor);
	skg_log(skg_log_info, version);

	eglChooseConfig   (egl_display, attribs, &egl_config, 1, &numConfigs);
	if (eglGetError() != EGL_SUCCESS) { skg_log(skg_log_critical, "Err eglChooseConfig"   ); return 0; }
	eglGetConfigAttrib(egl_display, egl_config, EGL_NATIVE_VISUAL_ID, &format);
	if (eglGetError() != EGL_SUCCESS) { skg_log(skg_log_critical, "Err eglGetConfigAttrib"); return 0; }

	egl_context = eglCreateContext      (egl_display, egl_config, nullptr, context_attribs);
	if (eglGetError() != EGL_SUCCESS) { skg_log(skg_log_critical, "Err eglCreateContext"  ); return 0; }

	if (eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, egl_context) == EGL_FALSE) {
		skg_log(skg_log_critical, "Unable to eglMakeCurrent");
		return -1;
	}
#endif // _SKG_GL_LOAD_EGL
	return 1;
}

///////////////////////////////////////////

int32_t gl_init_glx() {
#ifdef _SKG_GL_LOAD_GLX
	GLXContext old_ctx = glXCreateContext(xDisplay, visualInfo, NULL, GL_TRUE);
	glXMakeCurrent(xDisplay, glxDrawable, old_ctx);

	glewExperimental=true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		skg_log(skg_log_critical, "Failed to initialize GLEW");
		return -1;
	}

	int ctx_attribute_list[] = {
		GLX_RENDER_TYPE,               GLX_RGBA_TYPE,
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 5,
#if !defined(NDEBUG)
		GLX_CONTEXT_FLAGS_ARB,         GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
		GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};
	glxContext = glXCreateContextAttribsARB(xDisplay, glxFBConfig, NULL, GL_TRUE, ctx_attribute_list);
	glXDestroyContext(xDisplay, old_ctx);
	glXMakeCurrent(xDisplay, glxDrawable, glxContext);

#endif // _SKG_GL_LOAD_GLX
	return 1;
}

///////////////////////////////////////////

void skg_setup_xlib(void *dpy, void *vi, void *fbconfig, void *drawable) {
#ifdef _SKG_GL_LOAD_GLX
	xDisplay    =  (Display    *) dpy;
	visualInfo  =  (XVisualInfo*) vi;
	glxFBConfig = *(GLXFBConfig*) fbconfig;
	glxDrawable = *(Drawable   *) drawable;
#endif
}

///////////////////////////////////////////

int32_t skg_init(const char *app_name, void *adapter_id) {
#if   defined(_SKG_GL_LOAD_WGL)
	int32_t result = gl_init_wgl();
#elif defined(_SKG_GL_LOAD_EGL)
	int32_t result = gl_init_egl();
#elif defined(_SKG_GL_LOAD_GLX)
	int32_t result = gl_init_glx();
#elif defined(_SKG_GL_LOAD_EMSCRIPTEN)
	int32_t result = gl_init_emscripten();
#endif
	if (!result)
		return result;

	// Load OpenGL function pointers
#ifdef _SKG_GL_MAKE_FUNCTIONS
	gl_load_extensions();
#endif

	skg_log(skg_log_info, "Using OpenGL");
	skg_log(skg_log_info, (char*)glGetString(GL_VERSION));

#if !defined(NDEBUG) && !defined(_SKG_GL_WEB)
	skg_log(skg_log_info, "Debug info enabled.");
	// Set up debug info for development
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback([](uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length, const char *message, const void *userParam) {
		if (id == 0x7fffffff) return;

		const char *src = "OTHER";
		switch (source) {
		case 0x8246: src = "API"; break;
		case 0x8247: src = "WINDOW SYSTEM"; break;
		case 0x8248: src = "SHADER COMPILER"; break;
		case 0x8249: src = "THIRD PARTY"; break;
		case 0x824A: src = "APPLICATION"; break;
		case 0x824B: src = "OTHER"; break;
		}

		const char *type_str = "OTHER";
		switch (type) {
		case 0x824C: type_str = "ERROR"; break;
		case 0x824D: type_str = "DEPRECATED_BEHAVIOR"; break;
		case 0x824E: type_str = "UNDEFINED_BEHAVIOR"; break;
		case 0x824F: type_str = "PORTABILITY"; break;
		case 0x8250: type_str = "PERFORMANCE"; break;
		case 0x8268: type_str = "MARKER"; break;
		case 0x8251: type_str = "OTHER"; break;
		}

		char msg[1024];
		snprintf(msg, sizeof(msg), "%s/%s - 0x%x - %s", src, type_str, id, message);

		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: break;
		case GL_DEBUG_SEVERITY_LOW:    skg_log(skg_log_info,     msg); break;
		case GL_DEBUG_SEVERITY_MEDIUM: skg_log(skg_log_warning,  msg); break;
		case GL_DEBUG_SEVERITY_HIGH:   skg_log(skg_log_critical, msg); break;
		}
	}, nullptr);
#endif // !defined(NDEBUG) && !defined(_SKG_GL_WEB)
	
	// Some default behavior
	glEnable   (GL_DEPTH_TEST);  
	glEnable   (GL_CULL_FACE);
	glCullFace (GL_BACK);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
#ifdef _SKG_GL_DESKTOP
	glEnable   (GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif
	
	return 1;
}

///////////////////////////////////////////

void skg_shutdown() {
#if defined(_SKG_GL_LOAD_WGL)
	wglMakeCurrent(NULL, NULL);
	ReleaseDC(gl_hwnd, gl_hdc);
	wglDeleteContext(gl_hrc);
#elif defined(_SKG_GL_LOAD_EGL)
	if (egl_display != EGL_NO_DISPLAY) {
		eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (egl_context != EGL_NO_CONTEXT) eglDestroyContext(egl_display, egl_context);
		eglTerminate(egl_display);
	}
	egl_display = EGL_NO_DISPLAY;
	egl_context = EGL_NO_CONTEXT;
#endif
}

///////////////////////////////////////////

void skg_draw_begin() {
}

///////////////////////////////////////////

void skg_tex_target_bind(skg_tex_t *render_target) {
	gl_active_rendertarget = render_target;
	gl_current_framebuffer = render_target == nullptr ? 0 : render_target->_framebuffer;

	glBindFramebuffer(GL_FRAMEBUFFER, gl_current_framebuffer);
	if (render_target) {
		glViewport(0, 0, render_target->width, render_target->height);
	} else {
		glViewport(0, 0, gl_active_width, gl_active_height);
	}

#ifndef _SKG_GL_WEB
	if (render_target == nullptr || render_target->format == skg_tex_fmt_rgba32 || render_target->format == skg_tex_fmt_bgra32) {
		glEnable(GL_FRAMEBUFFER_SRGB);
	} else {
		glDisable(GL_FRAMEBUFFER_SRGB);
	}
#endif
}

///////////////////////////////////////////

void skg_target_clear(bool depth, const float *clear_color_4) {
	uint32_t clear_mask = 0;
	if (depth) {
		clear_mask = GL_DEPTH_BUFFER_BIT;
		// If DepthMask is false, glClear won't clear depth
		glDepthMask(true);
	}
	if (clear_color_4) {
		clear_mask = clear_mask | GL_COLOR_BUFFER_BIT;
		glClearColor(clear_color_4[0], clear_color_4[1], clear_color_4[2], clear_color_4[3]);
	}

	glClear(clear_mask);
}

///////////////////////////////////////////

skg_tex_t *skg_tex_target_get() {
	return gl_active_rendertarget;
}

///////////////////////////////////////////

skg_platform_data_t skg_get_platform_data() {
	skg_platform_data_t result = {};
#if   defined(_SKG_GL_LOAD_WGL)
	result._gl_hdc = gl_hdc;
	result._gl_hrc = gl_hrc;
#elif defined(_SKG_GL_LOAD_EGL)
	result._egl_display = egl_display;
	result._egl_config  = egl_config;
	result._egl_context = egl_context;
#elif defined(_SKG_GL_LOAD_GLX)
	result._x_display     = xDisplay;
	result._visual_id     = &visualInfo->visualid;
	result._glx_fb_config = glxFBConfig;
	result._glx_drawable  = &glxDrawable;
	result._glx_context   = glxContext;
#endif
	return result;
}

///////////////////////////////////////////

bool skg_capability(skg_cap_ capability) {
	bool (*check_ext)(const char *name) = [](const char *name) {
		int32_t ct;
		glGetIntegerv(GL_NUM_EXTENSIONS, &ct);
		for (int32_t i = 0; i < ct; i++) {
			if (strcmp(name, (const char *)glGetStringi(GL_EXTENSIONS, i)) == 0)
				return true;
		}
		return false;
	};

	switch (capability) {
	case skg_cap_tex_layer_select: return check_ext("GL_AMD_vertex_shader_layer");
	case skg_cap_wireframe:
#ifdef _SKG_GL_WEB
		return false;
#else
#pragma clang diagnostic push
		// On some platforms, glPolygonMode is a function and not a function pointer, so glPolygonMode != nullptr is trivially true, and Clang wants to warn us about that. This isn't an actual problem, so let's suppress that warning.
#pragma clang diagnostic ignored "-Wtautological-pointer-compare"
		return glPolygonMode != nullptr;
#pragma clang diagnostic pop
#endif
	default: return false;
	}
}

///////////////////////////////////////////

void skg_draw(int32_t index_start, int32_t index_base, int32_t index_count, int32_t instance_count) {
#ifdef _SKG_GL_WEB
	glDrawElementsInstanced(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, (void*)(index_start*sizeof(uint32_t)), instance_count);
#else
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, (void*)(index_start*sizeof(uint32_t)), instance_count, index_base);
#endif
}

///////////////////////////////////////////

void skg_compute(uint32_t thread_count_x, uint32_t thread_count_y, uint32_t thread_count_z) {
	glDispatchCompute(thread_count_x, thread_count_y, thread_count_z);
}

///////////////////////////////////////////

void skg_viewport(const int32_t *xywh) {
	glViewport(xywh[0], xywh[1], xywh[2], xywh[3]);
}

///////////////////////////////////////////

void skg_viewport_get(int32_t *out_xywh) {
	glGetIntegerv(GL_VIEWPORT, out_xywh);
}

///////////////////////////////////////////

void skg_scissor(const int32_t *xywh) {
	int32_t viewport[4];
	skg_viewport_get(viewport);
	glScissor(xywh[0], (viewport[3]-xywh[1])-xywh[3], xywh[2], xywh[3]);
}

///////////////////////////////////////////

skg_buffer_t skg_buffer_create(const void *data, uint32_t size_count, uint32_t size_stride, skg_buffer_type_ type, skg_use_ use) {
	skg_buffer_t result = {};
	result.use     = use;
	result.type    = type;
	result.stride  = size_stride;
	result._target = skg_buffer_type_to_gl(type);

	glGenBuffers(1, &result._buffer);
	glBindBuffer(result._target, result._buffer);
	glBufferData(result._target, size_count * size_stride, data, use == skg_use_static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

	return result;
}

///////////////////////////////////////////

bool skg_buffer_is_valid(const skg_buffer_t *buffer) {
	return buffer->_buffer != 0;
}

///////////////////////////////////////////

void skg_buffer_set_contents(skg_buffer_t *buffer, const void *data, uint32_t size_bytes) {
	if (buffer->use != skg_use_dynamic) {
		skg_log(skg_log_warning, "Attempting to dynamically set contents of a static buffer!");
		return;
	}

	glBindBuffer   (buffer->_target, buffer->_buffer);
	glBufferSubData(buffer->_target, 0, size_bytes, data);
}

///////////////////////////////////////////

void skg_buffer_bind(const skg_buffer_t *buffer, skg_bind_t bind, uint32_t offset) {
	if (buffer->type == skg_buffer_type_constant || buffer->type == skg_buffer_type_compute)
		glBindBufferBase(buffer->_target, bind.slot, buffer->_buffer);
	else if (buffer->type == skg_buffer_type_vertex) {
#ifdef _SKG_GL_WEB
		glBindBuffer(buffer->_target, buffer->_buffer);
#else
		glBindVertexBuffer(bind.slot, buffer->_buffer, offset, buffer->stride);
#endif
	} else
		glBindBuffer(buffer->_target, buffer->_buffer);
}

///////////////////////////////////////////

void skg_buffer_clear(skg_bind_t bind) {
	if (bind.stage_bits == skg_stage_compute) {
		if (bind.register_type == skg_register_constant)
			glBindBufferBase(GL_UNIFORM_BUFFER, bind.slot, 0);
		if (bind.register_type == skg_register_readwrite)
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bind.slot, 0);
	}
}

///////////////////////////////////////////

void skg_buffer_destroy(skg_buffer_t *buffer) {
	uint32_t buffer_list[] = { buffer->_buffer };
	glDeleteBuffers(1, buffer_list);
	*buffer = {};
}

///////////////////////////////////////////

skg_mesh_t skg_mesh_create(const skg_buffer_t *vert_buffer, const skg_buffer_t *ind_buffer) {
	skg_mesh_t result = {};
	skg_mesh_set_verts(&result, vert_buffer);
	skg_mesh_set_inds (&result, ind_buffer);

	return result;
}

///////////////////////////////////////////

void skg_mesh_set_verts(skg_mesh_t *mesh, const skg_buffer_t *vert_buffer) {
	mesh->_vert_buffer = vert_buffer ? vert_buffer->_buffer : 0;
	if (mesh->_vert_buffer != 0) {
		if (mesh->_layout != 0) {
			glDeleteVertexArrays(1, &mesh->_layout);
			mesh->_layout = 0;
		}

		glBindBuffer(GL_ARRAY_BUFFER, mesh->_vert_buffer);

		// Create a vertex layout
		glGenVertexArrays(1, &mesh->_layout);
		glBindVertexArray(mesh->_layout);
		// enable the vertex data for the shader
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		// tell the shader how our vertex data binds to the shader inputs
		glVertexAttribPointer(0, 3, GL_FLOAT,         0, sizeof(skg_vert_t), nullptr);
		glVertexAttribPointer(1, 3, GL_FLOAT,         0, sizeof(skg_vert_t), (void*)(sizeof(float) * 3));
		glVertexAttribPointer(2, 2, GL_FLOAT,         0, sizeof(skg_vert_t), (void*)(sizeof(float) * 6));
		glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, 1, sizeof(skg_vert_t), (void*)(sizeof(float) * 8));
	}
}

///////////////////////////////////////////

void skg_mesh_set_inds(skg_mesh_t *mesh, const skg_buffer_t *ind_buffer) {
	mesh->_ind_buffer = ind_buffer ? ind_buffer->_buffer : 0;
}

///////////////////////////////////////////

void skg_mesh_bind(const skg_mesh_t *mesh) {
	glBindVertexArray(mesh->_layout);
	glBindBuffer(GL_ARRAY_BUFFER,         mesh->_vert_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->_ind_buffer );
}

///////////////////////////////////////////

void skg_mesh_destroy(skg_mesh_t *mesh) {
	uint32_t vao_list[] = {mesh->_layout};
	glDeleteVertexArrays(1, vao_list);
	*mesh = {};
}

///////////////////////////////////////////
// skg_shader_t                          //
///////////////////////////////////////////

skg_shader_stage_t skg_shader_stage_create(const void *file_data, size_t shader_size, skg_stage_ type) {
	const char *file_chars = (const char *)file_data;

	skg_shader_stage_t result = {}; 
	result.type = type;

	// Include terminating character
	if (shader_size > 0 && file_chars[shader_size-1] != '\0')
		shader_size += 1;

	uint32_t gl_type = 0;
	switch (type) {
	case skg_stage_pixel:   gl_type = GL_FRAGMENT_SHADER; break;
	case skg_stage_vertex:  gl_type = GL_VERTEX_SHADER;   break;
	case skg_stage_compute: gl_type = GL_COMPUTE_SHADER;  break;
	}

	// Convert the prefix if it doesn't match the GL version we're using
#if   defined(_SKG_GL_ES)
	const char   *prefix_gl      = "#version 320 es";
#elif defined(_SKG_GL_DESKTOP)
	const char   *prefix_gl      = "#version 450";
#elif defined(_SKG_GL_WEB)
	const char   *prefix_gl      = "#version 300 es";
#endif
	const size_t  prefix_gl_size = strlen(prefix_gl);
	char         *final_data = (char*)file_chars;
	bool          needs_free = false;

	if (shader_size >= prefix_gl_size && memcmp(prefix_gl, file_chars, prefix_gl_size) != 0) {
		const char *end = file_chars;
		while (*end != '\n' && *end != '\r' && *end != '\0') end++;
		size_t version_size = end - file_chars;

		final_data = (char*)malloc(sizeof(char) * ((shader_size-version_size)+prefix_gl_size));
		memcpy(final_data, prefix_gl, prefix_gl_size);
		memcpy(&final_data[prefix_gl_size], &file_chars[version_size], shader_size - version_size);
		needs_free = true;
	}

	// create and compile the vertex shader
	result._shader = glCreateShader(gl_type);
	try {
		glShaderSource (result._shader, 1, &final_data, NULL);
		glCompileShader(result._shader);
	} catch (...) {
		// Some GL drivers have a habit of crashing during shader compile.
		const char *stage_name = "";
		char        text[64];
		switch (type) {
			case skg_stage_pixel:   stage_name = "Pixel";   break;
			case skg_stage_vertex:  stage_name = "Vertex";  break;
			case skg_stage_compute: stage_name = "Compute"; break; }
		snprintf(text, sizeof(text), "%s shader compile exception", stage_name);
		skg_log(skg_log_warning, text);
		glDeleteShader(result._shader);
		result._shader = 0;
		if (needs_free)
			free(final_data);
		return result;
	}

	// check for errors?
	int32_t err, length;
	glGetShaderiv(result._shader, GL_COMPILE_STATUS, &err);
	if (err == 0) {
		char *log;

		glGetShaderiv(result._shader, GL_INFO_LOG_LENGTH, &length);
		log = (char*)malloc(length);
		glGetShaderInfoLog(result._shader, length, &err, log);

		skg_log(skg_log_warning, "Unable to compile shader:\n");
		skg_log(skg_log_warning, log);
		free(log);

		glDeleteShader(result._shader);
		result._shader = 0;
	}
	if (needs_free)
		free(final_data);

	return result;
}

///////////////////////////////////////////

void skg_shader_stage_destroy(skg_shader_stage_t *shader) {
	//glDeleteShader(shader->shader);
	*shader = {};
}

///////////////////////////////////////////

skg_shader_t skg_shader_create_manual(skg_shader_meta_t *meta, skg_shader_stage_t v_shader, skg_shader_stage_t p_shader, skg_shader_stage_t c_shader) {
	if (v_shader._shader == 0 && p_shader._shader == 0 && c_shader._shader == 0) {
		char text[290];
		snprintf(text, sizeof(text), "Shader '%s' has no valid stages!", meta->name);
		skg_log(skg_log_warning, text);
		return {};
	}

	skg_shader_t result = {};
	result.meta     = meta;
	result._vertex  = v_shader._shader;
	result._pixel   = p_shader._shader;
	result._compute = c_shader._shader;
	skg_shader_meta_reference(result.meta);

	result._program = glCreateProgram();
	if (result._vertex)  glAttachShader(result._program, result._vertex);
	if (result._pixel)   glAttachShader(result._program, result._pixel);
	if (result._compute) glAttachShader(result._program, result._compute);
	try {
		glLinkProgram(result._program);
	} catch (...) {
		// Some GL drivers have a habit of crashing during shader compile.
		char text[286];
		snprintf(text, sizeof(text), "Shader link exception in %s:", meta->name);
		skg_log(skg_log_warning, text);
		glDeleteProgram(result._program);
		result._program = 0;
		return result;
	}

	// check for errors?
	int32_t err, length;
	glGetProgramiv(result._program, GL_LINK_STATUS, &err);
	if (err == 0) {
		char *log;

		glGetProgramiv(result._program, GL_INFO_LOG_LENGTH, &length);
		log = (char*)malloc(length);
		glGetProgramInfoLog(result._program, length, &err, log);

		char text[272];
		snprintf(text, sizeof(text), "Unable to link %s:", meta->name);
		skg_log(skg_log_warning, text);
		skg_log(skg_log_warning, log);
		free(log);

		glDeleteProgram(result._program);
		result._program = 0;
	} else {
#ifdef _SKG_GL_WEB
		for (size_t i = 0; i < meta->buffer_count; i++) {
			char t_name[64];
			snprintf(t_name, 64, "%s", meta->buffers[i].name);
			// $Global is a near universal buffer name, we need to scrape the
			// '$' character out.
			char *pr = t_name;
			while (*pr) {
				if (*pr == '$')
					*pr = '_';
				pr++;
			}

			uint32_t slot = glGetUniformBlockIndex(result._program, t_name);
			glUniformBlockBinding(result._program, slot, meta->buffers[i].bind.slot);

			if (slot == GL_INVALID_INDEX) {
				skg_log(skg_log_warning, "Couldn't find uniform block index for:");
				skg_log(skg_log_warning, meta->buffers[i].name);
			}
		}
		glUseProgram(result._program);
		for (size_t i = 0; i < meta->resource_count; i++) {
			uint32_t loc = glGetUniformLocation(result._program, meta->resources[i].name);
			glUniform1i(loc , meta->resources[i].bind.slot);
		}
#endif
	}

	return result;
}


///////////////////////////////////////////

void skg_shader_compute_bind(const skg_shader_t *shader) {
	if (shader) glUseProgram(shader->_program);
	else        glUseProgram(0);
}

///////////////////////////////////////////

bool skg_shader_is_valid(const skg_shader_t *shader) {
	return shader->meta
		&& shader->_program;
}

///////////////////////////////////////////

void skg_shader_destroy(skg_shader_t *shader) {
	skg_shader_meta_release(shader->meta);
	glDeleteProgram(shader->_program);
	glDeleteShader (shader->_vertex);
	glDeleteShader (shader->_pixel);
	*shader = {};
}

///////////////////////////////////////////
// skg_pipeline                          //
///////////////////////////////////////////

skg_pipeline_t skg_pipeline_create(skg_shader_t *shader) {
	skg_pipeline_t result = {};
	result.transparency = skg_transparency_none;
	result.cull         = skg_cull_back;
	result.wireframe    = false;
	result.depth_test   = skg_depth_test_less;
	result.depth_write  = true;
	result.meta         = shader->meta;
	result._shader      = *shader;
	skg_shader_meta_reference(result._shader.meta);

	return result;
}

///////////////////////////////////////////

void skg_pipeline_bind(const skg_pipeline_t *pipeline) {
	glUseProgram(pipeline->_shader._program);
	
	switch (pipeline->transparency) {
	case skg_transparency_blend:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
		break;
	case skg_transparency_add:
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		break;
	case skg_transparency_none:
		glDisable(GL_BLEND);
		break;
	}

	switch (pipeline->cull) {
	case skg_cull_back: {
		glEnable  (GL_CULL_FACE);
		glCullFace(GL_BACK);
	} break;
	case skg_cull_front: {
		glEnable  (GL_CULL_FACE);
		glCullFace(GL_FRONT);
	} break;
	case skg_cull_none: {
		glDisable(GL_CULL_FACE);
	} break;
	}

	if (pipeline->depth_test != skg_depth_test_always)
		 glEnable (GL_DEPTH_TEST);
	else glDisable(GL_DEPTH_TEST);

	if (pipeline->scissor) glEnable (GL_SCISSOR_TEST);
	else                   glDisable(GL_SCISSOR_TEST);

	glDepthMask(pipeline->depth_write);
	switch (pipeline->depth_test) {
	case skg_depth_test_always:        glDepthFunc(GL_ALWAYS);   break;
	case skg_depth_test_equal:         glDepthFunc(GL_EQUAL);    break;
	case skg_depth_test_greater:       glDepthFunc(GL_GREATER);  break;
	case skg_depth_test_greater_or_eq: glDepthFunc(GL_GEQUAL);   break;
	case skg_depth_test_less:          glDepthFunc(GL_LESS);     break;
	case skg_depth_test_less_or_eq:    glDepthFunc(GL_LEQUAL);   break;
	case skg_depth_test_never:         glDepthFunc(GL_NEVER);    break;
	case skg_depth_test_not_equal:     glDepthFunc(GL_NOTEQUAL); break; }
	
#ifdef _SKG_GL_DESKTOP
	if (pipeline->wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
#endif
}

///////////////////////////////////////////

void skg_pipeline_set_transparency(skg_pipeline_t *pipeline, skg_transparency_ transparency) {
	pipeline->transparency = transparency;
}

///////////////////////////////////////////

void skg_pipeline_set_cull(skg_pipeline_t *pipeline, skg_cull_ cull) {
	pipeline->cull = cull;
}

///////////////////////////////////////////

void skg_pipeline_set_wireframe(skg_pipeline_t *pipeline, bool wireframe) {
	pipeline->wireframe = wireframe;
}

///////////////////////////////////////////

void skg_pipeline_set_depth_write(skg_pipeline_t *pipeline, bool write) {
	pipeline->depth_write = write;
}

///////////////////////////////////////////

void skg_pipeline_set_depth_test (skg_pipeline_t *pipeline, skg_depth_test_ test) {
	pipeline->depth_test = test;
}

///////////////////////////////////////////

void skg_pipeline_set_scissor(skg_pipeline_t *pipeline, bool enable) {
	pipeline->scissor = enable;

}

///////////////////////////////////////////

skg_transparency_ skg_pipeline_get_transparency(const skg_pipeline_t *pipeline) {
	return pipeline->transparency;
}

///////////////////////////////////////////

skg_cull_ skg_pipeline_get_cull(const skg_pipeline_t *pipeline) {
	return pipeline->cull;
}

///////////////////////////////////////////

bool skg_pipeline_get_wireframe(const skg_pipeline_t *pipeline) {
	return pipeline->wireframe;
}

///////////////////////////////////////////

bool skg_pipeline_get_depth_write(const skg_pipeline_t *pipeline) {
	return pipeline->depth_write;
}

///////////////////////////////////////////

skg_depth_test_ skg_pipeline_get_depth_test(const skg_pipeline_t *pipeline) {
	return pipeline->depth_test;
}

///////////////////////////////////////////

bool skg_pipeline_get_scissor(const skg_pipeline_t *pipeline) {
	return pipeline->scissor;
}

///////////////////////////////////////////

void skg_pipeline_destroy(skg_pipeline_t *pipeline) {
	skg_shader_meta_release(pipeline->_shader.meta);
	*pipeline = {};
}

///////////////////////////////////////////

skg_swapchain_t skg_swapchain_create(void *hwnd, skg_tex_fmt_ format, skg_tex_fmt_ depth_format, int32_t requested_width, int32_t requested_height) {
	skg_swapchain_t result = {};

#if defined(_SKG_GL_LOAD_WGL)
	result._hwnd  = hwnd;
	result._hdc   = GetDC((HWND)hwnd);
	result.width  = requested_width;
	result.height = requested_height;

	// Find a pixel format
	const int format_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, true,
		WGL_SUPPORT_OPENGL_ARB, true,
		WGL_DOUBLE_BUFFER_ARB,  true,
		WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,     32,
		WGL_DEPTH_BITS_ARB,     24,
		WGL_STENCIL_BITS_ARB,   8,
		WGL_SAMPLE_BUFFERS_ARB, 1,
		WGL_SAMPLES_ARB,        4,
		0 };

	int  pixel_format = 0;
	UINT num_formats  = 0;
	if (!wglChoosePixelFormatARB((HDC)result._hdc, format_attribs, nullptr, 1, &pixel_format, &num_formats)) {
		skg_log(skg_log_critical, "Couldn't find pixel format!");
		result = {};
		return result;
	}

	PIXELFORMATDESCRIPTOR format_desc = { sizeof(PIXELFORMATDESCRIPTOR) };
	DescribePixelFormat((HDC)result._hdc, pixel_format, sizeof(format_desc), &format_desc);
	if (!SetPixelFormat((HDC)result._hdc, pixel_format, &format_desc)) {
		skg_log(skg_log_critical, "Couldn't set pixel format!");
		result = {};
		return result;
	}
#elif defined(_SKG_GL_LOAD_EGL)
	EGLint attribs[] = { 
		EGL_GL_COLORSPACE_KHR, EGL_GL_COLORSPACE_SRGB_KHR,
		EGL_NONE };
	result._egl_surface = eglCreateWindowSurface(egl_display, egl_config, (EGLNativeWindowType)hwnd, attribs);
	if (eglGetError() != EGL_SUCCESS) skg_log(skg_log_critical, "Err eglCreateWindowSurface");

	eglQuerySurface(egl_display, result._egl_surface, EGL_WIDTH,  &result.width );
	eglQuerySurface(egl_display, result._egl_surface, EGL_HEIGHT, &result.height);
#elif defined(_SKG_GL_LOAD_GLX)
	result._x_window = hwnd;
	result.width  = requested_width;
	result.height = requested_height;
#else
	int32_t viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	result.width  = viewport[2];
	result.height = viewport[3];
#endif

#if defined(_SKG_GL_WEB) && defined(SKG_MANUAL_SRGB)
	const char *vs = R"_(#version 300 es
layout(location = 0) in vec4 in_var_SV_POSITION;
layout(location = 1) in vec3 in_var_NORMAL;
layout(location = 2) in vec2 in_var_TEXCOORD0;
layout(location = 3) in vec4 in_var_COLOR;

out vec2 fs_var_TEXCOORD0;

void main() {
    gl_Position = in_var_SV_POSITION;
    fs_var_TEXCOORD0 = in_var_TEXCOORD0;
})_";
	const char *ps = R"_(#version 300 es
precision mediump float;
precision highp int;

uniform highp sampler2D tex;

in highp vec2 fs_var_TEXCOORD0;
layout(location = 0) out highp vec4 out_var_SV_TARGET;

void main() {
	vec4 color = texture(tex, fs_var_TEXCOORD0);
    out_var_SV_TARGET = vec4(pow(color.xyz, vec3(1.0 / 2.2)), color.w);
})_";

	skg_shader_meta_t *meta = (skg_shader_meta_t *)malloc(sizeof(skg_shader_meta_t));
	*meta = {};
	meta->resource_count = 1;
	meta->resources = (skg_shader_resource_t*)malloc(sizeof(skg_shader_resource_t));
	meta->resources[0].bind = { 0, skg_stage_pixel };
	strcpy(meta->resources[0].name, "tex");
	meta->resources[0].name_hash = skg_hash(meta->resources[0].name);

	skg_shader_stage_t v_stage = skg_shader_stage_create(vs, strlen(vs), skg_stage_vertex);
	skg_shader_stage_t p_stage = skg_shader_stage_create(ps, strlen(ps), skg_stage_pixel);
	result._convert_shader = skg_shader_create_manual(meta, v_stage, p_stage, {});
	result._convert_pipe   = skg_pipeline_create(&result._convert_shader);

	result._surface = skg_tex_create(skg_tex_type_rendertarget, skg_use_static, skg_tex_fmt_rgba32_linear, skg_mip_none);
	skg_tex_set_contents(&result._surface, nullptr, result.width, result.height);

	result._surface_depth = skg_tex_create(skg_tex_type_depth, skg_use_static, depth_format, skg_mip_none);
	skg_tex_set_contents(&result._surface_depth, nullptr, result.width, result.height);
	skg_tex_attach_depth(&result._surface, &result._surface_depth);

	skg_vert_t quad_verts[] = { 
		{ {-1, 1,0}, {0,0,1}, {0,1}, {255,255,255,255} },
		{ { 1, 1,0}, {0,0,1}, {1,1}, {255,255,255,255} },
		{ { 1,-1,0}, {0,0,1}, {1,0}, {255,255,255,255} },
		{ {-1,-1,0}, {0,0,1}, {0,0}, {255,255,255,255} } };
	uint32_t quad_inds[] = { 2,1,0, 3,2,0 };
	result._quad_vbuff = skg_buffer_create(quad_verts, 4, sizeof(skg_vert_t), skg_buffer_type_vertex, skg_use_static);
	result._quad_ibuff = skg_buffer_create(quad_inds,  6, sizeof(uint32_t  ), skg_buffer_type_index,  skg_use_static);
	result._quad_mesh  = skg_mesh_create(&result._quad_vbuff, &result._quad_ibuff);
#endif
	return result;
}

///////////////////////////////////////////

void skg_swapchain_resize(skg_swapchain_t *swapchain, int32_t width, int32_t height) {
	if (width == swapchain->width && height == swapchain->height)
		return;

	swapchain->width  = width;
	swapchain->height = height;

#ifdef _SKG_GL_WEB
	skg_tex_fmt_ color_fmt = swapchain->_surface.format;
	skg_tex_fmt_ depth_fmt = swapchain->_surface_depth.format;

	skg_tex_destroy(&swapchain->_surface);
	skg_tex_destroy(&swapchain->_surface_depth);

	swapchain->_surface = skg_tex_create(skg_tex_type_rendertarget, skg_use_static, color_fmt, skg_mip_none);
	skg_tex_set_contents(&swapchain->_surface, nullptr, swapchain->width, swapchain->height);

	swapchain->_surface_depth = skg_tex_create(skg_tex_type_depth, skg_use_static, depth_fmt, skg_mip_none);
	skg_tex_set_contents(&swapchain->_surface_depth, nullptr, swapchain->width, swapchain->height);
	skg_tex_attach_depth(&swapchain->_surface, &swapchain->_surface_depth);
#endif
}

///////////////////////////////////////////

void skg_swapchain_present(skg_swapchain_t *swapchain) {
#if   defined(_SKG_GL_LOAD_WGL)
	SwapBuffers((HDC)swapchain->_hdc);
#elif defined(_SKG_GL_LOAD_EGL)
	eglSwapBuffers(egl_display, swapchain->_egl_surface);
#elif defined(_SKG_GL_LOAD_GLX)
	glXSwapBuffers(xDisplay, (Drawable) swapchain->_x_window);
#elif defined(_SKG_GL_LOAD_EMSCRIPTEN) && defined(SKG_MANUAL_SRGB)
	float clear[4] = { 0,0,0,1 };
	skg_tex_target_bind(nullptr);
	skg_target_clear   (true, clear);
	skg_tex_bind      (&swapchain->_surface, {0, skg_stage_pixel});
	skg_mesh_bind     (&swapchain->_quad_mesh);
	skg_pipeline_bind (&swapchain->_convert_pipe);
	skg_draw          (0, 0, 6, 1);
#endif
}

///////////////////////////////////////////

void skg_swapchain_bind(skg_swapchain_t *swapchain) {
	gl_active_width  = swapchain->width;
	gl_active_height = swapchain->height;
#if   defined(_SKG_GL_LOAD_EMSCRIPTEN) && defined(SKG_MANUAL_SRGB)
	skg_tex_target_bind(&swapchain->_surface);
#elif defined(_SKG_GL_LOAD_WGL)
	wglMakeCurrent((HDC)swapchain->_hdc, gl_hrc);
	skg_tex_target_bind(nullptr);
#elif defined(_SKG_GL_LOAD_EGL)
	eglMakeCurrent(egl_display, swapchain->_egl_surface, swapchain->_egl_surface, egl_context);
	skg_tex_target_bind(nullptr);
#elif defined(_SKG_GL_LOAD_GLX)
	glXMakeCurrent(xDisplay, (Drawable)swapchain->_x_window, glxContext);
	skg_tex_target_bind(nullptr);
#endif
}

///////////////////////////////////////////

void skg_swapchain_destroy(skg_swapchain_t *swapchain) {
#if defined(_SKG_GL_LOAD_WGL)
	if (swapchain->_hdc != nullptr) {
		wglMakeCurrent(nullptr, nullptr);
		ReleaseDC((HWND)swapchain->_hwnd, (HDC)swapchain->_hdc);
		swapchain->_hwnd = nullptr;
		swapchain->_hdc  = nullptr;
	}
#elif defined(_SKG_GL_LOAD_EGL)
	eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	if (swapchain->_egl_surface != EGL_NO_SURFACE) eglDestroySurface(egl_display, swapchain->_egl_surface);
	swapchain->_egl_surface = EGL_NO_SURFACE;
#endif
}

///////////////////////////////////////////

skg_tex_t skg_tex_create_from_existing(void *native_tex, skg_tex_type_ type, skg_tex_fmt_ format, int32_t width, int32_t height, int32_t array_count) {
	skg_tex_t result = {};
	result.type        = type;
	result.use         = skg_use_static;
	result.mips        = skg_mip_none;
	result.format      = format;
	result.width       = width;
	result.height      = height;
	result.array_count = array_count;
	result._texture    = (uint32_t)(uint64_t)native_tex;
	result._format     = (uint32_t)skg_tex_fmt_to_native(result.format);
	result._target     = type == skg_tex_type_cubemap 
		? GL_TEXTURE_CUBE_MAP 
		: array_count > 1 
			? GL_TEXTURE_2D_ARRAY
			: GL_TEXTURE_2D;

	if (type == skg_tex_type_rendertarget) {
		glGenFramebuffers(1, &result._framebuffer);

		glBindFramebuffer(GL_FRAMEBUFFER, result._framebuffer);
		if (array_count != 1) {
#ifndef _SKG_GL_WEB
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, result._texture, 0);
#else
			skg_log(skg_log_critical, "sk_gpu doesn't support array textures with WebGL?");
#endif
		} else {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, result._target, result._texture, 0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, gl_current_framebuffer);
	}
	
	return result;
}

///////////////////////////////////////////

skg_tex_t skg_tex_create_from_layer(void *native_tex, skg_tex_type_ type, skg_tex_fmt_ format, int32_t width, int32_t height, int32_t array_layer) {
	skg_tex_t result = {};
	result.type        = type;
	result.use         = skg_use_static;
	result.mips        = skg_mip_none;
	result.format      = format;
	result.width       = width;
	result.height      = height;
	result.array_count = 1;
	result.array_start = array_layer;
	result._texture    = (uint32_t)(uint64_t)native_tex;
	result._format     = (uint32_t)skg_tex_fmt_to_native(result.format);
	result._target     = type == skg_tex_type_cubemap 
		? GL_TEXTURE_CUBE_MAP
		: GL_TEXTURE_2D_ARRAY;

	if (type == skg_tex_type_rendertarget) {
		glGenFramebuffers(1, &result._framebuffer);
		glBindFramebuffer        (GL_FRAMEBUFFER, result._framebuffer);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, result._texture, 0, array_layer);
		glBindFramebuffer        (GL_FRAMEBUFFER, gl_current_framebuffer);
	}

	return result;
}

///////////////////////////////////////////

skg_tex_t skg_tex_create(skg_tex_type_ type, skg_use_ use, skg_tex_fmt_ format, skg_mip_ mip_maps) {
	skg_tex_t result = {};
	result.type    = type;
	result.use     = use;
	result.format  = format;
	result.mips    = mip_maps;
	result._format = (uint32_t)skg_tex_fmt_to_native(result.format);
	result._target = type == skg_tex_type_cubemap 
		? GL_TEXTURE_CUBE_MAP 
		: GL_TEXTURE_2D;

	if      (use & skg_use_compute_read && use & skg_use_compute_write) result._access = GL_READ_WRITE;
	else if (use & skg_use_compute_read)                                result._access = GL_READ_ONLY;
	else if (use & skg_use_compute_write)                               result._access = GL_WRITE_ONLY;
	result._format = (uint32_t)skg_tex_fmt_to_native(result.format);

	glGenTextures(1, &result._texture);
	skg_tex_settings(&result, type == skg_tex_type_cubemap ? skg_tex_address_clamp : skg_tex_address_repeat, skg_tex_sample_linear, 1);

	if (type == skg_tex_type_rendertarget) {
		glGenFramebuffers(1, &result._framebuffer);
	}
	
	return result;
}

///////////////////////////////////////////

bool skg_tex_is_valid(const skg_tex_t *tex) {
	return tex->_texture != 0;
}

///////////////////////////////////////////

void skg_tex_copy_to(const skg_tex_t *tex, skg_tex_t *destination) {
	if (destination->width != tex->width || destination->height != tex->height) {
		skg_tex_set_contents_arr(destination, nullptr, tex->array_count, tex->width, tex->height, tex->multisample);
	}

	glBindFramebuffer  (GL_FRAMEBUFFER, tex->_framebuffer);
	glBindTexture      (destination->_target, destination->_texture);
	glCopyTexSubImage2D(destination->_target, 0, 0,0,0,0,tex->width,tex->height);
}

///////////////////////////////////////////

void skg_tex_copy_to_swapchain(const skg_tex_t *tex, skg_swapchain_t *destination) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, tex->_framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0,0,tex->width,tex->height,0,0,tex->width,tex->height,  GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT, GL_NEAREST);
}

///////////////////////////////////////////

void skg_tex_attach_depth(skg_tex_t *tex, skg_tex_t *depth) {
	if (tex->type == skg_tex_type_rendertarget) {
		uint32_t attach = depth->format == skg_tex_fmt_depthstencil 
			? GL_DEPTH_STENCIL_ATTACHMENT 
			: GL_DEPTH_ATTACHMENT;
		glBindFramebuffer(GL_FRAMEBUFFER, tex->_framebuffer);
		if (tex->_target == GL_TEXTURE_2D_ARRAY) {
			if (tex->array_count == 1) {
				glFramebufferTextureLayer(GL_FRAMEBUFFER, attach, depth->_texture, 0, tex->array_start);
			} else {
#ifndef _SKG_GL_WEB
				glFramebufferTexture(GL_FRAMEBUFFER, attach, depth->_texture, 0);
#else
				skg_log(skg_log_critical, "sk_gpu doesn't support array textures with WebGL?");
#endif
			}
		} else {
			glFramebufferTexture2D(GL_FRAMEBUFFER, attach, tex->_target, depth->_texture, 0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, gl_current_framebuffer);
	} else {
		skg_log(skg_log_warning, "Can't bind a depth texture to a non-rendertarget");
	}
}

///////////////////////////////////////////

void skg_tex_settings(skg_tex_t *tex, skg_tex_address_ address, skg_tex_sample_ sample, int32_t anisotropy) {
	if (!skg_tex_is_valid(tex)) return;

	glBindTexture(tex->_target, tex->_texture);

	uint32_t mode;
	switch (address) {
	case skg_tex_address_clamp:  mode = GL_CLAMP_TO_EDGE;   break;
	case skg_tex_address_repeat: mode = GL_REPEAT;          break;
	case skg_tex_address_mirror: mode = GL_MIRRORED_REPEAT; break;
	default: mode = GL_REPEAT;
	}

	uint32_t filter, min_filter;
	switch (sample) {
	case skg_tex_sample_linear:     filter = GL_LINEAR;  min_filter = tex->mips == skg_mip_generate ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR; break; // Technically trilinear
	case skg_tex_sample_point:      filter = GL_NEAREST; min_filter = GL_NEAREST;                                                          break;
	case skg_tex_sample_anisotropic:filter = GL_LINEAR;  min_filter = tex->mips == skg_mip_generate ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR; break;
	default: filter = GL_LINEAR; min_filter = GL_LINEAR;
	}

	glTexParameteri(tex->_target, GL_TEXTURE_WRAP_S, mode);
	glTexParameteri(tex->_target, GL_TEXTURE_WRAP_T, mode);
	if (tex->type == skg_tex_type_cubemap) {
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, mode);
	}
	glTexParameteri(tex->_target, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(tex->_target, GL_TEXTURE_MAG_FILTER, filter    );
#ifdef _SKG_GL_DESKTOP
	glTexParameterf(tex->_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, sample == skg_tex_sample_anisotropic ? anisotropy : 1.0f);
#endif
}

///////////////////////////////////////////

void skg_tex_set_contents(skg_tex_t *tex, const void *data, int32_t width, int32_t height) {
	const void *data_arr[1] = { data };
	return skg_tex_set_contents_arr(tex, data_arr, 1, width, height, 1);
}

///////////////////////////////////////////

void skg_tex_set_contents_arr(skg_tex_t *tex, const void **data_frames, int32_t data_frame_count, int32_t width, int32_t height, int32_t multisample) {
	tex->width       = width;
	tex->height      = height;
	tex->multisample = multisample;
	tex->array_count = data_frame_count;
	if (tex->type != skg_tex_type_cubemap && tex->array_count > 1)
		tex->_target = GL_TEXTURE_2D_ARRAY;

	glBindTexture(tex->_target, tex->_texture);

	tex->_format    = (uint32_t)skg_tex_fmt_to_native   (tex->format);
	uint32_t layout =           skg_tex_fmt_to_gl_layout(tex->format);
	uint32_t type   =           skg_tex_fmt_to_gl_type  (tex->format);
	if (tex->type == skg_tex_type_cubemap) {
		if (data_frame_count != 6) {
			skg_log(skg_log_warning, "Cubemaps need 6 data frames");
			return;
		}
		for (int32_t f = 0; f < 6; f++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+f , 0, tex->_format, width, height, 0, layout, type, data_frames[f]);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, tex->_format, width, height, 0, layout, type, data_frames == nullptr ? nullptr : data_frames[0]);
	}
	if (tex->mips == skg_mip_generate)
		glGenerateMipmap(tex->_target);

	if (tex->type == skg_tex_type_rendertarget) {
		glBindFramebuffer(GL_FRAMEBUFFER, tex->_framebuffer);
		if (tex->array_count != 1) {
#ifndef _SKG_GL_WEB
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex->_texture, 0);
#else
			skg_log(skg_log_critical, "sk_gpu doesn't support array textures with WebGL?");
#endif
		} else {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex->_target, tex->_texture, 0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, gl_current_framebuffer);
	}
}

///////////////////////////////////////////

bool skg_tex_get_contents(skg_tex_t *tex, void *ref_data, size_t data_size) {
	return skg_tex_get_mip_contents_arr(tex, 0, 0, ref_data, data_size);
}

///////////////////////////////////////////

bool skg_tex_get_mip_contents(skg_tex_t *tex, int32_t mip_level, void *ref_data, size_t data_size) {
	return skg_tex_get_mip_contents_arr(tex, mip_level, 0, ref_data, data_size);
}

///////////////////////////////////////////

bool skg_tex_get_mip_contents_arr(skg_tex_t *tex, int32_t mip_level, int32_t arr_index, void *ref_data, size_t data_size) {
	// Double check on mips first
	uint32_t mip_levels = tex->mips == skg_mip_generate ? skg_mip_count(tex->width, tex->height) : 1;
	if (mip_level != 0) {
		if (tex->mips != skg_mip_generate) {
			skg_log(skg_log_critical, "Can't get mip data from a texture with no mips!");
			return false;
		}
		if (mip_level >= mip_levels) {
			skg_log(skg_log_critical, "This texture doesn't have quite as many mip levels as you think.");
			return false;
		}
	}

	// Make sure we've been provided enough memory to hold this texture
	int32_t width       = 0;
	int32_t height      = 0;
	size_t  format_size = skg_tex_fmt_size(tex->format);
	skg_mip_dimensions(tex->width, tex->height, mip_level, &width, &height);

	if (data_size != (size_t)width * (size_t)height * format_size) {
		skg_log(skg_log_critical, "Insufficient buffer size for skg_tex_get_mip_contents_arr");
		return false;
	}

	int64_t layout = skg_tex_fmt_to_gl_layout(tex->format);
	glBindTexture (tex->_target, tex->_texture);

#if defined(_SKG_GL_WEB) || defined(_SKG_GL_ES)
	// Referenced from here:
	// https://stackoverflow.com/questions/53993820/opengl-es-2-0-android-c-glgetteximage-alternative
	uint32_t fbo = 0;
	glGenFramebuffers     (1, &fbo); 
	glBindFramebuffer     (GL_FRAMEBUFFER, fbo);
	if (tex->_target == GL_TEXTURE_CUBE_MAP) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+arr_index, tex->_texture, mip_level);
	} else {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex->_target, tex->_texture, mip_level);
	}

	glReadPixels(0, 0, width, height, layout, skg_tex_fmt_to_gl_type(tex->format), ref_data);

	glBindFramebuffer   (GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &fbo);
#else
	glBindTexture(tex->_target, tex->_texture);

	if (tex->_target == GL_TEXTURE_CUBE_MAP) {
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X+arr_index, mip_level, (uint32_t)layout, skg_tex_fmt_to_gl_type(tex->format), ref_data);
	} else {
		glGetTexImage(tex->_target, mip_level, (uint32_t)layout, skg_tex_fmt_to_gl_type(tex->format), ref_data);
	}
#endif

	uint32_t result = glGetError();
	if (result != 0) {
		char text[128];
		snprintf(text, 128, "skg_tex_get_mip_contents_arr error: %d", result);
		skg_log(skg_log_critical, text);
	}

	return result == 0;
}

///////////////////////////////////////////

void skg_tex_bind(const skg_tex_t *texture, skg_bind_t bind) {
	if (bind.stage_bits & skg_stage_compute) {
#if !defined(_SKG_GL_WEB)
		glBindImageTexture(bind.slot, texture->_texture, 0, false, 0, texture->_access, skg_tex_fmt_to_native( texture->format ));
#endif
	} else {
		glActiveTexture(GL_TEXTURE0 + bind.slot);
		glBindTexture(texture->_target, texture->_texture);
	}
}

///////////////////////////////////////////

void skg_tex_clear(skg_bind_t bind) {
}

///////////////////////////////////////////

void skg_tex_destroy(skg_tex_t *tex) {
	uint32_t tex_list[] = { tex->_texture     };
	uint32_t fb_list [] = { tex->_framebuffer };
	if (tex->_texture    ) glDeleteTextures    (1, tex_list);
	if (tex->_framebuffer) glDeleteFramebuffers(1, fb_list );  
	*tex = {};
}

///////////////////////////////////////////

uint32_t skg_buffer_type_to_gl(skg_buffer_type_ type) {
	switch (type) {
	case skg_buffer_type_vertex:   return GL_ARRAY_BUFFER;
	case skg_buffer_type_index:    return GL_ELEMENT_ARRAY_BUFFER;
	case skg_buffer_type_constant: return GL_UNIFORM_BUFFER;
	case skg_buffer_type_compute:  return GL_SHADER_STORAGE_BUFFER;
	default: return 0;
	}
}

///////////////////////////////////////////

int64_t skg_tex_fmt_to_native(skg_tex_fmt_ format) {
	switch (format) {
	case skg_tex_fmt_rgba32:        return GL_SRGB8_ALPHA8;
	case skg_tex_fmt_rgba32_linear: return GL_RGBA8;
	case skg_tex_fmt_bgra32:        return GL_RGBA8;
	case skg_tex_fmt_bgra32_linear: return GL_RGBA8;
	case skg_tex_fmt_rg11b10:       return GL_R11F_G11F_B10F;
	case skg_tex_fmt_rgb10a2:       return GL_RGB10_A2;
	case skg_tex_fmt_rgba64u:       return GL_RGBA16F;
	case skg_tex_fmt_rgba64s:       return GL_RGBA16F;
	case skg_tex_fmt_rgba64f:       return GL_RGBA16F;
	case skg_tex_fmt_rgba128:       return GL_RGBA32F;
	case skg_tex_fmt_depth16:       return GL_DEPTH_COMPONENT16;
	case skg_tex_fmt_depth32:       return GL_DEPTH_COMPONENT32F;
	case skg_tex_fmt_depthstencil:  return GL_DEPTH24_STENCIL8;
	case skg_tex_fmt_r8:            return GL_R8;
	case skg_tex_fmt_r16:           return GL_R16F;
	case skg_tex_fmt_r32:           return GL_R32F;
	default: return 0;
	}
}

///////////////////////////////////////////

skg_tex_fmt_ skg_tex_fmt_from_native(int64_t format) {
	switch (format) {
	case GL_SRGB8_ALPHA8:       return skg_tex_fmt_rgba32;
	case GL_RGBA8:              return skg_tex_fmt_rgba32_linear;
	case GL_R11F_G11F_B10F:     return skg_tex_fmt_rg11b10;
	case GL_RGB10_A2:           return skg_tex_fmt_rgb10a2;
	case GL_RGBA16UI:           return skg_tex_fmt_rgba64u;
	case GL_RGBA16I:            return skg_tex_fmt_rgba64s;
	case GL_RGBA16F:            return skg_tex_fmt_rgba64f;
	case GL_RGBA32F:            return skg_tex_fmt_rgba128;
	case GL_DEPTH_COMPONENT16:  return skg_tex_fmt_depth16;
	case GL_DEPTH_COMPONENT32F: return skg_tex_fmt_depth32;
	case GL_DEPTH24_STENCIL8:   return skg_tex_fmt_depthstencil;
	case GL_R8:                 return skg_tex_fmt_r8;
	case GL_R16UI:              return skg_tex_fmt_r16;
	case GL_R32F:               return skg_tex_fmt_r32;
	default: return skg_tex_fmt_none;
	}
}

///////////////////////////////////////////

uint32_t skg_tex_fmt_to_gl_layout(skg_tex_fmt_ format) {
	switch (format) {
	case skg_tex_fmt_rgba32:
	case skg_tex_fmt_rgba32_linear:
	case skg_tex_fmt_rgb10a2:
	case skg_tex_fmt_rgba64u:
	case skg_tex_fmt_rgba64s:
	case skg_tex_fmt_rgba64f:
	case skg_tex_fmt_rgba128:       return GL_RGBA;
	case skg_tex_fmt_rg11b10:       return GL_RGB;
	case skg_tex_fmt_bgra32:
	case skg_tex_fmt_bgra32_linear:
		#ifdef _SKG_GL_WEB // WebGL has no GL_BGRA?
		return GL_RGBA;
		#else
		return GL_BGRA;
		#endif
	case skg_tex_fmt_depth16:
	case skg_tex_fmt_depth32:       return GL_DEPTH_COMPONENT;
	case skg_tex_fmt_depthstencil:  return GL_DEPTH_STENCIL;
	case skg_tex_fmt_r8:
	case skg_tex_fmt_r16:
	case skg_tex_fmt_r32:           return GL_RED;
	default: return 0;
	}
}

///////////////////////////////////////////

uint32_t skg_tex_fmt_to_gl_type(skg_tex_fmt_ format) {
	switch (format) {
	case skg_tex_fmt_rgba32:        return GL_UNSIGNED_BYTE;
	case skg_tex_fmt_rgba32_linear: return GL_UNSIGNED_BYTE;
	case skg_tex_fmt_bgra32:        return GL_UNSIGNED_BYTE;
	case skg_tex_fmt_bgra32_linear: return GL_UNSIGNED_BYTE;
	case skg_tex_fmt_rgb10a2:       return GL_FLOAT;
	case skg_tex_fmt_rg11b10:       return GL_FLOAT;
	case skg_tex_fmt_rgba64u:       return GL_UNSIGNED_SHORT;
	case skg_tex_fmt_rgba64s:       return GL_SHORT;
	case skg_tex_fmt_rgba64f:       return GL_FLOAT;
	case skg_tex_fmt_rgba128:       return GL_FLOAT;
	case skg_tex_fmt_depth16:       return GL_UNSIGNED_SHORT;
	case skg_tex_fmt_depth32:       return GL_FLOAT;
	case skg_tex_fmt_depthstencil:  return GL_UNSIGNED_INT_24_8;
	case skg_tex_fmt_r8:            return GL_UNSIGNED_BYTE;
	case skg_tex_fmt_r16:           return GL_UNSIGNED_SHORT;
	case skg_tex_fmt_r32:           return GL_FLOAT;
	default: return 0;
	}
}

#endif

///////////////////////////////////////////
// Common Code                           //
///////////////////////////////////////////

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#if __ANDROID__
#include <android/asset_manager.h>
#endif

void (*_skg_log)(skg_log_ level, const char *text);
void skg_callback_log(void (*callback)(skg_log_ level, const char *text)) {
	_skg_log = callback;
}
void skg_log(skg_log_ level, const char *text) {
	if (_skg_log) _skg_log(level, text);
}

///////////////////////////////////////////

bool (*_skg_read_file)(const char *filename, void **out_data, size_t *out_size);
void skg_callback_file_read(bool (*callback)(const char *filename, void **out_data, size_t *out_size)) {
	_skg_read_file = callback;
}
bool skg_read_file(const char *filename, void **out_data, size_t *out_size) {
	if (_skg_read_file) return _skg_read_file(filename, out_data, out_size);
	FILE *fp;
#if _WIN32
	if (fopen_s(&fp, filename, "rb") != 0 || fp == nullptr) {
		return false;
	}
#else
	fp = fopen(filename, "rb");
	if (fp == nullptr) {
		return false;
	}
#endif

	fseek(fp, 0L, SEEK_END);
	*out_size = ftell(fp);
	rewind(fp);

	*out_data = malloc(*out_size);
	if (*out_data == nullptr) { *out_size = 0; fclose(fp); return false; }
	fread (*out_data, *out_size, 1, fp);
	fclose(fp);

	return true;
}

///////////////////////////////////////////

uint64_t skg_hash(const char *string) {
	uint64_t hash = 14695981039346656037UL;
	while (*string != '\0') {
		hash = (hash ^ *string) * 1099511628211;
		string++;
	}
	return hash;
}

///////////////////////////////////////////

uint32_t skg_mip_count(int32_t width, int32_t height) {
	return (uint32_t)log2f(fminf((float)width, (float)height)) + 1;
}

///////////////////////////////////////////

void skg_mip_dimensions(int32_t width, int32_t height, int32_t mip_level, int32_t *out_width, int32_t *out_height) {
	*out_width  = width  >> mip_level;
	*out_height = height >> mip_level;
}

///////////////////////////////////////////

skg_color32_t skg_col_hsv32(float h, float s, float v, float a) {
	skg_color128_t col = skg_col_hsv128(h,s,v,a);
	return skg_color32_t{
		(uint8_t)(col.r*255),
		(uint8_t)(col.g*255),
		(uint8_t)(col.b*255),
		(uint8_t)(col.a*255)};
}

///////////////////////////////////////////

// Reference from here: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
skg_color128_t skg_col_hsv128(float h, float s, float v, float a) {
	const float K[4] = { 1.0f, 2.0f/3.0f, 1.0f/3.0f, 3.0f };
	float p[3] = {
		fabsf(((h + K[0]) - floorf(h + K[0])) * 6.0f - K[3]),
		fabsf(((h + K[1]) - floorf(h + K[1])) * 6.0f - K[3]),
		fabsf(((h + K[2]) - floorf(h + K[2])) * 6.0f - K[3]) };

	// lerp: a + (b - a) * t
	return skg_color128_t {
		(K[0] + (fmaxf(0,fminf(p[0] - K[0], 1.0f)) - K[0]) * s) * v,
		(K[0] + (fmaxf(0,fminf(p[1] - K[0], 1.0f)) - K[0]) * s) * v,
		(K[0] + (fmaxf(0,fminf(p[2] - K[0], 1.0f)) - K[0]) * s) * v,
		a };
}

///////////////////////////////////////////

skg_color32_t skg_col_hsl32(float h, float c, float l, float a) {
	skg_color128_t col = skg_col_hsl128(h,c,l,a);
	return skg_color32_t{
		(uint8_t)(col.r*255),
		(uint8_t)(col.g*255),
		(uint8_t)(col.b*255),
		(uint8_t)(col.a*255)};
}

///////////////////////////////////////////

skg_color128_t skg_col_hsl128(float h, float s, float l, float a) {
	if (h < 0) h -= floorf(h);
	float r = fabsf(h * 6 - 3) - 1;
	float g = 2 - fabsf(h * 6 - 2);
	float b = 2 - fabsf(h * 6 - 4);
	r = fmaxf(0, fminf(1, r));
	g = fmaxf(0, fminf(1, g));
	b = fmaxf(0, fminf(1, b));

	float C = (1 - fabsf(2 * l - 1)) * s;
	return {
		(r - 0.5f) * C + l,
		(g - 0.5f) * C + l,
		(b - 0.5f) * C + l, a };
}

///////////////////////////////////////////

skg_color32_t skg_col_hcy32(float h, float c, float l, float a) {
	skg_color128_t col = skg_col_hcy128(h,c,l,a);
	return skg_color32_t{
		(uint8_t)(col.r*255),
		(uint8_t)(col.g*255),
		(uint8_t)(col.b*255),
		(uint8_t)(col.a*255)};
}

///////////////////////////////////////////

// Reference from here https://www.chilliant.com/rgb2hsv.html
skg_color128_t skg_col_hcy128(float h, float c, float y, float a) {
	if (h < 0) h -= floorf(h);
	float r = fabsf(h * 6 - 3) - 1;
	float g = 2 - fabsf(h * 6 - 2);
	float b = 2 - fabsf(h * 6 - 4);
	r = fmaxf(0, fminf(1, r));
	g = fmaxf(0, fminf(1, g));
	b = fmaxf(0, fminf(1, b));

	float Z = r*0.299f + g*0.587f + b*0.114f;
	if (y < Z) {
		c *= y / Z;
	} else if (Z < 1) {
		c *= (1 - y) / (1 - Z);
	}
	return skg_color128_t { 
		(r - Z) * c + y,
		(g - Z) * c + y,
		(b - Z) * c + y,
		a };
}

///////////////////////////////////////////

skg_color32_t skg_col_lch32(float h, float c, float l, float a) {
	skg_color128_t col = skg_col_lch128(h,c,l,a);
	return skg_color32_t{
		(uint8_t)(col.r*255),
		(uint8_t)(col.g*255),
		(uint8_t)(col.b*255),
		(uint8_t)(col.a*255)};
}

///////////////////////////////////////////

// Reference from here: https://www.easyrgb.com/en/math.php
skg_color128_t skg_col_lch128(float h, float c, float l, float alpha) {
	const float tau = 6.283185307179586476925286766559f;
	c = c * 200;
	l = l * 100;
	float a = cosf( h*tau ) * c;
	float b = sinf( h*tau ) * c;
	
	float
		y = (l + 16.f) / 116.f,
		x = (a / 500.f) + y,
		z = y - (b / 200.f);

	x = 0.95047f * ((x*x*x > 0.008856f) ? x*x*x : (x - 16/116.f) / 7.787f);
	y = 1.00000f * ((y*y*y > 0.008856f) ? y*y*y : (y - 16/116.f) / 7.787f);
	z = 1.08883f * ((z*z*z > 0.008856f) ? z*z*z : (z - 16/116.f) / 7.787f);

	float r = x *  3.2406f + y * -1.5372f + z * -0.4986f;
	float g = x * -0.9689f + y *  1.8758f + z *  0.0415f;
	      b = x *  0.0557f + y * -0.2040f + z *  1.0570f;

	r = (r > 0.0031308f) ? (1.055f * powf(r, 1/2.4f) - 0.055f) : 12.92f * r;
	g = (g > 0.0031308f) ? (1.055f * powf(g, 1/2.4f) - 0.055f) : 12.92f * g;
	b = (b > 0.0031308f) ? (1.055f * powf(b, 1/2.4f) - 0.055f) : 12.92f * b;

	return skg_color128_t { 
		fmaxf(0, fminf(1, r)),
		fmaxf(0, fminf(1, g)),
		fmaxf(0, fminf(1, b)), alpha };
}

///////////////////////////////////////////

skg_color32_t skg_col_helix32(float h, float c, float l, float a) {
	skg_color128_t col = skg_col_helix128(h,c,l,a);
	return skg_color32_t{
		(uint8_t)(col.r*255),
		(uint8_t)(col.g*255),
		(uint8_t)(col.b*255),
		(uint8_t)(col.a*255)};
}

///////////////////////////////////////////

// Reference here: http://www.mrao.cam.ac.uk/~dag/CUBEHELIX/
skg_color128_t skg_col_helix128(float h, float s, float l, float alpha) {
	const float tau = 6.28318f;
	l = fminf(1,l);
	float angle = tau * (h+(1/3.f));
	float amp   = s * l * (1.f - l); // Helix in some implementations will 
	// divide this by 2.0f and go at half s, but if we clamp rgb at the end, 
	// we can get full s at the cost of a bit of artifacting at high 
	// s+lightness values.

	float a_cos = cosf(angle);
	float a_sin = sinf(angle);
	float r = l + amp * (-0.14861f * a_cos + 1.78277f * a_sin);
	float g = l + amp * (-0.29227f * a_cos - 0.90649f * a_sin);
	float b = l + amp * ( 1.97294f * a_cos);
	r = fmaxf(0,fminf(1, r));
	g = fmaxf(0,fminf(1, g));
	b = fmaxf(0,fminf(1, b));
	return { r, g, b, alpha };
}

///////////////////////////////////////////

skg_color32_t skg_col_jab32(float j, float a, float b, float alpha) {
	skg_color128_t col = skg_col_jab128(j, a, b, alpha);
	return skg_color32_t{ (uint8_t)(col.r*255), (uint8_t)(col.g*255), (uint8_t)(col.b*255), (uint8_t)(col.a*255)};
}

///////////////////////////////////////////

float lms(float t) {
	if (t > 0.) {
		float r = powf(t, 0.007460772656268214f);
		float s = (0.8359375f - r) / (18.6875f*r + -18.8515625f);
		return powf(s, 6.277394636015326f);
	} else {
		return 0.f;
	}
}

float srgb(float c) {
	if (c <= 0.0031308049535603713f) {
		return c * 12.92f;
	} else {
		float c_ = powf(c, 0.41666666666666666f);
		return c_ * 1.055f + -0.055f;
	}
}

// ref : https://thebookofshaders.com/edit.php?log=180722032925
skg_color128_t jchz2srgb(float h, float s, float l, float alpha) {
	float jz = l*0.16717463120366200f + 1.6295499532821566e-11f;
	float cz = s*0.16717463120366200f;
	float hz = h*6.28318530717958647f;

	float iz = jz / (0.56f*jz + 0.44f);
	float az = cz * cosf(hz);
	float bz = cz * sinf(hz);

	float l_ = iz + az* +0.13860504327153930f + bz* +0.058047316156118830f;
	float m_ = iz + az* -0.13860504327153927f + bz* -0.058047316156118904f;
	float s_ = iz + az* -0.09601924202631895f + bz* -0.811891896056039000f;

	      l = lms(l_);
	float m = lms(m_);
	      s = lms(s_);

	float lr = l* +0.0592896375540425100e4f + m* -0.052239474257975140e4f + s* +0.003259644233339027e4f;
	float lg = l* -0.0222329579044572220e4f + m* +0.038215274736946150e4f + s* -0.005703433147128812e4f;
	float lb = l* +0.0006270913830078808e4f + m* -0.007021906556220012e4f + s* +0.016669756032437408e4f;

	lr = fmaxf(0,fminf(1, srgb(lr)));
	lg = fmaxf(0,fminf(1, srgb(lg)));
	lb = fmaxf(0,fminf(1, srgb(lb)));
	return skg_color128_t{lr, lg, lb, alpha };
}

// Reference here: https://observablehq.com/@jrus/jzazbz
float pqi(float x) {
	x = powf(x, .007460772656268214f);
	return x <= 0 ? 0 : powf(
		(0.8359375f - x) / (18.6875f*x - 18.8515625f),
		6.277394636015326f); 
};
skg_color128_t skg_col_jab128(float j, float a, float b, float alpha) {
	// JAB to XYZ
	j = j + 1.6295499532821566e-11f;
	float iz = j / (0.44f + 0.56f * j);
	float l  = pqi(iz + .1386050432715393f*a + .0580473161561187f*b);
	float m  = pqi(iz - .1386050432715393f*a - .0580473161561189f*b);
	float s  = pqi(iz - .0960192420263189f*a - .8118918960560390f*b);

	float r = l* +0.0592896375540425100e4f + m* -0.052239474257975140e4f + s* +0.003259644233339027e4f;
	float g = l* -0.0222329579044572220e4f + m* +0.038215274736946150e4f + s* -0.005703433147128812e4f;
	      b = l* +0.0006270913830078808e4f + m* -0.007021906556220012e4f + s* +0.016669756032437408e4f;

	/*float x = +1.661373055774069e+00f * L - 9.145230923250668e-01f * M + 2.313620767186147e-01f * S;
	float y = -3.250758740427037e-01f * L + 1.571847038366936e+00f * M - 2.182538318672940e-01f * S;
	float z = -9.098281098284756e-02f * L - 3.127282905230740e-01f * M + 1.522766561305260e+00f * S;

	// XYZ to sRGB
	float r = x *  3.2406f + y * -1.5372f + z * -0.4986f;
	float g = x * -0.9689f + y *  1.8758f + z *  0.0415f;
	      b = x *  0.0557f + y * -0.2040f + z *  1.0570f;*/

	// to sRGB
	r = (r > 0.0031308f) ? (1.055f * powf(r, 1/2.4f) - 0.055f) : 12.92f * r;
	g = (g > 0.0031308f) ? (1.055f * powf(g, 1/2.4f) - 0.055f) : 12.92f * g;
	b = (b > 0.0031308f) ? (1.055f * powf(b, 1/2.4f) - 0.055f) : 12.92f * b;

	return skg_color128_t { 
		fmaxf(0, fminf(1, r)),
		fmaxf(0, fminf(1, g)),
		fmaxf(0, fminf(1, b)), alpha };
}

skg_color32_t skg_col_jsl32(float h, float s, float l, float alpha) {
	skg_color128_t col = skg_col_jsl128(h, s, l, alpha);
	return skg_color32_t{ (uint8_t)(col.r*255), (uint8_t)(col.g*255), (uint8_t)(col.b*255), (uint8_t)(col.a*255)};
}
skg_color128_t skg_col_jsl128(float h, float s, float l, float alpha) {
	return jchz2srgb(h, s, l, alpha);/*
	const float tau = 6.28318f;
	h = h * tau - tau/2;
	s = s * 0.16717463120366200f;
	l = l * 0.16717463120366200f;
	return skg_col_jab128(fmaxf(0,l), s * cosf(h), s * sinf(h), alpha);*/
}

///////////////////////////////////////////

skg_color32_t skg_col_lab32(float l, float a, float b, float alpha) {
	skg_color128_t col = skg_col_lab128(l,a,b,alpha);
	return skg_color32_t{
		(uint8_t)(col.r*255),
		(uint8_t)(col.g*255),
		(uint8_t)(col.b*255),
		(uint8_t)(col.a*255)};
}

///////////////////////////////////////////

skg_color128_t skg_col_lab128(float l, float a, float b, float alpha) {
	l = l * 100;
	a = a * 400 - 200;
	b = b * 400 - 200;
	float
		y = (l + 16.f) / 116.f,
		x = (a / 500.f) + y,
		z = y - (b / 200.f);

	x = 0.95047f * ((x * x * x > 0.008856f) ? x * x * x : (x - 16/116.f) / 7.787f);
	y = 1.00000f * ((y * y * y > 0.008856f) ? y * y * y : (y - 16/116.f) / 7.787f);
	z = 1.08883f * ((z * z * z > 0.008856f) ? z * z * z : (z - 16/116.f) / 7.787f);

	float r = x *  3.2406f + y * -1.5372f + z * -0.4986f;
	float g = x * -0.9689f + y *  1.8758f + z *  0.0415f;
	      b = x *  0.0557f + y * -0.2040f + z *  1.0570f;

	r = (r > 0.0031308f) ? (1.055f * powf(r, 1/2.4f) - 0.055f) : 12.92f * r;
	g = (g > 0.0031308f) ? (1.055f * powf(g, 1/2.4f) - 0.055f) : 12.92f * g;
	b = (b > 0.0031308f) ? (1.055f * powf(b, 1/2.4f) - 0.055f) : 12.92f * b;

	return skg_color128_t { 
		fmaxf(0, fminf(1, r)),
		fmaxf(0, fminf(1, g)),
		fmaxf(0, fminf(1, b)), alpha };
}

///////////////////////////////////////////

skg_color128_t skg_col_rgb_to_lab128(skg_color128_t rgb) {
	rgb.r = (rgb.r > 0.04045f) ? powf((rgb.r + 0.055f) / 1.055f, 2.4f) : rgb.r / 12.92f;
	rgb.g = (rgb.g > 0.04045f) ? powf((rgb.g + 0.055f) / 1.055f, 2.4f) : rgb.g / 12.92f;
	rgb.b = (rgb.b > 0.04045f) ? powf((rgb.b + 0.055f) / 1.055f, 2.4f) : rgb.b / 12.92f;

	// D65, Daylight, sRGB, aRGB
	float x = (rgb.r * 0.4124f + rgb.g * 0.3576f + rgb.b * 0.1805f) / 0.95047f;
	float y = (rgb.r * 0.2126f + rgb.g * 0.7152f + rgb.b * 0.0722f) / 1.00000f;
	float z = (rgb.r * 0.0193f + rgb.g * 0.1192f + rgb.b * 0.9505f) / 1.08883f;

	x = (x > 0.008856f) ? powf(x, 1/3.f) : (7.787f * x) + 16/116.f;
	y = (y > 0.008856f) ? powf(y, 1/3.f) : (7.787f * y) + 16/116.f;
	z = (z > 0.008856f) ? powf(z, 1/3.f) : (7.787f * z) + 16/116.f;

	return {
		(1.16f * y) - .16f,
		1.25f * (x - y) + 0.5f,
		0.5f * (y - z) + 0.5f, rgb.a };
}

///////////////////////////////////////////

inline float _skg_to_srgb(float x) {
	return x < 0.0031308f
		? x * 12.92f
		: 1.055f * powf(x, 1 / 2.4f) - 0.055f;
}
skg_color128_t skg_col_to_srgb(skg_color128_t rgb_linear) {
	return {
		_skg_to_srgb(rgb_linear.r),
		_skg_to_srgb(rgb_linear.g),
		_skg_to_srgb(rgb_linear.b),
		rgb_linear.a };
}

///////////////////////////////////////////

inline float _skg_to_linear(float x) {
	return x < 0.04045f
		? x / 12.92f
		: powf((x + 0.055f) / 1.055f, 2.4f);
}
skg_color128_t skg_col_to_linear(skg_color128_t srgb) {
	return {
		_skg_to_linear(srgb.r),
		_skg_to_linear(srgb.g),
		_skg_to_linear(srgb.b),
		srgb.a };
}

///////////////////////////////////////////

bool skg_shader_file_load(const char *file, skg_shader_file_t *out_file) {
	void  *data = nullptr;
	size_t size = 0;

	if (!skg_read_file(file, &data, &size))
		return false;

	bool result = skg_shader_file_load_memory(data, size, out_file);
	free(data);

	return result;
}

///////////////////////////////////////////

bool skg_shader_file_verify(const void *data, size_t size, uint16_t *out_version, char *out_name, size_t out_name_size) {
	const char    *prefix  = "SKSHADER";
	const uint8_t *bytes   = (uint8_t*)data;

	// check the first 5 bytes to see if this is a SKS shader file
	if (size < 10 || memcmp(bytes, prefix, 8) != 0)
		return false;

	// Grab the file version
	if (out_version)
		memcpy(out_version, &bytes[8], sizeof(uint16_t));

	// And grab the name of the shader
	if (out_name != nullptr && out_name_size > 0) {
		memcpy(out_name, &bytes[14], out_name_size < 256 ? out_name_size : 256);
		out_name[out_name_size - 1] = '\0';
	}

	return true;
}

///////////////////////////////////////////

bool skg_shader_file_load_memory(const void *data, size_t size, skg_shader_file_t *out_file) {
	uint16_t file_version = 0;
	if (!skg_shader_file_verify(data, size, &file_version, nullptr, 0) || file_version != 2) {
		return false;
	}
	
	const uint8_t *bytes = (uint8_t*)data;
	size_t at = 10;
	memcpy(&out_file->stage_count, &bytes[at], sizeof(out_file->stage_count)); at += sizeof(out_file->stage_count);
	out_file->stages = (skg_shader_file_stage_t*)malloc(sizeof(skg_shader_file_stage_t) * out_file->stage_count);
	if (out_file->stages == nullptr) { skg_log(skg_log_critical, "Out of memory"); return false; }

	out_file->meta = (skg_shader_meta_t*)malloc(sizeof(skg_shader_meta_t));
	if (out_file->meta == nullptr) { skg_log(skg_log_critical, "Out of memory"); return false; }
	*out_file->meta = {};
	out_file->meta->global_buffer_id = -1;
	skg_shader_meta_reference(out_file->meta);
	memcpy( out_file->meta->name,            &bytes[at], sizeof(out_file->meta->name          )); at += sizeof(out_file->meta->name);
	memcpy(&out_file->meta->buffer_count,    &bytes[at], sizeof(out_file->meta->buffer_count  )); at += sizeof(out_file->meta->buffer_count);
	memcpy(&out_file->meta->resource_count,  &bytes[at], sizeof(out_file->meta->resource_count)); at += sizeof(out_file->meta->resource_count);
	out_file->meta->buffers   = (skg_shader_buffer_t  *)malloc(sizeof(skg_shader_buffer_t  ) * out_file->meta->buffer_count  );
	out_file->meta->resources = (skg_shader_resource_t*)malloc(sizeof(skg_shader_resource_t) * out_file->meta->resource_count);
	if (out_file->meta->buffers == nullptr || out_file->meta->resources == nullptr) { skg_log(skg_log_critical, "Out of memory"); return false; }
	memset(out_file->meta->buffers,   0, sizeof(skg_shader_buffer_t  ) * out_file->meta->buffer_count);
	memset(out_file->meta->resources, 0, sizeof(skg_shader_resource_t) * out_file->meta->resource_count);

	for (uint32_t i = 0; i < out_file->meta->buffer_count; i++) {
		skg_shader_buffer_t *buffer = &out_file->meta->buffers[i];
		memcpy( buffer->name,      &bytes[at], sizeof(buffer->name));      at += sizeof(buffer->name);
		memcpy(&buffer->bind,      &bytes[at], sizeof(buffer->bind));      at += sizeof(buffer->bind);
		memcpy(&buffer->size,      &bytes[at], sizeof(buffer->size));      at += sizeof(buffer->size);
		memcpy(&buffer->var_count, &bytes[at], sizeof(buffer->var_count)); at += sizeof(buffer->var_count);

		uint32_t default_size = 0;
		memcpy(&default_size, &bytes[at], sizeof(buffer->size)); at += sizeof(buffer->size);
		buffer->defaults = nullptr;
		if (default_size != 0) {
			buffer->defaults = malloc(buffer->size);
			memcpy(buffer->defaults, &bytes[at], default_size); at += default_size;
		}
		buffer->vars = (skg_shader_var_t*)malloc(sizeof(skg_shader_var_t) * buffer->var_count);
		if (buffer->vars == nullptr) { skg_log(skg_log_critical, "Out of memory"); return false; }
		memset(buffer->vars, 0, sizeof(skg_shader_var_t) * buffer->var_count);
		buffer->name_hash = skg_hash(buffer->name);

		for (uint32_t t = 0; t < buffer->var_count; t++) {
			skg_shader_var_t *var = &buffer->vars[t];
			memcpy( var->name,       &bytes[at], sizeof(var->name ));      at += sizeof(var->name  );
			memcpy( var->extra,      &bytes[at], sizeof(var->extra));      at += sizeof(var->extra );
			memcpy(&var->offset,     &bytes[at], sizeof(var->offset));     at += sizeof(var->offset);
			memcpy(&var->size,       &bytes[at], sizeof(var->size));       at += sizeof(var->size  );
			memcpy(&var->type,       &bytes[at], sizeof(var->type));       at += sizeof(var->type  );
			memcpy(&var->type_count, &bytes[at], sizeof(var->type_count)); at += sizeof(var->type_count);
			var->name_hash = skg_hash(var->name);
		}

		if (strcmp(buffer->name, "$Global") == 0)
			out_file->meta->global_buffer_id = i;
	}

	for (uint32_t i = 0; i < out_file->meta->resource_count; i++) {
		skg_shader_resource_t *res = &out_file->meta->resources[i];
		memcpy( res->name,  &bytes[at], sizeof(res->name )); at += sizeof(res->name );
		memcpy( res->extra, &bytes[at], sizeof(res->extra)); at += sizeof(res->extra);
		memcpy(&res->bind,  &bytes[at], sizeof(res->bind )); at += sizeof(res->bind );
		res->name_hash = skg_hash(res->name);
	}

	for (uint32_t i = 0; i < out_file->stage_count; i++) {
		skg_shader_file_stage_t *stage = &out_file->stages[i];
		memcpy( &stage->language, &bytes[at], sizeof(stage->language)); at += sizeof(stage->language);
		memcpy( &stage->stage,    &bytes[at], sizeof(stage->stage));    at += sizeof(stage->stage);
		memcpy( &stage->code_size,&bytes[at], sizeof(stage->code_size));at += sizeof(stage->code_size);

		stage->code = 0;
		if (stage->code_size > 0) {
			stage->code = malloc(stage->code_size);
			if (stage->code == nullptr) { skg_log(skg_log_critical, "Out of memory"); return false; }
			memcpy(stage->code, &bytes[at], stage->code_size); at += stage->code_size;
		}
	}

	return true;
}

///////////////////////////////////////////

skg_shader_stage_t skg_shader_file_create_stage(const skg_shader_file_t *file, skg_stage_ stage) {
	skg_shader_lang_ language;
#if defined(SKG_DIRECT3D11) || defined(SKG_DIRECT3D12)
	language = skg_shader_lang_hlsl;
#elif defined(SKG_OPENGL)
	#if   defined(_SKG_GL_WEB)
		language = skg_shader_lang_glsl_web;
	#elif defined(_SKG_GL_ES)
		language = skg_shader_lang_glsl_es;
	#elif defined(_SKG_GL_DESKTOP)
		language = skg_shader_lang_glsl;
	#endif
#elif defined(SKG_VULKAN)
	language = skg_shader_lang_spirv;
#endif

	for (uint32_t i = 0; i < file->stage_count; i++) {
		if (file->stages[i].language == language && file->stages[i].stage == stage)
			return skg_shader_stage_create(file->stages[i].code, file->stages[i].code_size, stage);
	}
	skg_shader_stage_t empty = {};
	return empty;
}

///////////////////////////////////////////

void skg_shader_file_destroy(skg_shader_file_t *file) {
	for (uint32_t i = 0; i < file->stage_count; i++) {
		free(file->stages[i].code);
	}
	free(file->stages);
	skg_shader_meta_release(file->meta);
	*file = {};
}

///////////////////////////////////////////
// skg_shader_meta_t                     //
///////////////////////////////////////////

skg_bind_t skg_shader_meta_get_bind(const skg_shader_meta_t *meta, const char *name) {
	uint64_t hash = skg_hash(name);
	for (uint32_t i = 0; i < meta->buffer_count; i++) {
		if (meta->buffers[i].name_hash == hash)
			return meta->buffers[i].bind;
	}
	for (uint32_t i = 0; i < meta->resource_count; i++) {
		if (meta->resources[i].name_hash == hash)
			return meta->resources[i].bind;
	}
	skg_bind_t empty = {};
	return empty;
}

///////////////////////////////////////////

int32_t skg_shader_meta_get_var_count(const skg_shader_meta_t *meta) {
	return meta->global_buffer_id != -1
		? meta->buffers[meta->global_buffer_id].var_count
		: 0;
}

///////////////////////////////////////////

int32_t skg_shader_meta_get_var_index(const skg_shader_meta_t *meta, const char *name) {
	return skg_shader_meta_get_var_index_h(meta, skg_hash(name));
}

///////////////////////////////////////////

int32_t skg_shader_meta_get_var_index_h(const skg_shader_meta_t *meta, uint64_t name_hash) {
	if (meta->global_buffer_id == -1) return -1;

	skg_shader_buffer_t *buffer = &meta->buffers[meta->global_buffer_id];
	for (uint32_t i = 0; i < buffer->var_count; i++) {
		if (buffer->vars[i].name_hash == name_hash) {
			return i;
		}
	}
	return -1;
}

///////////////////////////////////////////

const skg_shader_var_t *skg_shader_meta_get_var_info(const skg_shader_meta_t *meta, int32_t var_index) {
	if (meta->global_buffer_id == -1 || var_index == -1) return nullptr;

	skg_shader_buffer_t *buffer = &meta->buffers[meta->global_buffer_id];
	return &buffer->vars[var_index];
}

///////////////////////////////////////////

void skg_shader_meta_reference(skg_shader_meta_t *meta) {
	meta->references += 1;
}

///////////////////////////////////////////

void skg_shader_meta_release(skg_shader_meta_t *meta) {
	if (!meta) return;
	meta->references -= 1;
	if (meta->references == 0) {
		for (uint32_t i = 0; i < meta->buffer_count; i++) {
			free(meta->buffers[i].vars);
			free(meta->buffers[i].defaults);
		}
		free(meta->buffers);
		free(meta->resources);
		*meta = {};
	}
}

///////////////////////////////////////////
// skg_shader_t                          //
///////////////////////////////////////////

skg_shader_t skg_shader_create_file(const char *sks_filename) {
	skg_shader_file_t file;
	if (!skg_shader_file_load(sks_filename, &file)) {
		skg_shader_t empty = {};
		return empty;
	}

	skg_shader_stage_t vs     = skg_shader_file_create_stage(&file, skg_stage_vertex);
	skg_shader_stage_t ps     = skg_shader_file_create_stage(&file, skg_stage_pixel);
	skg_shader_stage_t cs     = skg_shader_file_create_stage(&file, skg_stage_compute);
	skg_shader_t       result = skg_shader_create_manual( file.meta, vs, ps, cs );

	skg_shader_stage_destroy(&vs);
	skg_shader_stage_destroy(&ps);
	skg_shader_stage_destroy(&cs);
	skg_shader_file_destroy (&file);

	return result;
}

///////////////////////////////////////////

skg_shader_t skg_shader_create_memory(const void *sks_data, size_t sks_data_size) {
	skg_shader_file_t file;
	if (!skg_shader_file_load_memory(sks_data, sks_data_size, &file)) {
		skg_shader_t empty = {};
		return empty;
	}

	skg_shader_stage_t vs     = skg_shader_file_create_stage(&file, skg_stage_vertex);
	skg_shader_stage_t ps     = skg_shader_file_create_stage(&file, skg_stage_pixel);
	skg_shader_stage_t cs     = skg_shader_file_create_stage(&file, skg_stage_compute);
	skg_shader_t       result = skg_shader_create_manual( file.meta, vs, ps, cs );

	skg_shader_stage_destroy(&vs);
	skg_shader_stage_destroy(&ps);
	skg_shader_stage_destroy(&cs);
	skg_shader_file_destroy (&file);

	return result;
}

///////////////////////////////////////////

skg_bind_t skg_shader_get_bind(const skg_shader_t *shader, const char *name) {
	return skg_shader_meta_get_bind(shader->meta, name);
}

///////////////////////////////////////////

int32_t skg_shader_get_var_count(const skg_shader_t *shader) {
	return skg_shader_meta_get_var_count(shader->meta);
}

///////////////////////////////////////////

int32_t skg_shader_get_var_index(const skg_shader_t *shader, const char *name) {
	return skg_shader_meta_get_var_index_h(shader->meta, skg_hash(name));
}

///////////////////////////////////////////

int32_t skg_shader_get_var_index_h(const skg_shader_t *shader, uint64_t name_hash) {
	return skg_shader_meta_get_var_index_h(shader->meta, name_hash);
}

///////////////////////////////////////////

const skg_shader_var_t *skg_shader_get_var_info(const skg_shader_t *shader, int32_t var_index) {
	return skg_shader_meta_get_var_info(shader->meta, var_index);
}

///////////////////////////////////////////

uint32_t skg_tex_fmt_size(skg_tex_fmt_ format) {
	switch (format) {
	case skg_tex_fmt_rgba32:
	case skg_tex_fmt_rgba32_linear:
	case skg_tex_fmt_bgra32:
	case skg_tex_fmt_bgra32_linear:
	case skg_tex_fmt_rg11b10: 
	case skg_tex_fmt_rgb10a2:       return sizeof(uint8_t )*4;
	case skg_tex_fmt_rgba64u:
	case skg_tex_fmt_rgba64s:
	case skg_tex_fmt_rgba64f:       return sizeof(uint16_t)*4;
	case skg_tex_fmt_rgba128:       return sizeof(uint32_t)*4;
	case skg_tex_fmt_depth16:       return sizeof(uint16_t);
	case skg_tex_fmt_depth32:       return sizeof(uint32_t);
	case skg_tex_fmt_depthstencil:  return sizeof(uint32_t);
	case skg_tex_fmt_r8:            return sizeof(uint8_t );
	case skg_tex_fmt_r16:           return sizeof(uint16_t);
	case skg_tex_fmt_r32:           return sizeof(uint32_t);
	default: return 0;
	}
}
#endif // SKG_IMPL
/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Nick Klingensmith
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
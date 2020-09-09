#pragma once

//#define SKR_VULKAN
//#define SKR_DIRECT3D12
//#define SKR_DIRECT3D11
//#define SKR_OPENGL

#include <stdint.h>

///////////////////////////////////////////

typedef enum skr_buffer_type_ {
	skr_buffer_type_vertex,
	skr_buffer_type_index,
	skr_buffer_type_constant,
} skr_buffer_type_;

typedef enum skr_tex_type_ {
	skr_tex_type_image,
	skr_tex_type_cubemap,
	skr_tex_type_rendertarget,
	skr_tex_type_depth,
} skr_tex_type_;

typedef enum skr_use_ {
	skr_use_static,
	skr_use_dynamic,
} skr_use_;

typedef enum skr_mip_ {
	skr_mip_generate,
	skr_mip_none,
} skr_mip_;

typedef enum skr_tex_address_ {
	skr_tex_address_repeat,
	skr_tex_address_clamp,
	skr_tex_address_mirror,
} skr_tex_address_;

typedef enum skr_tex_sample_ {
	skr_tex_sample_linear,
	skr_tex_sample_point,
	skr_tex_sample_anisotropic
} skr_tex_sample_;

typedef enum skr_tex_fmt_ {
	skr_tex_fmt_none = 0,
	skr_tex_fmt_rgba32,
	skr_tex_fmt_rgba32_linear,
	skr_tex_fmt_bgra32,
	skr_tex_fmt_bgra32_linear,
	skr_tex_fmt_rgba64,
	skr_tex_fmt_rgba128,
	skr_tex_fmt_r8,
	skr_tex_fmt_r16,
	skr_tex_fmt_r32,
	skr_tex_fmt_depthstencil,
	skr_tex_fmt_depth32,
	skr_tex_fmt_depth16,
} skr_tex_fmt_;

typedef enum skr_fmt_ {
	skr_fmt_none,
	skr_fmt_f32_1,    skr_fmt_f32_2,    skr_fmt_f32_3,    skr_fmt_f32_4,
	skr_fmt_f16_1,    skr_fmt_f16_2,                      skr_fmt_f16_4,
	skr_fmt_i32_1,    skr_fmt_i32_2,    skr_fmt_i32_3,    skr_fmt_i32_4,
	skr_fmt_i16_1,    skr_fmt_i16_2,                      skr_fmt_i16_4,
	skr_fmt_i8_1,     skr_fmt_i8_2,                       skr_fmt_i8_4,
	skr_fmt_ui32_1,   skr_fmt_ui32_2,   skr_fmt_ui32_3,   skr_fmt_ui32_4,
	skr_fmt_ui16_1,   skr_fmt_ui16_2,                     skr_fmt_ui16_4,
	skr_fmt_ui8_1,    skr_fmt_ui8_2,                      skr_fmt_ui8_4,
	skr_fmt_ui16_n_1, skr_fmt_ui16_n_2,                   skr_fmt_ui16_n_4,
	skr_fmt_ui8_n_1,  skr_fmt_ui8_n_2,                    skr_fmt_ui8_n_4,
} skr_fmt_;

typedef enum skr_el_semantic_ {
	skr_el_semantic_none,
	skr_el_semantic_position,
	skr_el_semantic_normal,
	skr_el_semantic_texcoord,
	skr_el_semantic_color,
	skr_el_semantic_target_index,
} skr_el_semantic_;

typedef enum skr_stage_ {
	skr_stage_vertex = 1 << 0,
	skr_stage_pixel  = 1 << 1,
} skr_stage_;

typedef enum skr_shader_var_ {
	skr_shader_var_none,
	skr_shader_var_int,
	skr_shader_var_uint,
	skr_shader_var_uint8,
	skr_shader_var_float,
	skr_shader_var_double,
} skr_shader_var_;

typedef enum skr_transparency_ {
	skr_transparency_none = 1,
	skr_transparency_blend,
	skr_transparency_clip,
} skr_transparency_;

typedef enum skr_cull_ {
	skr_cull_back = 0,
	skr_cull_front,
	skr_cull_none,
} skr_cull_;

typedef struct skr_vert_t {
	float   pos [3];
	float   norm[3];
	float   uv  [2];
	uint8_t col [4];
} skr_vert_t;

typedef struct skr_bind_t {
	uint16_t slot;
	uint16_t stage_bits;
} skr_bind_t;

typedef struct skr_shader_meta_var_t {
	char     name [32];
	uint64_t name_hash;
	char     extra[64];
	uint32_t offset;
	uint32_t size;
	uint16_t type;
	uint16_t type_count;
} skr_shader_meta_var_t;

typedef struct skr_shader_meta_buffer_t {
	char       name[32];
	uint64_t   name_hash;
	skr_bind_t bind;
	uint32_t   size;
	void      *defaults;
	uint32_t               var_count;
	skr_shader_meta_var_t *vars;
} skr_shader_meta_buffer_t;

typedef struct skr_shader_meta_texture_t {
	char       name [32];
	uint64_t   name_hash;
	char       extra[64];
	skr_bind_t bind;
} skr_shader_meta_texture_t;

typedef struct skr_shader_meta_t {
	char                       name[256];
	uint32_t                   buffer_count;
	skr_shader_meta_buffer_t  *buffers;
	uint32_t                   texture_count;
	skr_shader_meta_texture_t *textures;
	int32_t                    references;
	int32_t                    global_buffer_id;
} skr_shader_meta_t;

///////////////////////////////////////////

#if defined(SKR_DIRECT3D11)


#include <d3d11.h>
#include <dxgi1_6.h>

///////////////////////////////////////////

typedef struct skr_buffer_t {
	skr_use_         use;
	skr_buffer_type_ type;
	ID3D11Buffer    *buffer;
} skr_buffer_t;

typedef struct skr_mesh_t {
	ID3D11Buffer *ind_buffer;
	ID3D11Buffer *vert_buffer;
} skr_mesh_t;

typedef struct skr_shader_stage_t {
	skr_stage_  type;
	void       *shader;
} skr_shader_stage_t;

typedef struct skr_shader_t {
	skr_shader_meta_t  *meta;
	ID3D11VertexShader *vertex;
	ID3D11PixelShader  *pixel;
} skr_shader_t;

typedef struct skr_pipeline_t {
	skr_transparency_ transparency;
	skr_cull_         cull;
	bool              wireframe;
	ID3D11VertexShader    *vertex;
	ID3D11PixelShader     *pixel;
	ID3D11BlendState      *blend;
	ID3D11RasterizerState *rasterize;
} skr_pipeline_t;

typedef struct skr_tex_t {
	int32_t width;
	int32_t height;
	int32_t array_count;
	skr_use_                  use;
	skr_tex_type_             type;
	skr_tex_fmt_              format;
	skr_mip_                  mips;
	ID3D11Texture2D          *texture;
	ID3D11SamplerState       *sampler;
	ID3D11ShaderResourceView *resource;
	ID3D11RenderTargetView   *target_view;
	ID3D11DepthStencilView   *depth_view;
} skr_tex_t;

typedef struct skr_swapchain_t {
	int32_t   width;
	int32_t   height;
	skr_tex_t target;
	skr_tex_t depth;
	IDXGISwapChain1 *d3d_swapchain;
} skr_swapchain_t;

typedef struct skr_platform_data_t {
	void *d3d11_device;
} skr_platform_data_t;

#elif defined(SKR_DIRECT3D12)

#elif defined(SKR_VULKAN)

#elif defined(SKR_OPENGL)

typedef struct skr_buffer_t {
	skr_use_ use;
	uint32_t type;
	uint32_t buffer;
} skr_buffer_t;

typedef struct skr_mesh_t {
	uint32_t index_buffer;
	uint32_t vert_buffer;
	uint32_t layout;
} skr_mesh_t;

typedef struct skr_gl_attr_t {
	int32_t  location;
	int32_t  size;
	uint32_t type;
	uint8_t  normalized;
	void    *offset;
} skr_gl_attr_t;

typedef struct skr_shader_stage_t {
	skr_stage_ type;
	uint32_t   shader;
} skr_shader_stage_t;

typedef struct skr_shader_t {
	skr_shader_meta_t *meta;
	uint32_t           vertex;
	uint32_t           pixel;
	uint32_t           program;
} skr_shader_t;

typedef struct skr_pipeline_t {
	skr_transparency_ transparency;
	skr_cull_         cull;
	bool              wireframe;
	skr_shader_t      shader;
} skr_pipeline_t;

typedef struct skr_tex_t {
	int32_t       width;
	int32_t       height;
	int32_t       array_count;
	skr_use_      use;
	skr_tex_type_ type;
	skr_tex_fmt_  format;
	skr_mip_      mips;
	uint32_t      texture;
	uint32_t      framebuffer;
} skr_tex_t;

typedef struct skr_platform_data_t {
#if __ANDROID__
	void *egl_display;
	void *egl_config;
	void *egl_context;
#elif _WIN32
	void *gl_hdc;
	void *gl_hrc;
#endif
} skr_platform_data_t;

typedef struct skr_swapchain_t {
	int32_t   width;
	int32_t   height;
	skr_tex_t target;
	skr_tex_t depth;
	uint32_t  gl_framebuffer;
} skr_swapchain_t;
#endif

///////////////////////////////////////////

int32_t             skr_init                (const char *app_name, void *hwnd, void *adapter_id);
void                skr_shutdown            ();
void                skr_callback_log        (void (*callback)(const char *text));
void                skr_callback_file_read  (bool (*callback)(const char *filename, void **out_data, size_t *out_size));
skr_platform_data_t skr_get_platform_data   ();

void                skr_draw_begin          ();
void                skr_draw                (int32_t index_start, int32_t index_count, int32_t instance_count);

skr_buffer_t        skr_buffer_create       (const void *data, uint32_t size_bytes, skr_buffer_type_ type, skr_use_ use);
bool                skr_buffer_is_valid     (const skr_buffer_t *buffer);
void                skr_buffer_set_contents (      skr_buffer_t *buffer, const void *data, uint32_t size_bytes);
void                skr_buffer_bind         (const skr_buffer_t *buffer, skr_bind_t slot_vc, uint32_t stride_v, uint32_t offset_vi);
void                skr_buffer_destroy      (      skr_buffer_t *buffer);

skr_mesh_t          skr_mesh_create         (const skr_buffer_t *vert_buffer, const skr_buffer_t *ind_buffer);
void                skr_mesh_set_verts      (      skr_mesh_t *mesh, const skr_buffer_t *vert_buffer);
void                skr_mesh_set_inds       (      skr_mesh_t *mesh, const skr_buffer_t *ind_buffer);
void                skr_mesh_bind           (const skr_mesh_t *mesh);
void                skr_mesh_destroy        (      skr_mesh_t *mesh);

skr_shader_stage_t  skr_shader_stage_create (const void *shader_data, size_t shader_size, skr_stage_ type);
void                skr_shader_stage_destroy(skr_shader_stage_t *stage);

skr_shader_t        skr_shader_create_file    (const char *sks_filename);
skr_shader_t        skr_shader_create_mem     (void *sks_data, size_t sks_data_size);
skr_shader_t        skr_shader_create_manual  (skr_shader_meta_t *meta, skr_shader_stage_t v_shader, skr_shader_stage_t p_shader);
bool                skr_shader_is_valid       (const skr_shader_t *shader);
skr_bind_t          skr_shader_get_tex_bind   (const skr_shader_t *shader, const char *name);
skr_bind_t          skr_shader_get_buffer_bind(const skr_shader_t *shader, const char *name);
int32_t             skr_shader_get_var_count  (const skr_shader_t *shader);
int32_t             skr_shader_get_var_index  (const skr_shader_t *shader, const char *name);
int32_t             skr_shader_get_var_index_h(const skr_shader_t *shader, uint64_t name_hash);
const skr_shader_meta_var_t *skr_shader_get_var_info (const skr_shader_t *shader, int32_t var_index);
void                skr_shader_destroy        (      skr_shader_t *shader);

skr_pipeline_t      skr_pipeline_create          (skr_shader_t *shader);
void                skr_pipeline_bind            (const skr_pipeline_t *pipeline);
void                skr_pipeline_set_transparency(      skr_pipeline_t *pipeline, skr_transparency_ transparency);
skr_transparency_   skr_pipeline_get_transparency(const skr_pipeline_t *pipeline);
void                skr_pipeline_set_cull        (      skr_pipeline_t *pipeline, skr_cull_ cull);
skr_cull_           skr_pipeline_get_cull        (const skr_pipeline_t *pipeline);
void                skr_pipeline_set_wireframe   (      skr_pipeline_t *pipeline, bool wireframe);
bool                skr_pipeline_get_wireframe   (const skr_pipeline_t *pipeline);
void                skr_pipeline_destroy         (      skr_pipeline_t *pipeline);

skr_swapchain_t     skr_swapchain_create    (skr_tex_fmt_ format, skr_tex_fmt_ depth_format, int32_t width, int32_t height);
void                skr_swapchain_resize    (      skr_swapchain_t *swapchain, int32_t width, int32_t height);
void                skr_swapchain_present   (      skr_swapchain_t *swapchain);
skr_tex_t          *skr_swapchain_get_next  (      skr_swapchain_t *swapchain);
void                skr_swapchain_destroy   (      skr_swapchain_t *swapchain);

skr_tex_t           skr_tex_from_native     (void *native_tex, skr_tex_type_ type, skr_tex_fmt_ format, int32_t width, int32_t height, int32_t array_count);
skr_tex_t           skr_tex_create          (skr_tex_type_ type, skr_use_ use, skr_tex_fmt_ format, skr_mip_ mip_maps);
bool                skr_tex_is_valid        (const skr_tex_t *tex);
void                skr_tex_set_depth       (      skr_tex_t *tex, skr_tex_t *depth);
void                skr_tex_settings        (      skr_tex_t *tex, skr_tex_address_ address, skr_tex_sample_ sample, int32_t anisotropy);
void                skr_tex_set_contents    (      skr_tex_t *tex, void **data_frames, int32_t data_frame_count, int32_t width, int32_t height);
void                skr_tex_get_contents    (      skr_tex_t *tex);
void                skr_tex_bind            (const skr_tex_t *tex, skr_bind_t bind);
void                skr_tex_target_bind     (      skr_tex_t *render_target, bool clear, const float *clear_color_4);
skr_tex_t          *skr_tex_target_get      ();
void                skr_tex_destroy         (      skr_tex_t *tex);
int64_t             skr_tex_fmt_to_native   (skr_tex_fmt_ format);
skr_tex_fmt_        skr_tex_fmt_from_native (int64_t      format);


///////////////////////////////////////////
// API independant functions             //
///////////////////////////////////////////

typedef enum skr_shader_lang_ {
	skr_shader_lang_hlsl,
	skr_shader_lang_spirv,
	skr_shader_lang_glsl,
} skr_shader_lang_;

typedef struct skr_shader_file_stage_t {
	skr_shader_lang_ language;
	skr_stage_       stage;
	uint32_t         code_size;
	void            *code;
} skr_shader_file_stage_t;

typedef struct skr_shader_file_t {
	skr_shader_meta_t       *meta;
	uint32_t                 stage_count;
	skr_shader_file_stage_t *stages;
} skr_shader_file_t;

///////////////////////////////////////////

void               skr_log (const char *text);
uint64_t           skr_hash(const char *string);

bool               skr_shader_file_verify      (void *data, size_t size, uint16_t *out_version, char *out_name, size_t out_name_size);
bool               skr_shader_file_load        (const char *file, skr_shader_file_t *out_file);
bool               skr_shader_file_load_mem    (void *data, size_t size, skr_shader_file_t *out_file);
skr_shader_stage_t skr_shader_file_create_stage(const skr_shader_file_t *file, skr_stage_ stage);
void               skr_shader_file_destroy     (      skr_shader_file_t *file);

void               skr_shader_meta_reference   (      skr_shader_meta_t *meta);
void               skr_shader_meta_release     (      skr_shader_meta_t *meta);

///////////////////////////////////////////
// Implementations!                      //
///////////////////////////////////////////

#ifdef SKR_IMPL
#ifdef SKR_DIRECT3D11

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
ID3D11InputLayout       *d3d_vert_layout = nullptr;
ID3D11RasterizerState   *d3d_rasterstate = nullptr;
ID3D11DepthStencilState *d3d_depthstate  = nullptr;
void                    *d3d_hwnd        = nullptr;
skr_tex_t               *d3d_active_rendertarget = nullptr;

///////////////////////////////////////////

uint32_t skr_tex_fmt_size (skr_tex_fmt_ format);
bool     skr_tex_make_view(skr_tex_t *tex, uint32_t mip_count, uint32_t array_size, bool use_in_shader);

template <typename T>
void skr_downsample_1(T *data, int32_t width, int32_t height, T **out_data, int32_t *out_width, int32_t *out_height);
template <typename T>
void skr_downsample_4(T *data, int32_t width, int32_t height, T **out_data, int32_t *out_width, int32_t *out_height);

///////////////////////////////////////////

int32_t skr_init(const char *app_name, void *hwnd, void *adapter_id) {
	d3d_hwnd = hwnd;
	UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Find the right adapter to use:
	IDXGIAdapter1 *final_adapter = nullptr;
	if (adapter_id != nullptr) {
		IDXGIFactory1 *dxgi_factory;
		CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)(&dxgi_factory));

		int curr = 0;
		IDXGIAdapter1 *curr_adapter = nullptr;
		while (dxgi_factory->EnumAdapters1(curr++, &curr_adapter) == S_OK) {
			DXGI_ADAPTER_DESC1 adapterDesc;
			curr_adapter->GetDesc1(&adapterDesc);

			if (memcmp(&adapterDesc.AdapterLuid, adapter_id, sizeof(LUID)) == 0) {
				final_adapter = curr_adapter;
				break;
			}
			curr_adapter->Release();
		}
		dxgi_factory->Release();
	}

	D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	if (FAILED(D3D11CreateDevice(final_adapter, final_adapter == nullptr ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN, 0, creation_flags, feature_levels, _countof(feature_levels), D3D11_SDK_VERSION, &d3d_device, nullptr, &d3d_context))) {
		return -1;
	}
	skr_log("sk_gpu: Using Direct3D 11");

	if (final_adapter != nullptr)
		final_adapter->Release();

	// Hook into debug information
	ID3D11Debug *d3d_debug = nullptr;
	if (SUCCEEDED(d3d_device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3d_debug))) {
		d3d_info = nullptr;
		if (SUCCEEDED(d3d_debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3d_info))) {
			D3D11_MESSAGE_ID hide[] = {
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS, 
				(D3D11_MESSAGE_ID)351,
				(D3D11_MESSAGE_ID)49, // TODO: Figure out the Flip model for backbuffers!
									  // Add more message IDs here as needed
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
	d3d_device->CreateRasterizerState(&desc_rasterizer, &d3d_rasterstate);
	d3d_context->RSSetState(d3d_rasterstate);

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
	d3d_context->OMSetDepthStencilState(d3d_depthstate, 1);

	return 1;
}

///////////////////////////////////////////

void skr_shutdown() {
	if (d3d_rasterstate) { d3d_rasterstate->Release(); d3d_rasterstate = nullptr; }
	if (d3d_depthstate ) { d3d_depthstate ->Release(); d3d_depthstate  = nullptr; }
	if (d3d_info     ) { d3d_info     ->Release(); d3d_info      = nullptr; }
	if (d3d_context  ) { d3d_context  ->Release(); d3d_context   = nullptr; }
	if (d3d_device   ) { d3d_device   ->Release(); d3d_device    = nullptr; }
}

///////////////////////////////////////////

void skr_draw_begin() {
	d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3d_context->IASetInputLayout      (d3d_vert_layout);
}

///////////////////////////////////////////

skr_platform_data_t skr_get_platform_data() {
	skr_platform_data_t result = {};
	result.d3d11_device = d3d_device;

	return result;
}

///////////////////////////////////////////

void skr_tex_target_bind(skr_tex_t *render_target, bool clear, const float *clear_color_4) {
	d3d_active_rendertarget = render_target;

	if (render_target == nullptr) {
		d3d_context->OMSetRenderTargets(0, nullptr, nullptr);
		return;
	}
	if (render_target->type != skr_tex_type_rendertarget)
		return;

	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.f, 0.f, (float)render_target->width, (float)render_target->height);
	d3d_context->RSSetViewports(1, &viewport);

	if (clear) {
		d3d_context->ClearRenderTargetView(render_target->target_view, clear_color_4);
		if (&render_target->depth_view) {
			UINT clear_flags = D3D11_CLEAR_DEPTH;
			d3d_context->ClearDepthStencilView(render_target->depth_view, clear_flags, 1.0f, 0);
		}
	}
	d3d_context->OMSetRenderTargets(1, &render_target->target_view, render_target->depth_view);
}

///////////////////////////////////////////

skr_tex_t *skr_tex_target_get() {
	return d3d_active_rendertarget;
}

///////////////////////////////////////////

void skr_draw(int32_t index_start, int32_t index_count, int32_t instance_count) {
	d3d_context->DrawIndexedInstanced(index_count, instance_count, index_start, 0, 0);
}

///////////////////////////////////////////

skr_buffer_t skr_buffer_create(const void *data, uint32_t size_bytes, skr_buffer_type_ type, skr_use_ use) {
	skr_buffer_t result = {};
	result.use  = use;
	result.type = type;

	D3D11_SUBRESOURCE_DATA buffer_data = { data };
	D3D11_BUFFER_DESC      buffer_desc = {};
	buffer_desc.ByteWidth = size_bytes;
	switch (type) {
	case skr_buffer_type_vertex:   buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;   break;
	case skr_buffer_type_index:    buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;    break;
	case skr_buffer_type_constant: buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; break;
	default: throw "Not implemented yet!";
	}
	switch (use) {
	case skr_use_static:  buffer_desc.Usage = D3D11_USAGE_DEFAULT; break;
	case skr_use_dynamic: {
		buffer_desc.Usage          = D3D11_USAGE_DYNAMIC;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}break;
	default: throw "Not implemented yet!";
	}
	d3d_device->CreateBuffer(&buffer_desc, data==nullptr ? nullptr : &buffer_data, &result.buffer);
	return result;
}

/////////////////////////////////////////// 

bool skr_buffer_is_valid(const skr_buffer_t *buffer) {
	return buffer->buffer != nullptr;
}

/////////////////////////////////////////// 

void skr_buffer_set_contents(skr_buffer_t *buffer, const void *data, uint32_t size_bytes) {
	if (buffer->use != skr_use_dynamic)
		return;

	D3D11_MAPPED_SUBRESOURCE resource;
	d3d_context->Map(buffer->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, data, size_bytes);
	d3d_context->Unmap(buffer->buffer, 0);
}

/////////////////////////////////////////// 

void skr_buffer_bind(const skr_buffer_t *buffer, skr_bind_t bind, uint32_t stride, uint32_t offset) {
	switch (buffer->type) {
	case skr_buffer_type_vertex:   d3d_context->IASetVertexBuffers  (bind.slot, 1, &buffer->buffer, &stride, &offset); break;
	case skr_buffer_type_index:    d3d_context->IASetIndexBuffer    (buffer->buffer, DXGI_FORMAT_R32_UINT, offset); break;
	case skr_buffer_type_constant: {
		if (bind.stage_bits & skr_stage_vertex)
			d3d_context->VSSetConstantBuffers(bind.slot, 1, &buffer->buffer);
		if (bind.stage_bits & skr_stage_pixel)
			d3d_context->PSSetConstantBuffers(bind.slot, 1, &buffer->buffer);
	} break;
	}
}

/////////////////////////////////////////// 

void skr_buffer_destroy(skr_buffer_t *buffer) {
	if (buffer->buffer) buffer->buffer->Release();
	*buffer = {};
}

/////////////////////////////////////////// 

skr_mesh_t skr_mesh_create(const skr_buffer_t *vert_buffer, const skr_buffer_t *ind_buffer) {
	skr_mesh_t result = {};
	result.ind_buffer  = ind_buffer  ? ind_buffer ->buffer : nullptr;
	result.vert_buffer = vert_buffer ? vert_buffer->buffer : nullptr;
	if (result.ind_buffer ) result.ind_buffer ->AddRef();
	if (result.vert_buffer) result.vert_buffer->AddRef();

	return result;
}

/////////////////////////////////////////// 

void skr_mesh_set_verts(skr_mesh_t *mesh, const skr_buffer_t *vert_buffer) {
	if (mesh->vert_buffer) mesh->vert_buffer->Release();
	mesh->vert_buffer = vert_buffer->buffer;
	if (mesh->vert_buffer) mesh->vert_buffer->AddRef();
}

/////////////////////////////////////////// 

void skr_mesh_set_inds(skr_mesh_t *mesh, const skr_buffer_t *ind_buffer) {
	if (mesh->ind_buffer) mesh->ind_buffer->Release();
	mesh->ind_buffer = ind_buffer->buffer;
	if (mesh->ind_buffer) mesh->ind_buffer->AddRef();
}

/////////////////////////////////////////// 

void skr_mesh_bind(const skr_mesh_t *mesh) {
	UINT strides[] = { sizeof(skr_vert_t) };
	UINT offsets[] = { 0 };
	d3d_context->IASetVertexBuffers(0, 1, &mesh->vert_buffer, strides, offsets);
	d3d_context->IASetIndexBuffer  (mesh->ind_buffer, DXGI_FORMAT_R32_UINT, 0);
}

/////////////////////////////////////////// 

void skr_mesh_destroy(skr_mesh_t *mesh) {
	if (mesh->ind_buffer ) mesh->ind_buffer ->Release();
	if (mesh->vert_buffer) mesh->vert_buffer->Release();
	*mesh = {};
}

/////////////////////////////////////////// 

#include <stdio.h>
skr_shader_stage_t skr_shader_stage_create(const void *file_data, size_t shader_size, skr_stage_ type) {
	skr_shader_stage_t result = {};
	result.type = type;

	DWORD flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
	flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	ID3DBlob   *compiled = nullptr;
	const void *buffer;
	size_t      buffer_size;
	if (shader_size >= 4 && memcmp(file_data, "DXBC", 4) == 0) {
		buffer      = file_data;
		buffer_size = shader_size;
	} else {
		ID3DBlob *errors;
		if (FAILED(D3DCompile(file_data, shader_size, nullptr, nullptr, nullptr, type == skr_stage_pixel ? "ps" : "vs", type == skr_stage_pixel ? "ps_5_0" : "vs_5_0", flags, 0, &compiled, &errors))) {
			skr_log("Error - D3DCompile failed:");
			skr_log((char *)errors->GetBufferPointer());
		}
		if (errors) errors->Release();

		buffer      = compiled->GetBufferPointer();
		buffer_size = compiled->GetBufferSize();
	}

	switch(type) {
	case skr_stage_vertex: d3d_device->CreateVertexShader(buffer, buffer_size, nullptr, (ID3D11VertexShader**)&result.shader); break;
	case skr_stage_pixel : d3d_device->CreatePixelShader (buffer, buffer_size, nullptr, (ID3D11PixelShader **)&result.shader); break;
	}

	if (d3d_vert_layout == nullptr && type == skr_stage_vertex) {
		// Describe how our mesh is laid out in memory
		D3D11_INPUT_ELEMENT_DESC vert_desc[] = {
			{"SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR" ,      0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"SV_RenderTargetArrayIndex" ,  0, DXGI_FORMAT_R8_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0} };
		d3d_device->CreateInputLayout(vert_desc, (UINT)_countof(vert_desc), buffer, buffer_size, &d3d_vert_layout);
	}
	if (compiled) compiled->Release();

	return result;
}

/////////////////////////////////////////// 

void skr_shader_stage_destroy(skr_shader_stage_t *shader) {
	switch(shader->type) {
	case skr_stage_vertex: ((ID3D11VertexShader*)shader->shader)->Release(); break;
	case skr_stage_pixel : ((ID3D11PixelShader *)shader->shader)->Release(); break;
	}
}

///////////////////////////////////////////
// skr_shader_t                          //
///////////////////////////////////////////

skr_shader_t skr_shader_create_manual(skr_shader_meta_t *meta, skr_shader_stage_t v_shader, skr_shader_stage_t p_shader) {
	skr_shader_t result = {};
	result.meta   = meta;
	result.vertex = (ID3D11VertexShader*)v_shader.shader;
	result.pixel  = (ID3D11PixelShader *)p_shader.shader;
	skr_shader_meta_reference(result.meta);
	if (result.vertex) result.vertex->AddRef();
	if (result.pixel ) result.pixel ->AddRef();

	return result;
}

///////////////////////////////////////////

bool skr_shader_is_valid(const skr_shader_t *shader) {
	return shader->meta
		&& shader->vertex
		&& shader->pixel;
}

///////////////////////////////////////////

void skr_shader_destroy(skr_shader_t *shader) {
	if (shader->vertex) shader->vertex->Release();
	if (shader->pixel ) shader->pixel ->Release();
	*shader = {};
}

///////////////////////////////////////////
// skr_pipeline                          //
/////////////////////////////////////////// 

void skr_pipeline_update_blend(skr_pipeline_t *pipeline) {
	if (pipeline->blend) pipeline->blend->Release();

	D3D11_BLEND_DESC desc_blend = {};
	desc_blend.AlphaToCoverageEnable  = false;
	desc_blend.IndependentBlendEnable = false;
	desc_blend.RenderTarget[0].BlendEnable           = pipeline->transparency == skr_transparency_blend;
	desc_blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	desc_blend.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
	desc_blend.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
	desc_blend.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
	desc_blend.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
	desc_blend.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
	desc_blend.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	d3d_device->CreateBlendState(&desc_blend, &pipeline->blend);
}

/////////////////////////////////////////// 

void skr_pipeline_update_rasterizer(skr_pipeline_t *pipeline) {
	if (pipeline->rasterize) pipeline->rasterize->Release();

	D3D11_RASTERIZER_DESC desc_rasterizer = {};
	desc_rasterizer.FillMode              = pipeline->wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	desc_rasterizer.FrontCounterClockwise = true;
	switch (pipeline->cull) {
	case skr_cull_none:  desc_rasterizer.CullMode = D3D11_CULL_NONE;  break;
	case skr_cull_front: desc_rasterizer.CullMode = D3D11_CULL_FRONT; break;
	case skr_cull_back:  desc_rasterizer.CullMode = D3D11_CULL_BACK;  break;
	}
	d3d_device->CreateRasterizerState(&desc_rasterizer, &pipeline->rasterize);
}

/////////////////////////////////////////// 

skr_pipeline_t skr_pipeline_create(skr_shader_t *shader) {
	skr_pipeline_t result = {};
	result.transparency = skr_transparency_none;
	result.cull         = skr_cull_back;
	result.wireframe    = false;
	result.vertex       = shader->vertex;
	result.pixel        = shader->pixel;
	if (result.vertex) result.vertex->AddRef();
	if (result.pixel ) result.pixel ->AddRef();

	skr_pipeline_update_blend     (&result);
	skr_pipeline_update_rasterizer(&result);
	return result;
}

/////////////////////////////////////////// 

void skr_pipeline_bind(const skr_pipeline_t *pipeline) {
	d3d_context->OMSetBlendState(pipeline->blend,  nullptr, 0xFFFFFFFF);
	d3d_context->RSSetState     (pipeline->rasterize);
	d3d_context->VSSetShader    (pipeline->vertex, nullptr, 0);
	d3d_context->PSSetShader    (pipeline->pixel,  nullptr, 0);
}

/////////////////////////////////////////// 

void skr_pipeline_set_transparency(skr_pipeline_t *pipeline, skr_transparency_ transparency) {
	if (pipeline->transparency != transparency) {
		pipeline->transparency  = transparency;
		skr_pipeline_update_blend(pipeline);
	}
}

/////////////////////////////////////////// 
void skr_pipeline_set_cull(skr_pipeline_t *pipeline, skr_cull_ cull) {
	if (pipeline->cull != cull) {
		pipeline->cull  = cull;
		skr_pipeline_update_rasterizer(pipeline);
	}
}
/////////////////////////////////////////// 
void skr_pipeline_set_wireframe(skr_pipeline_t *pipeline, bool wireframe) {
	if (pipeline->wireframe != wireframe) {
		pipeline->wireframe  = wireframe;
		skr_pipeline_update_rasterizer(pipeline);
	}
}
/////////////////////////////////////////// 

skr_transparency_ skr_pipeline_get_transparency(const skr_pipeline_t *pipeline) {
	return pipeline->transparency;
}

/////////////////////////////////////////// 

skr_cull_ skr_pipeline_get_cull(const skr_pipeline_t *pipeline) {
	return pipeline->cull;
}

/////////////////////////////////////////// 

bool skr_pipeline_get_wireframe(const skr_pipeline_t *pipeline) {
	return pipeline->wireframe;
}

///////////////////////////////////////////

void skr_pipeline_destroy(skr_pipeline_t *pipeline) {
	if (pipeline->blend    ) pipeline->blend    ->Release();
	if (pipeline->rasterize) pipeline->rasterize->Release();
	if (pipeline->vertex   ) pipeline->vertex   ->Release();
	if (pipeline->pixel    ) pipeline->pixel    ->Release();
	*pipeline = {};
}

///////////////////////////////////////////
// skr_swapchain                         //
///////////////////////////////////////////

skr_swapchain_t skr_swapchain_create(skr_tex_fmt_ format, skr_tex_fmt_ depth_format, int32_t width, int32_t height) {
	skr_swapchain_t result = {};
	result.width  = width;
	result.height = height;

	DXGI_SWAP_CHAIN_DESC1 swapchain_desc = { };
	swapchain_desc.BufferCount = 2;
	swapchain_desc.Width       = width;
	swapchain_desc.Height      = height;
	swapchain_desc.Format      = (DXGI_FORMAT)skr_tex_fmt_to_native(format);
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchain_desc.SampleDesc.Count = 1;

	IDXGIDevice2  *dxgi_device;  d3d_device  ->QueryInterface(__uuidof(IDXGIDevice2),  (void **)&dxgi_device);
	IDXGIAdapter  *dxgi_adapter; dxgi_device ->GetParent     (__uuidof(IDXGIAdapter),  (void **)&dxgi_adapter);
	IDXGIFactory2 *dxgi_factory; dxgi_adapter->GetParent     (__uuidof(IDXGIFactory2), (void **)&dxgi_factory);

	dxgi_factory->CreateSwapChainForHwnd(d3d_device, (HWND)d3d_hwnd, &swapchain_desc, nullptr, nullptr, &result.d3d_swapchain);

	ID3D11Texture2D *back_buffer;
	result.d3d_swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
	result.target = skr_tex_from_native(back_buffer, skr_tex_type_rendertarget, format, width, height, 1);
	result.depth  = skr_tex_create(skr_tex_type_depth, skr_use_static, depth_format, skr_mip_none);
	skr_tex_set_contents(&result.depth, nullptr, 1, width, height);
	skr_tex_set_depth   (&result.target, &result.depth);
	back_buffer->Release();

	dxgi_factory->Release();
	dxgi_adapter->Release();
	dxgi_device ->Release();

	return result;
}

/////////////////////////////////////////// 

void skr_swapchain_resize(skr_swapchain_t *swapchain, int32_t width, int32_t height) {
	if (swapchain->d3d_swapchain == nullptr || (width == swapchain->width && height == swapchain->height))
		return;

	skr_tex_fmt_ target_fmt = swapchain->target.format;
	skr_tex_fmt_ depth_fmt  = swapchain->depth .format;
	skr_tex_destroy(&swapchain->target);
	skr_tex_destroy(&swapchain->depth);

	swapchain->width  = width;
	swapchain->height = height;
	swapchain->d3d_swapchain->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, 0);

	ID3D11Texture2D *back_buffer;
	swapchain->d3d_swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
	swapchain->target = skr_tex_from_native(back_buffer, skr_tex_type_rendertarget, target_fmt, width, height, 1);
	swapchain->depth  = skr_tex_create(skr_tex_type_depth, skr_use_static, depth_fmt, skr_mip_none);
	skr_tex_set_contents(&swapchain->depth, nullptr, 1, width, height);
	back_buffer->Release();
}

/////////////////////////////////////////// 

void skr_swapchain_present(skr_swapchain_t *swapchain) {
	swapchain->d3d_swapchain->Present(1, 0);
}

/////////////////////////////////////////// 

skr_tex_t *skr_swapchain_get_next(skr_swapchain_t *swapchain) {
	return swapchain->target.format != 0 ? &swapchain->target : nullptr;
}

/////////////////////////////////////////// 

void skr_swapchain_destroy(skr_swapchain_t *swapchain) {
	skr_tex_destroy(&swapchain->target);
	skr_tex_destroy(&swapchain->depth);
	swapchain->d3d_swapchain->Release();
	*swapchain = {};
}

/////////////////////////////////////////// 

skr_tex_t skr_tex_from_native(void *native_tex, skr_tex_type_ type, skr_tex_fmt_ override_format, int32_t width, int32_t height, int32_t array_count) {
	skr_tex_t result = {};
	result.type    = type;
	result.use     = skr_use_static;
	result.texture = (ID3D11Texture2D *)native_tex;
	result.texture->AddRef();

	// Get information about the image!
	D3D11_TEXTURE2D_DESC color_desc;
	result.texture->GetDesc(&color_desc);
	result.width       = color_desc.Width;
	result.height      = color_desc.Height;
	result.array_count = color_desc.ArraySize;
	result.format      = override_format != 0 ? override_format : skr_tex_fmt_from_native(color_desc.Format);
	skr_tex_make_view(&result, color_desc.MipLevels, color_desc.ArraySize, color_desc.BindFlags & D3D11_BIND_SHADER_RESOURCE);

	return result;
}

/////////////////////////////////////////// 

skr_tex_t skr_tex_create(skr_tex_type_ type, skr_use_ use, skr_tex_fmt_ format, skr_mip_ mip_maps) {
	skr_tex_t result = {};
	result.type   = type;
	result.use    = use;
	result.format = format;
	result.mips   = mip_maps;

	if (use == skr_use_dynamic && mip_maps == skr_mip_generate)
		skr_log("Dynamic textures don't support mip-maps!");

	return result;
}

/////////////////////////////////////////// 

bool skr_tex_is_valid(const skr_tex_t *tex) {
	return tex->texture != nullptr;
}

/////////////////////////////////////////// 

void skr_tex_set_depth(skr_tex_t *tex, skr_tex_t *depth) {
	if (depth->type == skr_tex_type_depth) {
		if (tex->depth_view) tex->depth_view->Release();
		tex->depth_view = depth->depth_view;
		tex->depth_view->AddRef();
	} else {
		skr_log("Can't bind a depth texture to a non-rendertarget");
	}
}

/////////////////////////////////////////// 

void skr_tex_settings(skr_tex_t *tex, skr_tex_address_ address, skr_tex_sample_ sample, int32_t anisotropy) {
	if (tex->sampler)
		tex->sampler->Release();

	D3D11_TEXTURE_ADDRESS_MODE mode;
	switch (address) {
	case skr_tex_address_clamp:  mode = D3D11_TEXTURE_ADDRESS_CLAMP;  break;
	case skr_tex_address_repeat: mode = D3D11_TEXTURE_ADDRESS_WRAP;   break;
	case skr_tex_address_mirror: mode = D3D11_TEXTURE_ADDRESS_MIRROR; break;
	default: mode = D3D11_TEXTURE_ADDRESS_WRAP;
	}

	D3D11_FILTER filter;
	switch (sample) {
	case skr_tex_sample_linear:     filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break; // Technically trilinear
	case skr_tex_sample_point:      filter = D3D11_FILTER_MIN_MAG_MIP_POINT;  break;
	case skr_tex_sample_anisotropic:filter = D3D11_FILTER_ANISOTROPIC;        break;
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

	// D3D will already return the same sampler when provided the same settings, so we
	// can just lean on that to prevent sampler duplicates :)
	if (FAILED(d3d_device->CreateSamplerState(&desc_sampler, &tex->sampler)))
		skr_log("skr_tex_settings: failed to create sampler state!");
}

///////////////////////////////////////////

void skr_make_mips(D3D11_SUBRESOURCE_DATA *tex_mem, void *curr_data, skr_tex_fmt_ format, int32_t width, int32_t height, uint32_t mip_levels) {
	void    *mip_data = curr_data;
	int32_t  mip_w    = width;
	int32_t  mip_h    = height;
	for (uint32_t m = 1; m < mip_levels; m++) {
		tex_mem[m] = {};
		switch (format) {
		case skr_tex_fmt_rgba32:
		case skr_tex_fmt_rgba32_linear: 
			skr_downsample_4((uint8_t  *)mip_data, mip_w, mip_h, (uint8_t  **)&tex_mem[m].pSysMem, &mip_w, &mip_h); 
			break;
		case skr_tex_fmt_rgba64:
			skr_downsample_4((uint16_t *)mip_data, mip_w, mip_h, (uint16_t **)&tex_mem[m].pSysMem, &mip_w, &mip_h);
			break;
		case skr_tex_fmt_rgba128:
			skr_downsample_4((float    *)mip_data, mip_w, mip_h, (float    **)&tex_mem[m].pSysMem, &mip_w, &mip_h);
			break;
		case skr_tex_fmt_depth32:
		case skr_tex_fmt_r32:
			skr_downsample_1((float    *)mip_data, mip_w, mip_h, (float    **)&tex_mem[m].pSysMem, &mip_w, &mip_h); 
			break;
		case skr_tex_fmt_depth16:
		case skr_tex_fmt_r16:
			skr_downsample_1((uint16_t *)mip_data, mip_w, mip_h, (uint16_t **)&tex_mem[m].pSysMem, &mip_w, &mip_h); 
			break;
		case skr_tex_fmt_r8:
			skr_downsample_1((uint8_t  *)mip_data, mip_w, mip_h, (uint8_t  **)&tex_mem[m].pSysMem, &mip_w, &mip_h); 
			break;
		}
		mip_data = (void*)tex_mem[m].pSysMem;
		tex_mem[m].SysMemPitch = (UINT)(skr_tex_fmt_size(format) * mip_w);
	}
}

///////////////////////////////////////////

bool skr_tex_make_view(skr_tex_t *tex, uint32_t mip_count, uint32_t array_size, bool use_in_shader) {
	DXGI_FORMAT format = (DXGI_FORMAT)skr_tex_fmt_to_native(tex->format);

	if (tex->type != skr_tex_type_depth) {
		D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {};
		res_desc.Format = format;
		if (tex->type == skr_tex_type_cubemap) {
			res_desc.TextureCube.MipLevels = mip_count;
			res_desc.ViewDimension         = D3D11_SRV_DIMENSION_TEXTURECUBE;
		} else if (array_size > 1) {
			res_desc.Texture2DArray.MipLevels = mip_count;
			res_desc.Texture2DArray.ArraySize = array_size;
			res_desc.ViewDimension            = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		} else {
			res_desc.Texture2D.MipLevels = mip_count;
			res_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
		}

		if (use_in_shader && FAILED(d3d_device->CreateShaderResourceView(tex->texture, &res_desc, &tex->resource))) {
			skr_log("Create Shader Resource View error!");
			return false;
		}
	} else {
		D3D11_DEPTH_STENCIL_VIEW_DESC stencil_desc = {};
		stencil_desc.Format = format;
		if (tex->type == skr_tex_type_cubemap || array_size > 1) {
			stencil_desc.Texture2DArray.ArraySize = array_size;
			stencil_desc.ViewDimension            = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		} else {
			stencil_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		}
		if (FAILED(d3d_device->CreateDepthStencilView(tex->texture, &stencil_desc, &tex->depth_view))) {
			skr_log("Create Depth Stencil View error!");
			return false;
		}
	}

	if (tex->type == skr_tex_type_rendertarget) {
		D3D11_RENDER_TARGET_VIEW_DESC target_desc = {};
		target_desc.Format = format;
		if (tex->type == skr_tex_type_cubemap || array_size > 1) {
			target_desc.Texture2DArray.ArraySize = array_size;
			target_desc.ViewDimension            = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		} else {
			target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		}

		if (FAILED(d3d_device->CreateRenderTargetView(tex->texture, &target_desc, &tex->target_view))) {
			skr_log("Create Render Target View error!");
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////

void skr_tex_set_contents(skr_tex_t *tex, void **data_frames, int32_t data_frame_count, int32_t width, int32_t height) {
	// Some warning messages
	if (tex->use != skr_use_dynamic && tex->texture) {
		skr_log("Only dynamic textures can be updated!");
		return;
	}
	if (tex->use == skr_use_dynamic && (tex->mips == skr_mip_generate || data_frame_count > 1)) {
		skr_log("Dynamic textures don't support mip-maps or texture arrays!");
		return;
	}

	tex->width       = width;
	tex->height      = height;
	tex->array_count = data_frame_count;

	uint32_t mip_levels = (tex->mips == skr_mip_generate ? (uint32_t)log2(width) + 1 : 1);
	uint32_t px_size    = skr_tex_fmt_size(tex->format);

	if (tex->texture == nullptr) {
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width            = width;
		desc.Height           = height;
		desc.MipLevels        = mip_levels;
		desc.ArraySize        = data_frame_count;
		desc.SampleDesc.Count = 1;
		desc.Format           = (DXGI_FORMAT)skr_tex_fmt_to_native(tex->format);
		desc.BindFlags        = tex->type == skr_tex_type_depth ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_SHADER_RESOURCE;
		desc.Usage            = tex->use  == skr_use_dynamic    ? D3D11_USAGE_DYNAMIC      : D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags   = tex->use  == skr_use_dynamic    ? D3D11_CPU_ACCESS_WRITE   : 0;
		if (tex->type == skr_tex_type_rendertarget) desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		if (tex->type == skr_tex_type_cubemap     ) desc.MiscFlags  = D3D11_RESOURCE_MISC_TEXTURECUBE;

		D3D11_SUBRESOURCE_DATA *tex_mem = nullptr;
		if (data_frames != nullptr && data_frames[0] != nullptr) {
			tex_mem = (D3D11_SUBRESOURCE_DATA *)malloc((int64_t)data_frame_count * mip_levels * sizeof(D3D11_SUBRESOURCE_DATA));
			if (!tex_mem) { skr_log("Out of memory"); return;  }

			for (int32_t i = 0; i < data_frame_count; i++) {
				tex_mem[i*mip_levels] = {};
				tex_mem[i*mip_levels].pSysMem     = data_frames[i];
				tex_mem[i*mip_levels].SysMemPitch = (UINT)(px_size * width);

				if (tex->mips == skr_mip_generate) {
					skr_make_mips(&tex_mem[i*mip_levels], data_frames[i], tex->format, width, height, mip_levels);
				}
			}
		}

		if (FAILED(d3d_device->CreateTexture2D(&desc, tex_mem, &tex->texture))) {
			skr_log("Create texture error!");
		}

		if (tex_mem != nullptr) {
			for (int32_t i = 0; i < data_frame_count; i++) {
				for (uint32_t m = 1; m < mip_levels; m++) {
					free((void*)tex_mem[i*mip_levels + m].pSysMem);
				} 
			}
			free(tex_mem);
		}

		skr_tex_make_view(tex, mip_levels, data_frame_count, true);
	} else {
		// For dynamic textures, just upload the new value into the texture!
		D3D11_MAPPED_SUBRESOURCE tex_mem = {};
		if (FAILED(d3d_context->Map(tex->texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &tex_mem))) {
			skr_log("Failed mapping a texture");
			return;
		}

		uint8_t *dest_line  = (uint8_t *)tex_mem.pData;
		uint8_t *src_line   = (uint8_t *)data_frames[0];
		for (int i = 0; i < height; i++) {
			memcpy(dest_line, src_line, (size_t)width * px_size);
			dest_line += tex_mem.RowPitch;
			src_line  += px_size * (uint64_t)width;
		}
		d3d_context->Unmap(tex->texture, 0);
	}

	// If the sampler has not been set up yet, we'll make a default one real quick.
	if (tex->sampler == nullptr) {
		skr_tex_settings(tex, skr_tex_address_repeat, skr_tex_sample_linear, 0);
	}
}

/////////////////////////////////////////// 

void skr_tex_bind(const skr_tex_t *texture, skr_bind_t bind) {
	if (texture != nullptr) {
		if (bind.stage_bits & skr_stage_pixel) {
			d3d_context->PSSetSamplers       (bind.slot, 1, &texture->sampler);
			d3d_context->PSSetShaderResources(bind.slot, 1, &texture->resource);
		}
		if (bind.stage_bits & skr_stage_vertex) {
			d3d_context->VSSetSamplers       (bind.slot, 1, &texture->sampler);
			d3d_context->VSSetShaderResources(bind.slot, 1, &texture->resource);
		}
	} else {
		if (bind.stage_bits & skr_stage_pixel) {
			d3d_context->PSSetShaderResources(bind.slot, 0, nullptr);
		}
		if (bind.stage_bits & skr_stage_vertex) {
			d3d_context->VSSetShaderResources(bind.slot, 0, nullptr);
		}
	}
}

/////////////////////////////////////////// 

void skr_tex_destroy(skr_tex_t *tex) {
	if (tex->target_view) tex->target_view->Release();
	if (tex->depth_view ) tex->depth_view ->Release();
	if (tex->resource) tex->resource->Release();
	if (tex->sampler ) tex->sampler ->Release();
	if (tex->texture ) tex->texture ->Release();
}

/////////////////////////////////////////// 

template <typename T>
void skr_downsample_4(T *data, int32_t width, int32_t height, T **out_data, int32_t *out_width, int32_t *out_height) {
	int w = (int32_t)log2(width);
	int h = (int32_t)log2(height);
	*out_width  = w = max(1, (1 << w) >> 1);
	*out_height = h = max(1, (1 << h) >> 1);

	*out_data = (T*)malloc((int64_t)w * h * sizeof(T) * 4);
	if (*out_data == nullptr) { skr_log("Out of memory"); return; }
	memset(*out_data, 0, (int64_t)w * h * sizeof(T) * 4);
	T *result = *out_data;

	for (int32_t y = 0; y < height; y++) {
		int32_t src_row_start  = y * width;
		int32_t dest_row_start = (y / 2) * w;
		for (int32_t x = 0; x < width;  x++) {
			int src  = (x + src_row_start)*4;
			int dest = ((x / 2) + dest_row_start)*4;
			T *cD = &result[dest];
			T *cS = &data  [src];

			cD[0] += cS[0] / 4;
			cD[1] += cS[1] / 4;
			cD[2] += cS[2] / 4;
			cD[3] += cS[3] / 4;
		}
	}
}

/////////////////////////////////////////// 

template <typename T>
void skr_downsample_1(T *data, int32_t width, int32_t height, T **out_data, int32_t *out_width, int32_t *out_height) {
	int w = (int32_t)log2(width);
	int h = (int32_t)log2(height);
	*out_width  = w = (1 << w) >> 1;
	*out_height = h = (1 << h) >> 1;

	*out_data = (T*)malloc((int64_t)w * h * sizeof(T));
	if (*out_data == nullptr) { skr_log("Out of memory"); return; }
	memset(*out_data, 0, (int64_t)w * h * sizeof(T));
	T *result = *out_data;

	for (int32_t y = 0; y < height; y++) {
		int32_t src_row_start  = y * width;
		int32_t dest_row_start = (y / 2) * w;
		for (int32_t x = 0; x < width;  x++) {
			int src  = (x + src_row_start);
			int dest = ((x / 2) + dest_row_start);
			result[dest] = result[dest] + (data[src] / 4);
		}
	}
}

/////////////////////////////////////////// 

int64_t skr_tex_fmt_to_native(skr_tex_fmt_ format){
	switch (format) {
	case skr_tex_fmt_rgba32:        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case skr_tex_fmt_rgba32_linear: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case skr_tex_fmt_bgra32:        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	case skr_tex_fmt_bgra32_linear: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case skr_tex_fmt_rgba64:        return DXGI_FORMAT_R16G16B16A16_UNORM;
	case skr_tex_fmt_rgba128:       return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case skr_tex_fmt_depth16:       return DXGI_FORMAT_D16_UNORM;
	case skr_tex_fmt_depth32:       return DXGI_FORMAT_D32_FLOAT;
	case skr_tex_fmt_depthstencil:  return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case skr_tex_fmt_r8:            return DXGI_FORMAT_R8_UNORM;
	case skr_tex_fmt_r16:           return DXGI_FORMAT_R16_UNORM;
	case skr_tex_fmt_r32:           return DXGI_FORMAT_R32_FLOAT;
	default: return DXGI_FORMAT_UNKNOWN;
	}
}

/////////////////////////////////////////// 

skr_tex_fmt_ skr_tex_fmt_from_native(int64_t format) {
	switch (format) {
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return skr_tex_fmt_rgba32;
	case DXGI_FORMAT_R8G8B8A8_UNORM:      return skr_tex_fmt_rgba32_linear;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return skr_tex_fmt_bgra32;
	case DXGI_FORMAT_B8G8R8A8_UNORM:      return skr_tex_fmt_bgra32_linear;
	case DXGI_FORMAT_R16G16B16A16_UNORM:  return skr_tex_fmt_rgba64;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:  return skr_tex_fmt_rgba128;
	case DXGI_FORMAT_D16_UNORM:           return skr_tex_fmt_depth16;
	case DXGI_FORMAT_D32_FLOAT:           return skr_tex_fmt_depth32;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:   return skr_tex_fmt_depthstencil;
	case DXGI_FORMAT_R8_UNORM:            return skr_tex_fmt_r8;
	case DXGI_FORMAT_R16_UNORM:           return skr_tex_fmt_r16;
	case DXGI_FORMAT_R32_FLOAT:           return skr_tex_fmt_r32;
	default: return skr_tex_fmt_none;
	}
}

/////////////////////////////////////////// 

uint32_t skr_tex_fmt_size(skr_tex_fmt_ format) {
	switch (format) {
	case skr_tex_fmt_rgba32:        return sizeof(uint8_t )*4;
	case skr_tex_fmt_rgba32_linear: return sizeof(uint8_t )*4;
	case skr_tex_fmt_bgra32:        return sizeof(uint8_t )*4;
	case skr_tex_fmt_bgra32_linear: return sizeof(uint8_t )*4;
	case skr_tex_fmt_rgba64:        return sizeof(uint16_t)*4;
	case skr_tex_fmt_rgba128:       return sizeof(uint32_t)*4;
	case skr_tex_fmt_depth16:       return sizeof(uint16_t);
	case skr_tex_fmt_depth32:       return sizeof(uint32_t);
	case skr_tex_fmt_depthstencil:  return sizeof(uint32_t);
	case skr_tex_fmt_r8:            return sizeof(uint8_t );
	case skr_tex_fmt_r16:           return sizeof(uint16_t);
	case skr_tex_fmt_r32:           return sizeof(uint32_t);
	default: return 0;
	}
}

/////////////////////////////////////////// 

const char *skr_semantic_to_d3d(skr_el_semantic_ semantic) {
	switch (semantic) {
	case skr_el_semantic_none:         return "";
	case skr_el_semantic_position:     return "SV_POSITION";
	case skr_el_semantic_normal:       return "NORMAL";
	case skr_el_semantic_texcoord:     return "TEXCOORD";
	case skr_el_semantic_color:        return "COLOR";
	case skr_el_semantic_target_index: return "SV_RenderTargetArrayIndex";
	default: return "";
	}
}

#endif
#ifdef SKR_OPENGL

///////////////////////////////////////////
// OpenGL Implementation                 //
///////////////////////////////////////////

#include <malloc.h>
#include <stdio.h>

///////////////////////////////////////////

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl32.h>
#elif __ANDROID__
#include <EGL/egl.h>
#include <EGL/eglext.h>
//#include <GLES/gl.h>
//#include <GLES3/gl3.h>
//#include <GLES3/gl3ext.h>

EGLDisplay egl_display;
EGLSurface egl_surface;
EGLContext egl_context;
EGLConfig  egl_config;
#elif _WIN32
#pragma comment(lib, "opengl32.lib")

#define EMSCRIPTEN_KEEPALIVE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

HWND  gl_hwnd;
HDC   gl_hdc;
HGLRC gl_hrc;
#endif

///////////////////////////////////////////
// GL loader                             //
///////////////////////////////////////////

// Reference from here:
// https://github.com/ApoorvaJ/Papaya/blob/3808e39b0f45d4ca4972621c847586e4060c042a/src/libs/gl_lite.h

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
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001

#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_UNIFORM_BUFFER 0x8A11
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_TRIANGLES 0x0004
#define GL_DEBUG_OUTPUT 0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#define GL_VERSION 0x1F02
#define GL_CULL_FACE 0x0B44
#define GL_BACK 0x0405
#define GL_FRONT 0x0404
#define GL_FRONT_AND_BACK 0x0408
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02
#define GL_DEPTH_TEST 0x0B71
#define GL_TEXTURE_2D 0x0DE1
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
#define GL_TEXTURE_MAX_ANISOTROPY 0x84FE
#define GL_TEXTURE0 0x84C0
#define GL_FRAMEBUFFER 0x8D40
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A

#define GL_BLEND 0x0BE2

#define GL_RED 0x1903
#define GL_RGBA 0x1908
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
#define GL_RGBA32F 0x8814
#define GL_RGBA16F 0x881A
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
#define GL_FLOAT 0x1406
#define GL_DOUBLE 0x140A

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84

#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B
#define GL_DEBUG_SEVERITY_HIGH 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#define GL_DEBUG_SEVERITY_LOW 0x9148

//#endif

#if defined(_WIN32) || defined(__ANDROID__)

#ifdef _WIN32
#define GLDECL __stdcall
#else
#define GLDECL
#endif

typedef void (GLDECL *GLDEBUGPROC)(uint32_t source, uint32_t type, uint32_t id, int32_t severity, int32_t length, const char* message, const void* userParam);

#define GL_API \
    GLE(void,     LinkProgram,             uint32_t program) \
	GLE(void,     ClearColor,              float r, float g, float b, float a) \
	GLE(void,     Clear,                   uint32_t mask) \
	GLE(void,     Enable,                  uint32_t cap) \
	GLE(void,     Disable,                 uint32_t cap) \
	GLE(void,     PolygonMode,             uint32_t face, uint32_t mode) \
	GLE(uint32_t, GetError,                ) \
    GLE(void,     GetProgramiv,            uint32_t program, uint32_t pname, int32_t *params) \
    GLE(uint32_t, CreateShader,            uint32_t type) \
    GLE(void,     ShaderSource,            uint32_t shader, int32_t count, const char* const *string, const int32_t *length) \
    GLE(void,     CompileShader,           uint32_t shader) \
    GLE(void,     GetShaderiv,             uint32_t shader, uint32_t pname, int32_t *params) \
    GLE(void,     GetShaderInfoLog,        uint32_t shader, int32_t bufSize, int32_t *length, char *infoLog) \
	GLE(void,     GetProgramInfoLog,       uint32_t program, int32_t maxLength, int32_t *length, char *infoLog) \
    GLE(void,     DeleteShader,            uint32_t shader) \
    GLE(uint32_t, CreateProgram,           void) \
    GLE(void,     AttachShader,            uint32_t program, uint32_t shader) \
    GLE(void,     DetachShader,            uint32_t program, uint32_t shader) \
    GLE(void,     UseProgram,              uint32_t program) \
    GLE(void,     DeleteProgram,           uint32_t program) \
    GLE(void,     GenVertexArrays,         int32_t n, uint32_t *arrays) \
    GLE(void,     BindVertexArray,         uint32_t array) \
    GLE(void,     BufferData,              uint32_t target, int32_t size, const void *data, uint32_t usage) \
    GLE(void,     GenBuffers,              int32_t n, uint32_t *buffers) \
    GLE(void,     BindBuffer,              uint32_t target, uint32_t buffer) \
    GLE(void,     DeleteBuffers,           int32_t n, const uint32_t *buffers) \
	GLE(void,     GenTextures,             int32_t n, uint32_t *textures) \
	GLE(void,     GenFramebuffers,         int32_t n, uint32_t *ids) \
	GLE(void,     DeleteFramebuffers,      int32_t n, uint32_t *ids) \
	GLE(void,     FramebufferTexture2D,    uint32_t target, uint32_t attachment, uint32_t textarget, uint32_t texture, int32_t level) \
	GLE(void,     DeleteTextures,          int32_t n, const uint32_t *textures) \
	GLE(void,     BindTexture,             uint32_t target, uint32_t texture) \
	GLE(void,     BindFramebuffer,         uint32_t target, uint32_t framebuffer) \
	GLE(void,     FramebufferTexture,      uint32_t target, uint32_t attachment, uint32_t texture, int32_t level) \
    GLE(void,     TexParameteri,           uint32_t target, uint32_t pname, int32_t param) \
	GLE(void,     GetInternalformativ,     uint32_t target, uint32_t internalformat, uint32_t pname, int32_t bufSize, int32_t *params)\
	GLE(void,     GetTexLevelParameteriv,  uint32_t target, int32_t level, uint32_t pname, int32_t *params) \
	GLE(void,     TexParameterf,           uint32_t target, uint32_t pname, float param) \
	GLE(void,     TexImage2D,              uint32_t target, int32_t level, int32_t internalformat, int32_t width, int32_t height, int32_t border, uint32_t format, uint32_t type, const void *data) \
    GLE(void,     ActiveTexture,           uint32_t texture) \
	GLE(void,     GenerateMipmap,          uint32_t target) \
    GLE(void,     BindAttribLocation,      uint32_t program, uint32_t index, const char *name) \
    GLE(int32_t,  GetUniformLocation,      uint32_t program, const char *name) \
    GLE(void,     Uniform4f,               int32_t location, float v0, float v1, float v2, float v3) \
    GLE(void,     Uniform4fv,              int32_t location, int32_t count, const float *value) \
    GLE(void,     DeleteVertexArrays,      int32_t n, const uint32_t *arrays) \
    GLE(void,     EnableVertexAttribArray, uint32_t index) \
    GLE(void,     VertexAttribPointer,     uint32_t index, int32_t size, uint32_t type, uint8_t normalized, int32_t stride, const void *pointer) \
    GLE(void,     Uniform1i,               int32_t location, int32_t v0) \
	GLE(void,     DrawElementsInstanced,   uint32_t mode, int32_t count, uint32_t type, const void *indices, int32_t primcount) \
	GLE(void,     DrawElements,            uint32_t mode, int32_t count, uint32_t type, const void *indices) \
	GLE(void,     DebugMessageCallback,    GLDEBUGPROC callback, const void *userParam) \
	GLE(void,     BindBufferBase,          uint32_t target, uint32_t index, uint32_t buffer) \
	GLE(void,     BufferSubData,           uint32_t target, int64_t offset, int32_t size, const void *data) \
	GLE(void,     Viewport,                int32_t x, int32_t y, int32_t width, int32_t height) \
	GLE(void,     CullFace,                uint32_t mode) \
	GLE(const char *, GetString,           uint32_t name)

#define GLE(ret, name, ...) typedef ret GLDECL name##proc(__VA_ARGS__); static name##proc * gl##name;
GL_API
#undef GLE

#ifdef _WIN32

// from https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions
// Some GL functions can only be loaded with wglGetProcAddress, and others
// can only be loaded by GetProcAddress.
void *gl_get_func(const char *name, HMODULE module) {
	void *f = (void *)wglGetProcAddress(name);
	if(f == 0 || (f == (void*)0x1) || (f == (void*)0x2) || (f == (void*)0x3) || (f == (void*)-1) ) {
		f = (void *)GetProcAddress(module, name);
	}
	return f;
}
static void gl_load_extensions( ) {
	HMODULE dll = LoadLibraryA("opengl32.dll");
#define GLE(ret, name, ...) gl##name = (name##proc *) gl_get_func("gl" #name, dll); if (gl##name == nullptr) skr_log("Couldn't load gl function gl" #name);
	GL_API
#undef GLE
}

#else

static void gl_load_extensions( ) {
#define GLE(ret, name, ...) gl##name = (name##proc *) eglGetProcAddress("gl" #name); if (gl##name == nullptr) skr_log("Couldn't load gl function gl" #name);
	GL_API
#undef GLE
}

#endif

#endif // _WIN32 or __ANDROID__

///////////////////////////////////////////

int32_t    gl_width  = 0;
int32_t    gl_height = 0;
skr_tex_t *gl_active_rendertarget = nullptr;
uint32_t   gl_current_framebuffer = 0;

///////////////////////////////////////////

uint32_t skr_buffer_type_to_gl   (skr_buffer_type_ type);
uint32_t skr_tex_fmt_to_gl_type  (skr_tex_fmt_ format);
uint32_t skr_tex_fmt_to_gl_layout(skr_tex_fmt_ format);

///////////////////////////////////////////

// Some nice reference: https://gist.github.com/nickrolfe/1127313ed1dbf80254b614a721b3ee9c
int32_t gl_init_win32(void *app_hwnd) {
#ifdef _WIN32
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
		skr_log("Failed to find a suitable pixel format.");
		return false;
	}
	if (!SetPixelFormat(dummy_dc, pixel_format, &format_desc)) {
		skr_log("Failed to set the pixel format.");
		return false;
	}
	HGLRC dummy_context = wglCreateContext(dummy_dc);
	if (!dummy_context) {
		skr_log("Failed to create a dummy OpenGL rendering context.");
		return false;
	}
	if (!wglMakeCurrent(dummy_dc, dummy_context)) {
		skr_log("Failed to activate dummy OpenGL rendering context.");
		return false;
	}

	// Load the function pointers we need to actually initialize OpenGL
	typedef BOOL  (*wglChoosePixelFormatARB_proc)    (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
	typedef HGLRC (*wglCreateContextAttribsARB_proc) (HDC hDC, HGLRC hShareContext, const int *attribList);
	wglChoosePixelFormatARB_proc    wglChoosePixelFormatARB    = (wglChoosePixelFormatARB_proc   )wglGetProcAddress("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB_proc wglCreateContextAttribsARB = (wglCreateContextAttribsARB_proc)wglGetProcAddress("wglCreateContextAttribsARB");

	// Shut down the dummy so we can set up OpenGL for real
	wglMakeCurrent  (dummy_dc, 0);
	wglDeleteContext(dummy_context);
	ReleaseDC       (dummy_window, dummy_dc);
	DestroyWindow   (dummy_window);

	///////////////////////////////////////////
	// Real OpenGL initialization            //
	///////////////////////////////////////////

	if (app_hwnd == nullptr) {
		WNDCLASSA win_class = { 0 };
		win_class.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		win_class.lpfnWndProc   = DefWindowProcA;
		win_class.hInstance     = GetModuleHandle(0);
		win_class.lpszClassName = "SKGPUWindow";
		if (!RegisterClassA(&win_class))
			return false;

		app_hwnd = CreateWindowExA(0, win_class.lpszClassName, "sk_gpu Window", 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, win_class.hInstance, 0);
		if (!app_hwnd)
			return false;
	}

	gl_hwnd = (HWND)app_hwnd;
	gl_hdc  = GetDC(gl_hwnd);

	RECT bounds;
	GetWindowRect(gl_hwnd, &bounds);
	gl_width  = bounds.right  - bounds.left;
	gl_height = bounds.bottom - bounds.top;

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
		skr_log("Couldn't find pixel format!");
		return false;
	}

	memset(&format_desc, 0, sizeof(format_desc));
	DescribePixelFormat(gl_hdc, pixel_format, sizeof(format_desc), &format_desc);
	if (!SetPixelFormat(gl_hdc, pixel_format, &format_desc)) {
		skr_log("Couldn't set pixel format!");
		return false;
	}

	// Create an OpenGL context
	int attributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3, 
		WGL_CONTEXT_MINOR_VERSION_ARB, 3, 
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0 };
	gl_hrc = wglCreateContextAttribsARB( gl_hdc, 0, attributes );
	if (!gl_hrc) {
		skr_log("Couldn't create GL context!");
		return false;
	}
	if (!wglMakeCurrent(gl_hdc, gl_hrc)) {
		skr_log("Couldn't activate GL context!");
		return false;
	}
#endif // _WIN32
	return 1;
}

///////////////////////////////////////////

int32_t gl_init_emscripten() {
#ifdef __EMSCRIPTEN__
	EmscriptenWebGLContextAttributes attrs;
	emscripten_webgl_init_context_attributes(&attrs);
	attrs.alpha = true;
	attrs.depth = true;
	attrs.enableExtensionsByDefault = true;
	attrs.majorVersion = 2;
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("canvas", &attrs);
	emscripten_webgl_make_context_current(ctx);
#endif // __EMSCRIPTEN__
	return 1;
}

///////////////////////////////////////////

int32_t gl_init_android(void *native_window) {
#ifdef __ANDROID__
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
		EGL_NONE, EGL_NONE };
	EGLint format;
	EGLint numConfigs;

	egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (eglGetError() != EGL_SUCCESS) skr_log("sk_gpu: err eglGetDisplay");

	int32_t major, minor;
	eglInitialize     (egl_display, &major, &minor);
	if (eglGetError() != EGL_SUCCESS) skr_log("sk_gpu: err eglInitialize");
	eglChooseConfig   (egl_display, attribs, &egl_config, 1, &numConfigs);
	if (eglGetError() != EGL_SUCCESS) skr_log("sk_gpu: err eglChooseConfig");
	eglGetConfigAttrib(egl_display, egl_config, EGL_NATIVE_VISUAL_ID, &format);
	if (eglGetError() != EGL_SUCCESS) skr_log("sk_gpu: err eglGetConfigAttrib");
	
	egl_surface = eglCreateWindowSurface(egl_display, egl_config, (EGLNativeWindowType)native_window, nullptr);
	if (eglGetError() != EGL_SUCCESS) skr_log("sk_gpu: err eglCreateWindowSurface");
	egl_context = eglCreateContext      (egl_display, egl_config, nullptr, context_attribs);
	if (eglGetError() != EGL_SUCCESS) skr_log("sk_gpu: err eglCreateContext");

	if (eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context) == EGL_FALSE) {
		skr_log("sk_gpu: Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(egl_display, egl_surface, EGL_WIDTH,  &gl_width);
	eglQuerySurface(egl_display, egl_surface, EGL_HEIGHT, &gl_height);
#endif // __ANDROID__
	return 1;
}

///////////////////////////////////////////


int32_t skr_init(const char *app_name, void *app_hwnd, void *adapter_id) {
#if defined(_WIN32)
	int32_t result = gl_init_win32(app_hwnd);
#elif defined(__ANDROID__)
	int32_t result = gl_init_android(app_hwnd);
#elif defined(__EMSCRIPTEN__)
	int32_t result = gl_init_emscripten();
#endif
	if (!result)
		return result;

	// Load OpenGL function pointers
	gl_load_extensions();

	skr_log("sk_gpu: Using OpenGL");
	skr_log(glGetString(GL_VERSION));

#if _DEBUG
	skr_log("sk_gpu: Debug info enabled.");
	// Set up debug info for development
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback([](uint32_t source, uint32_t type, uint32_t id, int32_t severity, int32_t length, const char *message, const void *userParam) {
		if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
			//skr_log(message);
		} else {
			skr_log(message);
		} }, nullptr);
#endif // _DEBUG
	
	// Some default behavior
	glEnable  (GL_DEPTH_TEST);  
	glEnable  (GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	return 1;
}

///////////////////////////////////////////

void skr_shutdown() {
#ifdef _WIN32
	wglMakeCurrent(NULL, NULL);
	ReleaseDC(gl_hwnd, gl_hdc);
	wglDeleteContext(gl_hrc);
#elif __ANDROID__
	if (egl_display != EGL_NO_DISPLAY) {
		eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (egl_context != EGL_NO_CONTEXT) eglDestroyContext(egl_display, egl_context);
		if (egl_surface != EGL_NO_SURFACE) eglDestroySurface(egl_display, egl_surface);
		eglTerminate(egl_display);
	}
	egl_display = EGL_NO_DISPLAY;
	egl_context = EGL_NO_CONTEXT;
	egl_surface = EGL_NO_SURFACE;
#endif
}

///////////////////////////////////////////

void skr_draw_begin() {
}

///////////////////////////////////////////

void skr_tex_target_bind(skr_tex_t *render_target, bool clear, const float *clear_color_4) {
	gl_active_rendertarget = render_target;
	gl_current_framebuffer = render_target == nullptr ? 0 : render_target->framebuffer;

	glBindFramebuffer(GL_FRAMEBUFFER, gl_current_framebuffer);
	if (render_target) {
		glViewport(0, 0, render_target->width, render_target->height);
	} else {
		glViewport(0, 0, gl_width, gl_height);
	}

	if (clear) {
		glClearColor(clear_color_4[0], clear_color_4[1], clear_color_4[2], clear_color_4[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

///////////////////////////////////////////

skr_tex_t *skr_tex_target_get() {
	return gl_active_rendertarget;
}

///////////////////////////////////////////

skr_platform_data_t skr_get_platform_data() {
	skr_platform_data_t result = {};
#ifdef _WIN32
	result.gl_hdc = gl_hdc;
	result.gl_hrc = gl_hrc;
#elif __ANDROID__
	result.egl_display = egl_display;
	result.egl_config  = egl_config;
	result.egl_context = egl_context;
#endif
	return result;
}

///////////////////////////////////////////

void skr_draw(int32_t index_start, int32_t index_count, int32_t instance_count) {
	glDrawElementsInstanced(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, (void*)(index_start*sizeof(uint32_t)), instance_count);
}

///////////////////////////////////////////

skr_buffer_t skr_buffer_create(const void *data, uint32_t size_bytes, skr_buffer_type_ type, skr_use_ use) {
	skr_buffer_t result = {};
	result.use  = use;
	result.type = skr_buffer_type_to_gl(type);

	glGenBuffers(1, &result.buffer);
	glBindBuffer(result.type, result.buffer);
	glBufferData(result.type, size_bytes, data, use == skr_use_static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

	return result;
}

/////////////////////////////////////////// 

bool skr_buffer_is_valid(const skr_buffer_t *buffer) {
	return buffer->buffer != 0;
}

/////////////////////////////////////////// 

void skr_buffer_set_contents(skr_buffer_t *buffer, const void *data, uint32_t size_bytes) {
	if (buffer->use != skr_use_dynamic)
		return;

	glBindBuffer(buffer->type, buffer->buffer);
	glBufferSubData(buffer->type, 0, size_bytes, data);
}

/////////////////////////////////////////// 

void skr_buffer_bind(const skr_buffer_t *buffer, skr_bind_t bind, uint32_t stride, uint32_t offset) {
	if (buffer->type == GL_UNIFORM_BUFFER)
		glBindBufferBase(buffer->type, bind.slot, buffer->buffer); 
	else
		glBindBuffer(buffer->type, buffer->buffer);
}

/////////////////////////////////////////// 

void skr_buffer_destroy(skr_buffer_t *buffer) {
	glDeleteBuffers(1, &buffer->buffer);
	*buffer = {};
}

/////////////////////////////////////////// 

skr_mesh_t skr_mesh_create(const skr_buffer_t *vert_buffer, const skr_buffer_t *ind_buffer) {
	skr_mesh_t result = {};
	result.index_buffer = ind_buffer  ? ind_buffer ->buffer : 0;
	result.vert_buffer  = vert_buffer ? vert_buffer->buffer : 0;

	// Create a vertex layout
	glGenVertexArrays(1, &result.layout);
	glBindVertexArray(result.layout);
	// enable the vertex data for the shader
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	// tell the shader how our vertex data binds to the shader inputs
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(skr_vert_t), nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, 0, sizeof(skr_vert_t), (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 2, GL_FLOAT, 0, sizeof(skr_vert_t), (void*)(sizeof(float) * 6));
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, 1, sizeof(skr_vert_t), (void*)(sizeof(float) * 8));

	return result;
}

/////////////////////////////////////////// 

void skr_mesh_set_verts(skr_mesh_t *mesh, const skr_buffer_t *vert_buffer) {
	mesh->vert_buffer = vert_buffer->buffer;
}

/////////////////////////////////////////// 

void skr_mesh_set_inds(skr_mesh_t *mesh, const skr_buffer_t *ind_buffer) {
	mesh->index_buffer = ind_buffer->buffer;
}

/////////////////////////////////////////// 

void skr_mesh_bind(const skr_mesh_t *mesh) {
	glBindVertexArray(mesh->layout);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vert_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer);
}

/////////////////////////////////////////// 

void skr_mesh_destroy(skr_mesh_t *mesh) {
	glDeleteVertexArrays(1, &mesh->layout);
	*mesh = {};
}

/////////////////////////////////////////// 

skr_shader_stage_t skr_shader_stage_create(const void *file_data, size_t shader_size, skr_stage_ type) {
	const char *file_chars = (const char *)file_data;

	skr_shader_stage_t result = {}; 
	result.type = type;

	// Include terminating character
	if (shader_size > 0 && file_chars[shader_size-1] != '\0')
		shader_size += 1;

	uint32_t gl_type = 0;
	switch (type) {
	case skr_stage_pixel:  gl_type = GL_FRAGMENT_SHADER; break;
	case skr_stage_vertex: gl_type = GL_VERTEX_SHADER;   break;
	}

	// Convert the prefix if it doesn't match the GL version we're using
	const char   *prefix_gl      = "#version 450";
	const size_t  prefix_gl_size = strlen(prefix_gl);
	const char   *prefix_es      = "#version 300 es";
	const size_t  prefix_es_size = strlen(prefix_es);
	char         *final_data = (char*)file_chars;
	bool          needs_free = false;
#if __ANDROID__
	if (shader_size >= prefix_gl_size && memcmp(prefix_gl, file_chars, prefix_gl_size) == 0) {
		final_data = (char*)malloc(sizeof(char) * ((shader_size-prefix_gl_size)+prefix_es_size));
		memcpy(final_data, prefix_es, prefix_es_size);
		memcpy(&final_data[prefix_es_size], &file_chars[prefix_gl_size], shader_size - prefix_gl_size);
		needs_free = true;
	}
#else
	if (shader_size >= prefix_es_size && memcmp(prefix_es, file_chars, prefix_es_size) == 0) {
		final_data = (char*)malloc(sizeof(char) * ((shader_size-prefix_es_size)+prefix_gl_size));
		memcpy(final_data, prefix_gl, prefix_gl_size);
		memcpy(&final_data[prefix_gl_size], &file_chars[prefix_es_size], shader_size - prefix_es_size);
		needs_free = true;
	}
#endif

	// create and compile the vertex shader
	result.shader = glCreateShader(gl_type);
	glShaderSource (result.shader, 1, &final_data, NULL);
	glCompileShader(result.shader);

	// check for errors?
	int32_t err, length;
	glGetShaderiv(result.shader, GL_COMPILE_STATUS, &err);
	if (err == 0) {
		char *log;

		glGetShaderiv(result.shader, GL_INFO_LOG_LENGTH, &length);
		log = (char*)malloc(length);
		glGetShaderInfoLog(result.shader, length, &err, log);

		skr_log("Unable to compile shader:\n");
		skr_log(log);
		free(log);
	}
	if (needs_free)
		free(final_data);

	return result;
}

/////////////////////////////////////////// 

void skr_shader_stage_destroy(skr_shader_stage_t *shader) {
	//glDeleteShader(shader->shader);
	*shader = {};
}

///////////////////////////////////////////
// skr_shader_t                          //
///////////////////////////////////////////

skr_shader_t skr_shader_create_manual(skr_shader_meta_t *meta, skr_shader_stage_t v_shader, skr_shader_stage_t p_shader) {
	skr_shader_t result = {};
	result.meta   = meta;
	result.vertex = v_shader.shader;
	result.pixel  = p_shader.shader;
	skr_shader_meta_reference(result.meta);

	result.program = glCreateProgram();
	if (result.vertex) glAttachShader(result.program, result.vertex);
	if (result.pixel)  glAttachShader(result.program, result.pixel);
	glLinkProgram (result.program);

	// check for errors?
	int32_t err, length;
	glGetProgramiv(result.program, GL_LINK_STATUS, &err);
	if (err == 0) {
		char *log;

		glGetProgramiv(result.program, GL_INFO_LOG_LENGTH, &length);
		log = (char*)malloc(length);
		glGetProgramInfoLog(result.program, length, &err, log);

		skr_log("Unable to compile shader program:");
		skr_log(log);
		free(log);

		glDeleteProgram(result.program);
		result.program = 0;
	}

	return result;
}

///////////////////////////////////////////

bool skr_shader_is_valid(const skr_shader_t *shader) {
	return shader->meta
		&& shader->program;
}

///////////////////////////////////////////

void skr_shader_destroy(skr_shader_t *shader) {
	glDeleteProgram(shader->program);
	glDeleteShader (shader->vertex);
	glDeleteShader (shader->pixel);
	*shader = {};
}

///////////////////////////////////////////
// skr_pipeline                          //
///////////////////////////////////////////

skr_pipeline_t skr_pipeline_create(skr_shader_t *shader) {
	skr_pipeline_t result = {};
	result.transparency = skr_transparency_none;
	result.cull         = skr_cull_none;
	result.wireframe    = false;
	result.shader       = *shader;

	return result;
}

/////////////////////////////////////////// 

void skr_pipeline_bind(const skr_pipeline_t *pipeline) {
	glUseProgram(pipeline->shader.program);
	
	switch (pipeline->transparency) {
	case skr_transparency_blend: {
		glEnable(GL_BLEND);
	}break;
	case skr_transparency_clip:
	case skr_transparency_none: {
		glDisable(GL_BLEND);
	}break;
	}

	switch (pipeline->cull) {
	case skr_cull_back: {
		glEnable  (GL_CULL_FACE);
		glCullFace(GL_BACK);
	} break;
	case skr_cull_front: {
		glEnable  (GL_CULL_FACE);
		glCullFace(GL_FRONT);
	} break;
	case skr_cull_none: {
		glDisable(GL_CULL_FACE);
	} break;
	}
	
#ifndef __ANDROID__
	if (pipeline->wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
#endif
}

/////////////////////////////////////////// 

void skr_pipeline_set_transparency(skr_pipeline_t *pipeline, skr_transparency_ transparency) {
	pipeline->transparency = transparency;
}

/////////////////////////////////////////// 
void skr_pipeline_set_cull(skr_pipeline_t *pipeline, skr_cull_ cull) {
	pipeline->cull = cull;
}
/////////////////////////////////////////// 
void skr_pipeline_set_wireframe(skr_pipeline_t *pipeline, bool wireframe) {
	pipeline->wireframe = wireframe;
}
/////////////////////////////////////////// 

skr_transparency_ skr_pipeline_get_transparency(const skr_pipeline_t *pipeline) {
	return pipeline->transparency;
}

/////////////////////////////////////////// 

skr_cull_ skr_pipeline_get_cull(const skr_pipeline_t *pipeline) {
	return pipeline->cull;
}

/////////////////////////////////////////// 

bool skr_pipeline_get_wireframe(const skr_pipeline_t *pipeline) {
	return pipeline->wireframe;
}

///////////////////////////////////////////

void skr_pipeline_destroy(skr_pipeline_t *pipeline) {
	*pipeline = {};
}

///////////////////////////////////////////

skr_swapchain_t skr_swapchain_create(skr_tex_fmt_ format, skr_tex_fmt_ depth_format, int32_t width, int32_t height) {
	skr_swapchain_t result = {};
	return result;
}

/////////////////////////////////////////// 

void skr_swapchain_resize(skr_swapchain_t *swapchain, int32_t width, int32_t height) {
	gl_width  = width;
	gl_height = height;
}

/////////////////////////////////////////// 

void skr_swapchain_present(skr_swapchain_t *swapchain) {
#ifdef _WIN32
	SwapBuffers(gl_hdc);
#elif __ANDROID__
	eglSwapBuffers(egl_display, egl_surface);
#endif
}

/////////////////////////////////////////// 

skr_tex_t *skr_swapchain_get_next(skr_swapchain_t *swapchain) {
	return nullptr;
}

/////////////////////////////////////////// 

void skr_swapchain_destroy(skr_swapchain_t *swapchain) {
}

/////////////////////////////////////////// 

skr_tex_t skr_tex_from_native(void *native_tex, skr_tex_type_ type, skr_tex_fmt_ format, int32_t width, int32_t height, int32_t array_count) {
	skr_tex_t result = {};
	result.type    = type;
	result.use     = skr_use_static;
	result.mips    = skr_mip_none;
	result.texture = (uint32_t)(uint64_t)native_tex;
	result.format  = format;
	result.width       = width;
	result.height      = height;
	result.array_count = array_count;

	if (type == skr_tex_type_rendertarget) {
		glGenFramebuffers(1, &result.framebuffer);

		glBindFramebuffer     (GL_FRAMEBUFFER, result.framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result.texture, 0); 
		glBindFramebuffer     (GL_FRAMEBUFFER, gl_current_framebuffer);
	}
	
	return result;
}

/////////////////////////////////////////// 

skr_tex_t skr_tex_create(skr_tex_type_ type, skr_use_ use, skr_tex_fmt_ format, skr_mip_ mip_maps) {
	skr_tex_t result = {};
	result.type   = type;
	result.use    = use;
	result.format = format;
	result.mips   = mip_maps;

	glGenTextures(1, &result.texture);
	skr_tex_settings(&result, type == skr_tex_type_cubemap ? skr_tex_address_clamp : skr_tex_address_repeat, skr_tex_sample_linear, 1);

	if (type == skr_tex_type_rendertarget) {
		glGenFramebuffers(1, &result.framebuffer);
	}
	
	return result;
}

/////////////////////////////////////////// 

bool skr_tex_is_valid(const skr_tex_t *tex) {
	return tex->texture != 0;
}

/////////////////////////////////////////// 

void skr_tex_set_depth(skr_tex_t *tex, skr_tex_t *depth) {
	bool stencil = depth->format == skr_tex_fmt_depthstencil;
	if (tex->type == skr_tex_type_rendertarget) {
		glBindFramebuffer     (GL_FRAMEBUFFER, tex->framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, stencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth->texture, 0); 
		glBindFramebuffer     (GL_FRAMEBUFFER, gl_current_framebuffer);
	} else {
		skr_log("Can't bind a depth texture to a non-rendertarget");
	}
}

/////////////////////////////////////////// 

void skr_tex_settings(skr_tex_t *tex, skr_tex_address_ address, skr_tex_sample_ sample, int32_t anisotropy) {
	uint32_t target = tex->type == skr_tex_type_cubemap 
		? GL_TEXTURE_CUBE_MAP 
		: GL_TEXTURE_2D;
	glBindTexture(target, tex->texture);

	uint32_t mode;
	switch (address) {
	case skr_tex_address_clamp:  mode = GL_CLAMP_TO_EDGE;   break;
	case skr_tex_address_repeat: mode = GL_REPEAT;          break;
	case skr_tex_address_mirror: mode = GL_MIRRORED_REPEAT; break;
	default: mode = GL_REPEAT;
	}

	uint32_t filter, min_filter;
	switch (sample) {
	case skr_tex_sample_linear:     filter = GL_LINEAR;  min_filter = tex->mips == skr_mip_generate ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR; break; // Technically trilinear
	case skr_tex_sample_point:      filter = GL_NEAREST; min_filter = GL_NEAREST;              break;
	case skr_tex_sample_anisotropic:filter = GL_LINEAR;  min_filter = tex->mips == skr_mip_generate ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR; break;
	default: filter = GL_LINEAR;
	}

	glTexParameteri(target, GL_TEXTURE_WRAP_S,     mode  );	
	glTexParameteri(target, GL_TEXTURE_WRAP_T,     mode  );
	if (tex->type == skr_tex_type_cubemap) {
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, mode);
	}
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
#ifdef _WIN32
	glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY, sample == skr_tex_sample_anisotropic ? anisotropy : 1.0f);
#endif
}

/////////////////////////////////////////// 

void skr_tex_set_contents(skr_tex_t *tex, void **data_frames, int32_t data_frame_count, int32_t width, int32_t height) {
	uint32_t target = tex->type == skr_tex_type_cubemap 
		? GL_TEXTURE_CUBE_MAP 
		: GL_TEXTURE_2D;
	tex->width       = width;
	tex->height      = height;
	tex->array_count = data_frame_count;
	glBindTexture(target, tex->texture);

	uint32_t format = (uint32_t)skr_tex_fmt_to_native(tex->format);
	uint32_t type   = skr_tex_fmt_to_gl_type         (tex->format);
	uint32_t layout = skr_tex_fmt_to_gl_layout       (tex->format);
	if (tex->type == skr_tex_type_cubemap) {
		if (data_frame_count != 6) {
			skr_log("sk_gpu: cubemaps need 6 data frames");
			return;
		}
		for (int32_t f = 0; f < 6; f++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+f , 0, format, width, height, 0, layout, type, data_frames[f]);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, layout, type, data_frames == nullptr ? nullptr : data_frames[0]);
	}
	if (tex->mips == skr_mip_generate)
		glGenerateMipmap(target);

	if (tex->type == skr_tex_type_rendertarget) {
		glBindFramebuffer     (GL_FRAMEBUFFER, tex->framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->texture, 0); 
		glBindFramebuffer     (GL_FRAMEBUFFER, gl_current_framebuffer);
	}
}

/////////////////////////////////////////// 

void skr_tex_bind(const skr_tex_t *texture, skr_bind_t bind) {
	uint32_t target = texture == nullptr || texture->type != skr_tex_type_cubemap 
		? GL_TEXTURE_2D
		: GL_TEXTURE_CUBE_MAP;

	// Added this in to fix textures initially? Removed it after I switched to
	// explicit binding locations in GLSL. This may need further attention? I
	// have no idea what's happening here!
	//if (texture)
	//	glUniform1i(bind.slot, bind.slot);

	glActiveTexture(GL_TEXTURE0 + bind.slot);
	glBindTexture  (target, texture == nullptr ? 0 : texture->texture);
}

/////////////////////////////////////////// 

void skr_tex_destroy(skr_tex_t *tex) {
	glDeleteTextures    (1, &tex->texture);
	glDeleteFramebuffers(1, &tex->framebuffer);  
	*tex = {};
}

/////////////////////////////////////////// 

uint32_t skr_buffer_type_to_gl(skr_buffer_type_ type) {
	switch (type) {
	case skr_buffer_type_vertex:   return GL_ARRAY_BUFFER;
	case skr_buffer_type_index:    return GL_ELEMENT_ARRAY_BUFFER;
	case skr_buffer_type_constant: return GL_UNIFORM_BUFFER;
	default: return 0;
	}
}

/////////////////////////////////////////// 

int64_t skr_tex_fmt_to_native(skr_tex_fmt_ format) {
	switch (format) {
	case skr_tex_fmt_rgba32:        return GL_SRGB8_ALPHA8;
	case skr_tex_fmt_rgba32_linear: return GL_RGBA8;
	case skr_tex_fmt_rgba64:        return GL_RGBA16UI;
	case skr_tex_fmt_rgba128:       return GL_RGBA32F;
	case skr_tex_fmt_depth16:       return GL_DEPTH_COMPONENT16;
	case skr_tex_fmt_depth32:       return GL_DEPTH_COMPONENT32F;
	case skr_tex_fmt_depthstencil:  return GL_DEPTH24_STENCIL8;
	case skr_tex_fmt_r8:            return GL_R8;
	case skr_tex_fmt_r16:           return GL_R16UI;
	case skr_tex_fmt_r32:           return GL_R32F;
	default: return 0;
	}
}

/////////////////////////////////////////// 

skr_tex_fmt_ skr_tex_fmt_from_native(int64_t format) {
	switch (format) {
	case GL_SRGB8_ALPHA8:       return skr_tex_fmt_rgba32;
	case GL_RGBA8:              return skr_tex_fmt_rgba32_linear;
	case GL_RGBA16UI:           return skr_tex_fmt_rgba64;
	case GL_RGBA32F:            return skr_tex_fmt_rgba128;
	case GL_DEPTH_COMPONENT16:  return skr_tex_fmt_depth16;
	case GL_DEPTH_COMPONENT32F: return skr_tex_fmt_depth32;
	case GL_DEPTH24_STENCIL8:   return skr_tex_fmt_depthstencil;
	case GL_R8:                 return skr_tex_fmt_r8;
	case GL_R16UI:              return skr_tex_fmt_r16;
	case GL_R32F:               return skr_tex_fmt_r32;
	default: return skr_tex_fmt_none;
	}
}

/////////////////////////////////////////// 

uint32_t skr_tex_fmt_to_gl_layout(skr_tex_fmt_ format) {
	switch (format) {
	case skr_tex_fmt_rgba32:
	case skr_tex_fmt_rgba32_linear:
	case skr_tex_fmt_rgba64:
	case skr_tex_fmt_rgba128:       return GL_RGBA;
	case skr_tex_fmt_depth16:
	case skr_tex_fmt_depth32:       return GL_DEPTH_COMPONENT;
	case skr_tex_fmt_depthstencil:  return GL_DEPTH_STENCIL;
	case skr_tex_fmt_r8:
	case skr_tex_fmt_r16:
	case skr_tex_fmt_r32:           return GL_RED;
	default: return 0;
	}
}

/////////////////////////////////////////// 

uint32_t skr_tex_fmt_to_gl_type(skr_tex_fmt_ format) {
	switch (format) {
	case skr_tex_fmt_rgba32:        return GL_UNSIGNED_BYTE;
	case skr_tex_fmt_rgba32_linear: return GL_UNSIGNED_BYTE;
	case skr_tex_fmt_rgba64:        return GL_UNSIGNED_SHORT;
	case skr_tex_fmt_rgba128:       return GL_FLOAT;
	case skr_tex_fmt_depth16:       return GL_UNSIGNED_SHORT;
	case skr_tex_fmt_depth32:       return GL_FLOAT;
	case skr_tex_fmt_depthstencil:  return GL_DEPTH24_STENCIL8;
	case skr_tex_fmt_r8:            return GL_UNSIGNED_BYTE;
	case skr_tex_fmt_r16:           return GL_UNSIGNED_SHORT;
	case skr_tex_fmt_r32:           return GL_FLOAT;
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

#if __ANDROID__
#include <android/asset_manager.h>
#endif

void (*_skr_log)(const char *text);
void skr_callback_log(void (*callback)(const char *text)) {
	_skr_log = callback;
}
void skr_log(const char *text) {
	if (_skr_log) _skr_log(text);
}

///////////////////////////////////////////

bool (*_skr_read_file)(const char *filename, void **out_data, size_t *out_size);
void skr_callback_file_read(bool (*callback)(const char *filename, void **out_data, size_t *out_size)) {
	_skr_read_file = callback;
}
bool skr_read_file(const char *filename, void **out_data, size_t *out_size) {
	if (_skr_read_file) return _skr_read_file(filename, out_data, out_size);
#if _WIN32
	FILE *fp;
	if (fopen_s(&fp, filename, "rb") != 0 || fp == nullptr) {
		return false;
	}

	fseek(fp, 0L, SEEK_END);
	*out_size = ftell(fp);
	rewind(fp);

	*out_data = malloc(*out_size);
	if (*out_data == nullptr) { *out_size = 0; fclose(fp); return false; }
	fread (*out_data, *out_size, 1, fp);
	fclose(fp);

	return true;
#else
	return false;
#endif
}

///////////////////////////////////////////

uint64_t skr_hash(const char *string) {
	uint64_t hash = 14695981039346656037;
	uint8_t  c;
	while (c = *string++)
		hash = (hash ^ c) * 1099511628211;
	return hash;
}

///////////////////////////////////////////

bool skr_shader_file_load(const char *file, skr_shader_file_t *out_file) {
	void  *data = nullptr;
	size_t size = 0;

	if (!skr_read_file(file, &data, &size))
		return false;

	bool result = skr_shader_file_load_mem(data, size, out_file);
	free(data);

	return result;
}

///////////////////////////////////////////

bool skr_shader_file_verify(void *data, size_t size, uint16_t *out_version, char *out_name, size_t out_name_size) {
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

bool skr_shader_file_load_mem(void *data, size_t size, skr_shader_file_t *out_file) {
	uint16_t file_version = 0;
	if (!skr_shader_file_verify(data, size, &file_version, nullptr, 0) || file_version != 1) {
		return false;
	}
	
	const uint8_t *bytes = (uint8_t*)data;
	size_t at = 10;
	memcpy(&out_file->stage_count, &bytes[at], sizeof(out_file->stage_count)); at += sizeof(out_file->stage_count);
	out_file->stages = (skr_shader_file_stage_t*)malloc(sizeof(skr_shader_file_stage_t) * out_file->stage_count);
	if (out_file->stages == nullptr) { skr_log("Out of memory"); return false; }

	out_file->meta = (skr_shader_meta_t*)malloc(sizeof(skr_shader_meta_t));
	if (out_file->meta == nullptr) { skr_log("Out of memory"); return false; }
	*out_file->meta = {};
	out_file->meta->global_buffer_id = -1;
	skr_shader_meta_reference(out_file->meta);
	memcpy( out_file->meta->name,          &bytes[at], sizeof(out_file->meta->name         )); at += sizeof(out_file->meta->name);
	memcpy(&out_file->meta->buffer_count,  &bytes[at], sizeof(out_file->meta->buffer_count )); at += sizeof(out_file->meta->buffer_count);
	memcpy(&out_file->meta->texture_count, &bytes[at], sizeof(out_file->meta->texture_count)); at += sizeof(out_file->meta->texture_count);
	out_file->meta->buffers  = (skr_shader_meta_buffer_t *)malloc(sizeof(skr_shader_meta_buffer_t ) * out_file->meta->buffer_count );
	out_file->meta->textures = (skr_shader_meta_texture_t*)malloc(sizeof(skr_shader_meta_texture_t) * out_file->meta->texture_count);
	if (out_file->meta->buffers == nullptr || out_file->meta->textures == nullptr) { skr_log("Out of memory"); return false; }
	memset(out_file->meta->buffers,  0, sizeof(skr_shader_meta_buffer_t ) * out_file->meta->buffer_count);
	memset(out_file->meta->textures, 0, sizeof(skr_shader_meta_texture_t) * out_file->meta->texture_count);

	for (uint32_t i = 0; i < out_file->meta->buffer_count; i++) {
		skr_shader_meta_buffer_t *buffer = &out_file->meta->buffers[i];
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
		buffer->vars = (skr_shader_meta_var_t*)malloc(sizeof(skr_shader_meta_var_t) * buffer->var_count);
		if (buffer->vars == nullptr) { skr_log("Out of memory"); return false; }
		memset(buffer->vars, 0, sizeof(skr_shader_meta_var_t) * buffer->var_count);
		buffer->name_hash = skr_hash(buffer->name);

		for (uint32_t t = 0; t < buffer->var_count; t++) {
			skr_shader_meta_var_t *var = &buffer->vars[t];
			memcpy( var->name,       &bytes[at], sizeof(var->name ));      at += sizeof(var->name  );
			memcpy( var->extra,      &bytes[at], sizeof(var->extra));      at += sizeof(var->extra );
			memcpy(&var->offset,     &bytes[at], sizeof(var->offset));     at += sizeof(var->offset);
			memcpy(&var->size,       &bytes[at], sizeof(var->size));       at += sizeof(var->size  );
			memcpy(&var->type,       &bytes[at], sizeof(var->type));       at += sizeof(var->type  );
			memcpy(&var->type_count, &bytes[at], sizeof(var->type_count)); at += sizeof(var->type_count);
			var->name_hash = skr_hash(var->name);
		}

		if (strcmp(buffer->name, "$Globals") == 0)
			out_file->meta->global_buffer_id = i;
	}

	for (uint32_t i = 0; i < out_file->meta->texture_count; i++) {
		skr_shader_meta_texture_t *tex = &out_file->meta->textures[i];
		memcpy( tex->name,  &bytes[at], sizeof(tex->name )); at += sizeof(tex->name );
		memcpy( tex->extra, &bytes[at], sizeof(tex->extra)); at += sizeof(tex->extra);
		memcpy(&tex->bind,  &bytes[at], sizeof(tex->bind )); at += sizeof(tex->bind );
		tex->name_hash = skr_hash(tex->name);
	}

	for (uint32_t i = 0; i < out_file->stage_count; i++) {
		skr_shader_file_stage_t *stage = &out_file->stages[i];
		memcpy( &stage->language, &bytes[at], sizeof(stage->language)); at += sizeof(stage->language);
		memcpy( &stage->stage,    &bytes[at], sizeof(stage->stage));    at += sizeof(stage->stage);
		memcpy( &stage->code_size,&bytes[at], sizeof(stage->code_size));at += sizeof(stage->code_size);

		stage->code = 0;
		if (stage->code_size > 0) {
			stage->code = malloc(stage->code_size);
			if (stage->code == nullptr) { skr_log("Out of memory"); return false; }
			memcpy(stage->code, &bytes[at], stage->code_size); at += stage->code_size;
		}
	}

	return true;
}

///////////////////////////////////////////

skr_shader_stage_t skr_shader_file_create_stage(const skr_shader_file_t *file, skr_stage_ stage) {
	skr_shader_lang_ language;
#if defined(SKR_DIRECT3D11) || defined(SKR_DIRECT3D12)
	language = skr_shader_lang_hlsl;
#elif defined(SKR_OPENGL)
	language = skr_shader_lang_glsl;
#elif defined(SKR_VULKAN)
	language = skr_shader_lang_spirv;
#endif

	for (uint32_t i = 0; i < file->stage_count; i++) {
		if (file->stages[i].language == language && file->stages[i].stage == stage)
			return skr_shader_stage_create(file->stages[i].code, file->stages[i].code_size, stage);
	}
	skr_log("Couldn't find a shader stage in shader file!");
	return {};
}

///////////////////////////////////////////

void skr_shader_file_destroy(skr_shader_file_t *file) {
	for (uint32_t i = 0; i < file->stage_count; i++) {
		free(file->stages[i].code);
	}
	free(file->stages);
	skr_shader_meta_release(file->meta);
	*file = {};
}

///////////////////////////////////////////

void skr_shader_meta_reference(skr_shader_meta_t *meta) {
	meta->references += 1;
}

///////////////////////////////////////////

void skr_shader_meta_release(skr_shader_meta_t *meta) {
	meta->references -= 1;
	if (meta->references == 0) {
		for (uint32_t i = 0; i < meta->buffer_count; i++) {
			free(meta->buffers[i].vars);
			free(meta->buffers[i].defaults);
		}
		free(meta->buffers);
		free(meta->textures);
		memset(meta, 0, sizeof(skr_shader_meta_t));
	}
}

///////////////////////////////////////////
// skr_shader_t                          //
///////////////////////////////////////////

skr_shader_t skr_shader_create_file(const char *sks_filename) {
	skr_shader_file_t file;
	if (!skr_shader_file_load(sks_filename, &file))
		return {};

	skr_shader_stage_t vs     = skr_shader_file_create_stage(&file, skr_stage_vertex);
	skr_shader_stage_t ps     = skr_shader_file_create_stage(&file, skr_stage_pixel);
	skr_shader_t       result = skr_shader_create_manual(file.meta, vs, ps );

	skr_shader_stage_destroy(&vs);
	skr_shader_stage_destroy(&ps);
	skr_shader_file_destroy (&file);

	return result;
}

///////////////////////////////////////////

skr_shader_t skr_shader_create_mem(void *sks_data, size_t sks_data_size) {
	skr_shader_file_t file;
	if (!skr_shader_file_load_mem(sks_data, sks_data_size, &file))
		return {};

	skr_shader_stage_t vs     = skr_shader_file_create_stage(&file, skr_stage_vertex);
	skr_shader_stage_t ps     = skr_shader_file_create_stage(&file, skr_stage_pixel);
	skr_shader_t       result = skr_shader_create_manual( file.meta, vs, ps );

	skr_shader_stage_destroy(&vs);
	skr_shader_stage_destroy(&ps);
	skr_shader_file_destroy (&file);

	return result;
}

///////////////////////////////////////////

skr_bind_t skr_shader_get_tex_bind(const skr_shader_t *shader, const char *name) {
	for (uint32_t i = 0; i < shader->meta->texture_count; i++) {
		if (strcmp(name, shader->meta->textures[i].name) == 0)
			return shader->meta->textures[i].bind;
	}
	return {};
}

///////////////////////////////////////////

skr_bind_t skr_shader_get_buffer_bind(const skr_shader_t *shader, const char *name) {
	for (uint32_t i = 0; i < shader->meta->buffer_count; i++) {
		if (strcmp(name, shader->meta->buffers[i].name) == 0)
			return shader->meta->buffers[i].bind;
	}
	return {};
}

///////////////////////////////////////////

int32_t skr_shader_get_var_count(const skr_shader_t *shader) {
	return shader->meta->global_buffer_id != -1
		? shader->meta->buffers[shader->meta->global_buffer_id].var_count
		: 0;
}

///////////////////////////////////////////

int32_t skr_shader_get_var_index(const skr_shader_t *shader, const char *name) {
	return skr_shader_get_var_index_h(shader, skr_hash(name));
}

///////////////////////////////////////////

int32_t skr_shader_get_var_index_h(const skr_shader_t *shader, uint64_t name_hash) {
	if (shader->meta->global_buffer_id == -1) return -1;

	skr_shader_meta_buffer_t *buffer = &shader->meta->buffers[shader->meta->global_buffer_id];
	for (uint32_t i = 0; i < buffer->var_count; i++) {
		if (buffer->vars[i].name_hash == name_hash) {
			return i;
		}
	}
	return -1;
}

///////////////////////////////////////////

const skr_shader_meta_var_t *skr_shader_get_var_info(const skr_shader_t *shader, int32_t var_id) {
	if (shader->meta->global_buffer_id == -1 || var_id == -1) return nullptr;

	skr_shader_meta_buffer_t *buffer = &shader->meta->buffers[shader->meta->global_buffer_id];
	return &buffer->vars[var_id];
}
#endif // SKR_IMPL

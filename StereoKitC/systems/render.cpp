#include "render.h"
#include "d3d.h"
#include "../math.h"
#include "../stereokit.h"
#include "../asset_types/mesh.h"
#include "../asset_types/texture.h"
#include "../asset_types/shader.h"
#include "../asset_types/material.h"
#include "../asset_types/model.h"
#include "../shaders_builtin/shader_builtin.h"

#include <vector>
#include <algorithm>
using namespace std;

#include <directxmath.h> // Matrix math functions and objects
using namespace DirectX;

namespace sk {

///////////////////////////////////////////

struct render_item_t {
	XMMATRIX    transform;
	color128    color;
	mesh_t      mesh;
	material_t  material;
	uint64_t    sort_id;
};
struct render_transform_buffer_t {
	XMMATRIX world;
	color128 color;
};
struct render_global_buffer_t {
	XMMATRIX view;
	XMMATRIX proj;
	XMMATRIX viewproj;
	vec4     light;
	color128 light_color;
	vec4     camera_pos;
	vec4     camera_dir;
};
struct render_blit_data_t {
	float width;
	float height;
	float pixel_width;
	float pixel_height;
};
struct render_inst_buffer {
	size_t       max;
	shaderargs_t buffer;
};

///////////////////////////////////////////

vector<render_transform_buffer_t> render_instance_list;
render_inst_buffer                render_instance_buffers[] = { { 1 }, { 5 }, { 10 }, { 20 }, { 50 }, { 100 }, { 250 }, { 500 }, { 800 } };

vector<render_item_t>  render_queue;
shaderargs_t           render_shader_globals;
shaderargs_t           render_shader_blit;
transform_t           *render_camera_transform = nullptr;
camera_t              *render_camera = nullptr;
transform_t            render_default_camera_tr;
camera_t               render_default_camera;
render_global_buffer_t render_global_buffer;
mesh_t                 render_blit_quad;
render_stats_t         render_stats = {};
tex2d_t                render_default_tex;

mesh_t     render_sky_mesh = nullptr;
material_t render_sky_mat = nullptr;
tex2d_t    render_sky_cubemap = nullptr;
bool32_t   render_sky_show = false;

material_t render_last_material;
shader_t   render_last_shader;
mesh_t     render_last_mesh;

///////////////////////////////////////////

shaderargs_t *render_fill_inst_buffer(vector<render_transform_buffer_t> &list, size_t &offset, size_t &out_count);

///////////////////////////////////////////

inline uint64_t render_queue_id(material_t material, mesh_t mesh) {
	return ((uint64_t)(material->mode*1000 + material->queue_offset) << 32) | (material->header.index << 16) | mesh->header.index;
}

///////////////////////////////////////////

void render_set_camera(camera_t &cam) {
	render_camera = &cam;
}

///////////////////////////////////////////

void render_set_view(transform_t &cam_transform) {
	render_camera_transform = &cam_transform;
}

///////////////////////////////////////////

void render_set_light(const vec3 &direction, float intensity, const color128 &color) {
	vec3 dir = vec3_normalize(direction);
	render_global_buffer.light       = { dir.x, dir.y, dir.z, intensity };
	render_global_buffer.light_color = color;
}

///////////////////////////////////////////

void render_set_skytex(tex2d_t sky_texture, bool32_t show_sky) {
	if (render_sky_cubemap != nullptr)
		tex2d_release(render_sky_cubemap);

	render_sky_show    = show_sky;
	render_sky_cubemap = sky_texture;
	if (render_sky_cubemap == nullptr)
		return;

	assets_addref(render_sky_cubemap->header);
}

///////////////////////////////////////////

void render_add_mesh_tr(mesh_t mesh, material_t material, transform_t &transform, color128 color) {
	transform_update(transform);
	render_add_mesh(mesh, material, transform._transform, color);
}

///////////////////////////////////////////

void render_add_mesh(mesh_t mesh, material_t material, const matrix &transform, color128 color) {
	render_item_t item;
	item.mesh     = mesh;
	item.material = material;
	item.color    = color;
	item.sort_id  = render_queue_id(material, mesh);
	math_matrix_to_fast(transform, &item.transform);
	render_queue.emplace_back(item);
}

///////////////////////////////////////////

void render_add_model_tr(model_t model, transform_t &transform, color128 color) {
	transform_update(transform);
	render_add_model(model, transform._transform, color);
}

///////////////////////////////////////////

void render_add_model(model_t model, const matrix &transform, color128 color) {
	for (int i = 0; i < model->subset_count; i++) {
		render_item_t item;
		item.mesh     = model->subsets[i].mesh;
		item.material = model->subsets[i].material;
		item.color    = color;
		item.sort_id  = render_queue_id(item.material, item.mesh);
		matrix_mul(model->subsets[i].offset, transform, item.transform);
		render_queue.emplace_back(item);
	}
}

///////////////////////////////////////////

void render_draw_queue(const matrix &view, const matrix &projection) {
	size_t queue_size = render_queue.size();
	if (queue_size == 0) return;

	// Sort the draw list
	sort(render_queue.begin(), render_queue.end(), [](const render_item_t &a, const render_item_t &b) -> bool { 
		return a.sort_id < b.sort_id;
	});

	XMMATRIX view_f, projection_f;
	math_matrix_to_fast(view,       &view_f);
	math_matrix_to_fast(projection, &projection_f);

	// Copy camera information into the global buffer
	XMMATRIX view_inv = XMMatrixInverse(nullptr, view_f);

	XMVECTOR cam_pos = XMVector3Transform(DirectX::g_XMIdentityR3, view_inv);
	XMVECTOR cam_dir = XMVector3TransformNormal(DirectX::g_XMNegIdentityR2, view_inv);
	XMStoreFloat3((XMFLOAT3*)&render_global_buffer.camera_pos, cam_pos);
	XMStoreFloat3((XMFLOAT3*)&render_global_buffer.camera_dir, cam_dir);

	render_global_buffer.view = XMMatrixTranspose(view_f);
	render_global_buffer.proj = XMMatrixTranspose(projection_f);
	render_global_buffer.viewproj = XMMatrixTranspose(view_f * projection_f);

	shaderargs_set_data  (render_shader_globals, &render_global_buffer);
	shaderargs_set_active(render_shader_globals);
	d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (render_sky_cubemap != nullptr) {
		d3d_context->PSSetSamplers       (11, 1, &render_sky_cubemap->sampler);
		d3d_context->PSSetShaderResources(11, 1, &render_sky_cubemap->resource);
	}

	render_item_t *item          = &render_queue[0];
	material_t     last_material = item->material;
	mesh_t         last_mesh     = item->mesh;
	
	for (size_t i = 0; i < queue_size; i++) {
		render_instance_list.emplace_back(render_transform_buffer_t { XMMatrixTranspose(item->transform), item->color } );

		render_item_t *next = i+1>=queue_size?nullptr:&render_queue[i+1];
		if (next == nullptr || last_material != next->material || last_mesh != next->mesh) {
			render_set_material(item->material);
			render_set_mesh    (item->mesh);

			size_t offsets = 0, count = 0;
			do {
				shaderargs_t *instances = render_fill_inst_buffer(render_instance_list, offsets, count);
				shaderargs_set_active(*instances, false);
				render_draw_item((int)count);
			} while (offsets != 0);
			render_instance_list.clear();
			
			if (next != nullptr) {
				last_material = next->material;
				last_mesh     = next->mesh;
			}
		}
		item = next;
	}
}

///////////////////////////////////////////

void render_draw() {
	if (render_camera != nullptr && render_camera_transform != nullptr)
		render_draw_from(*render_camera, *render_camera_transform);
}

///////////////////////////////////////////

void render_draw_from(camera_t &cam, transform_t &cam_transform) {
	matrix view, proj;
	camera_view(cam_transform, view);
	camera_proj(cam, proj);
	render_draw_queue(view, proj);
}

///////////////////////////////////////////

void render_draw_matrix(const matrix &cam_matrix, transform_t &cam_transform) {
	matrix view;
	camera_view(cam_transform, view);

	render_draw_queue(view, cam_matrix);
}

///////////////////////////////////////////

void render_clear() {
	//log_infof("draws: %d, material: %d, shader: %d, texture %d, mesh %d", render_stats.draw_calls, render_stats.swaps_material, render_stats.swaps_shader, render_stats.swaps_texture, render_stats.swaps_mesh);
	render_queue.clear();
	render_stats = {};

	render_last_material = nullptr;
	render_last_shader = nullptr;
	render_last_mesh = nullptr;
}

///////////////////////////////////////////

bool render_initialize() {
	shaderargs_create(render_shader_globals, sizeof(render_global_buffer_t), 0);
	shaderargs_create(render_shader_blit,    sizeof(render_blit_data_t),     1);

	for (size_t i = 0; i < _countof(render_instance_buffers); i++) {
		shaderargs_create(render_instance_buffers[i].buffer, sizeof(render_transform_buffer_t) * render_instance_buffers[i].max, 1);
	}

	// Setup a default camera
	camera_initialize(render_default_camera, 90, 0.01f, 50);
	transform_set    (render_default_camera_tr, vec3_one, vec3_one, quat_identity);
	transform_lookat (render_default_camera_tr, vec3_zero);
	render_set_camera(render_default_camera);
	render_set_view  (render_default_camera_tr);

	// Set default lighting
	vec3 dir = { -1,-2,-1 };
	dir = vec3_normalize(dir);
	render_set_light(dir, 3.14159f, { 1,1,1,1 });

	// Set up resources for doing blit operations
	render_blit_quad = mesh_find("default/quad");
	assets_addref(render_blit_quad->header);

	// Create a default skybox
	shader_t sky_shader = shader_create(sk_shader_builtin_skybox);
	shader_set_id(sky_shader, "render/skybox_shader");
	render_sky_mesh = mesh_gen_sphere(1, 3);
	mesh_set_id(render_sky_mesh, "render/skybox_mesh");
	render_sky_mat  = material_create(sky_shader);
	material_set_id          (render_sky_mat, "render/skybox_material");
	material_set_queue_offset(render_sky_mat, 100);
	material_set_cull        (render_sky_mat, material_cull_none);
	shader_release(sky_shader);

	render_default_tex = tex2d_find("default/tex2d");

	return true;
}

///////////////////////////////////////////

void render_update() {
	if (render_sky_show && sk_get_info().display_type == display_opaque) {
		render_add_mesh(render_sky_mesh, render_sky_mat, matrix_identity);
	}
}

///////////////////////////////////////////

void render_shutdown() {
	material_release(render_sky_mat);
	mesh_release    (render_sky_mesh);
	tex2d_release   (render_sky_cubemap);
	tex2d_release   (render_default_tex);

	for (size_t i = 0; i < _countof(render_instance_buffers); i++) {
		shaderargs_destroy(render_instance_buffers[i].buffer);
	}

	shaderargs_destroy(render_shader_blit);
	shaderargs_destroy(render_shader_globals);
	mesh_release(render_blit_quad);
}

///////////////////////////////////////////

void render_get_cam(camera_t **cam, transform_t **cam_transform) {
	*cam = render_camera;
	*cam_transform = render_camera_transform;
}

///////////////////////////////////////////

void render_blit(tex2d_t to, material_t material) {
	// Set up where on the render target we want to draw, the view has a 
	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.f, 0.f, (float)to->width, (float)to->height);
	d3d_context->RSSetViewports(1, &viewport);

	// Wipe our swapchain color and depth target clean, and then set them up for rendering!
	tex2d_rtarget_clear(to, { 0,0,0,0 });
	tex2d_rtarget_set_active(to);

	// Setup shader args for the blit operation
	render_blit_data_t data = {};
	data.width  = (float)to->width;
	data.height = (float)to->height;
	data.pixel_width  = 1.0f / to->width;
	data.pixel_height = 1.0f / to->height;

	// Setup render states for blitting
	shaderargs_set_data  (render_shader_blit, &data);
	shaderargs_set_active(render_shader_blit);
	render_set_material(material);
	render_set_mesh    (render_blit_quad);
	
	// And draw to it!
	render_draw_item(1);

	tex2d_rtarget_set_active(nullptr);

	render_last_material = nullptr;
	render_last_mesh = nullptr;
	render_last_shader = nullptr;
}

///////////////////////////////////////////

void render_set_material(material_t material) {
	if (material == render_last_material)
		return;
	render_last_material = material;
	render_stats.swaps_material++;

	render_set_shader    (material->shader);
	shaderargs_set_data  (material->shader->args, material->args.buffer);
	shaderargs_set_active(material->shader->args);

	// Fill an array of texture data so we can set them all at the same time
	ID3D11SamplerState       *samplers [10];
	ID3D11ShaderResourceView *resources[10];
	assert(material->shader->tex_slots.tex_count < 10);

	// Use a texture from the material, or use the default! But don't
	// leave any empty, or we can get overflow from previous renders.
	for (int i = 0; i < material->shader->tex_slots.tex_count; i++) {
		tex2d_t tex = material->args.textures[i];
		if (tex == nullptr)
			tex = material->shader->tex_slots.tex[i].default_tex;

		samplers [i] = tex->sampler;
		resources[i] = tex->resource;
	}
	if (material->shader->tex_slots.tex_count != 0) {
		d3d_context->PSSetSamplers       (0, material->shader->tex_slots.tex_count, samplers);
		d3d_context->PSSetShaderResources(0, material->shader->tex_slots.tex_count, resources);
	}

	if (material->mode == material_alpha_none) {
		d3d_context->OMSetBlendState(0,0, 0xFFFFFFFF);
	} else {
		d3d_context->OMSetBlendState(material->blend_state, nullptr, 0xFFFFFFFF);
	}
	if (material->rasterizer_state != nullptr) {
		d3d_context->RSSetState(material->rasterizer_state);
	} else {
		d3d_context->RSSetState(d3d_rasterstate);
	}
}

///////////////////////////////////////////

void render_set_shader(shader_t shader) {
	if (shader == render_last_shader)
		return;
	render_last_shader = shader;
	render_stats.swaps_shader++;

	d3d_context->VSSetShader(shader->vshader, nullptr, 0);
	d3d_context->PSSetShader(shader->pshader, nullptr, 0);
	d3d_context->IASetInputLayout(shader->vert_layout);
}

///////////////////////////////////////////

void render_set_mesh(mesh_t mesh) {
	if (mesh == render_last_mesh)
		return;
	render_last_mesh = mesh;
	render_stats.swaps_mesh++;

	UINT strides[] = { sizeof(vert_t) };
	UINT offsets[] = { 0 };
	d3d_context->IASetVertexBuffers(0, 1, &mesh->vert_buffer, strides, offsets);
#ifdef SK_32BIT_INDICES
	d3d_context->IASetIndexBuffer  (mesh->ind_buffer, DXGI_FORMAT_R32_UINT, 0);
#else
	d3d_context->IASetIndexBuffer  (mesh->ind_buffer, DXGI_FORMAT_R16_UINT, 0);
#endif
}

///////////////////////////////////////////

void render_draw_item(int count) {
	render_stats.draw_calls++;

	d3d_context->DrawIndexedInstanced(render_last_mesh->ind_draw, count, 0, 0, 0);
}

///////////////////////////////////////////

shaderargs_t *render_fill_inst_buffer(vector<render_transform_buffer_t> &list, size_t &offset, size_t &out_count) {
	// Find a buffer that can contain this list! Or the biggest one
	size_t size  = list.size() - offset;
	size_t index = 0;
	for (size_t i = 0; i < _countof(render_instance_buffers); i++) {
		index = i;
		if (render_instance_buffers[i].max >= size)
			break;
	}
	size_t start = offset;

	// Check if it fits, if it doesn't, then set up data so we only fill what we have!
	if (size > render_instance_buffers[index].max) {
		offset   += render_instance_buffers[index].max;
		out_count = render_instance_buffers[index].max;
	} else {
		// this means we've gotten through the whole list :)
		offset    = 0;
		out_count = size;
	}

	// Copy data into the buffer, and return it!
	shaderargs_set_data(render_instance_buffers[index].buffer, &list[start], sizeof(render_transform_buffer_t) * out_count);
	return &render_instance_buffers[index].buffer;
}

///////////////////////////////////////////

void render_get_device(void **device, void **context) {
	*device  = d3d_device;
	*context = d3d_context;
}

} // namespace sk
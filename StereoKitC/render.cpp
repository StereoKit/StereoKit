#include "stereokit.h"
#include "render.h"

#include "d3d.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "material.h"
#include "model.h"

#include <vector>
using namespace std;

#include <directxmath.h> // Matrix math functions and objects
using namespace DirectX;

struct render_item_t {
	XMMATRIX    transform;
	mesh_t      mesh;
	material_t  material;
};
struct render_transform_buffer_t {
	XMMATRIX world;
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

vector<render_item_t>  render_queue;
shaderargs_t           render_shader_transforms;
shaderargs_t           render_shader_globals;
shaderargs_t           render_shader_blit;
transform_t           *render_camera_transform = nullptr;
camera_t              *render_camera = nullptr;
transform_t            render_default_camera_tr;
camera_t               render_default_camera;
render_global_buffer_t render_global_buffer;
mesh_t                 render_blit_quad;

void render_set_camera(camera_t &cam, transform_t &cam_transform) {
	render_camera           = &cam;
	render_camera_transform = &cam_transform;
}
void render_set_light(const vec3 &direction, float intensity, const color128 &color) {
	vec3 dir = vec3_normalize(direction);
	render_global_buffer.light       = { dir.x, dir.y, dir.z, intensity };
	render_global_buffer.light_color = color;
}

void render_add_mesh(mesh_t mesh, material_t material, transform_t &transform) {
	render_item_t item;
	item.mesh     = mesh;
	item.material = material;
	transform_matrix(transform, item.transform);
	render_queue.emplace_back(item);
}

void render_add_model(model_t model, transform_t &transform) {
	XMMATRIX world;
	transform_matrix(transform, world);
	for (int i = 0; i < model->subset_count; i++) {
		render_item_t item;
		item.mesh     = model->subsets[i].mesh;
		item.material = model->subsets[i].material;
		transform_matrix(model->subsets[i].offset, item.transform);
		item.transform = item.transform * world;
		render_queue.emplace_back(item);
	}
}

void render_draw_queue(XMMATRIX view, XMMATRIX projection) {
	render_transform_buffer_t transform_buffer;

	// Copy camera information into the global buffer
	XMMATRIX view_inv = XMMatrixInverse(nullptr, view);

	XMVECTOR cam_pos = XMVector3Transform(DirectX::g_XMIdentityR3, view_inv);
	XMVECTOR cam_dir = XMVector3TransformNormal(DirectX::g_XMNegIdentityR2, view_inv);
	XMStoreFloat3((XMFLOAT3*)&render_global_buffer.camera_pos, cam_pos);
	XMStoreFloat3((XMFLOAT3*)&render_global_buffer.camera_dir, cam_dir);

	render_global_buffer.view = XMMatrixTranspose(view);
	render_global_buffer.proj = XMMatrixTranspose(projection);
	render_global_buffer.viewproj = XMMatrixTranspose(view * projection);

	shaderargs_set_data  (render_shader_globals, &render_global_buffer);
	shaderargs_set_active(render_shader_globals);
	shaderargs_set_active(render_shader_transforms);
	for (size_t i = 0; i < render_queue.size(); i++) {
		render_item_t &item = render_queue[i];

		transform_buffer.world = XMMatrixTranspose(item.transform);

		material_set_active(item.material);
		shaderargs_set_data(render_shader_transforms, &transform_buffer);

		mesh_set_active(item.mesh);
		mesh_draw      (item.mesh);
	}
}

void render_draw() {
	if (render_camera != nullptr && render_camera_transform != nullptr)
		render_draw_from(*render_camera, *render_camera_transform);
}
void render_draw_from(camera_t &cam, transform_t &cam_transform) {
	XMMATRIX view, proj;
	camera_view(cam_transform, view);
	camera_proj(cam, proj);
	render_draw_queue(view, proj);
}
void render_draw_matrix(const float *cam_matrix, transform_t &cam_transform) {
	XMMATRIX view, proj;
	camera_view(cam_transform, view);
	proj = XMLoadFloat4x4((XMFLOAT4X4 *)cam_matrix);

	render_draw_queue(view, proj);
}

void render_clear() {
	render_queue.clear();
}

void render_initialize() {
	shaderargs_create(render_shader_transforms, sizeof(render_transform_buffer_t), 1);
	shaderargs_create(render_shader_globals,    sizeof(render_global_buffer_t),    0);
	shaderargs_create(render_shader_blit,       sizeof(render_blit_data_t),        1);

	camera_initialize(render_default_camera, 90, 0.1f, 50);
	transform_set    (render_default_camera_tr, { 1,1,1 }, { 1,1,1 }, { 0,0,0,1 });
	transform_lookat (render_default_camera_tr, { 0,0,0 });
	render_set_camera(render_default_camera, render_default_camera_tr);

	vec3 dir = { -1,-2,-1 };
	dir = vec3_normalize(dir);
	render_set_light(dir, 1, { 1,1,1,1 });

	render_blit_quad = mesh_create("render/blitquad");
	vert_t verts[4] = {
		vec3{-1,-1,0}, vec3{0,0,-1}, vec2{0,0}, color32{255,255,255,255},
		vec3{ 1,-1,0}, vec3{0,0,-1}, vec2{1,0}, color32{255,255,255,255},
		vec3{ 1, 1,0}, vec3{0,0,-1}, vec2{1,1}, color32{255,255,255,255},
		vec3{-1, 1,0}, vec3{0,0,-1}, vec2{0,1}, color32{255,255,255,255},
	};
	uint16_t inds[6] = { 0,1,2, 0,2,3 };
	mesh_set_verts(render_blit_quad, verts, 4);
	mesh_set_inds(render_blit_quad,  inds,  6);
}
void render_shutdown() {
	shaderargs_destroy(render_shader_blit);
	shaderargs_destroy(render_shader_globals);
	shaderargs_destroy(render_shader_transforms);
	mesh_release(render_blit_quad);
}

void render_get_cam(camera_t **cam, transform_t **cam_transform) {
	*cam = render_camera;
	*cam_transform = render_camera_transform;
}

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
	material_set_active(material);
	mesh_set_active(render_blit_quad);
	
	// And draw to it!
	mesh_draw(render_blit_quad);

	tex2d_rtarget_set_active(nullptr);
}
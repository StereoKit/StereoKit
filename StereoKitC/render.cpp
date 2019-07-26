#include "stereokit.h"
#include "render.h"

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
	XMMATRIX viewproj;
};

vector<render_item_t> render_queue;
shaderargs_t          render_shader_transforms;
transform_t          *render_camera_transform = nullptr;
camera_t             *render_camera = nullptr;
transform_t           render_default_camera_tr;
camera_t              render_default_camera;

void render_set_camera(camera_t &cam, transform_t &cam_transform) {
	render_camera           = &cam;
	render_camera_transform = &cam_transform;
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

void render_draw_queue(render_transform_buffer_t &transform_buffer) {
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
	render_transform_buffer_t transform_buffer;
	camera_viewproj (cam, cam_transform, transform_buffer.viewproj);
	transform_buffer.viewproj = XMMatrixTranspose(transform_buffer.viewproj);
	
	render_draw_queue(transform_buffer);
}
void render_draw_matrix(const float *cam_matrix, transform_t &cam_transform) {
	render_transform_buffer_t transform_buffer;
	XMMATRIX mat_projection = XMLoadFloat4x4((XMFLOAT4X4 *)cam_matrix);
	XMMATRIX mat_view;
	transform_matrix(cam_transform, mat_view);
	mat_view = XMMatrixInverse(nullptr, mat_view);

	transform_buffer.viewproj = XMMatrixTranspose(mat_view * mat_projection);

	render_draw_queue(transform_buffer);
}

void render_clear() {
	render_queue.clear();
}

void render_initialize() {
	shaderargs_create(render_shader_transforms, sizeof(render_transform_buffer_t), 0);

	camera_initialize(render_default_camera, 90, 0.1f, 50);
	transform_set    (render_default_camera_tr, { 1,1,1 }, { 1,1,1 }, { 0,0,0,1 });
	transform_lookat (render_default_camera_tr, { 0,0,0 });
	render_set_camera(render_default_camera, render_default_camera_tr);
}
void render_shutdown() {
	shaderargs_destroy(render_shader_transforms);
}

void render_get_cam(camera_t **cam, transform_t **cam_transform) {
	*cam = render_camera;
	*cam_transform = render_camera_transform;
}
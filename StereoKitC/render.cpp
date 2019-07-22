#include "stereokit.h"
#include "render.h"

#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "material.h"

#include <vector>
using namespace std;

#include <directxmath.h> // Matrix math functions and objects
using namespace DirectX;

struct render_item_t {
	transform_t transform;
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

void render_set_camera(camera_t &cam, transform_t &cam_transform) {
	render_camera           = &cam;
	render_camera_transform = &cam_transform;
}

void render_add(mesh_t mesh, material_t material, transform_t &transform) {
	render_queue.push_back({ transform, mesh, material });
}

void render_draw_queue(render_transform_buffer_t &transform_buffer) {
	shaderargs_set_active(render_shader_transforms);
	for (size_t i = 0; i < render_queue.size(); i++) {
		render_item_t &item = render_queue[i];

		transform_matrix(item.transform, transform_buffer.world);
		transform_buffer.world = XMMatrixTranspose(transform_buffer.world);

		shader_set_active(item.material->shader);
		shaderargs_set_data(render_shader_transforms, &transform_buffer);
		tex2d_set_active(item.material->texture, 0);

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
}
void render_shutdown() {
	shaderargs_destroy(render_shader_transforms);
}

void render_get_cam(camera_t **cam, transform_t **cam_transform) {
	*cam = render_camera;
	*cam_transform = render_camera_transform;
}
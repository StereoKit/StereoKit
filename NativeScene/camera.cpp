#include "camera.h"

#include "d3d.h"
#include "stereo_kit.h"

#include <directxmath.h> // Matrix math functions and objects
using namespace DirectX;

void camera_viewproj(camera_t &cam, transform_t &cam_transform, XMMATRIX &result) {
	XMMATRIX mat_projection = XMMatrixPerspectiveFovLH(cam.fov * deg2rad, (float)d3d_screen_width/d3d_screen_height, cam.clip_near, cam.clip_far);
	XMMATRIX mat_view;
	transform_matrix(cam_transform, mat_view);
	mat_view = XMMatrixInverse(nullptr, mat_view);

	result = XMMatrixTranspose(mat_view * mat_projection);
}
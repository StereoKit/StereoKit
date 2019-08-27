#include "stereokit.h"
#include "systems/d3d.h"

#include <directxmath.h> // Matrix math functions and objects
using namespace DirectX;

///////////////////////////////////////////

void camera_initialize(camera_t &cam, float fov, float clip_near, float clip_far) {
	cam           = {};
	cam.fov       = fov;
	cam.clip_near = clip_near;
	cam.clip_far  = clip_far;
}

///////////////////////////////////////////

void camera_view(transform_t &cam_transform, XMMATRIX &result) {
	transform_matrix(cam_transform, result);
	result = XMMatrixInverse(nullptr, result);
}

///////////////////////////////////////////

void camera_proj(camera_t &cam, XMMATRIX &result) {
	result = XMMatrixPerspectiveFovRH(cam.fov * deg2rad, (float)d3d_screen_width/d3d_screen_height, cam.clip_near, cam.clip_far);
}
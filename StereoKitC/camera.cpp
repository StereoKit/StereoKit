#include "stereokit.h"
#include "systems/d3d.h"
#include "math.h"

#include <directxmath.h> // Matrix math functions and objects
using namespace DirectX;

namespace sk {

///////////////////////////////////////////

void camera_initialize(camera_t &cam, float fov, float clip_near, float clip_far) {
	cam           = {};
	cam.fov       = fov;
	cam.clip_near = clip_near;
	cam.clip_far  = clip_far;
}

///////////////////////////////////////////

void camera_view(transform_t &cam_transform, matrix &result) {
	transform_matrix_out(cam_transform, result);
	matrix_inverse(result, result);
}

///////////////////////////////////////////

void camera_proj(camera_t &cam, matrix &result) {
	math_fast_to_matrix( XMMatrixPerspectiveFovRH(cam.fov * deg2rad, (float)d3d_screen_width/d3d_screen_height, cam.clip_near, cam.clip_far), &result);
}

} // namespace sk
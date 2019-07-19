#pragma once

#include "sk_math.h"
#include "transform.h"

struct camera_t {
	float fov;
	float clip_near;
	float clip_far;
};

void camera_initialize(camera_t &cam, float fov, float clip_near, float clip_far);
void camera_viewproj(camera_t &cam, transform_t &cam_transform, DirectX::XMMATRIX &result);
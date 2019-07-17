#pragma once

#include "math.h"

#include <directxmath.h>

struct transform_t {
	vec3 _position;
	vec3 _scale;
	quat _rotation;

	bool _dirty;
	DirectX::XMMATRIX _transform;
};

void transform_initialize(transform_t &transform);
void transform_set       (transform_t &transform, const vec3 &position, const vec3 &scale, const quat &rotation );
void transform_set_pos   (transform_t &transform, const vec3 &position);
void transform_set_scale (transform_t &transform, const vec3 &scale);
void transform_set_rot   (transform_t &transform, const quat &rotation);

void transform_matrix(transform_t &transform, DirectX::XMFLOAT4X4 &result);
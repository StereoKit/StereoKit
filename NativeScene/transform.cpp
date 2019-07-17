#include "transform.h"

#include <directxmath.h> // Matrix math functions and objects
using namespace DirectX;

void transform_initialize(transform_t &transform) {
	transform._position = { 0,0,0 };
	transform._rotation = { 0,0,0,1 };
	transform._scale    = { 1,1,1 };
	transform._dirty   = true;
}
void transform_set(transform_t &transform, const vec3 &position, const vec3 &scale, const quat &rotation) {
	transform._position = position;
	transform._scale    = scale;
	transform._rotation = rotation;
	transform._dirty   = true;
}
void transform_set_pos(transform_t &transform, const vec3 &position) {
	transform._position = position;
	transform._dirty   = true;
}
void transform_set_scale(transform_t &transform, const vec3 &scale) {
	transform._scale    = scale;
	transform._dirty   = true;
}
void transform_set_rot  (transform_t &transform, const quat &rotation) {
	transform._rotation = rotation;
	transform._dirty   = true;
}
void transform_matrix(transform_t &transform, XMFLOAT4X4 &result) {
	if (transform._dirty) {
		transform._dirty = false;
		transform._transform = XMMatrixAffineTransformation(
			XMLoadFloat3((XMFLOAT3*)&transform._scale), DirectX::g_XMZero,
			XMLoadFloat4((XMFLOAT4*)&transform._rotation),
			XMLoadFloat3((XMFLOAT3*)&transform._position));
	}
	XMStoreFloat4x4(&result, transform._transform);
}
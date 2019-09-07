#include "stereokit.h"

using namespace DirectX;

///////////////////////////////////////////

inline XMVECTOR to_fast3  (const vec3 &vec)   { return XMLoadFloat3((XMFLOAT3 *)& vec); }
inline vec3     from_fast3(const XMVECTOR &a) { vec3 result; XMStoreFloat3((XMFLOAT3 *)& result, a); return result; }
inline quat     from_fastq(const XMVECTOR &a) { quat result; XMStoreFloat4((XMFLOAT4 *)& result, a); return result; }

///////////////////////////////////////////

vec3 operator*(const quat &a, const vec3 &b) {
	XMVECTOR rotation = XMVector3Rotate(XMLoadFloat3((XMFLOAT3*)&b), XMLoadFloat4((XMFLOAT4*)&a));
	vec3 result;
	XMStoreFloat3((XMFLOAT3 *)&result, rotation);
	return result;
}

///////////////////////////////////////////

quat operator*(const quat &a, const quat &b) {
	XMVECTOR rotation = XMQuaternionMultiply(XMLoadFloat4((XMFLOAT4*)&a),XMLoadFloat4((XMFLOAT4*)&b));
	quat result;
	XMStoreFloat4((XMFLOAT4 *)&result, rotation);
	return result;
}

///////////////////////////////////////////

quat quat_difference(const quat &a, const quat &b) {
	XMVECTOR inv = XMQuaternionInverse(XMLoadFloat4((XMFLOAT4 *)& a));
	XMVECTOR difference = XMQuaternionMultiply(XMLoadFloat4((XMFLOAT4 *)& b), inv);
	quat result;
	XMStoreFloat4((XMFLOAT4 *)&result, difference);
	return result;
}

///////////////////////////////////////////

quat quat_lookat(const vec3 &from, const vec3 &at) {
	XMMATRIX mat = XMMatrixLookAtRH(to_fast3(from), to_fast3(at), XMVectorSet(0, 1, 0, 0));
	return from_fastq(XMQuaternionRotationMatrix(XMMatrixTranspose(mat)));
}

///////////////////////////////////////////

quat quat_lerp(const quat &a, const quat &b, float t) {
	XMVECTOR blend = XMQuaternionSlerp(XMLoadFloat4((XMFLOAT4 *)& a), XMLoadFloat4((XMFLOAT4 *)& b), t);
	quat result;
	XMStoreFloat4((XMFLOAT4 *)&result, blend);
	return result;
}

///////////////////////////////////////////

quat quat_mul(const quat &a, const quat &b) {
	XMVECTOR mul = XMQuaternionMultiply(XMLoadFloat4((XMFLOAT4 *)& a), XMLoadFloat4((XMFLOAT4 *)& b));
	quat result;
	XMStoreFloat4((XMFLOAT4 *)&result, mul);
	return result;
}

///////////////////////////////////////////

void pose_matrix(const pose_t &pose, matrix &out_result) {
	XMMATRIX mat = XMMatrixAffineTransformation(
		DirectX::g_XMOne, DirectX::g_XMZero,
		XMLoadFloat4((XMFLOAT4*)&pose.orientation),
		XMLoadFloat3((XMFLOAT3*)&pose.position));
}

///////////////////////////////////////////

bool32_t ray_intersect_plane(ray_t ray, vec3 plane_pt, vec3 plane_normal, float &out_t) {
	float denom = vec3_dot(plane_normal, ray.dir); 
	if (denom > 1e-6) { 
		vec3 difference = plane_pt - ray.pos; 
		out_t = vec3_dot(difference, plane_normal) / denom; 
		return (out_t >  1e-6); 
	}
	return false; 
}
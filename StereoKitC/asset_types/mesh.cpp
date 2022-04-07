#include "../stereokit.h"
#include "../sk_memory.h"
#include "mesh.h"
#include "assets.h"

#include <stdio.h>
#include <string.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>

using namespace DirectX;

namespace sk {

///////////////////////////////////////////

void mesh_set_keep_data(mesh_t mesh, bool32_t keep_data) {
	if (mesh_has_skin(mesh) && !keep_data) {
		log_warn("Skinned meshes must keep their data, ignoring mesh_set_keep_data call.");
		return;
	}

	mesh->discard_data = !keep_data;
	if (mesh->discard_data) {
		free(mesh->verts); mesh->verts = nullptr;
		free(mesh->inds ); mesh->inds  = nullptr;
	}
}

///////////////////////////////////////////

bool32_t mesh_get_keep_data(mesh_t mesh) {
	return !mesh->discard_data;
}

///////////////////////////////////////////

void _mesh_set_verts(mesh_t mesh, const vert_t *vertices, uint32_t vertex_count, bool32_t calculate_bounds, bool update_original) {
	// Keep track of vertex data for use on CPU side
	if (!mesh->discard_data && update_original) {
		if (mesh->vert_capacity < vertex_count)
			mesh->verts = sk_realloc_t(vert_t, mesh->verts, vertex_count);
		memcpy(mesh->verts, vertices, sizeof(vert_t) * vertex_count);
	}

	if (!skg_buffer_is_valid( &mesh->vert_buffer )) {
		// Create a static vertex buffer the first time we call this function!
		mesh->vert_dynamic  = false;
		mesh->vert_capacity = vertex_count;
		mesh->vert_buffer   = skg_buffer_create(vertices, vertex_count, sizeof(vert_t), skg_buffer_type_vertex, skg_use_static);
		if (!skg_buffer_is_valid(&mesh->vert_buffer))
			log_err("mesh_set_verts: Failed to create vertex buffer");
		skg_mesh_set_verts(&mesh->gpu_mesh, &mesh->vert_buffer);
	} else if (mesh->vert_dynamic == false || vertex_count > mesh->vert_capacity) {
		// If they call this a second time, or they need more verts than will
		// fit in this buffer, lets make a new dynamic buffer!
		skg_buffer_destroy(&mesh->vert_buffer);
		mesh->vert_dynamic  = true;
		mesh->vert_capacity = vertex_count;
		mesh->vert_buffer   = skg_buffer_create(vertices, vertex_count, sizeof(vert_t), skg_buffer_type_vertex, skg_use_dynamic);
		if (!skg_buffer_is_valid(&mesh->vert_buffer))
			log_err("mesh_set_verts: Failed to create dynamic vertex buffer");
		skg_mesh_set_verts(&mesh->gpu_mesh, &mesh->vert_buffer);
	} else {
		// And if they call this a third time, or their verts fit in the same
		// buffer, just copy things over!
		skg_buffer_set_contents(&mesh->vert_buffer, vertices, sizeof(vert_t)*vertex_count);
	}

	mesh->vert_count = vertex_count;

	// Calculate the bounds for this mesh by searching it for min and max values!
	if (calculate_bounds && vertex_count > 0) {
		vec3 min = vertices[0].pos;
		vec3 max = vertices[0].pos;
		for (uint32_t i = 1; i < vertex_count; i++) {
			min.x = fminf(vertices[i].pos.x, min.x);
			min.y = fminf(vertices[i].pos.y, min.y);
			min.z = fminf(vertices[i].pos.z, min.z);

			max.x = fmaxf(vertices[i].pos.x, max.x);
			max.y = fmaxf(vertices[i].pos.y, max.y);
			max.z = fmaxf(vertices[i].pos.z, max.z);
		}
		mesh->bounds = bounds_t{ min / 2 + max / 2, max - min };
	}
}
///////////////////////////////////////////

void mesh_set_verts(mesh_t mesh, const vert_t *vertices, int32_t vertex_count, bool32_t calculate_bounds) {
	struct vert_upload_job_t {
		mesh_t        mesh;
		const vert_t *vertices;
		int32_t       vertex_count;
		bool32_t      calculate_bounds;
	};
	vert_upload_job_t job_data = {mesh, vertices, vertex_count, calculate_bounds};

	assets_execute_gpu([](void *data) {
		vert_upload_job_t *job_data = (vert_upload_job_t *)data;
		_mesh_set_verts(job_data->mesh, job_data->vertices, job_data->vertex_count, job_data->calculate_bounds, true);
		
		return (bool32_t)true;
	}, &job_data);
}

///////////////////////////////////////////

void mesh_get_verts(mesh_t mesh, vert_t *&out_vertices, int32_t &out_vertex_count) {
	out_vertices     = mesh->verts;
	out_vertex_count = mesh->verts == nullptr ? 0 : mesh->vert_count;
}

///////////////////////////////////////////

int32_t mesh_get_vert_count(mesh_t mesh) {
	return mesh->vert_count;
}

///////////////////////////////////////////

void _mesh_set_inds (mesh_t mesh, const vind_t *indices, uint32_t index_count) {
	if (index_count % 3 != 0) {
		log_err("mesh_set_inds index_count must be a multiple of 3!");
		return;
	}

	// Keep track of index data for use on CPU side
	if (!mesh->discard_data) {
		if (mesh->ind_capacity < index_count)
			mesh->inds = sk_realloc_t(vind_t, mesh->inds, index_count);
		memcpy(mesh->inds, indices, sizeof(vind_t) * index_count);
	}

	if (!skg_buffer_is_valid( &mesh->ind_buffer )) {
		// Create a static vertex buffer the first time we call this function!
		mesh->ind_dynamic  = false;
		mesh->ind_capacity = index_count;
		mesh->ind_buffer   = skg_buffer_create(indices, index_count, sizeof(vind_t), skg_buffer_type_index, skg_use_static);
		if (!skg_buffer_is_valid( &mesh->ind_buffer ))
			log_err("mesh_set_inds: Failed to create index buffer");
		skg_mesh_set_inds(&mesh->gpu_mesh, &mesh->ind_buffer);
	} else if (mesh->ind_dynamic == false || index_count > mesh->ind_capacity) {
		// If they call this a second time, or they need more inds than will
		// fit in this buffer, lets make a new dynamic buffer!
		skg_buffer_destroy(&mesh->ind_buffer);
		mesh->ind_dynamic  = true;
		mesh->ind_capacity = index_count;
		mesh->ind_buffer   = skg_buffer_create(indices, index_count, sizeof(vind_t), skg_buffer_type_index, skg_use_dynamic);
		if (!skg_buffer_is_valid( &mesh->ind_buffer ))
			log_err("mesh_set_inds: Failed to create dynamic index buffer");
		skg_mesh_set_inds(&mesh->gpu_mesh, &mesh->ind_buffer);
	} else {
		// And if they call this a third time, or their inds fit in the same
		// buffer, just copy things over!
		skg_buffer_set_contents(&mesh->ind_buffer, indices, sizeof(vind_t) * index_count);
	}

	mesh->ind_count = index_count;
	mesh->ind_draw  = index_count;
}

///////////////////////////////////////////

void mesh_set_inds(mesh_t mesh, const vind_t *indices,  int32_t index_count) {
	struct ind_upload_job_t {
		mesh_t        mesh;
		const vind_t *indices;
		int32_t       index_count;
	};
	ind_upload_job_t job_data = {mesh, indices, index_count};

	assets_execute_gpu([](void *data) {
		ind_upload_job_t *job_data = (ind_upload_job_t *)data;
		_mesh_set_inds(job_data->mesh, job_data->indices, job_data->index_count);
		
		return (bool32_t)true;
	}, &job_data);
}

///////////////////////////////////////////

void mesh_set_data(mesh_t mesh, const vert_t *vertices, int32_t vertex_count, const vind_t *indices, int32_t index_count, bool32_t calculate_bounds) {
	struct mesh_upload_job_t {
		mesh_t        mesh;
		const vert_t *vertices;
		int32_t       vertex_count;
		const vind_t *indices;
		int32_t       index_count;
		bool32_t      calculate_bounds;
	};
	mesh_upload_job_t job_data = {mesh, vertices, vertex_count, indices, index_count, calculate_bounds};

	assets_execute_gpu([](void *data) {
		mesh_upload_job_t *job_data = (mesh_upload_job_t *)data;
		_mesh_set_verts(job_data->mesh, job_data->vertices, job_data->vertex_count, job_data->calculate_bounds, true);
		_mesh_set_inds (job_data->mesh, job_data->indices,  job_data->index_count);
		
		return (bool32_t)true;
	}, &job_data);
}

///////////////////////////////////////////

void mesh_get_inds(mesh_t mesh, vind_t *&out_indices, int32_t &out_index_count) {
	out_indices     = mesh->inds;
	out_index_count = mesh->inds == nullptr ? 0 : (int32_t)mesh->ind_count;
}

///////////////////////////////////////////

int32_t mesh_get_ind_count(mesh_t mesh) {
	return (int32_t)mesh->ind_count;
}

///////////////////////////////////////////

void mesh_calculate_normals(vert_t *verts, int32_t vert_count, const vind_t *inds, int32_t ind_count) {
	for (int32_t i = 0; i < vert_count; i++) verts[i].norm = vec3_zero;
	for (int32_t i = 0; i < ind_count; i+=3) {
		vert_t *v1 = &verts[inds[i  ]];
		vert_t *v2 = &verts[inds[i+1]];
		vert_t *v3 = &verts[inds[i+2]];
		// Length of cross product is twice the area of the triangle it's 
		// from, so if we don't 'normalize' it, then we get trangle area
		// weighting on our normals for free!
		vec3 normal = vec3_cross(v3->pos - v2->pos, v1->pos - v2->pos);
		v1->norm += normal;
		v2->norm += normal;
		v3->norm += normal;
	}
	for (int32_t i = 0; i < vert_count; i++) verts[i].norm = vec3_normalize(verts[i].norm);
}

///////////////////////////////////////////

void mesh_set_draw_inds(mesh_t mesh, int32_t index_count) {
	uint32_t u_count = index_count;
	if (u_count > mesh->ind_count) {
		u_count = mesh->ind_count;
		log_warn("mesh_set_draw_inds: Can't render more indices than the mesh has! Capping...");
	}
	mesh->ind_draw = u_count;
}

///////////////////////////////////////////

void mesh_set_bounds(mesh_t mesh, const bounds_t &bounds) {
	mesh->bounds = bounds;
}

///////////////////////////////////////////

bounds_t mesh_get_bounds(mesh_t mesh) {
	return mesh->bounds;
}

bool32_t mesh_has_skin(mesh_t mesh) {
	return mesh->skin_data.bone_ids != nullptr;
}

///////////////////////////////////////////

bool _mesh_set_skin(mesh_t mesh, const uint16_t *bone_ids_4, uint32_t bone_id_4_count, const vec4 *bone_weights, uint32_t bone_weight_count, int32_t bone_count) {
	if (mesh->discard_data) {
		log_err("mesh_set_skin: can't work with a mesh that doesn't keep data, ensure mesh_get_keep_data() is true");
		return false;
	}
	if (bone_weight_count != bone_id_4_count || bone_weight_count != mesh->vert_count) {
		log_err("mesh_set_skin: bone_weights, bone_ids_4 and vertex counts must match exactly");
		return false;
	}

	mesh->skin_data.bone_ids                = sk_malloc_t(uint16_t, bone_id_4_count * 4);
	mesh->skin_data.weights                 = sk_malloc_t(vec4,     bone_weight_count);
	mesh->skin_data.deformed_verts          = sk_malloc_t(vert_t,   mesh->vert_count);
	memcpy(mesh->skin_data.bone_ids,       bone_ids_4,   sizeof(uint16_t) * bone_id_4_count * 4);
	memcpy(mesh->skin_data.weights,        bone_weights, sizeof(vec4)     * bone_weight_count);
	memcpy(mesh->skin_data.deformed_verts, mesh->verts,  sizeof(vert_t)   * mesh->vert_count);

	mesh->skin_data.bone_inverse_transforms = sk_malloc_t(matrix,   bone_count);
	mesh->skin_data.bone_transforms         = sk_malloc_t(XMMATRIX, bone_count);
	memset(mesh->skin_data.bone_inverse_transforms, 0, sizeof(matrix  ) * bone_count);
	memset(mesh->skin_data.bone_transforms,         0, sizeof(XMMATRIX) * bone_count);

	mesh->skin_data.bone_count = bone_count;

	return true;
}

///////////////////////////////////////////

void mesh_set_skin(mesh_t mesh, const uint16_t *bone_ids_4, int32_t bone_id_4_count, const vec4 *bone_weights, int32_t bone_weight_count, const matrix *bone_resting_transforms, int32_t bone_count) {
	if (_mesh_set_skin(mesh, bone_ids_4, bone_id_4_count, bone_weights, bone_weight_count, bone_count)) {
		for (int32_t i = 0; i < bone_count; i++) {
			mesh->skin_data.bone_inverse_transforms[i] = matrix_invert(bone_resting_transforms[i]);
		}
	}
}

///////////////////////////////////////////

void mesh_set_skin_inv(mesh_t mesh, const uint16_t *bone_ids_4, int32_t bone_id_4_count, const vec4 *bone_weights, int32_t bone_weight_count, const matrix *bone_resting_transforms_inverted, int32_t bone_count) {
	if (_mesh_set_skin(mesh, bone_ids_4, bone_id_4_count, bone_weights, bone_weight_count, bone_count)) {
		memcpy(mesh->skin_data.bone_inverse_transforms, bone_resting_transforms_inverted, sizeof(matrix) * bone_count);
	}
}

///////////////////////////////////////////

void mesh_update_skin(mesh_t mesh, const matrix *bone_transforms, int32_t bone_count) {
	for (int32_t i = 0; i < bone_count; i++) {
		math_matrix_to_fast(mesh->skin_data.bone_inverse_transforms[i] * bone_transforms[i], &mesh->skin_data.bone_transforms[i]);
	}

	for (uint32_t i = 0; i < mesh->vert_count; i++) {
		XMVECTOR pos  = XMLoadFloat3((XMFLOAT3 *)&mesh->verts[i].pos);
		XMVECTOR norm = XMLoadFloat3((XMFLOAT3 *)&mesh->verts[i].norm);

		const uint16_t *bones   = &mesh->skin_data.bone_ids[i*4];
		const vec4      weights =  mesh->skin_data.weights [i];

		XMVECTOR new_pos  =                XMVectorScale(XMVector3Transform      (pos,  mesh->skin_data.bone_transforms[bones[0]]), weights.x);
		XMVECTOR new_norm =                XMVectorScale(XMVector3TransformNormal(norm, mesh->skin_data.bone_transforms[bones[0]]), weights.x);
		if (weights.y != 0) {
			new_pos          = XMVectorAdd(XMVectorScale(XMVector3Transform      (pos,  mesh->skin_data.bone_transforms[bones[1]]), weights.y), new_pos);
			new_norm         = XMVectorAdd(XMVectorScale(XMVector3TransformNormal(norm, mesh->skin_data.bone_transforms[bones[1]]), weights.y), new_norm);
		}
		if (weights.z != 0) {
			new_pos          = XMVectorAdd(XMVectorScale(XMVector3Transform      (pos,  mesh->skin_data.bone_transforms[bones[2]]), weights.z), new_pos);
			new_norm         = XMVectorAdd(XMVectorScale(XMVector3TransformNormal(norm, mesh->skin_data.bone_transforms[bones[2]]), weights.z), new_norm);
		}
		if (weights.w != 0) {
			new_pos          = XMVectorAdd(XMVectorScale(XMVector3Transform      (pos,  mesh->skin_data.bone_transforms[bones[3]]), weights.w), new_pos);
			new_norm         = XMVectorAdd(XMVectorScale(XMVector3TransformNormal(norm, mesh->skin_data.bone_transforms[bones[3]]), weights.w), new_norm);
		}
		XMStoreFloat3((DirectX::XMFLOAT3 *)&mesh->skin_data.deformed_verts[i].pos,  new_pos );
		XMStoreFloat3((DirectX::XMFLOAT3 *)&mesh->skin_data.deformed_verts[i].norm, new_norm);
	}
	_mesh_set_verts(mesh, mesh->skin_data.deformed_verts, mesh->vert_count, false, false);
}

///////////////////////////////////////////

mesh_t mesh_find(const char *id) {
	mesh_t result = (mesh_t)assets_find(id, asset_type_mesh);
	if (result != nullptr) {
		mesh_addref(result);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

void mesh_set_id(mesh_t mesh, const char *id) {
	assets_set_id(mesh->header, id);
}

///////////////////////////////////////////

void mesh_addref(mesh_t mesh) {
	assets_addref(mesh->header);
}

///////////////////////////////////////////

mesh_t mesh_create() {
	mesh_t result = (_mesh_t*)assets_allocate(asset_type_mesh);
	result->gpu_mesh = skg_mesh_create(nullptr, nullptr);
	return result;
}

///////////////////////////////////////////

mesh_t mesh_copy(mesh_t mesh) {
	if (mesh == nullptr) {
		log_err("mesh_copy was provided a null mesh!");
		return nullptr;
	}

	mesh_t result = (mesh_t)assets_allocate(asset_type_mesh);
	result->bounds       = mesh->bounds;
	result->discard_data = mesh->discard_data;
	result->ind_draw     = mesh->ind_draw;

	if (mesh->discard_data) {
		log_err("mesh_copy not yet implemented for meshes with discard data set!");
	} else {
		mesh_set_inds (result, mesh->inds,  mesh->ind_count);
		mesh_set_verts(result, mesh->verts, mesh->vert_count, false);
		if (mesh_has_skin(mesh))
			mesh_set_skin_inv(result, mesh->skin_data.bone_ids, mesh->vert_count, mesh->skin_data.weights, mesh->vert_count, mesh->skin_data.bone_inverse_transforms, mesh->skin_data.bone_count);
	}

	return result;
}

///////////////////////////////////////////

const mesh_collision_t *mesh_get_collision_data(mesh_t mesh) {
	if (mesh->collision_data.pts != nullptr)
		return &mesh->collision_data;
	if (mesh->discard_data)
		return nullptr;

	mesh_collision_t &coll = mesh->collision_data;
	coll.pts    = sk_malloc_t(vec3   , mesh->ind_count);
	coll.planes = sk_malloc_t(plane_t, mesh->ind_count/3);

	for (uint32_t i = 0; i < mesh->ind_count; i++) coll.pts[i] = mesh->verts[mesh->inds[i]].pos;

	for (uint32_t i = 0; i < mesh->ind_count; i += 3) {
		vec3    dir1   = coll.pts[i+1] - coll.pts[i];
		vec3    dir2   = coll.pts[i+1] - coll.pts[i+2];
		vec3    normal = vec3_normalize( vec3_cross(dir2, dir1) );
		plane_t plane  = { normal, -vec3_dot(coll.pts[i + 1], normal) };
		coll.planes[i/3] = plane;
	}

	return &mesh->collision_data;
}

///////////////////////////////////////////

void mesh_release(mesh_t mesh) {
	if (mesh == nullptr)
		return;
	assets_releaseref(mesh->header);
}

///////////////////////////////////////////

void mesh_destroy(mesh_t mesh) {
	skg_mesh_destroy  (&mesh->gpu_mesh);
	skg_buffer_destroy(&mesh->vert_buffer);
	skg_buffer_destroy(&mesh->ind_buffer);
	free(mesh->verts);
	free(mesh->inds);
	free(mesh->collision_data.pts   );
	free(mesh->collision_data.planes);
	*mesh = {};
}


///////////////////////////////////////////

void mesh_draw(mesh_t mesh, material_t material, matrix transform, color128 color_linear, render_layer_ layer) {
	render_add_mesh(mesh, material, transform, color_linear, layer);
}

///////////////////////////////////////////

bool32_t mesh_ray_intersect(mesh_t mesh, ray_t model_space_ray, ray_t *out_pt, uint32_t* out_start_inds) {
	vec3 result = {};

	const mesh_collision_t *data = mesh_get_collision_data(mesh);
	if (data == nullptr)
		return false;
	if (!bounds_ray_intersect(mesh->bounds, model_space_ray, &result))
		return false;

	vec3  pt = {};
	float nearest_dist = FLT_MAX;
	for (uint32_t i = 0; i < mesh->ind_count; i+=3) {
		if (!plane_ray_intersect(data->planes[i / 3], model_space_ray, &pt))
			continue;

		// point in triangle, implementation based on:
		// https://blackpawn.com/texts/pointinpoly/default.html

		// Compute vectors
		vec3 v0 = data->pts[i+1] - data->pts[i];
		vec3 v1 = data->pts[i+2] - data->pts[i];
		vec3 v2 = pt - data->pts[i];

		// Compute dot products
		float dot00 = vec3_dot(v0, v0);
		float dot01 = vec3_dot(v0, v1);
		float dot02 = vec3_dot(v0, v2);
		float dot11 = vec3_dot(v1, v1);
		float dot12 = vec3_dot(v1, v2);

		// Compute barycentric coordinates
		float inv_denom = 1.0f / (dot00 * dot11 - dot01 * dot01);
		float u = (dot11 * dot02 - dot01 * dot12) * inv_denom;
		float v = (dot00 * dot12 - dot01 * dot02) * inv_denom;

		// Check if point is in triangle
		if ((u >= 0) && (v >= 0) && (u + v < 1)) {
			float dist = vec3_magnitude_sq(pt - model_space_ray.pos);
			if (dist < nearest_dist) {
				nearest_dist = dist;
				if (out_start_inds != nullptr) {
					*out_start_inds = i;
				}
				*out_pt = {pt, data->planes[i / 3].normal};
			}
		}
	}

	return nearest_dist != FLT_MAX;
}
///////////////////////////////////////////

bool32_t mesh_get_triangle(mesh_t mesh, uint32_t triangle_index, vert_t* a, vert_t* b, vert_t* c) {
	if (mesh->discard_data) {
		log_err("mesh_get_triangle: can't work with a mesh that doesn't keep data, ensure mesh_get_keep_data() is true");
		return false;
	}
	if (mesh->ind_count > triangle_index) {
		*a = mesh->verts[mesh->inds[triangle_index]];
		*b = mesh->verts[mesh->inds[triangle_index + 1]];
		*c = mesh->verts[mesh->inds[triangle_index + 2]];
		return true;
	}else {
		return false;
	}
}

///////////////////////////////////////////

void mesh_gen_cube_vert(int i, const vec3 &size, vec3 &pos, vec3 &norm, vec2 &uv) {
	float neg = (float)((i / 4) % 2 ? -1 : 1);
	int nx  = ((i+24) / 16) % 2;
	int ny  = (i / 8)       % 2;
	int nz  = (i / 16)      % 2;
	int u   = ((i+1) / 2)   % 2; // U: 0,1,1,0
	int v   = (i / 2)       % 2; // V: 0,0,1,1

	uv   = { (float)u, 1.0f-(float)v };
	norm = { nx*neg, ny*neg, nz*neg };
	pos  = {
		size.x * (nx ? neg : ny ? (u?-1:1)*neg : (u?1:-1)*neg),
		size.y * (nx || nz ? (v?1:-1) : neg),
		size.z * (nx ? (u?-1:1)*neg : ny ? (v?1:-1) : neg)
	};
}

///////////////////////////////////////////

mesh_t mesh_gen_plane(vec2 dimensions, vec3 plane_normal, vec3 plane_top_direction, int32_t subdivisions) {
	vind_t subd   = (vind_t)subdivisions;
	mesh_t result = mesh_create();

	subd = maxi(0,(int32_t)subd) + 2;

	int vert_count = subd*subd;
	int ind_count  = 6*(subd-1)*(subd-1);
	vert_t *verts = sk_malloc_t(vert_t, vert_count);
	vind_t *inds  = sk_malloc_t(vind_t, ind_count );

	vec3 right = vec3_cross(plane_top_direction, plane_normal);
	vec3 up    = vec3_cross(right,               plane_normal);

	// Make vertices
	for (vind_t y = 0; y < subd; y++) {
		float yp = y / (float)(subd-1);
	for (vind_t x = 0; x < subd; x++) {
		float xp = x / (float)(subd-1);

		verts[x + y*subd] = vert_t{ 
			right * ((xp - 0.5f) * dimensions.x) +
			up    * ((yp - 0.5f) * dimensions.y), 
			plane_normal, {xp,yp}, {255,255,255,255} };
	} }

	// make indices
	int ind = 0;
	for (vind_t y = 0; y < subd-1; y++) {
	for (vind_t x = 0; x < subd-1; x++) {
			inds[ind++] = (x+1) + (y+1) * subd;
			inds[ind++] = (x+1) +  y    * subd;
			inds[ind++] =  x    +  y    * subd;

			inds[ind++] =  x    + (y+1) * subd;
			inds[ind++] = (x+1) + (y+1) * subd;
			inds[ind++] =  x    +  y    * subd;
	} }

	mesh_set_data(result, verts, vert_count, inds, ind_count);

	free(verts);
	free(inds);
	return result;
}

///////////////////////////////////////////

mesh_t mesh_gen_cube(vec3 dimensions, int32_t subdivisions) {
	vind_t subd   = (vind_t)subdivisions;
	mesh_t result = mesh_create();

	subd = maxi((int32_t)0,(int32_t)subd) + 2;

	int vert_count = 6*subd*subd;
	int ind_count  = 6*(subd-1)*(subd-1)*6;
	vert_t *verts = sk_malloc_t(vert_t, vert_count);
	vind_t *inds  = sk_malloc_t(vind_t, ind_count);

	vec3   size   = dimensions / 2;
	int    ind    = 0;
	vind_t offset = 0;
	for (vind_t i = 0; i < 6*4; i+=4) {
		vec3 p1, p2, p3, p4;
		vec3 n1, n2, n3, n4;
		vec2 u1, u2, u3, u4;

		mesh_gen_cube_vert(i,   size, p1, n1, u1);
		mesh_gen_cube_vert(i+1, size, p2, n2, u2);
		mesh_gen_cube_vert(i+2, size, p3, n3, u3);
		mesh_gen_cube_vert(i+3, size, p4, n4, u4);

		offset = (i/4) * (subd)*(subd);
		for (vind_t y = 0; y < subd; y++) {
			float  py    = y / (float)(subd-1);
			vind_t yOff  = offset + y * subd;
			vind_t yOffN = offset + (y+1) * subd;

			vec3 pl = vec3_lerp(p1, p4, py);
			vec3 pr = vec3_lerp(p2, p3, py);
			vec3 nl = vec3_lerp(n1, n4, py);
			vec3 nr = vec3_lerp(n2, n3, py);
			vec2 ul = vec2_lerp(u1, u4, py);
			vec2 ur = vec2_lerp(u2, u3, py);

			for (vind_t x = 0; x < subd; x++) {
				float px = x / (float)(subd-1);
				vind_t  ptIndex = x + yOff;
				vert_t *pt      = &verts[ptIndex];

				pt->pos = vec3_lerp(pl, pr, px);
				pt->norm= vec3_lerp(nl, nr, px);
				pt->uv  = vec2_lerp(ul, ur, px);
				pt->col = {255,255,255,255};

				if (y != subd-1 && x != subd-1) {

					inds[ind++] = (x  ) + yOff;
					inds[ind++] = (x+1) + yOff;
					inds[ind++] = (x+1) + yOffN;

					inds[ind++] = (x  ) + yOff;
					inds[ind++] = (x+1) + yOffN;
					inds[ind++] = (x  ) + yOffN;
				}
			}
		}
	}

	mesh_set_data(result, verts, vert_count, inds, ind_count);

	free(verts);
	free(inds);
	return result;
}

///////////////////////////////////////////

mesh_t mesh_gen_sphere(float diameter, int32_t subdivisions) {
	vind_t subd   = (vind_t)subdivisions;
	mesh_t result = mesh_create();

	subd = maxi(0,(int32_t)subd) + 2;

	int vert_count = 6*subd*subd;
	int ind_count  = 6*(subd-1)*(subd-1)*6;
	vert_t *verts = sk_malloc_t(vert_t, vert_count);
	vind_t *inds  = sk_malloc_t(vind_t, ind_count);

	vec3   size = vec3_one;
	float  radius = diameter / 2;
	int    ind    = 0;
	vind_t offset = 0;
	for (vind_t i = 0; i < 6*4; i+=4) {
		vec3 p1, p2, p3, p4;
		vec3 n1, n2, n3, n4;
		vec2 u1, u2, u3, u4;

		mesh_gen_cube_vert(i,   size, p1, n1, u1);
		mesh_gen_cube_vert(i+1, size, p2, n2, u2);
		mesh_gen_cube_vert(i+2, size, p3, n3, u3);
		mesh_gen_cube_vert(i+3, size, p4, n4, u4);

		offset = (i/4) * (subd)*(subd);
		for (vind_t y = 0; y < subd; y++) {
			float  py    = y / (float)(subd-1);
			vind_t yOff  = offset + y * subd;
			vind_t yOffN = offset + (y+1) * subd;

			vec3 pl = vec3_lerp(p1, p4, py);
			vec3 pr = vec3_lerp(p2, p3, py);
			vec2 ul = vec2_lerp(u1, u4, py);
			vec2 ur = vec2_lerp(u2, u3, py);

			for (vind_t x = 0; x < subd; x++) {
				float px = x / (float)(subd-1);
				vind_t  ptIndex = x + yOff;
				vert_t *pt = &verts[ptIndex];

				pt->norm= vec3_normalize(vec3_lerp(pl, pr, px));
				pt->pos = pt->norm*radius;
				pt->uv  = vec2_lerp(ul, ur, px);
				pt->col = {255,255,255,255};

				if (y != subd-1 && x != subd-1) {

					inds[ind++] = (x  ) + yOff;
					inds[ind++] = (x+1) + yOff;
					inds[ind++] = (x+1) + yOffN;

					inds[ind++] = (x  ) + yOff;
					inds[ind++] = (x+1) + yOffN;
					inds[ind++] = (x  ) + yOffN;
				}
			}
		}
	}

	mesh_set_data(result, verts, vert_count, inds, ind_count);

	free(verts);
	free(inds);
	return result;
}

///////////////////////////////////////////

mesh_t mesh_gen_cylinder(float diameter, float depth, vec3 dir, int32_t subdivisions) {
	mesh_t result = mesh_create();
	dir = vec3_normalize(dir);
	float radius = diameter / 2;

	vind_t subd = (vind_t)subdivisions;
	int vert_count = (subdivisions+1) * 4 + 2;
	int ind_count  = subdivisions * 12;
	vert_t *verts = sk_malloc_t(vert_t, vert_count);
	vind_t *inds  = sk_malloc_t(vind_t, ind_count);

	// Calculate any perpendicular vector
	vec3 perp = vec3{dir.z, dir.z, -dir.x-dir.y};
	if (vec3_magnitude_sq(perp) == 0)
		perp = vec3{-dir.y-dir.z, dir.x, dir.x};

	vec3 axis_x = vec3_normalize(vec3_cross(dir, perp));
	vec3 axis_y = vec3_normalize(vec3_cross(dir, axis_x));
	vec3 z_off  = dir * (depth / 2.f);
	vind_t ind = 0;

	for (vind_t i = 0; i <= subd; i++) {
		float u   = ((float)i / subd);
		float ang = u * (float)M_PI * 2;
		float x   = cosf(ang);
		float y   = sinf(ang);
		vec3 normal  = axis_x * x + axis_y * y;
		vec3 top_pos = normal*radius + z_off;
		vec3 bot_pos = normal*radius - z_off;

		// strip first
		verts[i * 4  ] = { top_pos,  normal, {u,0}, {255,255,255,255} };
		verts[i * 4+1] = { bot_pos,  normal, {u,1}, {255,255,255,255} };
		// now circular faces
		verts[i * 4+2] = { top_pos,  dir,    {u,0}, {255,255,255,255} };
		verts[i * 4+3] = { bot_pos, -dir,    {u,1}, {255,255,255,255} };

		if (i == subd) continue;

		vind_t in = (i + 1) % (subd+1);
		// Top slice
		inds[ind++] = i  * 4 + 2;
		inds[ind++] = in * 4 + 2;
		inds[ind++] = (subd+1) * 4;
		// Bottom slice
		inds[ind++] = (subd+1) * 4+1;
		inds[ind++] = in * 4 + 3;
		inds[ind++] = i  * 4 + 3;
		// Now edge strip quad
		inds[ind++] = in * 4+1;
		inds[ind++] = in * 4;
		inds[ind++] = i  * 4;
		inds[ind++] = i  * 4+1;
		inds[ind++] = in * 4+1;
		inds[ind++] = i  * 4;
	}
	// center points for the circle
	verts[(subdivisions+1)*4]   = {  z_off,  dir, {0.5f,0.01f}, {255,255,255,255} };
	verts[(subdivisions+1)*4+1] = { -z_off, -dir, {0.5f,0.99f}, {255,255,255,255} };

	mesh_set_data(result, verts, vert_count, inds, ind_count);

	free(verts);
	free(inds);
	return result;
}

///////////////////////////////////////////

mesh_t mesh_gen_rounded_cube(vec3 dimensions, float edge_radius, int32_t subdivisions) {
	vind_t subd   = (vind_t)subdivisions;
	mesh_t result = mesh_create();

	subd = maxi(0,(int32_t)subd) + 2;
	if (subd % 2 == 1) // need an even number of subdivisions
		subd += 1;

	vind_t  vert_count = 6*subd*subd;
	vind_t  ind_count  = 6*(subd-1)*(subd-1)*6;
	vert_t *verts = sk_malloc_t(vert_t, vert_count);
	vind_t *inds  = sk_malloc_t(vind_t, ind_count );

	vec3   off = (dimensions / 2) - vec3_one*edge_radius;
	vec3   size = vec3_one;
	float  radius = edge_radius;
	vind_t ind    = 0;
	vind_t offset = 0;
	for (vind_t i = 0; i < 6*4; i+=4) {
		vec3 p1, p2, p3, p4;
		vec3 n1, n2, n3, n4;
		vec2 u1, u2, u3, u4;

		mesh_gen_cube_vert(i,   size, p1, n1, u1);
		mesh_gen_cube_vert(i+1, size, p2, n2, u2);
		mesh_gen_cube_vert(i+2, size, p3, n3, u3);
		mesh_gen_cube_vert(i+3, size, p4, n4, u4);

		float sizeU = vec3_magnitude((p4 - p1) * (dimensions/2));
		float sizeV = vec3_magnitude((p2 - p1) * (dimensions/2));

		offset = (i/4) * (subd)*(subd);
		vind_t x, y;
		for (vind_t sy = 0; sy < subd; sy++) {
			bool first_half_y = sy < subd / 2;
			y = first_half_y ? sy : sy-1;
			vec3 stretchA  = first_half_y ? p1 : p4;
			vec3 stretchB  = first_half_y ? p2 : p3;
			float stretchV = (radius*2)/sizeV;
			float offV     = first_half_y ? 0 : sizeV-(radius*2);
			
			float py    = y / (float)(subd-2);
			float pv    = py * stretchV + offV;
			vind_t yOff  = offset + sy * subd;
			vind_t yOffN = offset + (sy+1) * subd;
			
			vec3 pl = vec3_lerp(p1, p4, py);
			vec3 pr = vec3_lerp(p2, p3, py);
			vec2 ul = vec2_lerp(u1, u4, pv);
			vec2 ur = vec2_lerp(u2, u3, pv);

			for (vind_t sx = 0; sx < subd; sx++) {
				bool first_half_x = sx < subd / 2;
				x = first_half_x ? sx : sx-1;
				vec3  stretch = first_half_x ? stretchA : stretchB;
				float stretchU = (radius*2)/sizeU;
				float offU     = first_half_x ? 0 : sizeU-(radius*2);

				float px      = x / (float)(subd-2);
				float pu      = px * stretchU + offU;
				int   ptIndex = sx + yOff;
				vert_t *pt    = &verts[ptIndex];

				pt->norm= vec3_normalize(vec3_lerp(pl, pr, px));
				pt->pos = pt->norm*radius + stretch*off;
				pt->uv = vec2_lerp(ul, ur, pu);
				pt->col = {255,255,255,255};

				if (sy != subd-1 && sx != subd-1) {
					inds[ind++] = (sx  ) + yOff;
					inds[ind++] = (sx+1) + yOff;
					inds[ind++] = (sx+1) + yOffN;

					inds[ind++] = (sx  ) + yOff;
					inds[ind++] = (sx+1) + yOffN;
					inds[ind++] = (sx  ) + yOffN;
				}
			}
		}
	}

	mesh_set_data(result, verts, vert_count, inds, ind_count);

	free(verts);
	free(inds);
	return result;
}

} // namespace sk
#define _CRT_SECURE_NO_WARNINGS 1

#include "../sk_math.h"
#include "../sk_math_dx.h"
#include "../sk_memory.h"
#include "model.h"
#include "mesh.h"
#include "../libraries/stref.h"
#include "../platforms/platform.h"

using namespace DirectX;

#include <stdio.h>
#include <string.h>
#include <float.h>

namespace sk {

///////////////////////////////////////////

model_t model_create() {
	model_t result = (_model_t*)assets_allocate(asset_type_model);
	result->anim_inst.anim_id = -1;
	return result;
}

///////////////////////////////////////////

void model_set_id(model_t model, const char *id) {
	assets_set_id(&model->header, id);
}

///////////////////////////////////////////

const char* model_get_id(const model_t model) {
	return model->header.id_text;
}

///////////////////////////////////////////

model_t model_find(const char *id) {
	model_t result = (model_t)assets_find(id, asset_type_model);
	if (result != nullptr) {
		model_addref(result);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

model_t model_copy(model_t model) {
	if (model == nullptr) {
		log_err("model_copy was provided a null model!");
		return nullptr;
	}

	model_t result = (model_t)assets_allocate(asset_type_model);
	result->visuals      = model->visuals.copy();
	result->nodes        = model->nodes  .copy();
	result->bounds       = model->bounds;
	result->nodes_used   = model->nodes_used;
	result->anim_inst.anim_id = -1;
	for (int32_t i = 0; i < result->visuals.count; i++) {
		material_addref(result->visuals[i].material);
		mesh_addref    (result->visuals[i].mesh);
	}
	for (int32_t i = 0; i < result->nodes.count; i++) {
		result->nodes[i].name = string_copy(result->nodes[i].name);
	}

	// If the original Model is animating, we want to set this Model up with
	// the original meshes, not the active, modified meshes.
	if (model->anim_inst.skinned_meshes != nullptr) {
		for (int32_t i = 0; i < model->anim_inst.skinned_mesh_count; i++) {
			model_node_id node   = model->anim_data.skeletons[i].skin_node;
			int32_t       visual = result->nodes[node].visual;
			assets_safeswap_ref(
				(asset_header_t**)&result->visuals[visual].mesh,
				(asset_header_t* ) model ->anim_inst.skinned_meshes[i].original_mesh);
		}
	}
	result->anim_data = anim_data_copy(&model->anim_data);

	return result;
}

///////////////////////////////////////////

model_t model_create_mesh(mesh_t mesh, material_t material) {
	model_t result = model_create();
	model_node_add(result, nullptr, matrix_identity, mesh, material, true);
	return result;
}

///////////////////////////////////////////

model_t model_create_mem(const char *filename, const void *data, size_t data_size, shader_t shader) {
	model_t result = model_create();
	
	if (string_endswith(filename, ".glb",  false) || 
		string_endswith(filename, ".gltf", false) ||
		string_endswith(filename, ".vrm",  false)) {
		if (!modelfmt_gltf(result, filename, data, data_size, shader))
			log_errf("Issue loading GLTF file: %s!", filename);
	} else if (string_endswith(filename, ".obj", false)) {
		if (!modelfmt_obj (result, filename, data, data_size, shader))
			log_errf("Issue loading Wavefront OBJ file: %s!", filename);
	} else if (string_endswith(filename, ".stl", false)) {
		if (!modelfmt_stl (result, filename, data, data_size, shader))
			log_errf("Issue loading STL file: %s!", filename);
	} else if (string_endswith(filename, ".ply", false)) {
		if (!modelfmt_ply (result, filename, data, data_size, shader))
			log_errf("Issue loading PLY file: %s!", filename);
	} else {
		log_errf("Issue loading %s! Unrecognized file extension.", filename);
	}

	return result;
}

///////////////////////////////////////////

model_t model_create_file(const char *filename, shader_t shader) {
	model_t result = model_find(filename);
	if (result != nullptr)
		return result;

	void*    data;
	size_t   length;
	bool32_t loaded         = platform_read_file(filename, &data, &length);
	if (!loaded) {
		log_warnf("Model file failed to load: %s", filename);
		return nullptr;
	}

	result = model_create_mem(filename, data, length, shader);
	if (result != nullptr) {
		model_set_id(result, filename);
	}
	
	sk_free(data);
	return result;
}

///////////////////////////////////////////

void model_recalculate_bounds(model_t model) {
	model->bounds_dirty = false;
	if (model->visuals.count <= 0) {
		model->bounds = {};
		return;
	}
	
	// Get an initial size
	vec3 first_corner = bounds_corner(mesh_get_bounds(model->visuals[0].mesh), 0);
	vec3 min, max;
	min = max = matrix_transform_pt( model->visuals[0].transform_model, first_corner);
	
	// Find the corners for each bounding cube, and factor them in!
	for (int32_t m = 0; m < model->visuals.count; m += 1) {
		bounds_t bounds = mesh_get_bounds(model->visuals[m].mesh);
		for (int32_t i = 0; i < 8; i += 1) {
			vec3 corner = bounds_corner      (bounds, i);
			vec3 pt     = matrix_transform_pt(model->visuals[m].transform_model, corner);
			min.x = fminf(pt.x, min.x);
			min.y = fminf(pt.y, min.y);
			min.z = fminf(pt.z, min.z);

			max.x = fmaxf(pt.x, max.x);
			max.y = fmaxf(pt.y, max.y);
			max.z = fmaxf(pt.z, max.z);
		}
	}
	
	// Final bounds value
	model->bounds = bounds_t{ min / 2 + max / 2, max - min };
}

///////////////////////////////////////////

void model_recalculate_bounds_exact(model_t model) {
	model->bounds_dirty = false;
	if (model->visuals.count <= 0) {
		model->bounds = {};
		return;
	}

	// Get an initial size
	vec3 first_corner = model->visuals[0].mesh->verts != nullptr
		? model->visuals[0].mesh->verts[0].pos
		: bounds_corner(model->visuals[0].mesh->bounds, 0);

	vec3     minf = matrix_transform_pt( model->visuals[0].transform_model, first_corner);
	XMVECTOR min  = XMLoadFloat3((XMFLOAT3*)&minf);
	XMVECTOR max  = XMLoadFloat3((XMFLOAT3*)&minf);

	// Use all the transformed vertices, and factor them in!
	for (int32_t m = 0; m < model->visuals.count; m += 1) {
		XMMATRIX      transform_model = XMLoadFloat4x4((XMFLOAT4X4*)&model->visuals[m].transform_model.row);
		const mesh_t  mesh            = model->visuals[m].mesh;
		const vert_t* verts           = mesh->verts;

		if (verts != nullptr) {
			for (uint32_t i = 0; i < mesh->vert_count; i += 1) {
				XMVECTOR pt = matrix_mul_pointx(transform_model, verts[i].pos);

				min = XMVectorMin(min, pt);
				max = XMVectorMax(max, pt);
			}
		} else {
			for (int32_t i = 0; i < 8; i += 1) {
				vec3     corner = bounds_corner(mesh->bounds, i);
				XMVECTOR pt     = matrix_mul_pointx(transform_model, corner);

				min = XMVectorMin(min, pt);
				max = XMVectorMax(max, pt);
			}
		}
	}

	// Final bounds value

	XMVECTOR center     = XMVectorMultiplyAdd(min, g_XMOneHalf, XMVectorMultiply(max, g_XMOneHalf));
	XMVECTOR dimensions = XMVectorSubtract   (max, min);

	XMStoreFloat3((XMFLOAT3*)&(model->bounds.center),     center);
	XMStoreFloat3((XMFLOAT3*)&(model->bounds.dimensions), dimensions);
}

///////////////////////////////////////////

void model_addref(model_t model) {
	assets_addref(&model->header);
}

///////////////////////////////////////////

void model_release(model_t model) {
	if (model == nullptr)
		return;

	assets_releaseref(&model->header);
}

///////////////////////////////////////////

void model_draw(model_t model, matrix transform, color128 color_linear, render_layer_ layer) {
	render_add_model_mat(model, nullptr, transform, color_linear, layer);
}

///////////////////////////////////////////

void model_draw_mat(model_t model, material_t material_override, matrix transform, color128 color_linear, render_layer_ layer) {
	render_add_model_mat(model, material_override, transform, color_linear, layer);
}

///////////////////////////////////////////

void model_set_bounds(model_t model, const bounds_t &bounds) {
	model->bounds = bounds;
}

///////////////////////////////////////////

bounds_t model_get_bounds(model_t model) {
	if (model->bounds_dirty) {
		model_recalculate_bounds(model);
	}
	return model->bounds;
}

///////////////////////////////////////////

bool32_t model_ray_intersect(model_t model, ray_t model_space_ray, cull_ cull_mode, ray_t *out_pt) {
	vec3 bounds_at;
	if (!bounds_ray_intersect(model->bounds, model_space_ray, &bounds_at))
		return false;

	float closest = FLT_MAX;
	*out_pt = {};
	for (int32_t i = 0; i < model->nodes.count; i++) {
		model_node_t *n = &model->nodes[i];
		if (!n->solid || n->visual == -1)
			continue;

		matrix inverse   = matrix_invert(n->transform_model);
		ray_t  local_ray = matrix_transform_ray(inverse, model_space_ray);
		ray_t  at;
		if (mesh_ray_intersect(model->visuals[n->visual].mesh, local_ray, cull_mode, &at, nullptr)) {
			float d = vec3_distance_sq(local_ray.pos, at.pos);
			if (d < closest) {
				closest = d;
				*out_pt = matrix_transform_ray(n->transform_model, at);
			}
		}
	}
	return closest != FLT_MAX;
}

///////////////////////////////////////////

bool32_t model_ray_intersect_bvh(model_t model, ray_t model_space_ray, cull_ cull_mode, ray_t *out_pt) {
	vec3 bounds_at;
	if (!bounds_ray_intersect(model->bounds, model_space_ray, &bounds_at))
		return false;

	float closest = FLT_MAX;
	*out_pt = {};
	for (int32_t i = 0; i < model->nodes.count; i++) {
		model_node_t *n = &model->nodes[i];
		if (!n->solid || n->visual == -1)
			continue;

		matrix inverse   = matrix_invert(n->transform_model);
		ray_t  local_ray = matrix_transform_ray(inverse, model_space_ray);
		ray_t  at;
		if (mesh_ray_intersect_bvh(model->visuals[n->visual].mesh, local_ray, cull_mode, &at, nullptr)) {
			float d = vec3_distance_sq(local_ray.pos, at.pos);
			if (d < closest) {
				closest = d;
				*out_pt = matrix_transform_ray(n->transform_model, at);
			}
		}
	}
	return closest != FLT_MAX;
}

///////////////////////////////////////////

// Same as model_ray_intersect_bvh, but returns mesh, mesh transform and start index if intersection found
bool32_t model_ray_intersect_bvh_detailed(model_t model, ray_t model_space_ray, cull_ cull_mode, ray_t *out_pt, mesh_t *out_mesh, matrix *out_matrix, uint32_t* out_start_inds) {
	vec3 bounds_at;
	if (!bounds_ray_intersect(model->bounds, model_space_ray, &bounds_at))
		return false;

	float closest = FLT_MAX;
	*out_pt = {};
	for (int32_t i = 0; i < model->nodes.count; i++) {
		model_node_t *n = &model->nodes[i];
		if (!n->solid || n->visual == -1)
			continue;

		matrix inverse   = matrix_invert(n->transform_model);
		ray_t  local_ray = matrix_transform_ray(inverse, model_space_ray);
		ray_t  at;
		uint32_t local_start_inds;
		if (mesh_ray_intersect_bvh(model->visuals[n->visual].mesh, local_ray, cull_mode , &at, &local_start_inds)) {
			float d = vec3_distance_sq(local_ray.pos, at.pos);
			if (d < closest) {
				closest = d;
				if (out_mesh != nullptr && out_start_inds != nullptr) {
					*out_mesh = model->visuals[n->visual].mesh;
					*out_matrix = n->transform_model;
					*out_start_inds = local_start_inds;
				}
				*out_pt = matrix_transform_ray(n->transform_model, at);
			}
		}
	}
	return closest != FLT_MAX;
}

///////////////////////////////////////////

void model_destroy(model_t model) {
	anim_inst_destroy(&model->anim_inst);
	anim_data_destroy(&model->anim_data);
	for (int32_t i = 0; i < model->nodes.count; i++) {
		sk_free(model->nodes[i].name);
		model_node_info_clear(model, i);
	}
	for (int32_t i = 0; i < model->visuals.count; i++) {
		mesh_release    (model->visuals[i].mesh);
		material_release(model->visuals[i].material);
	}
	model->nodes  .free();
	model->visuals.free();
	*model = {};
}

///////////////////////////////////////////

model_node_id model_node_add(model_t model, const char *name, matrix transform, mesh_t mesh, material_t material, bool32_t solid) {
	return model_node_add_child(model, -1, name, transform, mesh, material, solid);
}

///////////////////////////////////////////

model_node_id model_node_add_child(model_t model, model_node_id parent, const char *name, matrix local_transform, mesh_t mesh, material_t material, bool32_t solid) {
	if ((mesh != nullptr && material == nullptr) ||
		(mesh == nullptr && material != nullptr)) {
		log_err("model_node_add_child: mesh and material must either both be null, or neither be null!");
		return -1;
	}

	model_node_id node_id = (model_node_id)model->nodes.count;
	char          tmp_name[32];
	if (name == nullptr) {
		snprintf(tmp_name, sizeof(tmp_name), "node%d", node_id);
		name = tmp_name;
	}

	model_node_t node = {};
	node.name            = string_copy(name);
	node.parent          = parent >= 0 ? parent : -1;
	node.child           = -1;
	node.sibling         = -1;
	node.visual          = -1;
	node.solid           = solid;
	node.transform_local = local_transform;
	if (node.parent >= 0) {
		node.transform_model = local_transform * model->nodes[node.parent].transform_model;
		// Find the parent's last child, and tack this one onto the chain after
		// it.
		if (model->nodes[node.parent].child == -1)
			model->nodes[node.parent].child = node_id;
		else {
			model_node_id curr = model->nodes[node.parent].child;
			while (model->nodes[curr].sibling != -1)
				curr = model->nodes[curr].sibling;
			model->nodes[curr].sibling = node_id;
		}
	} else {
		if (model->nodes.count > 0) {
			model_node_id sibling_id = 0;
			while (model->nodes[sibling_id].sibling != -1)
				sibling_id = model->nodes[sibling_id].sibling;
			model->nodes[sibling_id].sibling = node_id;
		}
		node.transform_model = local_transform;
	}

	if (mesh != nullptr) {
		material_addref(material);
		mesh_addref    (mesh);

		model_visual_t visual = {};
		visual.material        = material;
		visual.mesh            = mesh;
		visual.transform_model = node.transform_model;
		visual.node            = node_id;
		visual.visible         = true;
		node.visual = model->visuals.add(visual);
		model->bounds = model->visuals.count == 1
			? bounds_transform(mesh_get_bounds(mesh), node.transform_model)
			: bounds_grow_to_fit_box(model->bounds, mesh_get_bounds(mesh), &node.transform_model);
	}

	model->nodes.add(node);
	return node_id;
}

///////////////////////////////////////////

model_node_id model_node_find(model_t model, const char *name) {
	for (int32_t i = 0; i < model->nodes.count; i++) {
		if (string_eq(model->nodes[i].name, name))
			return (model_node_id)i;
	}
	return -1;
}

///////////////////////////////////////////

model_node_id model_node_sibling(model_t model, model_node_id node) {
	return model->nodes[node].sibling;
}

///////////////////////////////////////////

model_node_id model_node_parent(model_t model, model_node_id node) {
	return model->nodes[node].parent;
}

///////////////////////////////////////////

model_node_id model_node_child(model_t model, model_node_id node) {
	if (node < 0)
		return model_node_get_root(model);
	return model->nodes[node].child;
}

///////////////////////////////////////////

int32_t model_node_count(model_t model) {
	return model->nodes.count;
}

///////////////////////////////////////////

model_node_id model_node_index(model_t, int32_t index) {
	return index;
}

///////////////////////////////////////////

int32_t model_node_visual_count(model_t model){
	return model->visuals.count;
}

///////////////////////////////////////////

model_node_id model_node_visual_index(model_t model, int32_t index) {
	return model->visuals[index].node;
}

///////////////////////////////////////////

model_node_id model_node_iterate(model_t model, model_node_id node) {
	if (node == -1) return model_node_get_root(model);

	// walk down
	if (model->nodes[node].child != -1)
		return model->nodes[node].child;

	// if not down, check to the right
	if (model->nodes[node].sibling != -1)
		return model->nodes[node].sibling;

	// if not to the right, then see if we can go up and right
	while (model->nodes[node].parent != -1) {
		node = model->nodes[node].parent;
		if (model->nodes[node].sibling != -1)
			return model->nodes[node].sibling;
	}

	return -1;
}

///////////////////////////////////////////

model_node_id model_node_get_root(model_t model) {
	return model->nodes.count > 0
		? 0
		: -1;
}

///////////////////////////////////////////

const char* model_node_get_name(model_t model, model_node_id node) {
	return model->nodes[node].name;
}

///////////////////////////////////////////

bool32_t model_node_get_solid(model_t model, model_node_id node) {
	return model->nodes[node].solid;
}

///////////////////////////////////////////

bool32_t model_node_get_visible(model_t model, model_node_id node) {
	int32_t vis = model->nodes[node].visual;
	return vis == -1
		? false
		: model->visuals[vis].visible;
}

///////////////////////////////////////////

material_t  model_node_get_material(model_t model, model_node_id node) {
	int32_t vis = model->nodes[node].visual;
	if (vis < 0) {
		return nullptr;
	} else {
		material_addref(model->visuals[vis].material);
		return model->visuals[vis].material;
	}
}

///////////////////////////////////////////

mesh_t model_node_get_mesh(model_t model, model_node_id node) {
	int32_t vis = model->nodes[node].visual;
	if (vis < 0) {
		return nullptr;
	} else {
		mesh_addref(model->visuals[vis].mesh);
		return model->visuals[vis].mesh;
	}
}

///////////////////////////////////////////

matrix model_node_get_transform_model(model_t model, model_node_id node) {
	return model->nodes[node].transform_model;
}

///////////////////////////////////////////

matrix model_node_get_transform_local(model_t model, model_node_id node) {
	return model->nodes[node].transform_local;
}

///////////////////////////////////////////

void model_node_set_name(model_t model, model_node_id node, const char* name) {
	sk_free(model->nodes[node].name);
	char tmp_name[32];
	if (name == nullptr) {
		snprintf(tmp_name, sizeof(tmp_name), "node%d", node);
		name = tmp_name;
	}
	model->nodes[node].name = string_copy(name);
}

///////////////////////////////////////////

void model_node_set_solid(model_t model, model_node_id node, bool32_t solid) {
	model->nodes[node].solid = solid;
}

///////////////////////////////////////////

void model_node_set_visible(model_t model, model_node_id node, bool32_t visible) {
	int32_t vis = model->nodes[node].visual;
	if (vis != -1)
		model->visuals[vis].visible = visible;
}

///////////////////////////////////////////

void model_node_set_material(model_t model, model_node_id node, material_t material) {
	int32_t vis = model->nodes[node].visual;
	if (vis < 0) {
		vis = model->visuals.add({});
		model->nodes[node].visual = vis;
	}
	assets_safeswap_ref(
		(asset_header_t**)&model->visuals[vis].material,
		(asset_header_t* )material);
}

///////////////////////////////////////////

void model_node_set_mesh(model_t model, model_node_id node, mesh_t mesh) {
	int32_t vis = model->nodes[node].visual;
	if (vis < 0) {
		vis = model->visuals.add({});
		model->nodes[node].visual = vis;
	}
	mesh_t prev_mesh = model->visuals[vis].mesh;
	model->visuals[vis].mesh = mesh;
	model->bounds_dirty = true;
	mesh_addref (model->visuals[vis].mesh);
	mesh_release(prev_mesh);
}

///////////////////////////////////////////

void _model_node_update_transforms(model_t model, model_node_id node) {
	if (model->nodes[node].parent >= 0)
		model->nodes[node].transform_model = model->nodes[node].transform_local * model->nodes[model->nodes[node].parent].transform_model;
	else
		model->nodes[node].transform_model = model->nodes[node].transform_local;

	if (model->nodes[node].visual >= 0)
		model->visuals[model->nodes[node].visual].transform_model = model->nodes[node].transform_model;

	model_node_id curr = model->nodes[node].child;
	while (curr >= 0) {
		_model_node_update_transforms(model, curr);
		curr = model->nodes[curr].sibling;
	}
}

///////////////////////////////////////////

void model_node_set_transform_model(model_t model, model_node_id node, matrix transform_model_space) {
	model->nodes[node].transform_model = transform_model_space;
	if (model->nodes[node].parent >= 0) {
		matrix inv = matrix_invert(model->nodes[model->nodes[node].parent].transform_model);
		model->nodes[node].transform_local = transform_model_space * inv;
	} else {
		model->nodes[node].transform_local = transform_model_space;
	}

	if (model->nodes[node].visual >= 0)
		model->visuals[model->nodes[node].visual].transform_model = model->nodes[node].transform_model;

	model_node_id curr = model->nodes[node].child;
	while (curr >= 0) {
		_model_node_update_transforms(model, curr);
		curr = model->nodes[curr].sibling;
	}
	model->transforms_changed = true;
	model->bounds_dirty       = true;
}

///////////////////////////////////////////

void model_node_set_transform_local(model_t model, model_node_id node, matrix transform_local_space) {
	model->nodes[node].transform_local = transform_local_space;
	_model_node_update_transforms(model, node);
	model->transforms_changed = true;
	model->bounds_dirty       = true;
}

///////////////////////////////////////////

const char* model_node_info_get(model_t model, model_node_id node, const char* info_key_u8) {
	char** result = model->nodes[node].info.get(info_key_u8);
	return result == nullptr
		? nullptr
		: *result;
}

///////////////////////////////////////////

void model_node_info_set(model_t model, model_node_id node, const char* info_key_u8, const char* info_value_u8) {
	if (info_value_u8 == nullptr) {
		model_node_info_remove(model, node, info_key_u8);
		return;
	}

	dictionary_t<char*>* info = &model->nodes[node].info;
	int32_t              at   = info->contains(info_key_u8);
	if (at != -1) {
		sk_free(info->items[at].value);
		info->items[at].value = string_copy(info_value_u8);
	} else {
		info->set(info_key_u8, string_copy(info_value_u8));
	}
}

///////////////////////////////////////////

bool32_t model_node_info_remove(model_t model, model_node_id node, const char* info_key_u8) {
	int32_t idx = model->nodes[node].info.contains(info_key_u8);
	if (idx < 0) return false;

	sk_free(model->nodes[node].info.items[idx].value);
	model->nodes[node].info.remove_at(idx);

	return true;
}

///////////////////////////////////////////

void model_node_info_clear(model_t model, model_node_id node) {
	model->nodes[node].info.each([](char*& val) { sk_free(val); });
	model->nodes[node].info.free();
}

///////////////////////////////////////////

int32_t model_node_info_count(model_t model, model_node_id node) {
	return model->nodes[node].info.count;
}

///////////////////////////////////////////

bool32_t model_node_info_iterate(model_t model, model_node_id node, int32_t* ref_iterator, const char** out_key_utf8, const char** out_value_utf8) {
	dictionary_t<char*> *info = &model->nodes[node].info;

	if (*ref_iterator >= info->capacity) return false;
	while (info->items[*ref_iterator].hash == 0) {
		*ref_iterator = *ref_iterator + 1;
		if (*ref_iterator >= info->capacity) return false;
	}

	if (out_key_utf8   != nullptr) *out_key_utf8   = info->items[*ref_iterator].key;
	if (out_value_utf8 != nullptr) *out_value_utf8 = info->items[*ref_iterator].value;
	*ref_iterator = *ref_iterator + 1;

	return true;
}

///////////////////////////////////////////

void model_step_anim(model_t model) {
	anim_update_model(model);
}

///////////////////////////////////////////

bool32_t model_play_anim(model_t model, const char *animation_name, anim_mode_ mode) {
	int32_t idx = model_anim_find(model, animation_name);
	if (idx >= 0)
		model_play_anim_idx(model, idx, mode);
	return idx >= 0;
}

///////////////////////////////////////////

void model_play_anim_idx(model_t model, int32_t index, anim_mode_ mode) {
	anim_inst_play(model, index, mode);
}

///////////////////////////////////////////

void model_set_anim_time(model_t model, float time) {
	if (model->anim_inst.anim_id < 0)
		return;

	if (model->anim_inst.mode == anim_mode_manual) {
		float max_time = model->anim_data.anims[model->anim_inst.anim_id].duration;
		model->anim_inst.start_time = fmaxf(0, fminf(time, max_time));
	} else {
		model->anim_inst.start_time = time_totalf() - time;
	}
}

///////////////////////////////////////////

void model_set_anim_completion(model_t model, float percent) {
	if (model->anim_inst.anim_id < 0)
		return;
	model_set_anim_time(model, model->anim_data.anims[model->anim_inst.anim_id].duration * percent);
}

///////////////////////////////////////////

int32_t model_anim_find(model_t model, const char *animation_name) {
	for (int32_t i = 0; i < model->anim_data.anims.count; i++)
		if (string_eq(model->anim_data.anims[i].name, animation_name))
			return i;
	return -1;
}

///////////////////////////////////////////

int32_t model_anim_count(model_t model) {
	return model->anim_data.anims.count;
}

///////////////////////////////////////////

int32_t model_anim_active(model_t model) {
	return model->anim_inst.anim_id;
}

///////////////////////////////////////////

anim_mode_ model_anim_active_mode(model_t model) {
	return model->anim_inst.mode;
}

///////////////////////////////////////////

float model_anim_active_time(model_t model) {
	if (model->anim_inst.anim_id < 0)
		return 0;

	float max_time = model->anim_data.anims[model->anim_inst.anim_id].duration;
	switch (model->anim_inst.mode) {
	case anim_mode_manual: return fminf(                model->anim_inst.start_time, max_time);
	case anim_mode_once:   return fminf(time_totalf() - model->anim_inst.start_time, max_time);
	case anim_mode_loop:   return fmodf(time_totalf() - model->anim_inst.start_time, max_time);
	default:               return 0;
	}
}

///////////////////////////////////////////

float model_anim_active_completion(model_t model) {
	if (model->anim_inst.anim_id < 0)
		return 0;
	return model_anim_active_time(model) / model->anim_data.anims[model->anim_inst.anim_id].duration;
}

///////////////////////////////////////////

const char *model_anim_get_name(model_t model, int32_t index) {
	assert(index < model->anim_data.anims.count);
	return model->anim_data.anims[index].name;
}

///////////////////////////////////////////

float model_anim_get_duration(model_t model, int32_t index) {
	assert(index < model->anim_data.anims.count);
	return model->anim_data.anims[index].duration;
}

} // namespace sk

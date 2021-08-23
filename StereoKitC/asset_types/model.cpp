#define _CRT_SECURE_NO_WARNINGS 1

#include "../sk_math.h"
#include "../sk_memory.h"
#include "model.h"
#include "../libraries/stref.h"
#include "../systems/platform/platform_utils.h"

#include <stdio.h>
#include <string.h>

namespace sk {

///////////////////////////////////////////

model_t model_create() {
	model_t result = (_model_t*)assets_allocate(asset_type_model);
	return result;
}

///////////////////////////////////////////

void model_set_id(model_t model, const char *id) {
	assets_set_id(model->header, id);
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
	for (size_t i = 0; i < result->visuals.count; i++) {
		material_addref(result->visuals[i].material);
		mesh_addref    (result->visuals[i].mesh);
	}
	for (size_t i = 0; i < result->nodes.count; i++) {
		result->nodes[i].name = string_copy(result->nodes[i].name);
	}

	return result;
}

///////////////////////////////////////////

model_t model_create_mesh(mesh_t mesh, material_t material) {
	model_t result = model_create();

	model_add_subset(result, mesh, material, matrix_identity);

	return result;
}

///////////////////////////////////////////

model_t model_create_mem(const char *filename, void *data, size_t data_size, shader_t shader) {
	model_t result = model_create();
	
	if (string_endswith(filename, ".glb",  false) || 
		string_endswith(filename, ".gltf", false)) {
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

	void  *data;
	size_t length;
	if (!platform_read_file(assets_file(filename), &data, &length)) {
		log_warnf("Model file failed to load: %s", filename);
		return nullptr;
	}

	result = model_create_mem(filename, data, length, shader);
	if (result != nullptr) {
		model_set_id(result, filename);
	}
	
	free(data);
	return result;
}

///////////////////////////////////////////

void model_recalculate_bounds(model_t model) {
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
		for (int32_t i = 0; i < 8; i += 1) {
			vec3 corner = bounds_corner      (mesh_get_bounds(model->visuals[m].mesh), i);
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

const char *model_get_name(model_t model, int32_t subset) {
	assert(subset < model->visuals.count);
	return model_node_get_name(model, model->visuals[subset].node);
}

///////////////////////////////////////////

material_t model_get_material(model_t model, int32_t subset) {
	assert(subset < model->visuals.count);
	material_addref(model->visuals[subset].material);
	return model->visuals[subset].material;
}

///////////////////////////////////////////

mesh_t model_get_mesh(model_t model, int32_t subset) {
	assert(subset < model->visuals.count);
	mesh_addref(model->visuals[subset].mesh);
	return model->visuals[subset].mesh;
}

///////////////////////////////////////////

matrix model_get_transform(model_t model, int32_t subset) {
	assert(subset < model->visuals.count);
	return model->visuals[subset].transform_model;
}

///////////////////////////////////////////

void model_set_material(model_t model, int32_t subset, material_t material) {
	assert(subset < model->visuals.count);
	assert(material != nullptr);

	material_release(model->visuals[subset].material);
	model->visuals[subset].material = material;
	material_addref(model->visuals[subset].material);
}

///////////////////////////////////////////

void model_set_mesh(model_t model, int32_t subset, mesh_t mesh) {
	assert(subset < model->visuals.count);
	assert(mesh != nullptr);

	mesh_release(model->visuals[subset].mesh);
	model->visuals[subset].mesh = mesh;
	mesh_addref(model->visuals[subset].mesh);

	model_recalculate_bounds(model);
}

///////////////////////////////////////////

void model_set_transform(model_t model, int32_t subset, const matrix &transform) {
	assert(subset < model->visuals.count);
	model_node_set_transform_model(model, model->visuals[subset].node, transform);
}

///////////////////////////////////////////

int32_t model_subset_count(model_t model) {
	return model->visuals.count;
}

///////////////////////////////////////////

int32_t model_add_named_subset(model_t model, const char *name, mesh_t mesh, material_t material, const sk_ref(matrix) transform) {
	assert(model    != nullptr);
	assert(mesh     != nullptr);
	assert(material != nullptr);

	model_node_id id = model_node_add(model, nullptr, transform, mesh, material);
	return model->nodes[id].visual;
}

///////////////////////////////////////////

int32_t model_add_subset(model_t model, mesh_t mesh, material_t material, const matrix &transform) {
	model_node_id id = model_node_add(model, nullptr, transform, mesh, material);
	return model->nodes[id].visual;
}

///////////////////////////////////////////

void model_remove_subset(model_t model, int32_t subset) {
	assert(subset < model->visuals.count);

	model->nodes[model->visuals[subset].node].visual = -1;
	mesh_release    (model->visuals[subset].mesh);
	material_release(model->visuals[subset].material);
	model->visuals.remove(subset);

	for (size_t i = 0; i < model->nodes.count; i++) {
		if (model->nodes[i].visual > subset)
			model->nodes[i].visual--;
	}
}

///////////////////////////////////////////

void model_addref(model_t model) {
	assets_addref(model->header);
}

///////////////////////////////////////////

void model_release(model_t model) {
	if (model == nullptr)
		return;

	assets_releaseref(model->header);
}

///////////////////////////////////////////

void model_set_bounds(model_t model, const bounds_t &bounds) {
	model->bounds = bounds;
}

///////////////////////////////////////////

bounds_t model_get_bounds(model_t model) {
	return model->bounds;
}

///////////////////////////////////////////

void model_destroy(model_t model) {
	for (size_t i = 0; i < model->nodes.count; i++) {
		free(model->nodes[i].name);
	}
	for (size_t i = 0; i < model->visuals.count; i++) {
		mesh_release    (model->visuals[i].mesh);
		material_release(model->visuals[i].material);
	}
	model->nodes  .free();
	model->visuals.free();
	*model = {};
}

///////////////////////////////////////////

model_node_id model_node_add(model_t model, const char *name, matrix transform, mesh_t mesh, material_t material) {
	return model_node_add_child(model, -1, name, transform, mesh, material);
}

///////////////////////////////////////////

model_node_id model_node_add_child(model_t model, model_node_id parent, const char *name, matrix local_transform, mesh_t mesh, material_t material) {
	if ((mesh != nullptr && material == nullptr) ||
		(mesh == nullptr && material != nullptr)) {
		log_err("model_node_add_child: mesh and material must either both be null, or neither be null!");
		return -1;
	}

	model_node_id node_id = model->nodes.count;
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
		node.visual = model->visuals.add(visual);
		model_recalculate_bounds(model);
	}

	model->nodes.add(node);
	return node_id;
}

///////////////////////////////////////////

model_node_id model_node_find(model_t model, const char *name) {
	for (size_t i = 0; i < model->nodes.count; i++) {
		if (string_eq(model->nodes[i].name, name))
			return i;
	}
	return -1;
}

///////////////////////////////////////////

model_node_id model_node_next(model_t model, model_node_id node) {
	return model->nodes[node].sibling;
}

///////////////////////////////////////////

model_node_id model_node_parent(model_t model, model_node_id node) {
	return model->nodes[node].parent;
}

///////////////////////////////////////////

model_node_id model_node_child(model_t model, model_node_id node) {
	if (node < 0)
		return model->nodes.count > 0 ? 0 : -1;
	return model->nodes[node].child;
}

///////////////////////////////////////////

model_node_id model_node_iterate(model_t model, model_node_id node) {
	if (node == -1) return 0;

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

	// move on to see if there's another root node
	for (size_t i = node+1; i < model->nodes.count; i++) {
		if (model->nodes[i].parent == -1)
			return i;
	}

	return -1;
}

///////////////////////////////////////////

model_node_id model_node_get_root(model_t model) {
	return 0;
}

///////////////////////////////////////////

const char* model_node_get_name(model_t model, model_node_id node) {
	return model->nodes[node].name;
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

matrix model_node_get_transform(model_t model, model_node_id node) {
	return model->nodes[node].transform_model;
}

///////////////////////////////////////////

matrix model_node_get_transform_local(model_t model, model_node_id node) {
	return model->nodes[node].transform_local;
}

///////////////////////////////////////////

void model_node_set_name(model_t model, model_node_id node, const char* name) {
	free(model->nodes[node].name);
	char tmp_name[32];
	if (name == nullptr) {
		snprintf(tmp_name, sizeof(tmp_name), "node%d", node);
		name = tmp_name;
	}
	model->nodes[node].name = string_copy(name);
}

///////////////////////////////////////////

void model_node_set_material(model_t model, model_node_id node, material_t material) {
	int32_t vis = model->nodes[node].visual;
	if (vis < 0) {
		vis = model->visuals.add({});
		model->nodes[node].visual = vis;
	}
	material_release(model->visuals[vis].material);
	model->visuals[vis].material = material;
	material_addref(model->visuals[vis].material);
}

///////////////////////////////////////////

void model_node_set_mesh(model_t model, model_node_id node, mesh_t mesh) {
	int32_t vis = model->nodes[node].visual;
	if (vis < 0) {
		vis = model->visuals.add({});
		model->nodes[node].visual = vis;
	}
	mesh_release(model->visuals[vis].mesh);
	model->visuals[vis].mesh = mesh;
	mesh_addref(model->visuals[vis].mesh);

	model_recalculate_bounds(model);
}

///////////////////////////////////////////

void _model_node_update_transforms(model_t model, model_node_id node) {
	if (model->nodes[node].parent >= 0)
		model->nodes[node].transform_model = model->nodes[model->nodes[node].parent].transform_model * model->nodes[node].transform_local;
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
}

///////////////////////////////////////////

void model_node_set_transform_local(model_t model, model_node_id node, matrix transform_local_space) {
	model->nodes[node].transform_local = transform_local_space;
	_model_node_update_transforms(model, node);
}

} // namespace sk
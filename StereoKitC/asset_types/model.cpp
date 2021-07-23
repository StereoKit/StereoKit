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
	} else if (string_endswith(filename, ".fbx", false)) {
		if (!modelfmt_fbx (result, filename, data, data_size, shader))
			log_errf("Issue loading FBX file: %s!", filename);
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
	if (model->subset_count <= 0) {
		model->bounds = {};
		return;
	}
	
	// Get an initial size
	vec3 first_corner = bounds_corner(mesh_get_bounds(model->subsets[0].mesh), 0);
	vec3 min, max;
	min = max = matrix_transform_pt( model->subsets[0].offset, first_corner);
	
	// Find the corners for each bounding cube, and factor them in!
	for (int32_t m = 0; m < model->subset_count; m += 1) {
		for (int32_t i = 0; i < 8; i += 1) {
			vec3 corner = bounds_corner      (mesh_get_bounds(model->subsets[m].mesh), i);
			vec3 pt     = matrix_transform_pt(model->subsets[m].offset, corner);
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
	assert(subset < model->subset_count);
	return model->subsets[subset].name;
}

///////////////////////////////////////////

material_t model_get_material(model_t model, int32_t subset) {
	assert(subset < model->subset_count);
	material_addref(model->subsets[subset].material);
	return model->subsets[subset].material;
}

///////////////////////////////////////////

mesh_t model_get_mesh(model_t model, int32_t subset) {
	assert(subset < model->subset_count);
	mesh_addref(model->subsets[subset].mesh);
	return model->subsets[subset].mesh;
}

///////////////////////////////////////////

matrix model_get_transform(model_t model, int32_t subset) {
	assert(subset < model->subset_count);
	return model->subsets[subset].offset;
}

///////////////////////////////////////////

void model_set_material(model_t model, int32_t subset, material_t material) {
	assert(subset < model->subset_count);
	assert(material != nullptr);

	material_release(model->subsets[subset].material);
	model->subsets[subset].material = material;
	material_addref(model->subsets[subset].material);
}

///////////////////////////////////////////

void model_set_mesh(model_t model, int32_t subset, mesh_t mesh) {
	assert(subset < model->subset_count);
	assert(mesh != nullptr);

	mesh_release(model->subsets[subset].mesh);
	model->subsets[subset].mesh = mesh;
	mesh_addref(model->subsets[subset].mesh);

	model_recalculate_bounds(model);
}

///////////////////////////////////////////

void model_set_transform(model_t model, int32_t subset, const matrix &transform) {
	assert(subset < model->subset_count);
	model->subsets[subset].offset = transform;
}
///////////////////////////////////////////

int32_t model_subset_count(model_t model) {
	return model->subset_count;
}

///////////////////////////////////////////

int32_t model_add_named_subset(model_t model, const char *name, mesh_t mesh, material_t material, const sk_ref(matrix) transform) {
	assert(model    != nullptr);
	assert(mesh     != nullptr);
	assert(material != nullptr);

	char tmp_name[32];
	if (name == nullptr) {
		snprintf(tmp_name, sizeof(tmp_name), "subset%d", model->subset_count);
		name = tmp_name;
	}

	model->subsets                      = sk_realloc_t(model_subset_t, model->subsets, model->subset_count + 1);
	model->subsets[model->subset_count] = model_subset_t{ string_copy(name), mesh, material, transform };
	mesh_addref    (mesh);
	material_addref(material);

	model->subset_count += 1;
	model_recalculate_bounds(model);

	return model->subset_count - 1;
}

///////////////////////////////////////////

int32_t model_add_subset(model_t model, mesh_t mesh, material_t material, const matrix &transform) {
	char name[32];
	snprintf(name, sizeof(name), "subset%d", model->subset_count);
	return model_add_named_subset(model, name, mesh, material, transform);
}

///////////////////////////////////////////

void model_remove_subset(model_t model, int32_t subset) {
	assert(subset < model->subset_count);

	free            (model->subsets[subset].name);
	mesh_release    (model->subsets[subset].mesh);
	material_release(model->subsets[subset].material);
	if (subset < model->subset_count - 1) {
		memmove(
			&model->subsets[subset],
			&model->subsets[subset + 1],
			sizeof(model_subset_t) * (model->subset_count - (subset + 1)));
	}
	model->subset_count -= 1;
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
	for (size_t i = 0; i < model->subset_count; i++) {
		free            (model->subsets[i].name);
		mesh_release    (model->subsets[i].mesh);
		material_release(model->subsets[i].material);
	}
	free(model->subsets);
	*model = {};
}

///////////////////////////////////////////

} // namespace sk
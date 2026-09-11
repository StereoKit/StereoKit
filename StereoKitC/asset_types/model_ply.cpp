#include "model.h"
#include "../libraries/array.h"
#include "../libraries/stref.h"
#include "../sk_math.h"
#include "../sk_memory.h"

#define MICRO_PLY_IMPL
// Parse decimals with our own locale-independent parser instead of atof, which
// misparses '.' decimals under locales like fr_FR.UTF-8 (see string_to_float).
#define MICRO_PLY_ATOF(str) string_to_float(str)
#include "../libraries/micro_ply.h"

#include <stdio.h>

namespace sk {

struct simple_load_t {
	const void *file_data;
	size_t      file_size;
};

///////////////////////////////////////////

bool modelfmt_ply_build(const void *file_data, size_t file_size, const char *name, mesh_load_t *out_mesh) {
	ply_file_t file;
	if (!ply_read(file_data, file_size, &file)) {
		log_warnf("Couldn't parse PLY: %s", name);
		return false;
	}

	vert_t *verts      = nullptr;
	vind_t *inds       = nullptr;
	int32_t vert_count = 0;
	int32_t ind_count  = 0;

	float     fzero = 0;
	uint8_t   white = 255;
	ply_map_t map_verts[] = {
		{ PLY_PROP_POSITION_X,  ply_prop_decimal, sizeof(float), 0,  &fzero },
		{ PLY_PROP_POSITION_Y,  ply_prop_decimal, sizeof(float), 4,  &fzero },
		{ PLY_PROP_POSITION_Z,  ply_prop_decimal, sizeof(float), 8,  &fzero },
		{ PLY_PROP_NORMAL_X,    ply_prop_decimal, sizeof(float), 12, &fzero },
		{ PLY_PROP_NORMAL_Y,    ply_prop_decimal, sizeof(float), 16, &fzero },
		{ PLY_PROP_NORMAL_Z,    ply_prop_decimal, sizeof(float), 20, &fzero },
		{ PLY_PROP_TEXCOORD_X,  ply_prop_decimal, sizeof(float), 24, &fzero },
		{ PLY_PROP_TEXCOORD_Y,  ply_prop_decimal, sizeof(float), 28, &fzero },
		{ PLY_PROP_COLOR_R,     ply_prop_uint,    sizeof(uint8_t), 32, &white },
		{ PLY_PROP_COLOR_G,     ply_prop_uint,    sizeof(uint8_t), 33, &white },
		{ PLY_PROP_COLOR_B,     ply_prop_uint,    sizeof(uint8_t), 34, &white },
		{ PLY_PROP_COLOR_A,     ply_prop_uint,    sizeof(uint8_t), 35, &white },
		{ PLY_PROP_COLOR_DIFF_R,ply_prop_uint,    sizeof(uint8_t), 32, NULL },
		{ PLY_PROP_COLOR_DIFF_G,ply_prop_uint,    sizeof(uint8_t), 33, NULL },
		{ PLY_PROP_COLOR_DIFF_B,ply_prop_uint,    sizeof(uint8_t), 34, NULL },
		{ PLY_PROP_COLOR_DIFF_A,ply_prop_uint,    sizeof(uint8_t), 35, NULL }, };
	ply_convert(&file, PLY_ELEMENT_VERTICES, map_verts, sizeof(map_verts)/sizeof(map_verts[0]), sizeof(vert_t), (void**)&verts, &vert_count);

	uint32_t  izero      = 0;
	ply_map_t map_inds[] = { { PLY_PROP_INDICES, ply_prop_uint, sizeof(uint32_t), 0, &izero } };
	ply_convert(&file, PLY_ELEMENT_FACES, map_inds, sizeof(map_inds)/sizeof(map_inds[0]), sizeof(uint32_t), (void**)&inds, &ind_count);

	ply_free(&file);

	if (vert_count == 0) {
		log_warnf("PLY has no vertices: %s", name);
		sk_free(verts);
		sk_free(inds);
		return false;
	}

	// Point clouds have no faces, a degenerate triangle keeps the mesh valid.
	if (ind_count == 0) {
		inds      = sk_malloc_t(vind_t, 3);
		inds[0]   = inds[1] = inds[2] = 0;
		ind_count = 3;
	}

	out_mesh->verts      = verts;
	out_mesh->vert_count = vert_count;
	out_mesh->inds       = inds;
	out_mesh->ind_count  = ind_count;
	return true;
}

///////////////////////////////////////////

static mesh_t ply_load_mesh(const void *file_data, size_t file_length, const char *filename, int32_t priority) {
	char id[512];
	snprintf(id, sizeof(id), "%s/mesh", filename);
	mesh_t mesh = mesh_find(id);
	if (mesh) return mesh;

	mesh_load_t data = {};
	if (!modelfmt_ply_build(file_data, file_length, filename, &data))
		return nullptr;

	mesh = mesh_create();
	mesh_set_id  (mesh, id);
	mesh_set_data(mesh, (vert_t*)data.verts, data.vert_count, data.inds, data.ind_count, mesh_data_calc_bounds, priority);
	sk_free(data.verts);
	sk_free(data.inds);
	return mesh;
}

///////////////////////////////////////////

bool modelfmt_ply_metadata(model_t model, const char *filename, const void *file_data, size_t file_size, shader_t shader, int32_t, void **out_format_data) {
	material_t material = shader == nullptr
		? material_find(default_id_material)
		: material_create(shader);
		
	model_node_add(model, nullptr, matrix_identity, nullptr, material, true);
	material_release(material);

	simple_load_t *load = sk_malloc_zero_t(simple_load_t, 1);
	load->file_data = file_data;
	load->file_size = file_size;
	*out_format_data = load;
	return true;
}

///////////////////////////////////////////

bool modelfmt_ply_meshes(model_t model, const char *filename, shader_t, int32_t priority, void *format_data) {
	simple_load_t *load = (simple_load_t *)format_data;

	mesh_t mesh = ply_load_mesh(load->file_data, load->file_size, filename, priority);
	if (mesh) {
		model_node_set_mesh(model, 0, mesh);
		mesh_release(mesh);
	}
	return true;
}

///////////////////////////////////////////

void modelfmt_ply_free(void *format_data) {
	sk_free(format_data);
}

}
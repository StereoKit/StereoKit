#include "../stereokit.h"
#include "../sk_memory.h"
#include "model.h"
#include "mesh_svg.h"

#include <stdio.h>

namespace sk {

typedef struct svg_model_load_t {
	const void* file_data;
	size_t      file_size;
} svg_model_load_t;

///////////////////////////////////////////

bool modelfmt_svg_metadata(model_t model, const char *filename, const void *file_data, size_t file_size, shader_t shader, int32_t, void **out_format_data) {
	// Flat vector art reads best unlit, unless the caller has other plans.
	material_t material = shader == nullptr
		? material_copy_id(default_id_material_unlit)
		: material_create (shader);
	model_node_add(model, nullptr, matrix_identity, nullptr, material, true);
	material_release(material);

	svg_model_load_t *load = sk_malloc_zero_t(svg_model_load_t, 1);
	load->file_data = file_data;
	load->file_size = file_size;
	*out_format_data = load;
	return true;
}

///////////////////////////////////////////

bool modelfmt_svg_meshes(model_t model, const char *filename, shader_t, int32_t priority, void *format_data) {
	svg_model_load_t *load = (svg_model_load_t *)format_data;

	// Parsing still happens when the mesh already exists from Mesh.FromFile,
	// the material needs to know whether anything is translucent.
	mesh_load_t data        = {};
	bool32_t    translucent = false;
	if (!svg_build_mem(load->file_data, load->file_size, filename, svg_options_default, &data, &translucent))
		return false;

	char id[512];
	snprintf(id, sizeof(id), "%s/mesh", filename);
	mesh_t mesh = mesh_find(id);
	if (mesh == nullptr) {
		mesh = mesh_create();
		mesh_set_id  (mesh, id);
		mesh_set_data(mesh, (vert_t*)data.verts, data.vert_count, data.inds, data.ind_count, mesh_data_calc_bounds, priority);
	}
	sk_free(data.verts);
	sk_free(data.inds);

	model_node_set_mesh(model, 0, mesh);
	mesh_release(mesh);

	if (translucent) {
		material_t material = model_node_get_material(model, 0);
		material_set_transparency(material, transparency_blend);
		material_release(material);
	}
	return true;
}

///////////////////////////////////////////

void modelfmt_svg_free(void *format_data) {
	sk_free(format_data);
}

}

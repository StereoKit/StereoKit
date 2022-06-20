#include "model.h"
#include "../libraries/array.h"
#include "../sk_math.h"

#define MICRO_PLY_IMPL
#include "../libraries/micro_ply.h"

#include <stdio.h>

namespace sk {

///////////////////////////////////////////

bool modelfmt_ply(model_t model, const char *filename, void *file_data, size_t file_length, shader_t shader) {
	material_t material = shader == nullptr ? material_find(default_id_material) : material_create(shader);
	bool       result   = true;

	char id[512];
	snprintf(id, sizeof(id), "%s/mesh", filename);
	mesh_t mesh = mesh_find(id);

	if (mesh) {
		model_add_subset(model, mesh, material, matrix_identity);
	} else {

		// Parse the data using ply_read
		ply_file_t file;
		if (!ply_read(file_data, file_length, &file))
			return false;

		vert_t *verts = nullptr;
		vind_t *inds  = nullptr;
		int32_t vert_count = 0;
		int32_t ind_count  = 0;

		// Describe the way the contents of the PLY file map to our own vertex 
		// format. If the property can't be found in the file, the default value
		// will be assigned.
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
			{ PLY_PROP_COLOR_A,     ply_prop_uint,    sizeof(uint8_t), 35, &white }, };
		ply_convert(&file, PLY_ELEMENT_VERTICES, map_verts, sizeof(map_verts)/sizeof(map_verts[0]), sizeof(vert_t), (void**)&verts, &vert_count);

		// Properties defined as lists in the PLY format will get triangulated 
		// during conversion, so you don't need to worry about quads or n-gons in 
		// the geometry.
		uint32_t  izero = 0;
		ply_map_t map_inds[] = { { PLY_PROP_INDICES, ply_prop_uint, sizeof(uint32_t), 0, &izero } };
		ply_convert(&file, PLY_ELEMENT_FACES, map_inds, sizeof(map_inds)/sizeof(map_inds[0]), sizeof(uint32_t), (void**)&inds, &ind_count);

		// You gotta free the memory manually!
		ply_free(&file);

		// Make a mesh out of it all
		mesh = mesh_create();
		mesh_set_id  (mesh, id);
		mesh_set_data(mesh, verts, vert_count, inds, ind_count);

		model_add_subset(model, mesh, material, matrix_identity);

		sk_free(verts);
		sk_free(inds);
	}

	mesh_release    (mesh);
	material_release(material);

	return result;
}

}
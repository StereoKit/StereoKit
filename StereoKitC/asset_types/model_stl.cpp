#include "model.h"
#include "../libraries/stref.h"
#include "../libraries/array.h"
#include "../sk_math.h"
#include "../sk_memory.h"

#include <stdio.h>

namespace sk {

struct simple_load_t {
	const void *file_data;
	size_t      file_size;
};

struct stl_header_t {
	uint8_t  header[80];
	uint32_t tri_count;
};

#pragma pack(1) 
struct stl_triangle_t {
	vec3     normal;
	vec3     verts[3];
	uint16_t attribute;
};

///////////////////////////////////////////

vind_t indexof(vec3 pt, vec3 normal, array_t<vert_t> *verts, hashmap_t<vec3, vind_t> *indmap) {
	vind_t  result = 0;
	int32_t id     = indmap->contains(pt);
	if (id < 0) {
		result = (vind_t)verts->add(vert_t{ pt, {}, {}, {255,255,255,255} });
		indmap->set(pt, result);
	} else {
		result = indmap->items[id].value;
	}
	verts->get(result).norm += normal;
	return result;
}

///////////////////////////////////////////

bool modelfmt_stl_binary_smooth(const void *file_data, size_t, array_t<vert_t> *verts, array_t<vind_t> *faces) {
	stl_header_t *header = (stl_header_t *)file_data;
	hashmap_t<vec3, vind_t> indmap = {};

	stl_triangle_t *tris = (stl_triangle_t *)(((uint8_t *)file_data) + sizeof(stl_header_t));
	for (uint32_t i = 0; i < header->tri_count; i++) {
		faces->add(indexof(tris[i].verts[0], tris[i].normal, verts, &indmap));
		faces->add(indexof(tris[i].verts[1], tris[i].normal, verts, &indmap));
		faces->add(indexof(tris[i].verts[2], tris[i].normal, verts, &indmap));
	}

	indmap.free();
	return true;
}

///////////////////////////////////////////

bool modelfmt_stl_text_smooth(const void *file_data, size_t, array_t<vert_t> *verts, array_t<vind_t> *faces) {
	hashmap_t<vec3, vind_t> indmap = {};
	
	vec3    normal     = {};
	vind_t  curr[4]    = {};
	int32_t curr_count = 0;

	stref_t data = stref_make((const char *)file_data);
	stref_t line = {};
	while (stref_nextline(data, line)) {
		stref_t word = {};
		if (!stref_nextword(line, word))
			continue;

		if (stref_equals(word, "facet")) {
			if (stref_nextword(line, word) && stref_equals(word, "normal")) {
				normal = {};
				if (stref_nextword(line, word)) normal.x = stref_to_f(word);
				if (stref_nextword(line, word)) normal.y = stref_to_f(word);
				if (stref_nextword(line, word)) normal.z = stref_to_f(word);
			}
		} else if (stref_equals(word, "endfacet")) {
			faces->add(curr[0]); faces->add(curr[1]); faces->add(curr[2]);
			if (curr_count == 4) {
				faces->add(curr[0]); faces->add(curr[2]); faces->add(curr[3]);
			}
			curr_count = 0;
		} else if (stref_equals(word, "vertex")) {
			if (curr_count != 4) {
				vec3 pt = {};
				if (stref_nextword(line, word)) pt.x = stref_to_f(word);
				if (stref_nextword(line, word)) pt.y = stref_to_f(word);
				if (stref_nextword(line, word)) pt.z = stref_to_f(word);
				curr[curr_count] = indexof(pt, normal, verts, &indmap);
				curr_count = mini(4, curr_count + 1);
			}
		}
	}

	indmap.free();
	return true;
}

///////////////////////////////////////////

bool modelfmt_stl_binary_flat(const void *file_data, size_t, array_t<vert_t> *verts, array_t<vind_t> *faces) {
	stl_header_t   *header = (stl_header_t *)file_data;
	stl_triangle_t *tris   = (stl_triangle_t *)(((uint8_t *)file_data) + sizeof(stl_header_t));

	verts->resize(header->tri_count * 3);
	faces->resize(header->tri_count * 3);
	for (uint32_t i = 0; i < header->tri_count; i++) {
		vind_t ind1 = (vind_t)verts->add(vert_t{ tris[i].verts[0], tris[i].normal, {}, {255,255,255,255} });
		vind_t ind2 = (vind_t)verts->add(vert_t{ tris[i].verts[1], tris[i].normal, {}, {255,255,255,255} });
		vind_t ind3 = (vind_t)verts->add(vert_t{ tris[i].verts[2], tris[i].normal, {}, {255,255,255,255} });
		
		faces->add(ind1);
		faces->add(ind2);
		faces->add(ind3);
	}
	return true;
}

///////////////////////////////////////////

bool modelfmt_stl_text_flat(const void *file_data, size_t, array_t<vert_t> *verts, array_t<vind_t> *faces) {
	vec3    normal     = {};
	vec3    curr[4]    = {};
	int32_t curr_count = 0;

	stref_t data = stref_make((const char *)file_data);
	stref_t line = {};
	while (stref_nextline(data, line)) {
		stref_t word = {};
		if (!stref_nextword(line, word))
			continue;

		if (stref_equals(word, "facet")) {
			if (stref_nextword(line, word) && stref_equals(word, "normal")) {
				normal = {};
				if (stref_nextword(line, word)) normal.x = stref_to_f(word);
				if (stref_nextword(line, word)) normal.y = stref_to_f(word);
				if (stref_nextword(line, word)) normal.z = stref_to_f(word);
			}
		} else if (stref_equals(word, "endfacet")) {
			vind_t ind1 = (vind_t)verts->add(vert_t{ curr[0], normal, {}, {255,255,255,255} });
			vind_t ind2 = (vind_t)verts->add(vert_t{ curr[1], normal, {}, {255,255,255,255} });
			vind_t ind3 = (vind_t)verts->add(vert_t{ curr[2], normal, {}, {255,255,255,255} });

			faces->add(ind1); faces->add(ind2); faces->add(ind3);
			if (curr_count == 4) {
				vind_t ind4 = (vind_t)verts->add(vert_t{ curr[3], normal, {}, {255,255,255,255} });
				faces->add(ind1); faces->add(ind3); faces->add(ind4);
			}
			curr_count = 0;
		} else if (stref_equals(word, "vertex")) {
			if (curr_count != 4) {
				vec3 pt = {};
				if (stref_nextword(line, word)) pt.x = stref_to_f(word);
				if (stref_nextword(line, word)) pt.y = stref_to_f(word);
				if (stref_nextword(line, word)) pt.z = stref_to_f(word);
				curr[curr_count] = pt;
				curr_count = mini(4, curr_count + 1);
			}
		}
	}

	return true;
}

///////////////////////////////////////////

bool modelfmt_stl_build(const void *file_data, size_t file_size, const char *name, mesh_load_t *out_mesh) {
	array_t<vert_t> verts = {};
	array_t<vind_t> faces = {};

	bool ok = file_size > 5 && memcmp(file_data, "solid", sizeof(char) * 5) == 0
		? modelfmt_stl_text_flat  (file_data, file_size, &verts, &faces)
		: modelfmt_stl_binary_flat(file_data, file_size, &verts, &faces);
	if (!ok || verts.count == 0) {
		log_warnf("Couldn't parse STL: %s", name);
		verts.free();
		faces.free();
		return false;
	}

	for (int32_t i = 0; i < verts.count; i++)
		verts[i].norm = vec3_normalize(verts[i].norm);

	out_mesh->verts      = verts.data;
	out_mesh->vert_count = verts.count;
	out_mesh->inds       = faces.data;
	out_mesh->ind_count  = faces.count;
	return true;
}

///////////////////////////////////////////

static mesh_t stl_load_mesh(const void *file_data, size_t file_length, const char *filename, int32_t priority) {
	char id[512];
	snprintf(id, sizeof(id), "%s/mesh", filename);
	mesh_t mesh = mesh_find(id);
	if (mesh) return mesh;

	mesh_load_t data = {};
	if (!modelfmt_stl_build(file_data, file_length, filename, &data))
		return nullptr;

	mesh = mesh_create();
	mesh_set_id  (mesh, id);
	mesh_set_data(mesh, (vert_t*)data.verts, data.vert_count, data.inds, data.ind_count, mesh_data_calc_bounds, priority);
	sk_free(data.verts);
	sk_free(data.inds);
	return mesh;
}

///////////////////////////////////////////

bool modelfmt_stl_metadata(model_t model, const char *filename, const void *file_data, size_t file_size, shader_t shader, int32_t, void **out_format_data) {
	material_t material = shader == nullptr
		? material_find  (default_id_material)
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

bool modelfmt_stl_meshes(model_t model, const char *filename, shader_t, int32_t priority, void *format_data) {
	simple_load_t *load = (simple_load_t *)format_data;

	mesh_t mesh = stl_load_mesh(load->file_data, load->file_size, filename, priority);
	if (mesh) {
		model_node_set_mesh(model, 0, mesh);
		mesh_release(mesh);
	}
	return true;
}

///////////////////////////////////////////

void modelfmt_stl_free(void *format_data) {
	sk_free(format_data);
}

}
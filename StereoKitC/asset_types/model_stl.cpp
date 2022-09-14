#include "model.h"
#include "../libraries/stref.h"
#include "../libraries/array.h"
#include "../sk_math.h"

#include <stdio.h>

namespace sk {

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

bool modelfmt_stl_binary_smooth(void *file_data, size_t, array_t<vert_t> *verts, array_t<vind_t> *faces) {
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

bool modelfmt_stl_text_smooth(void *file_data, size_t, array_t<vert_t> *verts, array_t<vind_t> *faces) {
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

bool modelfmt_stl_binary_flat(void *file_data, size_t, array_t<vert_t> *verts, array_t<vind_t> *faces) {
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

bool modelfmt_stl_text_flat(void *file_data, size_t, array_t<vert_t> *verts, array_t<vind_t> *faces) {
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

bool modelfmt_stl(model_t model, const char *filename, void *file_data, size_t file_length, shader_t shader) {
	material_t material = shader == nullptr ? material_find(default_id_material) : material_create(shader);
	bool       result   = true;

	char id[512];
	snprintf(id, sizeof(id), "%s/mesh", filename);
	mesh_t mesh = mesh_find(id);

	if (mesh) {
		model_add_subset(model, mesh, material, matrix_identity);
	} else {
		array_t<vert_t> verts = {};
		array_t<vind_t> faces = {};

		result = file_length > 5 && memcmp(file_data, "solid", sizeof(char) * 5) == 0 ?
			modelfmt_stl_text_flat  (file_data, file_length, &verts, &faces) :
			modelfmt_stl_binary_flat(file_data, file_length, &verts, &faces);

		// Normalize all the normals
		for (int32_t i = 0; i < verts.count; i++)
			verts[i].norm = vec3_normalize(verts[i].norm);

		mesh = mesh_create();
		mesh_set_id  (mesh, id);
		mesh_set_data(mesh, &verts[0], verts.count, &faces[0], faces.count);

		model_add_subset(model, mesh, material, matrix_identity);

		verts.free();
		faces.free();
	}

	mesh_release    (mesh);
	material_release(material);

	return result;
}

}
#include "model.h"
#include "../libraries/stref.h"
#include "../libraries/stb_ds.h"
#include "../math.h"

#include <stdio.h>

namespace sk {

struct stl_header_t {
	uint8_t header[80];
	uint32_t tri_count;
};

#pragma pack(1) 
struct stl_triangle_t {
	vec3 normal;
	vec3 verts[3];
	uint16_t attribute;
};

struct point_hash_t {
	vec3   key;
	vind_t value;
};

///////////////////////////////////////////

vind_t indexof(vec3 pt, vec3 normal, vert_t **verts, point_hash_t **indmap) {
	vind_t result = hmget(*indmap, pt);
	if (result == -1) {
		result = (vind_t)arrlen(*verts);
		hmput(*indmap, pt, result);
		arrput(*verts, (vert_t{ pt, {}, {}, {255,255,255,255} }));
	}
	(*verts)[result].norm += normal;
	return result;
}

///////////////////////////////////////////

bool modelfmt_stl_binary(void *file_data, size_t, vert_t **verts, vind_t **faces) {
	stl_header_t *header = (stl_header_t *)file_data;
	point_hash_t *indmap = nullptr;
	hmdefault(indmap, (vind_t)-1);

	stl_triangle_t *tris = (stl_triangle_t *)(((uint8_t *)file_data) + sizeof(stl_header_t));
	for (uint32_t i = 0; i < header->tri_count; i++) {
		arrput(*faces, indexof(tris[i].verts[0], tris[i].normal, verts, &indmap) );
		arrput(*faces, indexof(tris[i].verts[1], tris[i].normal, verts, &indmap) );
		arrput(*faces, indexof(tris[i].verts[2], tris[i].normal, verts, &indmap) );
	}

	hmfree(indmap);
	return true;
}

///////////////////////////////////////////

bool modelfmt_stl_text(void *file_data, size_t, vert_t **verts, vind_t **faces) {
	point_hash_t *indmap = nullptr;
	hmdefault(indmap, (vind_t)-1);
	
	vec3    normal = {};
	vind_t  curr[4] = {};
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
			arrput(*faces, curr[0]); arrput(*faces, curr[1]); arrput(*faces, curr[2]);
			if (curr_count == 4) {
				arrput(*faces, curr[0]); arrput(*faces, curr[2]); arrput(*faces, curr[3]);
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

	hmfree(indmap);
	return true;
}

///////////////////////////////////////////

bool modelfmt_stl(model_t model, const char *filename, void *file_data, size_t file_length, shader_t shader) {
	vert_t *verts = nullptr;
	vind_t *faces = nullptr;

	bool result = file_length > 5 && memcmp(file_data, "solid", sizeof(char) * 5) == 0 ?
		modelfmt_stl_text  (file_data, file_length, &verts, &faces) :
		modelfmt_stl_binary(file_data, file_length, &verts, &faces);

	// Normalize all the normals
	for (int i = 0, len = (int)arrlen(verts); i < len; i++)
		verts[i].norm = vec3_normalize(verts[i].norm);

	char id[512];
	snprintf(id, sizeof(id), "%s/mesh", filename);
	mesh_t mesh = mesh_create();
	mesh_set_id   (mesh, id);
	mesh_set_verts(mesh, &verts[0], (int)arrlen(verts));
	mesh_set_inds (mesh, &faces[0], (int)arrlen(faces));

	model_add_subset(model, mesh, shader == nullptr ? material_find(default_id_material) : material_create(shader), matrix_identity);

	mesh_release(mesh);

	arrfree(verts);
	arrfree(faces);
	return result;
}

}
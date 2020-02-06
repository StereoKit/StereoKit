#include "model.h"
#include "../libraries/stref.h"
#include "../math.h"

#include <vector>
#include <map>
using namespace std;

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

struct vec3_cmp {
	bool operator() (const vec3& a, const vec3& b) const {
		return (a.x+a.y+a.z < b.x+b.y+b.z);
	}
};

///////////////////////////////////////////

vind_t indexof(vec3 pt, vec3 normal, vector<vert_t> &verts, map<vec3, vind_t, vec3_cmp> &indmap) {
	//verts.push_back(vert_t{ pt, normal, {}, {255,255,255,255} });
	//return verts.size() - 1;
	auto find = indmap.find(pt);
	vind_t result = 0;
	if (find == indmap.end()) {
		result = (vind_t)verts.size();
		indmap[pt] = result;
		verts.push_back(vert_t{ pt, {}, {}, {255,255,255,255} });
	} else {
		result = find->second;
	}
	verts[result].norm += normal;
	return result;
}

///////////////////////////////////////////

bool modelfmt_stl_binary(void *file_data, size_t file_length, vector<vert_t> &verts, vector<vind_t> &faces) {
	stl_header_t *header = (stl_header_t *)file_data;
	map<vec3, vind_t, vec3_cmp> indmap;
	
	size_t sz = sizeof(stl_triangle_t);
	stl_triangle_t *tris = (stl_triangle_t *)(((uint8_t *)file_data) + sizeof(stl_header_t));
	for (uint32_t i = 0; i < header->tri_count; i++) {
		faces.push_back( indexof(tris[i].verts[0], tris[i].normal, verts, indmap) );
		faces.push_back( indexof(tris[i].verts[1], tris[i].normal, verts, indmap) );
		faces.push_back( indexof(tris[i].verts[2], tris[i].normal, verts, indmap) );
	}

	return true;
}

///////////////////////////////////////////

bool modelfmt_stl_text(void *file_data, size_t file_length, vector<vert_t> &verts, vector<vind_t> &faces) {
	map<vec3, vind_t, vec3_cmp> indmap;
	
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
			faces.push_back(curr[0]); faces.push_back(curr[1]); faces.push_back(curr[2]);
			if (curr_count == 4) {
				faces.push_back(curr[0]); faces.push_back(curr[2]); faces.push_back(curr[3]);
			}
			curr_count = 0;
		} else if (stref_equals(word, "vertex")) {
			if (curr_count != 4) {
				vec3 pt = {};
				if (stref_nextword(line, word)) pt.x = stref_to_f(word);
				if (stref_nextword(line, word)) pt.y = stref_to_f(word);
				if (stref_nextword(line, word)) pt.z = stref_to_f(word);
				curr[curr_count] = indexof(pt, normal, verts, indmap);
				curr_count = mini(4, curr_count + 1);
			}
		}
	}

	return true;
}

///////////////////////////////////////////

bool modelfmt_stl(model_t model, const char *filename, void *file_data, size_t file_length, shader_t shader) {
	vector<vert_t> verts;
	vector<vind_t> faces;

	bool result = file_length > 5 && memcmp(file_data, "solid", sizeof(char) * 5) == 0 ?
		modelfmt_stl_text  (file_data, file_length, verts, faces) :
		modelfmt_stl_binary(file_data, file_length, verts, faces);

	// Normalize all the normals
	for (size_t i = 0; i < verts.size(); i++)
		verts[i].norm = vec3_normalize(verts[i].norm);

	char id[512];
	sprintf_s(id, 512, "%s/mesh", filename);
	mesh_t mesh = mesh_create();
	mesh_set_id   (mesh, id);
	mesh_set_verts(mesh, &verts[0], (int32_t)verts.size());
	mesh_set_inds (mesh, &faces[0], (int32_t)faces.size());

	model_add_subset(model, mesh, shader == nullptr ? material_find("default/material") : material_create(shader), matrix_identity);

	mesh_release(mesh);

	return result;
}

}
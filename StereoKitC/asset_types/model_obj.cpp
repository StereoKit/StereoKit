#include "model.h"
#include "../libraries/stref.h"

#include <vector>
#include <map>
using namespace std;

namespace sk {

///////////////////////////////////////////

inline int meshfmt_obj_idx(int i1, int i2, int i3, int vSize, int nSize) {
	return i1 + (vSize+1) * (i2 + (nSize+1) * i3);
}

///////////////////////////////////////////

int indexof(int iV, int iT, int iN, vector<vec3> &verts, vector<vec3> &norms, vector<vec2> &uvs, map<int, vind_t> indmap, vector<vert_t> &mesh_verts) {
	if (uvs.size() == 0)
		uvs.push_back(vec2{ 0,0 });
	if (norms.size() == 0)
		norms.push_back(vec3{ 0,1,0 });

	int  id = meshfmt_obj_idx(iV, iN, iT, (int)verts.size(), (int)norms.size());
	map<int, vind_t>::iterator item = indmap.find(id);
	if (item == indmap.end()) {
		mesh_verts.push_back({ verts[iV - 1LL], norms[iN - 1LL], uvs[iT - 1LL], {255,255,255,255} });
		indmap[id] = (vind_t)(mesh_verts.size() - 1);
		return (int)mesh_verts.size() - 1;
	}
	return item->second;
}

///////////////////////////////////////////

bool modelfmt_obj(model_t model, const char *filename, void *file_data, size_t file_length, shader_t shader) {

	vector<vec3> poss;
	vector<vec3> norms;
	vector<vec2> uvs;

	map<int, vind_t> indmap;
	vector<vert_t>   verts;
	vector<vind_t>   faces;

	vec3 in;
	int inds[12];

	stref_t data = stref_make((const char *)file_data);
	stref_t line = {};
	while (stref_nextline(data, line)) {
		stref_t word = {};
		if (!stref_nextword(line, word))
			continue;

		if        (stref_equals(word, "v" )) {
			in = {};
			if (stref_nextword(line, word)) in.x = stref_to_f(word);
			if (stref_nextword(line, word)) in.y = stref_to_f(word);
			if (stref_nextword(line, word)) in.z = stref_to_f(word);
			poss.push_back(in);
		} else if (stref_equals(word, "vn")) {
			in = {};
			if (stref_nextword(line, word)) in.x = stref_to_f(word);
			if (stref_nextword(line, word)) in.y = stref_to_f(word);
			if (stref_nextword(line, word)) in.z = stref_to_f(word);
			norms.push_back(in);
		} else if (stref_equals(word, "vt")) {
			in = {};
			if (stref_nextword(line, word)) in.x = stref_to_f(word);
			if (stref_nextword(line, word)) in.y = stref_to_f(word);
			uvs.push_back(vec2{ in.x, in.y });
		} else if (stref_equals(word, "f" )) {
			int sides = 0;
			for (; sides < 4; sides++) {
				if (!stref_nextword(line, word))
					break;
				stref_t ind = {};
				inds[sides * 3  ] = stref_nextword(word, ind, '/') && ind.length > 0 ? stref_to_i(ind) : 1;
				inds[sides * 3+1] = stref_nextword(word, ind, '/') && ind.length > 0 ? stref_to_i(ind) : 1;
				inds[sides * 3+2] = stref_nextword(word, ind, '/') && ind.length > 0 ? stref_to_i(ind) : 1;
			}

			vind_t id1 = (vind_t)indexof(inds[0], inds[1],  inds[2],  poss, norms, uvs, indmap, verts);
			vind_t id2 = (vind_t)indexof(inds[3], inds[4],  inds[5],  poss, norms, uvs, indmap, verts);
			vind_t id3 = (vind_t)indexof(inds[6], inds[7],  inds[8],  poss, norms, uvs, indmap, verts);
			faces.push_back(id1); faces.push_back(id2); faces.push_back(id3);
			if (sides == 4) {
				vind_t id4 = (vind_t)indexof(inds[9], inds[10], inds[11], poss, norms, uvs, indmap, verts);
				faces.push_back(id1); faces.push_back(id3); faces.push_back(id4);
			}
		}
	}

	char id[512];
	sprintf_s(id, 512, "%s/mesh", filename);
	mesh_t mesh = mesh_create();
	mesh_set_id   (mesh, id);
	mesh_set_verts(mesh, &verts[0], (int32_t)verts.size());
	mesh_set_inds (mesh, &faces[0], (int32_t)faces.size());

	model_add_subset(model, mesh, shader == nullptr ? material_find("default/material") : material_create(shader), matrix_identity);

	mesh_release(mesh);

	return true;
}

}
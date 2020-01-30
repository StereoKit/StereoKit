#include "model.h"

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

	// Parse the file
	size_t len = file_length;
	const char *line = (const char *)file_data;
	int         read = 0;

	vector<vec3> poss;
	vector<vec3> norms;
	vector<vec2> uvs;

	map<int, vind_t> indmap;
	vector<vert_t>   verts;
	vector<vind_t> faces;

	vec3 in;
	int inds[12];
	int count = 0;

	while ((size_t)(line-(const char *)file_data)+1 < len) {
		if        (sscanf_s(line, "v %f %f %f\n%n",  &in.x, &in.y, &in.z, &read) > 0) {
			poss.push_back(in);
		} else if (sscanf_s(line, "vn %f %f %f\n%n", &in.x, &in.y, &in.z, &read) > 0) {
			norms.push_back(in);
		} else if (sscanf_s(line, "vt %f %f\n%n",    &in.x, &in.y,        &read) > 0) {
			vec2 uv = { in.x, in.y };
			uvs.push_back(uv);
		} else if ((count = sscanf_s(line, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n%n", &inds[0], &inds[1], &inds[2],&inds[3],&inds[4],&inds[5],&inds[6],&inds[7],&inds[8],&inds[9],&inds[10],&inds[11],  &read)) > 0) {
			vind_t id1 = (vind_t)indexof(inds[0], inds[1],  inds[2],  poss, norms, uvs, indmap, verts);
			vind_t id2 = (vind_t)indexof(inds[3], inds[4],  inds[5],  poss, norms, uvs, indmap, verts);
			vind_t id3 = (vind_t)indexof(inds[6], inds[7],  inds[8],  poss, norms, uvs, indmap, verts);
			faces.push_back(id1); faces.push_back(id2); faces.push_back(id3);
			if (count > 9) {
				vind_t id4 = (vind_t)indexof(inds[9], inds[10], inds[11], poss, norms, uvs, indmap, verts);
				faces.push_back(id1); faces.push_back(id3); faces.push_back(id4);
			}
		} else {
			char str[512];
			sscanf_s(line, "%[^\n]\n%n", str, 512, &read);
		}
		line += read;
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
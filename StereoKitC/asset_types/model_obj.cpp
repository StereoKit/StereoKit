#include "model.h"
#include "../libraries/stref.h"
#include "../libraries/stb_ds.h"

#include <stdio.h>

namespace sk {

struct index_hash_t {
	int    key;
	vind_t value;
};

///////////////////////////////////////////

inline int meshfmt_obj_idx(int i1, int i2, int i3, int vSize, int nSize) {
	return i1 + (vSize+1) * (i2 + (nSize+1) * i3);
}

///////////////////////////////////////////

int indexof(int iV, int iT, int iN, vec3 *verts, vec3 **norms, vec2 **uvs, index_hash_t **indmap, vert_t **mesh_verts) {
	if (arrlen(*uvs) == 0)
		arrput(*uvs, (vec2{ 0,0 }));
	if (arrlen(*norms) == 0)
		arrput(*norms, (vec3{ 0,1,0 }));

	int  id = meshfmt_obj_idx(iV, iN, iT, (int)arrlen(verts), (int)arrlen(norms));
	vind_t ind = hmget(*indmap, id);
	if (ind == -1) {
		arrput(*mesh_verts, (vert_t{ verts[iV - 1LL], (*norms)[iN - 1LL], (*uvs)[iT - 1LL], {255,255,255,255} }) );
		ind = (int)arrlen(*mesh_verts) - 1;
		hmput(*indmap, id, ind);
		
	}
	return ind;
}

///////////////////////////////////////////

bool modelfmt_obj(model_t model, const char *filename, void *file_data, size_t, shader_t shader) {
	vec3 *poss  = nullptr;
	vec3 *norms = nullptr;
	vec2 *uvs   = nullptr;
	vert_t *verts = nullptr;
	vind_t *faces = nullptr;

	index_hash_t  *indmap = nullptr;
	hmdefault(indmap, (vind_t)-1);
	
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
			arrput(poss, in);
		} else if (stref_equals(word, "vn")) {
			in = {};
			if (stref_nextword(line, word)) in.x = stref_to_f(word);
			if (stref_nextword(line, word)) in.y = stref_to_f(word);
			if (stref_nextword(line, word)) in.z = stref_to_f(word);
			arrput(norms, in);
		} else if (stref_equals(word, "vt")) {
			in = {};
			if (stref_nextword(line, word)) in.x = stref_to_f(word);
			if (stref_nextword(line, word)) in.y = stref_to_f(word);
			arrput(uvs, (vec2{ in.x, in.y }));
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

			vind_t id1 = (vind_t)indexof(inds[0], inds[1],  inds[2],  poss, &norms, &uvs, &indmap, &verts);
			vind_t id2 = (vind_t)indexof(inds[3], inds[4],  inds[5],  poss, &norms, &uvs, &indmap, &verts);
			vind_t id3 = (vind_t)indexof(inds[6], inds[7],  inds[8],  poss, &norms, &uvs, &indmap, &verts);
			arrput(faces, id1); arrput(faces, id2); arrput(faces, id3);
			if (sides == 4) {
				vind_t id4 = (vind_t)indexof(inds[9], inds[10], inds[11], poss, &norms, &uvs, &indmap, &verts);
				arrput(faces, id1); arrput(faces, id3); arrput(faces, id4);
			}
		}
	}

	char id[512];
	sprintf_s(id, 512, "%s/mesh", filename);
	mesh_t mesh = mesh_create();
	mesh_set_id   (mesh, id);
	mesh_set_verts(mesh, &verts[0], (int)arrlen(verts));
	mesh_set_inds (mesh, &faces[0], (int)arrlen(faces));

	model_add_subset(model, mesh, shader == nullptr ? material_find("default/material") : material_create(shader), matrix_identity);

	mesh_release(mesh);

	arrfree(poss);
	arrfree(norms);
	arrfree(uvs);
	arrfree(verts);
	arrfree(faces);
	hmfree(indmap);
	return true;
}

}
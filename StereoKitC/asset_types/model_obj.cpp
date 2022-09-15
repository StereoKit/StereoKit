#include "model.h"
#include "../libraries/stref.h"
#include "../libraries/array.h"

#include <stdio.h>

namespace sk {

///////////////////////////////////////////

int indexof(int iV, int iT, int iN, array_t<vec3> *verts, array_t<vec3> *norms, array_t<vec2> *uvs, hashmap_t<vert_t, vind_t> *indmap, array_t<vert_t> *mesh_verts) {
	if (uvs  ->count == 0) uvs  ->add(vec2{ 0,0 }  );
	if (norms->count == 0) norms->add(vec3{ 0,1,0 });
	vert_t v = vert_t{ verts->get(iV - 1LL), norms->get(iN - 1LL), uvs->get(iT - 1LL), {255,255,255,255} };

	int32_t index = indmap->contains(v);
	if (index < 0) {
		index = mesh_verts->add(v);
		indmap->set(v, (vind_t)index);
	} else {
		index = indmap->items[index].value;
	}
	return (int)index;
}

///////////////////////////////////////////

bool modelfmt_obj(model_t model, const char *filename, void *file_data, size_t, shader_t shader) {
	material_t material = shader == nullptr ? material_find(default_id_material) : material_create(shader);
	char id[512];
	snprintf(id, sizeof(id), "%s/mesh", filename);
	mesh_t mesh = mesh_find(id);

	if (mesh) {
		model_add_subset(model, mesh, material, matrix_identity);

		material_release(material);
		mesh_release(mesh);
		return true;
	}

	array_t<vec3>   poss  = {};
	array_t<vec3>   norms = {};
	array_t<vec2>   uvs   = {};
	array_t<vert_t> verts = {};
	array_t<vind_t> faces = {};
	hashmap_t<vert_t, vind_t> indmap = {};
	
	vec3 in;
	int inds[12] = {};

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
			poss.add(in);
		} else if (stref_equals(word, "vn")) {
			in = {};
			if (stref_nextword(line, word)) in.x = stref_to_f(word);
			if (stref_nextword(line, word)) in.y = stref_to_f(word);
			if (stref_nextword(line, word)) in.z = stref_to_f(word);
			norms.add(in);
		} else if (stref_equals(word, "vt")) {
			in = {};
			if (stref_nextword(line, word)) in.x = stref_to_f(word);
			if (stref_nextword(line, word)) in.y = stref_to_f(word);
			uvs.add(vec2{ in.x, in.y });
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

			vind_t id1 = (vind_t)indexof(inds[0], inds[1],  inds[2], &poss, &norms, &uvs, &indmap, &verts);
			vind_t id2 = (vind_t)indexof(inds[3], inds[4],  inds[5], &poss, &norms, &uvs, &indmap, &verts);
			vind_t id3 = (vind_t)indexof(inds[6], inds[7],  inds[8], &poss, &norms, &uvs, &indmap, &verts);
			faces.add(id1);
			faces.add(id2);
			faces.add(id3);
			if (sides == 4) {
				vind_t id4 = (vind_t)indexof(inds[9], inds[10], inds[11], &poss, &norms, &uvs, &indmap, &verts);
				faces.add(id1);
				faces.add(id3);
				faces.add(id4);
			}
		}
	}

	mesh = mesh_create();
	mesh_set_id  (mesh, id);
	mesh_set_data(mesh, &verts[0], verts.count, &faces[0], faces.count);

	model_add_subset(model, mesh, material, matrix_identity);

	material_release(material);
	mesh_release(mesh);

	poss  .free();
	norms .free();
	uvs   .free();
	verts .free();
	faces .free();
	indmap.free();
	return true;
}

}
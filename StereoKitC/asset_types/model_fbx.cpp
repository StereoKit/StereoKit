#include "../stereokit.h"
#include "model.h"
#include "../libraries/miniz.h"
#include "../libraries/ofbx.h"
#include "../libraries/stref.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

namespace sk {

///////////////////////////////////////////

tex_t modelfmt_fbx_texture(const char *filename, const char *folder, const ofbx::Texture *tex, bool color_data) {
	if (tex == nullptr) return nullptr;

	ofbx::DataView name     = tex->getFileName();
	stref_t        name_str = stref_t{ (char*)name.begin, (uint32_t)(name.end-name.begin) };
	stref_t        tex_path, tex_name_ref;
	stref_file_path(name_str, tex_path, tex_name_ref);
	char *tex_name = stref_copy(tex_name_ref);

	tex_t       result = nullptr;
	char        tex_file[512];
	const char *asset_filename;
	struct stat check = {};

	snprintf(tex_file, sizeof(tex_file), "%s/%s", folder, tex_name);
	asset_filename = assets_file(tex_file);
	if (stat(asset_filename, &check) == 0)
		result = tex_create_file(tex_file);

	if (result == nullptr) {
		snprintf(tex_file, sizeof(tex_file), "%s/textures/%s", folder, tex_name);
		asset_filename = assets_file(tex_file);
		if (stat(asset_filename, &check) == 0)
			result = tex_create_file(tex_file);
	}

	if (result == nullptr)
		log_warnf("Issue in '<~cyn>%s<~clr>', couldn't find texture: <~cyn>%s<~clr>", filename, tex_name);

	free(tex_name);
	return result;
}

///////////////////////////////////////////

material_t modelfmt_fbx_material(const char *filename, const char *folder, shader_t shader, const ofbx::Material *mat) {
	char id[512];
	snprintf(id, sizeof(id), "%s/mat/%s", filename, mat->name);
	material_t result = material_find(id);
	if (result != nullptr)
		return result;

	result = material_create(shader == nullptr ? shader_find(default_id_shader) : shader);
	material_set_id(result, id);

	tex_t diffuse  = modelfmt_fbx_texture(filename, folder, mat->getTexture(ofbx::Texture::DIFFUSE ), true);
	tex_t normal   = modelfmt_fbx_texture(filename, folder, mat->getTexture(ofbx::Texture::NORMAL  ), false);
	tex_t specular = modelfmt_fbx_texture(filename, folder, mat->getTexture(ofbx::Texture::SPECULAR), false);

	ofbx::Color c = mat->getDiffuseColor();
	material_set_color(result, "color", { c.r, c.g, c.b, 1 });
	if (diffuse != nullptr && material_has_param(result, "diffuse", material_param_texture))
		material_set_texture(result, "diffuse", diffuse);
	if (normal != nullptr && material_has_param(result, "normal", material_param_texture))
		material_set_texture(result, "normal", normal);
	if (specular != nullptr && material_has_param(result, "metal", material_param_texture))
		material_set_texture(result, "metal", specular);

	return result;
}

///////////////////////////////////////////

mesh_t modelfmt_fbx_geometry(const char *filename, const char *folder, const char *name, const ofbx::Geometry *geo) {
	char id[512];
	snprintf(id, sizeof(id), "%s/mesh/%s", filename, name);
	mesh_t result = mesh_find(id);
	if (result != nullptr)
		return result;

	// Assemble vertex data
	int32_t vert_count = geo->getVertexCount();
	vert_t *verts = (vert_t *)malloc(vert_count * sizeof(vert_t));
	const ofbx::Vec3 *source_verts  = geo->getVertices();
	const ofbx::Vec3 *source_norms  = geo->getNormals();
	const ofbx::Vec2 *source_uvs    = geo->getUVs();
	const ofbx::Vec4 *source_colors = geo->getColors();

	if (source_verts != nullptr)
		for (int32_t i = 0; i < vert_count; i++)
			verts[i].pos = vec3{ (float)source_verts[i].x, (float)source_verts[i].y, (float)source_verts[i].z };// *cm2m;

	if (source_norms != nullptr)
		for (int32_t i = 0; i < vert_count; i++)
			verts[i].norm = { (float)source_norms[i].x, (float)source_norms[i].y, (float)source_norms[i].z };

	if (source_uvs != nullptr)
		for (int32_t i = 0; i < vert_count; i++)
			verts[i].uv = { (float)source_uvs[i].x, 1-(float)source_uvs[i].y };

	if (source_colors != nullptr)
		for (int32_t i = 0; i < vert_count; i++)
			verts[i].col = {
				(uint8_t)(source_colors[i].x * 255),
				(uint8_t)(source_colors[i].y * 255),
				(uint8_t)(source_colors[i].z * 255),
				(uint8_t)(source_colors[i].w * 255) };
	else
		for (int32_t i = 0; i < vert_count; i++)
			verts[i].col = { 255,255,255,255 };

	// Assemble face data
	int32_t ind_count = geo->getIndexCount();
	vind_t *inds      = (vind_t *)malloc(ind_count * sizeof(vind_t));
	const int32_t *source_inds = geo->getFaceIndices();
	for (int32_t i = 0; i < ind_count; i++) {
		inds[i] = (vind_t)(source_inds[i] < 0 ? -source_inds[i]-1 : source_inds[i]);
	}

	// Assemble a mesh
	result = mesh_create();
	mesh_set_id   (result, id);
	mesh_set_verts(result, verts, vert_count);
	mesh_set_inds (result, inds,  ind_count);

	free(verts);
	free(inds);
	return result;
}

///////////////////////////////////////////

bool modelfmt_fbx(model_t model, const char *filename, void *file_data, size_t file_length, shader_t shader) {
	ofbx::IScene *scene = ofbx::load((ofbx::u8*)file_data, file_length, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);

	stref_t path, name;
	stref_file_path(stref_make(filename), path, name);
	char *folder = stref_copy(path);

	int32_t count = scene->getMeshCount();
	for (int32_t i = 0; i < count; i++) {
		const ofbx::Mesh *fbx_mesh = scene->getMesh(i);
		mesh_t     mesh     = modelfmt_fbx_geometry(filename, folder, fbx_mesh->name, fbx_mesh->getGeometry());
		material_t material = fbx_mesh->getMaterialCount() > 0 
			? modelfmt_fbx_material(filename, folder, shader, fbx_mesh->getMaterial(0))
			: material_find(default_id_material);

		ofbx::Matrix transform = fbx_mesh->getGlobalTransform();
		matrix sk_transform;
		for (int32_t m = 0; m < 4; m++) {
			sk_transform.row[m].x = (float)transform.m[m*4];
			sk_transform.row[m].y = (float)transform.m[m*4+1];
			sk_transform.row[m].z = (float)transform.m[m*4+2];
			sk_transform.row[m].w = (float)transform.m[m*4+3];
		}

		model_add_subset(model, mesh, material, sk_transform * matrix_trs(vec3_zero, quat_identity, vec3_one * cm2m));
	}

	free(folder);
	scene->destroy();
	return true;
}

}
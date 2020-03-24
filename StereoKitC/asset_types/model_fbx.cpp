#include "model.h"
#include "../libraries/miniz.h"
#include "../libraries/ofbx.h"

#include <stdio.h>
#include <stdlib.h>

namespace sk {

///////////////////////////////////////////

material_t modelfmt_fbx_material(const char *filename, shader_t shader, const ofbx::Material *mat) {
	char id[512];
	sprintf_s(id, 512, "%s/%s", filename, mat->name);
	material_t result = material_find(id);
	if (result != nullptr)
		return result;

	result = material_create(shader == nullptr ? shader_find("default/shader") : shader);

	ofbx::Color c = mat->getDiffuseColor();
	material_set_color(result, "color", { c.r, c.g, c.b, 1 });

	return result;
}

///////////////////////////////////////////

mesh_t modelfmt_fbx_geometry(const char *filename, const char *name, const ofbx::Geometry *geo) {
	char id[512];
	sprintf_s(id, 512, "%s/%s", filename, name);
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
			verts[i].uv = { (float)source_uvs[i].x, (float)source_uvs[i].y };

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

	int32_t count = scene->getMeshCount();
	for (int32_t i = 0; i < count; i++) {
		const ofbx::Mesh *fbx_mesh = scene->getMesh(i);
		mesh_t     mesh     = modelfmt_fbx_geometry(filename, fbx_mesh->name, fbx_mesh->getGeometry());
		material_t material = fbx_mesh->getMaterialCount() > 0 
			? modelfmt_fbx_material(filename, shader, fbx_mesh->getMaterial(0))
			: material_find("default/material");
		ofbx::Matrix transform = fbx_mesh->getGlobalTransform();
		ofbx::Vec3 scale = fbx_mesh->getLocalScaling();
		ofbx::Vec3 pos   = fbx_mesh->getLocalTranslation();
		ofbx::Vec3 rot   = fbx_mesh->getLocalRotation();
		matrix sk_transform;
		for (int32_t m = 0; m < 4; m++) {
			sk_transform.row[m].x = (float)transform.m[m*4];
			sk_transform.row[m].y = (float)transform.m[m*4+1];
			sk_transform.row[m].z = (float)transform.m[m*4+2];
			sk_transform.row[m].w = (float)transform.m[m*4+3];
		}
		
		ofbx::Vec3 fbx_pivot = fbx_mesh->getScalingPivot();
		vec3 pivot = { fbx_pivot.x, fbx_pivot.y, fbx_pivot.z };
		matrix sk_scale = matrix_trs(-pivot) *  matrix_trs(vec3_zero, quat_identity, vec3_one * cm2m) * matrix_trs(pivot);
		
		model_add_subset(model, mesh, material, sk_transform * sk_scale);
		/*matrix_trs(
			vec3{ (float)pos.x, (float)pos.y, (float)pos.z } *cm2m, 
			quat_from_angles(rot.x * rad2deg, rot.y * rad2deg, rot.z * rad2deg), 
			vec3{(float)scale.x, (float)scale.y, (float)scale.z} *cm2m));*/
	}

	scene->destroy();
	return false;
}

}
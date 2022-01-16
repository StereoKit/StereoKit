#include "render_list.h"
#include "../hierarchy.h"
#include "../sk_math.h"
#include "../systems/render_sort.h"
#include "../asset_types/mesh.h"
#include "../asset_types/model.h"
#include "../asset_types/material.h"

#include <DirectXMath.h> // Matrix math functions and objects
using namespace DirectX;

namespace sk {

array_t<_render_list_t> render_lists = {};

///////////////////////////////////////////

render_list_t render_list_create() {
	render_list_t result = (render_list_t)assets_allocate(asset_type_render_list);
	return result;
}

///////////////////////////////////////////

render_list_t render_list_find(const char *id) {
	render_list_t result = (render_list_t)assets_find(id, asset_type_render_list);
	if (result != nullptr) {
		render_list_addref(result);
		return result;
	}
	return result;
}

///////////////////////////////////////////

void render_list_set_id(render_list_t list, const char *id) {
	assets_set_id(list->header, id);
}

///////////////////////////////////////////

void render_list_addref(render_list_t list) {
	assets_addref(list->header);
}

///////////////////////////////////////////

void render_list_release(render_list_t list) {
	if (list == nullptr)
		return;
	assets_releaseref(list->header);
}

///////////////////////////////////////////

void render_list_destroy(render_list_t list) {
	list->queue.free();
	*list = {};
	list->state = render_list_state_destroyed;
}

///////////////////////////////////////////

void render_list_add_to(render_list_t list, const render_item_t *item) {
	list->queue.add(*item);
	list->sorted = false;
}

///////////////////////////////////////////

void render_list_clear(render_list_t list) {
	list->queue.clear();
	list->stats  = {};
	list->sorted = false;
	list->state  = render_list_state_empty;
}

///////////////////////////////////////////

void render_list_sort(render_list_t list) {
	if (!list->sorted) {
		radix_sort7(&list->queue[0], list->queue.count);
		list->sorted = true;
	}
}

///////////////////////////////////////////

inline uint64_t render_queue_id(material_t material, mesh_t mesh) {
	return ((uint64_t)(material->alpha_mode*1000 + material->queue_offset) << 32) | (material->header.index << 16) | mesh->header.index;
}

///////////////////////////////////////////

void render_list_add_mesh(render_list_t list, mesh_t mesh, material_t material, const matrix &transform, color128 color, render_layer_ layer) {
	render_item_t item;
	item.mesh     = &mesh->gpu_mesh;
	item.mesh_inds= mesh->ind_draw;
	item.material = material;
	item.color    = color;
	item.sort_id  = render_queue_id(material, mesh);
	item.layer    = (uint16_t)layer;
	if (hierarchy_enabled) {
		matrix_mul(transform, hierarchy_stack.last().transform, item.transform);
	} else {
		math_matrix_to_fast(transform, &item.transform);
	}
	render_list_add_to(list, &item);
}

///////////////////////////////////////////

void render_list_add_model(render_list_t list, model_t model, const matrix &transform, color128 color, render_layer_ layer) {
	XMMATRIX root;
	if (hierarchy_enabled) {
		matrix_mul(transform, hierarchy_stack.last().transform, root);
	} else {
		math_matrix_to_fast(transform, &root);
	}

	anim_update_model(model);
	for (size_t i = 0; i < model->visuals.count; i++) {
		render_item_t item;
		item.mesh     = &model->visuals[i].mesh->gpu_mesh;
		item.mesh_inds= model->visuals[i].mesh->ind_count;
		item.material = model->visuals[i].material;
		item.color    = color;
		item.sort_id  = render_queue_id(item.material, model->visuals[i].mesh);
		item.layer    = (uint16_t)layer;
		matrix_mul(model->visuals[i].transform_model, root, item.transform);
		render_list_add_to(list, &item);
	}

	if (model->transforms_changed && model->anim_data.skeletons.count > 0) {
		model->transforms_changed = false;
		anim_update_skin(model);
	}
}

} // namespace sk
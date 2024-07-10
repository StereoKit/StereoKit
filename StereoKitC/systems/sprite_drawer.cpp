#include "sprite_drawer.h"

#include "../asset_types/sprite.h"

#include "../libraries/array.h"
#include "../systems/render.h"
#include "../systems/defaults.h"
#include "../hierarchy.h"
#include "../sk_math_dx.h"
#include "../sk_memory.h"

using namespace DirectX;

namespace sk {

///////////////////////////////////////////

array_t<sprite_buffer_t> sprite_buffers = {};
mesh_t                   sprite_quad_old;
mesh_t                   sprite_quad;
material_t               sprite_blit_mat;

///////////////////////////////////////////

void sprite_drawer_add_buffer(material_t material) {
	sprite_buffers.add({});
	sprite_buffer_t &buffer = sprite_buffers.last();
	buffer.material = material;
	buffer.mesh     = mesh_create();
}

///////////////////////////////////////////

void sprite_buffer_ensure_capacity(sprite_buffer_t *buffer) {
	if (buffer->vert_count + 4 <= buffer->vert_cap)
		return;

	buffer->vert_cap = buffer->vert_count + 4;
	buffer->verts    = sk_realloc_t(vert_t, buffer->verts, buffer->vert_cap);

	// regenerate indices
	vind_t  quads = (vind_t)(buffer->vert_cap / 4);
	vind_t *inds  = sk_malloc_t(vind_t, quads * 6);
	for (vind_t i = 0; i < quads; i++) {
		vind_t q = i * 4;
		vind_t c = i * 6;
		inds[c+0] = q+2;
		inds[c+1] = q+1;
		inds[c+2] = q;

		inds[c+3] = q+3;
		inds[c+4] = q+2;
		inds[c+5] = q;
	}
	mesh_set_inds(buffer->mesh, inds, quads * 6);
	sk_free(inds);
}

///////////////////////////////////////////

void sprite_draw(sprite_t sprite, matrix at, text_align_ anchor_position, color32 color) {
	// Check if this one does get batched
	if (sprite->buffer_index == -1) {
		vec3 offset = vec3_zero;
		if      (anchor_position & text_align_x_left  ) offset.x = -sprite->aspect/2;
		else if (anchor_position & text_align_x_right ) offset.x =  sprite->aspect/2;
		if      (anchor_position & text_align_y_bottom) offset.y =  0.5f;
		else if (anchor_position & text_align_y_top   ) offset.y = -0.5f;

		// Just plop a quad onto the render queue
		render_add_mesh(sprite_quad, sprite->material, matrix_ts(offset, {sprite->aspect, 1, 1}) * at, color32_to_128(color));
		return;
	} else {
		sprite_buffer_t* buffer = &sprite_buffers[sprite->buffer_index];
		float            width  = sprite->size * sprite->aspect;
		float            height = sprite->size;

		vec3 offset = { -width / 2.0f, -height / 2.0f };
		if      (anchor_position & text_align_x_left  ) offset.x = -width;
		else if (anchor_position & text_align_x_right ) offset.x = 0;
		if      (anchor_position & text_align_y_bottom) offset.y = 0;
		else if (anchor_position & text_align_y_top   ) offset.y = -height;

		// Resize array if we need more room for this
		sprite_buffer_ensure_capacity(buffer);

		// Get the hierarchy based transform
		XMMATRIX tr;
		if (hierarchy_is_enabled()) {
			matrix_mul(at, hierarchy_top(), tr);
		} else {
			math_matrix_to_fast(at, &tr);
		}

		// Add a sprite quad
		int32_t start  = buffer->vert_count;
		vec3    normal = vec3_normalize(matrix_transform_dir(at, vec3_forward));
		buffer->verts[start + 0] = { matrix_mul_point(tr, vec3{offset.x,       offset.y,        0}), normal, sprite->uvs[1],                           color };
		buffer->verts[start + 1] = { matrix_mul_point(tr, vec3{offset.x+width, offset.y,        0}), normal, vec2{sprite->uvs[0].x, sprite->uvs[1].y}, color };
		buffer->verts[start + 2] = { matrix_mul_point(tr, vec3{offset.x+width, offset.y+height, 0}), normal, sprite->uvs[0],                           color };
		buffer->verts[start + 3] = { matrix_mul_point(tr, vec3{offset.x,       offset.y+height, 0}), normal, vec2{sprite->uvs[1].x, sprite->uvs[0].y}, color };
		buffer->vert_count += 4;
	}
}

///////////////////////////////////////////

bool sprite_drawer_init() {
	sprite_quad     = mesh_find(default_id_mesh_quad);
	sprite_blit_mat = material_create(sk_default_shader_blit);
	material_set_cull(sprite_blit_mat, cull_none);	

	// Default rendering quad
	sprite_quad_old = mesh_create();
	vert_t verts[4] = {
		{ vec3{0, 0,0}, vec3{0,0,-1}, vec2{1,0}, color32{255,255,255,255} },
		{ vec3{1, 0,0}, vec3{0,0,-1}, vec2{0,0}, color32{255,255,255,255} },
		{ vec3{1,-1,0}, vec3{0,0,-1}, vec2{0,1}, color32{255,255,255,255} },
		{ vec3{0,-1,0}, vec3{0,0,-1}, vec2{1,1}, color32{255,255,255,255} },
	};	
	vind_t inds[6] = { 0,1,2, 0,2,3 };
	mesh_set_id       (sprite_quad_old, "render/sprite_quad");
	mesh_set_keep_data(sprite_quad_old, false);
	mesh_set_data     (sprite_quad_old, verts, 4, inds, 6, false);

	return true;
}

///////////////////////////////////////////

void sprite_drawer_update_atlas(tex_t target, array_t<sprite_t> sprites) {
	render_bind_target_push(target, -1, 0);
	for (int32_t i = 0; i < sprites.count; i++) {
		material_set_texture(sprite_blit_mat, "source", sprites[i]->texture);
		material_set_vector4(sprite_blit_mat, "blit_to", {
			sprites[i]->uvs[0].x, 
			sprites[i]->uvs[0].y,
			sprites[i]->uvs[1].x - sprites[i]->uvs[0].x,
			sprites[i]->uvs[1].y - sprites[i]->uvs[0].y });
		render_blit_to_bound_noclear(sprite_blit_mat);
	}
	render_bind_target_pop(true);
}

///////////////////////////////////////////

void sprite_drawer_step() {
	for (int32_t i = 0; i < sprite_atlases.count; i++) {
		sprite_atlas_t* atlas = &sprite_atlases[i];
		if (atlas->rects.w != tex_get_width (atlas->texture) ||
			atlas->rects.h != tex_get_height(atlas->texture)) {

			tex_set_colors(atlas->texture, atlas->rects.w, atlas->rects.h, nullptr);
		}

		if (atlas->dirty_full) {
			sprite_drawer_update_atlas(atlas->texture, atlas->sprites);
			atlas->dirty_full = false;
			// Clear out the dirty queue in case there were a few there before
			// we filled up.
			atlas->dirty_queue.clear();
		} else if (atlas->dirty_queue.count > 0) {
			sprite_drawer_update_atlas(atlas->texture, atlas->dirty_queue);
			atlas->dirty_queue.clear();
		}
	}

	for (int32_t i = 0; i < sprite_buffers.count; i++) {
		sprite_buffer_t &buffer = sprite_buffers[i];
		if (buffer.vert_count <= 0)
			continue;

		mesh_set_verts    (buffer.mesh, buffer.verts, buffer.vert_count, false);
		mesh_set_draw_inds(buffer.mesh, (buffer.vert_count / 4) * 6);
		mesh_draw         (buffer.mesh, buffer.material, matrix_identity);
		buffer.vert_count = 0;
	}
}

///////////////////////////////////////////

void sprite_drawer_shutdown() {
	material_release(sprite_blit_mat);
	mesh_release(sprite_quad);
	mesh_release(sprite_quad_old);
	for (int32_t i = 0; i < sprite_buffers.count; i++) {
		sprite_buffer_t &buffer = sprite_buffers[i];
		mesh_release(buffer.mesh);
		material_release(buffer.material);
		sk_free(buffer.verts);
	}
	sprite_buffers.clear();
}

} // namespace sk

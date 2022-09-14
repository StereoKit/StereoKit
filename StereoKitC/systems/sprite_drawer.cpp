#include "sprite_drawer.h"

#include "../asset_types/sprite.h"

#include "../libraries/array.h"
#include "../hierarchy.h"
#include "../sk_math_dx.h"
#include "../sk_memory.h"

using namespace DirectX;

namespace sk {

///////////////////////////////////////////

array_t<sprite_buffer_t> sprite_buffers = {};
mesh_t                   sprite_quad_old;
mesh_t                   sprite_quad;

///////////////////////////////////////////

void sprite_drawer_add_buffer(material_t material) {
	sprite_buffers.add({});
	sprite_buffer_t &buffer = sprite_buffers.last();
	buffer.material = material;
	buffer.mesh     = mesh_create();
}

///////////////////////////////////////////

void sprite_buffer_ensure_capacity(sprite_buffer_t &buffer) {
	if (buffer.vert_count + 4 <= buffer.vert_cap)
		return;

	buffer.vert_cap = buffer.vert_count + 4;
	buffer.verts    = sk_realloc_t(vert_t, buffer.verts, buffer.vert_cap);

	// regenerate indices
	vind_t  quads = (vind_t)(buffer.vert_cap / 4);
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
	mesh_set_inds(buffer.mesh, inds, quads * 6);
	sk_free(inds);
}

///////////////////////////////////////////

void sprite_drawer_add     (sprite_t sprite, const matrix &at, color32 color) {
	float width  = (sprite->uvs[1].x - sprite->uvs[0].x) * sprite->size;
	float height = (sprite->uvs[1].y - sprite->uvs[0].y) * sprite->size;

	// Check if this one does get batched
	if (sprite->buffer_index == -1) {
		// Just plop a quad onto the render queue
		render_add_mesh(sprite_quad_old, sprite->material, at, {color.r/255.f, color.g/255.f, color.b/255.f, color.a/255.f });
		return;
	}

	sprite_buffer_t &buffer = sprite_buffers[sprite->buffer_index];

	// Resize array if we need more room for this
	sprite_buffer_ensure_capacity(buffer);

	// Get the heirarchy based transform
	XMMATRIX tr;
	if (hierarchy_enabled) {
		matrix_mul(hierarchy_stack.last().transform, at, tr);
	} else {
		math_matrix_to_fast(at, &tr);
	}
	
	// Add a sprite quad
	int32_t offset = buffer.vert_count;
	vec3    normal = vec3_normalize( matrix_transform_dir(at, vec3_forward) );
	buffer.verts[offset + 0] = { matrix_mul_point(tr, vec3{0,     0,      0}), normal, sprite->uvs[0],                           color };
	buffer.verts[offset + 1] = { matrix_mul_point(tr, vec3{width, 0,      0}), normal, vec2{sprite->uvs[1].x, sprite->uvs[0].y}, color };
	buffer.verts[offset + 2] = { matrix_mul_point(tr, vec3{width, height, 0}), normal, sprite->uvs[1],                           color };
	buffer.verts[offset + 3] = { matrix_mul_point(tr, vec3{0,     height, 0}), normal, vec2{sprite->uvs[0].x, sprite->uvs[1].y}, color };

	buffer.vert_count += 4;
}


///////////////////////////////////////////

void sprite_drawer_add_at(sprite_t sprite, matrix at, text_align_ anchor_position, color32 color) {
	// Check if this one does get batched
	if (sprite->buffer_index == -1) {
		// Just plop a quad onto the render queue
		vec3 offset = vec3_zero;
		if      (anchor_position & text_align_x_left  ) offset.x = -sprite->aspect/2;
		else if (anchor_position & text_align_x_right ) offset.x =  sprite->aspect/2;
		if      (anchor_position & text_align_y_bottom) offset.y =  0.5f;
		else if (anchor_position & text_align_y_top   ) offset.y = -0.5f;
		render_add_mesh(sprite_quad, sprite->material, matrix_ts(offset, {sprite->aspect, 1, 1}) * at, { color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f });
		return;
	} else {
		log_err("Not implemented");
	}
}

///////////////////////////////////////////

bool sprite_drawer_init() {
	sprite_quad = mesh_find(default_id_mesh_quad);

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

void sprite_drawer_update() {
	for (int32_t i = 0; i < sprite_buffers.count; i++) {
		sprite_buffer_t &buffer = sprite_buffers[i];
		if (buffer.vert_count <= 0)
			continue;

		mesh_set_verts(buffer.mesh, buffer.verts, buffer.vert_count, false);
		mesh_set_draw_inds(buffer.mesh, (buffer.vert_count / 4) * 6);

		render_add_mesh(buffer.mesh, buffer.material, matrix_identity);
		buffer.vert_count = 0;
	}
}

///////////////////////////////////////////

void sprite_drawer_shutdown() {
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
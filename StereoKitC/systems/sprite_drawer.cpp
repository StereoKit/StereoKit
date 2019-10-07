#include "sprite_drawer.h"

#include "../asset_types/material.h"
#include "../asset_types/mesh.h"
#include "../asset_types/assets.h"

#include <vector>
using namespace std;

namespace sk {

///////////////////////////////////////////

vector<sprite_buffer_t> sprite_buffers;
mesh_t                  sprite_quad;

///////////////////////////////////////////

void sprite_drawer_add_buffer(material_t material) {
	sprite_buffers.push_back({});
	sprite_buffer_t &buffer = sprite_buffers[sprite_buffers.size() - 1];
	buffer.material = material;
	buffer.mesh     = mesh_create();
}

///////////////////////////////////////////

void sprite_buffer_ensure_capacity(sprite_buffer_t &buffer) {
	if (buffer.vert_count + 4 <= buffer.vert_cap)
		return;

	buffer.vert_cap = buffer.vert_count + 4;
	buffer.verts    = (vert_t *)realloc(buffer.verts, sizeof(vert_t) * buffer.vert_cap);

	// regenerate indices
	int32_t quads = buffer.vert_cap / 4;
	vind_t *inds  = (vind_t *)malloc(quads * 6 * sizeof(vind_t));
	for (int32_t i = 0; i < quads; i++) {
		int32_t q = i * 4;
		int32_t c = i * 6;
		inds[c+0] = q+2;
		inds[c+1] = q+1;
		inds[c+2] = q;

		inds[c+3] = q+3;
		inds[c+4] = q+2;
		inds[c+5] = q;
	}
	mesh_set_inds(buffer.mesh, inds, quads * 6);
	free(inds);
}

///////////////////////////////////////////

void sprite_drawer_add     (sprite_t sprite, const matrix &at, color32 color) {
	float width  = (sprite->uvs[1].x - sprite->uvs[0].x) * sprite->size;
	float height = (sprite->uvs[1].y - sprite->uvs[0].y) * sprite->size;

	// Check if this one does get batched
	if (sprite->buffer_index == -1) {
		// Just plop a quad onto the render queue
		//material_set_color32(sprite->material, "color", color);
		render_add_mesh(sprite_quad, sprite->material, at);
		return;
	}

	sprite_buffer_t &buffer = sprite_buffers[sprite->buffer_index];

	// Resize array if we need more room for this
	sprite_buffer_ensure_capacity(buffer);
	
	// Add a sprite quad
	size_t offset = buffer.vert_count;
	vec3   normal = vec3_normalize( matrix_mul_direction(at, vec3_forward) );
	buffer.verts[offset + 0] = { matrix_mul_point(at, vec3{0,     0,      0}), normal, sprite->uvs[0],                           color };
	buffer.verts[offset + 1] = { matrix_mul_point(at, vec3{width, 0,      0}), normal, vec2{sprite->uvs[1].x, sprite->uvs[0].y}, color };
	buffer.verts[offset + 2] = { matrix_mul_point(at, vec3{width, height, 0}), normal, sprite->uvs[1],                           color };
	buffer.verts[offset + 3] = { matrix_mul_point(at, vec3{0,     height, 0}), normal, vec2{sprite->uvs[0].x, sprite->uvs[1].y}, color };

	buffer.vert_count += 4;
}

///////////////////////////////////////////

bool sprite_drawer_init() {

	// Default rendering quad
	sprite_quad = mesh_create();
	vert_t verts[4] = {
		vec3{0, 0,0}, vec3{0,0,-1}, vec2{0,0}, color32{255,255,255,255},
		vec3{1, 0,0}, vec3{0,0,-1}, vec2{1,0}, color32{255,255,255,255},
		vec3{1,-1,0}, vec3{0,0,-1}, vec2{1,1}, color32{255,255,255,255},
		vec3{0,-1,0}, vec3{0,0,-1}, vec2{0,1}, color32{255,255,255,255},
	};	
	vind_t inds[6] = { 0,1,2, 0,2,3 };
	mesh_set_id   (sprite_quad, "render/sprite_quad");
	mesh_set_verts(sprite_quad, verts, 4);
	mesh_set_inds (sprite_quad, inds,  6);

	return true;
}

///////////////////////////////////////////

void sprite_drawer_update() {
	for (size_t i = 0; i < sprite_buffers.size(); i++) {
		sprite_buffer_t &buffer = sprite_buffers[i];
		if (buffer.vert_count <= 0)
			continue;

		mesh_set_verts(buffer.mesh, buffer.verts, buffer.vert_count);
		mesh_set_draw_inds(buffer.mesh, (buffer.vert_count / 4) * 6);

		render_add_mesh(buffer.mesh, buffer.material, matrix_identity);
		buffer.vert_count = 0;
	}
}

///////////////////////////////////////////

void sprite_drawer_shutdown() {
	mesh_release(sprite_quad);
	for (size_t i = 0; i < sprite_buffers.size(); i++) {
		sprite_buffer_t &buffer = sprite_buffers[i];
		mesh_release(buffer.mesh);
		material_release(buffer.material);
		free(buffer.verts);
	}
	sprite_buffers.clear();
}

} // namespace sk
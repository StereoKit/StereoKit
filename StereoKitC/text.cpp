#include "text.h"

#include "assets.h"
#include <map>
#include <vector>
using namespace std;

vector<_text_style_t> text_styles;
map<text_style_t, text_buffer_t> text_buffers;

text_style_t text_make_style(font_t font, material_t material) {
	text_styles.push_back({ font, material });
	material_set_texture(material, "diffuse", font_get_tex(font));
	material_set_cull(material, material_cull_none);
	material_set_alpha_mode(material, material_alpha_test);
	return text_styles.size() - 1;
}

void text_add(text_style_t style, transform_t &transform, const char *text) {
	text_buffer_t                             *buffer;
	map<text_style_t, text_buffer_t>::iterator item = text_buffers.find(style);
	if (item == text_buffers.end()) {
		text_buffers[style] = {};
		buffer = &text_buffers[style];

		// Make a mesh for this style
		char id[26];
		assets_unique_name("auto/txt_buf/", id, 20);
		buffer->mesh = mesh_create(id);
	} else {
		buffer = &item->second;
	}

	// Resize array if we need more room for this text
	size_t length = strlen(text);
	if (buffer->vert_count + length * 4 > buffer->vert_cap) {
		buffer->vert_cap = buffer->vert_count + length * 4;
		buffer->verts = (vert_t *)realloc(buffer->verts, sizeof(vert_t) * buffer->vert_cap);

		// regenerate indices
		int       quads = buffer->vert_cap / 4;
		uint16_t *inds  = (uint16_t *)malloc(quads * 6 * sizeof(uint16_t));
		for (size_t i = 0; i < quads; i++) {
			int q = i * 4;
			int c = i * 6;
			inds[c+0] = q+2;
			inds[c+1] = q+1;
			inds[c+2] = q;

			inds[c+3] = q+3;
			inds[c+4] = q+2;
			inds[c+5] = q;
		}
		mesh_set_inds(buffer->mesh, inds, quads * 6);
		free(inds);
	}
	
	vec3    normal  = transform_local_to_world_dir(transform, -vec3_forward);
	color32 col     = {255,255,255,255};
	font_t  font    = text_styles[style].font;
	const char *curr = text;
	float   x       = 0;
	float   y       = 0;
	size_t  offset  = buffer->vert_count;
	while (*curr != '\0') {
		char currch = *curr;
		curr += 1;
		font_char_t &ch = font->characters[(int)currch];

		// Do spacing for whitespace characters
		switch (currch) {
		case '\t': x += font->characters[(int)' '].xadvance * 4; continue;
		case ' ':  x += ch.xadvance; continue;
		case '\n': x = 0; y -= 1; continue;
		default:break;
		}
		
		// Add a character quad
		buffer->verts[offset+0].pos  = transform_local_to_world(transform, vec3{x+ch.x0, y+ch.y0, 0});
		buffer->verts[offset+0].uv   = vec2{ch.u0, ch.v0};
		buffer->verts[offset+0].norm = normal;
		buffer->verts[offset+0].col  = col;

		buffer->verts[offset+1].pos  = transform_local_to_world(transform, vec3{x+ch.x1, y+ch.y0, 0});
		buffer->verts[offset+1].uv   = vec2{ch.u1, ch.v0};
		buffer->verts[offset+1].norm = normal;
		buffer->verts[offset+1].col  = col;

		buffer->verts[offset+2].pos  = transform_local_to_world(transform, vec3{x+ch.x1, y+ch.y1, 0});
		buffer->verts[offset+2].uv   = vec2{ch.u1, ch.v1};
		buffer->verts[offset+2].norm = normal;
		buffer->verts[offset+2].col  = col;

		buffer->verts[offset+3].pos  = transform_local_to_world(transform, vec3{x+ch.x0, y+ch.y1, 0});
		buffer->verts[offset+3].uv   = vec2{ch.u0, ch.v1};
		buffer->verts[offset+3].norm = normal;
		buffer->verts[offset+3].col  = col;

		buffer->vert_count += 4;
		x += ch.xadvance;
		offset += 4;
	}
}

void text_render_style(text_style_t style) {
	map<text_style_t, text_buffer_t>::iterator item = text_buffers.find(style);
	if (item == text_buffers.end())
		return;

	text_buffer_t *buffer = &item->second;
	transform_t tr;
	transform_initialize(tr);

	mesh_set_verts(buffer->mesh, buffer->verts, buffer->vert_count);
	
	render_add_mesh(buffer->mesh, text_styles[style].material, tr);
	buffer->vert_count = 0;
}
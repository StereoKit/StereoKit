#include "font.h"

#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "../libraries/stb_rect_pack.h"
#include "../rect_atlas.h"
#include "../libraries/stb_truetype.h"
#include "../systems/platform/platform_utils.h"
#include "../sk_memory.h"

#include <stdio.h>

namespace sk {

array_t<font_t> font_list = {};

///////////////////////////////////////////

font_char_t font_place_glyph   (font_t font, int32_t glyph);
void        font_render_glyph  (font_t font, int32_t glyph, const font_char_t *ch);
int64_t     font_add_character (font_t font, char32_t character);
void        font_upsize_texture(font_t font);
void        font_update_texture(font_t font);
void        font_update_cache  (font_t font);

///////////////////////////////////////////

font_t font_find(const char *id) {
	font_t result = (font_t)assets_find(id, asset_type_font);
	if (result != nullptr) {
		font_addref(result);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

font_t font_create(const char *file) {
	font_t result = font_find(file);
	if (result != nullptr)
		return result;
	result = (font_t)assets_allocate(asset_type_font);
	assets_set_id(result->header, file);

	size_t length;
	if (!platform_read_file(file, (void **)&result->font_file, &length)) {
		log_warnf("Font file failed to load: %s", file);
		return nullptr;
	}

	const int32_t atlas_resolution_x = 256;
	const int32_t atlas_resolution_y = 256;
	stbtt_InitFont(&result->font_info, (const unsigned char *)result->font_file, stbtt_GetFontOffsetForIndex((const unsigned char *)result->font_file,0));
	result->character_resolution = 64;
	result->font_scale           = stbtt_ScaleForPixelHeight(&result->font_info, (float)result->character_resolution);
	result->atlas                = rect_atlas_create( atlas_resolution_x, atlas_resolution_y );
	result->atlas_data           = sk_malloc_t(uint8_t, atlas_resolution_x * atlas_resolution_y);
	memset(result->atlas_data, 0, result->atlas.w * result->atlas.h);

	for (size_t i = 32; i < 128; i++) {
		font_add_character(result, i);
	}
	font_update_cache(result);

	// Get information about character sizes for this font
	stbtt_GetFontVMetrics(&result->font_info, &result->character_ascend, &result->character_descend, &result->line_gap);
	int32_t x0, y0, x1, y1;
	stbtt_GetCodepointBox(&result->font_info, 'T', &x0, &y0, &x1, &y1);
	result->character_height = y1 * result->font_scale;
	int32_t advance, lsb; 
	stbtt_GetGlyphHMetrics(&result->font_info, ' ', &advance, &lsb);
	result->space_width = advance * result->font_scale;

	font_list.add(result);
	return result;
}

///////////////////////////////////////////

void font_set_id(font_t font, const char* id) {
	assets_set_id(font->header, id);
}

///////////////////////////////////////////

void font_addref(font_t font) {
	assets_addref(font->header);
}

///////////////////////////////////////////

void font_release(font_t font) {
	if (font == nullptr)
		return;
	assets_releaseref(font->header);
}

///////////////////////////////////////////

void font_destroy(font_t font) {
	font_list.remove(font_list.index_of(font));

	tex_release       ( font->font_tex);
	rect_atlas_destroy(&font->atlas);
	free              ( font->atlas_data);
	free              ( font->font_file);
	font->character_map.free();
	font->update_queue .free();

	*font = {};
}

///////////////////////////////////////////

tex_t font_get_tex(font_t font) {
	return font->font_tex;
}

///////////////////////////////////////////

font_char_t font_place_glyph(font_t font, int32_t glyph) {
	if (glyph == 0) return {};

	const int32_t pad  = 1;
	float   to_u = 1.0f / font->atlas.w;
	float   to_v = 1.0f / font->atlas.h;

	int advance, lsb;
	int x0, x1, y0, y1;
	stbtt_GetGlyphBitmapBox(&font->font_info, glyph, font->font_scale, font->font_scale, &x0, &y0, &x1, &y1);
	stbtt_GetGlyphHMetrics (&font->font_info, glyph, &advance, &lsb);

	font_char_t char_info = {};
	char_info.xadvance = advance * font->font_scale;

	if (x1-x0 <= 0) return char_info;

	int32_t  sw         = (x1-x0) + pad*2;
	int32_t  sh         = (y1-y0) + pad*2;
	int64_t  rect_idx   = rect_atlas_add(&font->atlas, sw, sh);
	if (rect_idx == -1) {
		font_upsize_texture(font);
		rect_idx = rect_atlas_add(&font->atlas, sw, sh);
		to_u     = 1.0f / font->atlas.w; 
		to_v     = 1.0f / font->atlas.h;
	}
	recti_t  rect       = font->atlas.packed[rect_idx];
	recti_t  rect_unpad = { rect.x + pad, rect.y + pad, rect.w - pad * 2, rect.h - pad * 2};

	char_info.x0 =  x0-0.5f;
	char_info.y0 = -y0-0.5f;
	char_info.x1 =  x1+0.5f;
	char_info.y1 = -y1+0.5f;
	char_info.u0 = (rect_unpad.x-0.5f) * to_u;
	char_info.v0 = (rect_unpad.y-0.5f) * to_v;
	char_info.u1 = (rect_unpad.x+rect_unpad.w+0.5f) * to_u;
	char_info.v1 = (rect_unpad.y+rect_unpad.h+0.5f) * to_v;
	return char_info;
}

///////////////////////////////////////////

void font_render_glyph(font_t font, int32_t glyph, const font_char_t *ch) {
	int32_t x = ch->u0 * font->atlas.w + 0.5f;
	int32_t y = ch->v0 * font->atlas.h + 0.5f;
	int32_t w = (ch->u1 * font->atlas.w) - x - 0.5f;
	int32_t h = (ch->v1 * font->atlas.h) - y - 0.5f;

	// This is a really simple low quality render, very fast
	//stbtt_MakeGlyphBitmap(&font->font_info, &font->atlas_data[x + y * font->atlas.w], w, h, font->atlas.w, font->font_scale, font->font_scale, glyph);
	//return;

	// This is a multisampling high quality render, about 7x slower?
	int32_t multisample = 3;

	// Following code based on stbtt_GetGlyphBitmap, but modified to always
	// produce a bitmap that's a multiple of `multisample`
	int ix0,iy0,ix1,iy1;
	stbtt_vertex *vertices;
	int num_verts = stbtt_GetGlyphShape(&font->font_info, glyph, &vertices);

	stbtt_GetGlyphBitmapBoxSubpixel(&font->font_info, glyph, font->font_scale*multisample, font->font_scale*multisample, 0, 0, &ix0,&iy0,&ix1,&iy1);

	// now we get the size
	stbtt__bitmap gbm;
	gbm.w = w*multisample;
	gbm.h = h*multisample;
	gbm.pixels = (unsigned char *)malloc(gbm.w * gbm.h);
	gbm.stride = gbm.w;
	stbtt_Rasterize(&gbm, 0.35f, vertices, num_verts, font->font_scale*multisample, font->font_scale*multisample, 0, 0, ix0, iy0, 1, nullptr);
	free(vertices);

	// Now average the multisamples to get a final value, and add it to the
	// atlas data.
	for (int32_t py = 0; py < gbm.h; py+=multisample) {
		int32_t yoff = (y + py/multisample) * font->atlas.w;
	for (int32_t px = 0; px < gbm.w; px+=multisample) {
		int32_t total = 0;
		for (int32_t oy = 0; oy < multisample; oy+=1) {
			int32_t oyoff = (py+oy) * gbm.w;
		for (int32_t ox = 0; ox < multisample; ox+=1) {
			total += gbm.pixels[(px+ox) + oyoff];
		}}
		total = total / 9;

		font->atlas_data[(x + px/multisample) + yoff] = total;
	}}
	free(gbm.pixels);
}

///////////////////////////////////////////

inline void font_char_reuv(font_char_t *ch, float scale_x, float scale_y) {
	ch->u0 = ch->u0 * scale_x;
	ch->v0 = ch->v0 * scale_y;
	ch->u1 = ch->u1 * scale_x;
	ch->v1 = ch->v1 * scale_y;
}

void font_upsize_texture(font_t font) {
	// Double the space horizontally first, then double it vertically the
	// next time.
	int32_t new_w = font->atlas.w;
	int32_t new_h = font->atlas.h;
	if (new_w == new_h) { new_w = new_w * 2; font->atlas.free_space.add({font->atlas.w, 0, font->atlas.w,   font->atlas.h}); }
	else                { new_h = new_h * 2; font->atlas.free_space.add({0, font->atlas.h, font->atlas.w*2, font->atlas.h}); }
	float scale_x = font->atlas.w / (float)new_w;
	float scale_y = font->atlas.h / (float)new_h;

	// Move all our glyphs over to a new chunk of memory that uses the larger
	// size. We'll copy glyph by glyph to make sorting easier later on.
	uint8_t *new_data = sk_malloc_t(uint8_t, new_w*new_h);
	memset(new_data, 0, new_w * new_h);
	for (size_t i = 0; i < font->glyph_map.items.count; i++) {
		font_char_t ch     = font->glyph_map.items[i];
		font_char_t new_ch = ch;
		font_char_reuv(&new_ch, scale_x, scale_y);
		font->glyph_map.items[i] = new_ch;

		// Copy memory
		recti_t src = {ch.u0 * font->atlas.w, ch.v0 * font->atlas.h, (ch.u1-ch.u0) * font->atlas.w, (ch.v1-ch.v0) * font->atlas.h };
		for (size_t y = 0; y < src.h; y++) { 
			memcpy(&new_data[src.x + (src.y+y)*new_w], &font->atlas_data[src.x + (src.y+y)*font->atlas.w], src.w * sizeof(uint8_t));
		}
	}
	// Update the rest of our rectangles
	for (size_t i = 32; i < 128;                             i++) font_char_reuv(&font->characters         [i], scale_x, scale_y);
	for (size_t i = 0;  i < font->character_map.items.count; i++) font_char_reuv(&font->character_map.items[i], scale_x, scale_y);
	
	// This could be a faster copy, but may not make a big difference. Also
	// doesn't allow for copying to new locations.
	/*for (size_t i = 0;  i < font->glyph_map.items.count; i++) font_char_reuv(&font->glyph_map.items[i], scale_x, scale_y);
	for (int32_t y = 0; y < font->atlas.h; y++) {
		memcpy(&new_data[y * new_w], &font->atlas_data[y * font->atlas.w], font->atlas.w * sizeof(uint8_t));
	}*/

	// Update the atlas to the new values
	free(font->atlas_data);
	font->atlas_data = new_data;
	font->atlas.w = new_w;
	font->atlas.h = new_h;
}

///////////////////////////////////////////

void font_update_texture(font_t font) {
	if (font->font_tex == nullptr) {
		font->font_tex = tex_create(tex_type_image, tex_format_r8);
	}
	tex_set_colors(font->font_tex, font->atlas.w, font->atlas.h, font->atlas_data);
}

///////////////////////////////////////////

int64_t font_add_character(font_t font, char32_t character) {
	int64_t index   = -1;
	int32_t glyph   = stbtt_FindGlyphIndex(&font->font_info, character);
	int64_t g_index = font->glyph_map.contains(glyph);
	if (g_index >= 0) {
		if (character < 128) {
			font->characters[character] = font->glyph_map.items[g_index];
			index = -1;
		} else index = font->character_map.add(character, font->glyph_map.items[g_index]);
	} else {
		font_char_t ch = font_place_glyph(font, glyph);
		if (character < 128) {
			font->characters[character] = ch;
			index = -1;
		} else index = font->character_map.add(character, ch);
		font->glyph_map   .add(glyph, ch);
		font->update_queue.add(glyph);
	}
	return index;
}

///////////////////////////////////////////

const font_char_t *font_get_glyph(font_t font, char32_t character) {
	if (character < 128)
		return &font->characters[character];

	int64_t index = font->character_map.contains(character);
	if (index < 0) {
		index = font_add_character(font, character);
	}
	return &font->character_map.items[index];
}

///////////////////////////////////////////

void font_update_cache(font_t font) {
	for (size_t i = 0; i < font->update_queue.count; i++) {
		font_render_glyph(font, font->update_queue[i], font->glyph_map.get(font->update_queue[i]));
	}
	if (font->update_queue.count > 0) {
		font_update_texture(font);
		font->update_queue.clear();
	}
}

///////////////////////////////////////////

void font_update_fonts() {
	for (size_t i = 0; i < font_list.count; i++) {
		font_update_cache(font_list[i]);
	}
}

} // namespace sk
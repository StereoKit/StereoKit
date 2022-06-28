#include "font.h"

#pragma warning(push)
#pragma warning(disable : 26451 26819 6387 6011 6385 )
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,y) sk::sk_malloc(x)
#define STBTT_free(x,y) sk::_sk_free(x)
#include "../libraries/stb_truetype.h"
#pragma warning(pop)

#include "../libraries/aileron_font_data.h"
#include "../libraries/ferr_hash.h"
#include "../libraries/stref.h"
#include "../rect_atlas.h"
#include "../platforms/platform_utils.h"
#include "../sk_memory.h"

#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

namespace sk {

typedef struct font_source_t {
	int64_t        name_hash;
	char          *name;
	int32_t        references;
	stbtt_fontinfo info;
	void          *file;
	float          scale;
	float          char_height;
} font_source_t;

array_t<font_t>        font_list       = {};
array_t<font_source_t> font_sources    = {};
const int32_t          font_resolution = 64;

///////////////////////////////////////////

font_glyph_t font_find_glyph    (font_t font, char32_t character);
font_char_t  font_place_glyph   (font_t font, font_glyph_t glyph);
void         font_render_glyph  (font_t font, font_glyph_t glyph, const font_char_t *ch);
int32_t      font_add_character (font_t font, char32_t character);
void         font_upsize_texture(font_t font);
void         font_update_texture(font_t font);
void         font_update_cache  (font_t font);

int32_t      font_source_add     (const char *filename);
int32_t      font_source_add_data(const char *name, const void *data, size_t data_size);
void         font_source_release (int32_t id);

///////////////////////////////////////////

int32_t font_source_add(const char *filename) {
	int64_t hash = hash_fnv64_string(filename);
	int32_t id   = font_sources.index_where(&font_source_t::name_hash, hash);

	if (id == -1) {
		font_source_t new_file = {};
		new_file.name_hash = hash;
		new_file.name      = string_copy(filename);
		id = font_sources.add(new_file);
	}
	font_sources[id].references += 1;

	if (font_sources[id].references == 1) {
		size_t length;
		if (!platform_read_file(filename, (void **)&font_sources[id].file, &length)) {
			log_warnf("Font file failed to load: %s", filename);
		} else {
			stbtt_InitFont(&font_sources[id].info, (const unsigned char *)font_sources[id].file, stbtt_GetFontOffsetForIndex((const unsigned char *)font_sources[id].file,0));
			font_sources[id].scale = stbtt_ScaleForPixelHeight(&font_sources[id].info, (float)font_resolution);
			int32_t x0, y0, x1, y1;
			stbtt_GetCodepointBox(&font_sources[id].info, 'T', &x0, &y0, &x1, &y1);
			font_sources[id].char_height = y1 * font_sources[id].scale;
		}
	}

	return font_sources[id].file == nullptr
		? -1
		: id;
}

///////////////////////////////////////////

int32_t font_source_add_data(const char *name, const void *data, size_t data_size) {
	int64_t hash = hash_fnv64_string(name);
	int32_t id   = font_sources.index_where(&font_source_t::name_hash, hash);

	if (id == -1) {
		font_source_t new_file = {};
		new_file.name_hash = hash;
		new_file.name      = string_copy(name);
		id = font_sources.add(new_file);
	}
	font_sources[id].references += 1;

	if (font_sources[id].references == 1) {
		font_sources[id].file = sk_malloc(data_size);
		memcpy(font_sources[id].file, data, data_size);

		stbtt_InitFont(&font_sources[id].info, (const unsigned char *)font_sources[id].file, stbtt_GetFontOffsetForIndex((const unsigned char *)font_sources[id].file,0));
		font_sources[id].scale = stbtt_ScaleForPixelHeight(&font_sources[id].info, (float)font_resolution);
		int32_t x0, y0, x1, y1;
		stbtt_GetCodepointBox(&font_sources[id].info, 'T', &x0, &y0, &x1, &y1);
		font_sources[id].char_height = y1 * font_sources[id].scale;
	}

	return font_sources[id].file == nullptr
		? -1
		: id;
}

///////////////////////////////////////////

void font_source_release(int32_t id) {
	assert(font_sources[id].references > 0);

	font_sources[id].references -= 1;
	if (font_sources[id].references == 0) {
		sk_free(font_sources[id].file);
		font_sources[id].file = nullptr;
		font_sources[id].info = {};
	}
}

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
	return font_create_files(&file, 1);
}

///////////////////////////////////////////

bool font_setup(font_t font) {
	if (font->font_ids.count == 0) {
		log_err("All font files provided to font_create_files were invalid!");
		return false;
	}

	const int32_t atlas_resolution_x = 256;
	const int32_t atlas_resolution_y = 256;
	font->atlas      = rect_atlas_create( atlas_resolution_x, atlas_resolution_y );
	font->atlas_data = sk_malloc_t(uint8_t, font->atlas.w * font->atlas.h);
	memset(font->atlas_data, 0, font->atlas.w * font->atlas.h);

	for (char32_t i = 65; i < 128; i++) font_add_character(font, i);
	for (char32_t i = 32; i < 65;  i++) font_add_character(font, i);
	font_update_cache(font);

	// Get information about character sizes for this font
	font_source_t *src = &font_sources[font->font_ids[0]];
	int32_t x0, y0, x1, y1;
	stbtt_GetCodepointBox(&src->info, 'T', &x0, &y0, &x1, &y1);
	int32_t ascend, descend, gap;
	stbtt_GetFontVMetrics(&src->info, &ascend, &descend, &gap);
	int32_t advance, lsb; 
	stbtt_GetGlyphHMetrics(&src->info, ' ', &advance, &lsb);
	font->space_width       = advance / (float)y1;
	font->character_ascend  = ascend  / (float)y1;
	font->character_descend = descend / (float)y1;
	font->line_gap          = gap     / (float)y1;
	font->characters['\t'].xadvance = font->space_width * 2;

	return true;
}

///////////////////////////////////////////

font_t font_create_default() { 
	font_t result = font_find("sk_font::default");
	if (result != nullptr)
		return result;
	result = (font_t)assets_allocate(asset_type_font);
	assets_set_id(&result->header, "sk_font::default");

	int32_t id = font_source_add_data("sk_font::default", aileron_font_ttf, aileron_font_ttf_len);
	if (id >= 0)
		result->font_ids.add(id);

	if (!font_setup(result)) {
		font_release(result);
		return nullptr;
	}

	font_list.add(result);
	return result;
}

///////////////////////////////////////////

font_t font_create_files(const char **files, int32_t file_count) {
	if (file_count <= 0) {
		log_err("No font files provided to font_create_files");
		return nullptr;
	}

	// Hash the names of all of the files together
	uint64_t hash = HASH_FNV64_START;
	for (int32_t i = 0; i < file_count; i++) {
		hash = hash_fnv64_string(files[i], hash);
	}
	char file_id[64];
	snprintf(file_id, sizeof(file_id), "sk_font::%" PRIu64, hash);

	font_t result = font_find(file_id);
	if (result != nullptr)
		return result;
	result = (font_t)assets_allocate(asset_type_font);
	assets_set_id(&result->header, file_id);

	// Load relevant fonts
	result->font_ids.resize(file_count);
	for (int32_t i = 0; i < file_count; i++) {
		int32_t id = font_source_add(files[i]);
		if (id >= 0)
			result->font_ids.add(id);
	}

	if (!font_setup(result)) {
		font_release(result);
		return nullptr;
	}

	font_list.add(result);
	return result;
}

///////////////////////////////////////////

void font_set_id(font_t font, const char* id) {
	assets_set_id(&font->header, id);
}

///////////////////////////////////////////

void font_addref(font_t font) {
	assets_addref(&font->header);
}

///////////////////////////////////////////

void font_release(font_t font) {
	if (font == nullptr)
		return;
	assets_releaseref(&font->header);
}

///////////////////////////////////////////

void font_destroy(font_t font) {
	int32_t idx = font_list.index_of(font);
	if (idx >= 0)
		font_list.remove(idx);

	for (int32_t i = 0; i < font->font_ids.count; i++) {
		font_source_release(font->font_ids[i]);
	}

	tex_release       ( font->font_tex);
	rect_atlas_destroy(&font->atlas);
	sk_free           ( font->atlas_data);
	font->character_map.free();
	font->update_queue .free();
	font->font_ids     .free();

	*font = {};
}

///////////////////////////////////////////

tex_t font_get_tex(font_t font) {
	tex_addref(font->font_tex);
	return font->font_tex;
}

///////////////////////////////////////////

font_char_t font_place_glyph(font_t font, font_glyph_t glyph) {
	if (glyph.idx == 0) return {};
	font_source_t *source = &font_sources[glyph.font];

	const int32_t pad = 4;
	float   to_u = 1.0f / font->atlas.w;
	float   to_v = 1.0f / font->atlas.h;

	int advance, lsb;
	int x0, x1, y0, y1;
	stbtt_GetGlyphBitmapBox(&source->info, glyph.idx, source->scale, source->scale, &x0, &y0, &x1, &y1);
	stbtt_GetGlyphHMetrics (&source->info, glyph.idx, &advance, &lsb);

	font_char_t char_info = {};
	char_info.xadvance = (advance/source->char_height) * source->scale;

	if (x1-x0 <= 0) return char_info;

	int32_t  sw         = (x1-x0) + pad;
	int32_t  sh         = (y1-y0) + pad;
	int32_t  rect_idx   = rect_atlas_add(&font->atlas, sw, sh);
	if (rect_idx == -1) {
		font_upsize_texture(font);
		rect_idx = rect_atlas_add(&font->atlas, sw, sh);
		to_u     = 1.0f / font->atlas.w; 
		to_v     = 1.0f / font->atlas.h;
	}
	recti_t  rect       = font->atlas.packed[rect_idx];
	recti_t  rect_unpad = { rect.x, rect.y, rect.w - pad, rect.h - pad};

	char_info.x0 = ( x0-0.5f) / source->char_height;
	char_info.y0 = (-y0-0.5f) / source->char_height;
	char_info.x1 = ( x1+0.5f) / source->char_height;
	char_info.y1 = (-y1+0.5f) / source->char_height;
	char_info.u0 = (rect_unpad.x-0.5f) * to_u;
	char_info.v0 = (rect_unpad.y-0.5f) * to_v;
	char_info.u1 = (rect_unpad.x+rect_unpad.w+0.5f) * to_u;
	char_info.v1 = (rect_unpad.y+rect_unpad.h+0.5f) * to_v;
	return char_info;
}

///////////////////////////////////////////

void font_render_glyph(font_t font, font_glyph_t glyph, const font_char_t *ch) {
	int32_t x = (int32_t)(ch->u0 * font->atlas.w + 0.5f);
	int32_t y = (int32_t)(ch->v0 * font->atlas.h + 0.5f);
	int32_t w = (int32_t)((ch->u1 * font->atlas.w) - x - 0.5f);
	int32_t h = (int32_t)((ch->v1 * font->atlas.h) - y - 0.5f);
	font_source_t *source = &font_sources[glyph.font];

	// This is a really simple low quality render, very fast
	//stbtt_MakeGlyphBitmap(&font->font_info, &font->atlas_data[x + y * font->atlas.w], w, h, font->atlas.w, font->font_scale, font->font_scale, glyph);
	//return;

	// This is a multisampling high quality render, about 7x slower?
	const int32_t multisample = 3;

	// Following code based on stbtt_GetGlyphBitmap, but modified to always
	// produce a bitmap that's a multiple of `multisample`
	int ix0,iy0,ix1,iy1;
	stbtt_vertex *vertices;
	int num_verts = stbtt_GetGlyphShape(&source->info, glyph.idx, &vertices);

	stbtt_GetGlyphBitmapBoxSubpixel(&source->info, glyph.idx, source->scale*multisample, source->scale*multisample, 0, 0, &ix0,&iy0,&ix1,&iy1);

	// now we get the size
	stbtt__bitmap gbm;
	gbm.w = w*multisample;
	gbm.h = h*multisample;
	gbm.pixels = (unsigned char *)sk_malloc(gbm.w * gbm.h);
	gbm.stride = gbm.w;
	stbtt_Rasterize(&gbm, 0.35f, vertices, num_verts, source->scale*multisample, source->scale*multisample, 0, 0, ix0, iy0, 1, nullptr);
	sk_free(vertices);

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
		total = total / (multisample*multisample);

		font->atlas_data[(x + px/multisample) + yoff] = (uint8_t)total;
	}}
	sk_free(gbm.pixels);
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
	if (new_w == new_h) { new_w = new_w * 2; font->atlas.free_space.add({font->atlas.w, 0, font->atlas.w, font->atlas.h}); }
	else                { new_h = new_h * 2; font->atlas.free_space.add({0, font->atlas.h, font->atlas.w, font->atlas.h}); }
	float scale_x = font->atlas.w / (float)new_w;
	float scale_y = font->atlas.h / (float)new_h;

	// Move all our glyphs over to a new chunk of memory that uses the larger
	// size. We'll copy glyph by glyph to make sorting easier later on.
	uint8_t *new_data = sk_malloc_t(uint8_t, new_w*new_h);
	memset(new_data, 0, new_w * new_h);
	for (int32_t i = 0; i < font->glyph_map.items.count; i++) {
		font_char_t ch     = font->glyph_map.items[i];
		font_char_t new_ch = ch;
		font_char_reuv(&new_ch, scale_x, scale_y);
		font->glyph_map.items[i] = new_ch;

		// Copy memory
		recti_t src = {
			(int32_t)(ch.u0 * font->atlas.w),
			(int32_t)(ch.v0 * font->atlas.h),
			(int32_t)((ch.u1-ch.u0) * font->atlas.w),
			(int32_t)((ch.v1-ch.v0) * font->atlas.h) };
		for (int32_t y = 0; y < src.h; y++) { 
			memcpy(&new_data[src.x + (src.y+y)*new_w], &font->atlas_data[src.x + (src.y+y)*font->atlas.w], src.w * sizeof(uint8_t));
		}
	}
	// Update the rest of our rectangles
	for (int32_t i = 32; i < 128;                             i++) font_char_reuv(&font->characters         [i], scale_x, scale_y);
	for (int32_t i = 0;  i < font->character_map.items.count; i++) font_char_reuv(&font->character_map.items[i], scale_x, scale_y);
	
	// This could be a faster copy, but may not make a big difference. Also
	// doesn't allow for copying to new locations.
	/*for (int32_t i = 0;  i < font->glyph_map.items.count; i++) font_char_reuv(&font->glyph_map.items[i], scale_x, scale_y);
	for (int32_t y = 0; y < font->atlas.h; y++) {
		memcpy(&new_data[y * new_w], &font->atlas_data[y * font->atlas.w], font->atlas.w * sizeof(uint8_t));
	}*/

	// Update the atlas to the new values
	sk_free(font->atlas_data);
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

font_glyph_t font_find_glyph(font_t font, char32_t character) {
	for (int32_t i = 0; i < font->font_ids.count; i++) {
		int32_t glyph = stbtt_FindGlyphIndex(&font_sources[font->font_ids[i]].info, character);
		if (glyph > 0) {
			return { glyph, font->font_ids[i] };
		}
	}
	return { 0, -1 };
}

///////////////////////////////////////////

int32_t font_add_character(font_t font, char32_t character) {
	int32_t      index   = -1;
	font_glyph_t glyph   = font_find_glyph(font, character);
	if (glyph.idx == 0) return font->character_map.add(character, font->characters['?']);

	int32_t g_index = font->glyph_map.contains(glyph);
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

	int32_t index = font->character_map.contains(character);
	if (index < 0) {
		index = font_add_character(font, character);
	}
	return &font->character_map.items[index];
}

///////////////////////////////////////////

void font_update_cache(font_t font) {
	for (int32_t i = 0; i < font->update_queue.count; i++) {
		font_render_glyph(font, font->update_queue[i], font->glyph_map.get(font->update_queue[i]));
	}
	if (font->update_queue.count > 0) {
		font_update_texture(font);
		font->update_queue.clear();
	}
}

///////////////////////////////////////////

void font_update_fonts() {
	for (int32_t i = 0; i < font_list.count; i++) {
		font_update_cache(font_list[i]);
	}
}

} // namespace sk
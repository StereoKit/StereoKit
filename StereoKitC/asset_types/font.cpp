#include "font.h"

#include "../libraries/stb_truetype.h"
#include "../libraries/aileron_font_data.h"
#include "../libraries/ferr_hash.h"
#include "../libraries/stref.h"
#include "../libraries/sk_fontfile.h"
#include "../rect_atlas.h"
#include "../platforms/platform.h"
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
	float          total_height; // px height from descender to ascender
	float          cap_height;   // px height of baseline to the top of a normal capital letter 'T'
	float          descender;    // px layout (positive) depth below the baseline
	float          ascender;     // px layout height above baseline
	float          render_descender; // px maximum possible height above baseline
	float          render_ascender;  // px maximum (positive) depth below the baseline
} font_source_t;

/*
Here's how a number of these typography terms look on text. Note that ascenders
and descenders are not quite so clear cut, as plenty of characters go above and
below! In SK here, we separate this into layout and render ascender/descenders.
Layout asc/desc values come from the font vertical metrics, and render asc/desc
comes from the font's reported bounding box.

¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯ll¯¯¯¯¯¯¯¯¯¯¯¯¯¯ ascender
¯¯¯¯TTTTTTTTTT¯¯¯ll¯¯¯¯¯¯¯¯¯¯¯¯¯¯ cap_height
        TT       ll              
        TT       ll  vy    yy    
        TT       ll   vy  yy     
        TT       ll    vyyy      
________TT_______ll_____yy_______ baseline
                       yy        
______________________yy_________ descender

Note that when font size is specified (like 16px), this is refering to the
total height from the layout ascender to the descender.

Spacing between lines of text varies between different text rendering engines,
and here SK opts to follow the browser model for familiarity. In browsers, the
next line's ascender starts right after the current line's descender. Line
spacing is then a result of defining the line's total height as a multiple of
the text size.

Line height: 1    Line height: 1.4

 ¯¯¯¯l¯¯¯¯¯¯¯¯     ¯¯¯¯l¯¯¯¯¯¯¯¯
     l                 l        
     l y y             l y y    
     l  y              l  y     
 ______y______           y      
 ¯¯¯¯l¯¯¯¯¯¯¯¯                  
     l             _____________
     l y y         ¯¯¯¯l¯¯¯¯¯¯¯¯
     l  y              l        
 ______y______         l y y    
                       l  y     
                   ______y______
*/

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

int32_t      font_source_add      (const char *filename);
int32_t      font_source_add_data (const char *name, const void *data, size_t data_size);
void         font_source_load_data(font_source_t* font);
void         font_source_release  (int32_t id);

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
	font_source_t* font = &font_sources[id];
	font->references += 1;

	if (font->references == 1) {
		size_t length;
		if (platform_read_file(filename, (void **)&font->file, &length)) {
			font_source_load_data(font);
		} else {
			log_warnf("Font file failed to load: %s", filename);
		}
	}

	return font->file == nullptr
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
	font_source_t* font = &font_sources[id];
	font->references += 1;

	if (font->references == 1) {
		font->file = sk_malloc(data_size);
		memcpy(font->file, data, data_size);
		font_source_load_data(font);
	}

	return font->file == nullptr
		? -1
		: id;
}

///////////////////////////////////////////

void font_source_load_data(font_source_t* font) {
	stbtt_InitFont(&font->info, (const unsigned char *)font->file, stbtt_GetFontOffsetForIndex((const unsigned char *)font->file,0));
	font->scale = stbtt_ScaleForPixelHeight(&font->info, (float)font_resolution);

	int32_t ascender, descender, line_gap;
	if (stbtt_GetFontVMetricsOS2(&font->info, &ascender, &descender, &line_gap)) {
		font->ascender  = ceilf(      ascender   * font->scale);
		font->descender = ceilf(fabsf(descender) * font->scale);
	} else {
		stbtt_GetFontVMetrics(&font->info, &ascender, &descender, &line_gap);
		font->ascender  = ceilf(      ascender   * font->scale);
		font->descender = ceilf(fabsf(descender) * font->scale);
	}
	font->total_height = font->ascender + font->descender;

	int32_t x0, y0, x1, y1;
	stbtt_GetFontBoundingBox(&font->info, &x0, &y0, &x1, &y1);
	font->render_ascender  = ceilf(y1        * font->scale);
	font->render_descender = ceilf(fabsf(y0) * font->scale);

	stbtt_GetCodepointBitmapBox(&font->info, 'T', font->scale, font->scale, &x0, &y0, &x1, &y1);
	font->cap_height = y1-y0;
}

///////////////////////////////////////////

void font_source_release(int32_t id) {
	assert(font_sources[id].references > 0);

	font_sources[id].references -= 1;
	if (font_sources[id].references == 0) {
		sk_free(font_sources[id].file);
		sk_free(font_sources[id].name);
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
	float total_height = src->ascender + src->descender;

	int32_t x0, y0, x1, y1;
	stbtt_GetCodepointBox(&src->info, 'T', &x0, &y0, &x1, &y1);
	int32_t advance, lsb;
	stbtt_GetGlyphHMetrics(&src->info, ' ', &advance, &lsb);
	font->characters['\t'].xadvance = (advance / (float)y1) * 2;
	font->cap_height_pct     = src->cap_height              / total_height;
	font->layout_ascend_pct  = src->ascender                / total_height;
	font->layout_descend_pct = fabsf(src->descender)        / total_height;
	font->render_ascend_pct  = src->render_ascender         / total_height;
	font->render_descend_pct = fabsf(src->render_descender) / total_height;

	return true;
}

///////////////////////////////////////////

font_t font_create_default() { 
	font_t result = font_find("default/font");
	if (result != nullptr)
		return result;
	result = (font_t)assets_allocate(asset_type_font);
	assets_set_id(&result->header, "default/font");

	int32_t id = font_source_add_data("default/font", aileron_font_ttf, aileron_font_ttf_len);
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
	snprintf(file_id, sizeof(file_id), "sk/font/%" PRIu64, hash);

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

const char* font_get_id(const font_t font) {
	return font->header.id_text;
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
	font->glyph_map    .free();
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

//#define SK_USE_SDF
#if defined(SK_USE_SDF)
#define PAD_SIZE 4
#else
#define PAD_SIZE 0
#endif

font_char_t font_place_glyph(font_t font, font_glyph_t glyph) {
	if (glyph.idx == 0) return {};
	font_source_t *source = &font_sources[glyph.font];

	const int32_t pad_empty   = 2;
	const int32_t pad_content = PAD_SIZE;
	float   to_u = 1.0f / font->atlas.w;
	float   to_v = 1.0f / font->atlas.h;

	int advance, lsb;
	int x0, x1, y0, y1;
	stbtt_GetGlyphBitmapBox(&source->info, glyph.idx, source->scale, source->scale, &x0, &y0, &x1, &y1);
	stbtt_GetGlyphHMetrics (&source->info, glyph.idx, &advance, &lsb);

	font_char_t char_info = {};
	char_info.xadvance = (advance * source->scale) / source->total_height;

	if (x1-x0 <= 0) return char_info;

	int32_t  sw         = (x1-x0) + pad_empty*2 + pad_content*2;
	int32_t  sh         = (y1-y0) + pad_empty*2 + pad_content*2;
	int32_t  rect_idx   = rect_atlas_add(&font->atlas, sw, sh);
	if (rect_idx == -1) {
		font_upsize_texture(font);
		rect_idx = rect_atlas_add(&font->atlas, sw, sh);
		to_u     = 1.0f / font->atlas.w;
		to_v     = 1.0f / font->atlas.h;
	}
	rect_area_t rect       = font->atlas.packed[rect_idx];
	rect_area_t rect_unpad = { 
		rect.x+pad_empty+pad_content,
		rect.y+pad_empty+pad_content,
		rect.w - (pad_empty*2 + pad_content*2),
		rect.h - (pad_empty*2 + pad_content*2)};

	// Add half a pixel to the outside of the glyph's mesh, so when we're
	// sampling from the texture, we have some room for interpolation. Without
	// this, pixels adjacent to the edge of the mesh will look sharper, or like
	// they're cut off too early.
	const float half_pixel = 0.5f;
	char_info.x0 = ( x0-half_pixel) / source->total_height;
	char_info.y0 = (-y0+half_pixel) / source->total_height;
	char_info.x1 = ( x1+half_pixel) / source->total_height;
	char_info.y1 = (-y1-half_pixel) / source->total_height;
	char_info.u0 = (rect_unpad.x             -half_pixel) * to_u;
	char_info.v0 = (rect_unpad.y             -half_pixel) * to_v;
	char_info.u1 = (rect_unpad.x+rect_unpad.w+half_pixel) * to_u;
	char_info.v1 = (rect_unpad.y+rect_unpad.h+half_pixel) * to_v;
	return char_info;
}

///////////////////////////////////////////

void font_render_glyph(font_t font, font_glyph_t glyph, const font_char_t *ch) {
	// We don't store the pixel rect of the glyph frm font_place_glyph. Instead
	// we're recalculating the pixel rect from the UVs, which do directly map
	// to the rect, but also include some padding and other extra space.
	const int32_t pad_content = PAD_SIZE;
	const float   half_pixel  = 0.5f;
	int32_t x = (int32_t) (ch->u0 * font->atlas.w      + half_pixel)-pad_content;
	int32_t y = (int32_t) (ch->v0 * font->atlas.h      + half_pixel)-pad_content;
	int32_t w = (int32_t)((ch->u1 * font->atlas.w) - x - half_pixel);
	int32_t h = (int32_t)((ch->v1 * font->atlas.h) - y - half_pixel);
	font_source_t *source = &font_sources[glyph.font];

	stbtt__bitmap gbm;

	// SDF based glyph generation
#if defined (SK_USE_SDF)
	int ix0, iy0;
	const int32_t multisample = 1;
	int width, height;
	gbm.pixels = stbtt_GetGlyphSDF(&source->info, source->scale, glyph.idx, pad_content, 128, 10, &width, &height, &ix0, &iy0);
	gbm.w      = width;
	gbm.h      = height;
	gbm.stride = gbm.w;
#else
	// Raster based glyph generation
	// This is a multisampling high quality render, about 7x slower?

	// Following code based on stbtt_GetGlyphBitmap, but modified to always
	// produce a bitmap that's a multiple of `multisample`
	int ix0, iy0, ix1, iy1;
	const int32_t multisample = 3;

	stbtt_vertex* vertices;
	int num_verts = stbtt_GetGlyphShape(&source->info, glyph.idx, &vertices);
	stbtt_GetGlyphBitmapBoxSubpixel(&source->info, glyph.idx, source->scale * multisample, source->scale * multisample, 0, 0, &ix0, &iy0, &ix1, &iy1);
	// now we get the size
	gbm.w      = w*multisample;
	gbm.h      = h*multisample;
	gbm.pixels = (unsigned char *)sk_malloc(gbm.w * gbm.h);
	gbm.stride = gbm.w;
	stbtt_Rasterize(&gbm, 0.35f, vertices, num_verts, source->scale*multisample, source->scale*multisample, 0, 0, ix0, iy0, 1, nullptr);
	sk_free(vertices);
#endif

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
	for (int32_t i = 0; i < font->glyph_map.capacity; i++) {
		if (font->glyph_map.items[i].hash == 0) continue;
		
		font_char_t ch     = font->glyph_map.items[i].value;
		font_char_t new_ch = ch;
		font_char_reuv(&new_ch, scale_x, scale_y);
		font->glyph_map.items[i].value = new_ch;

		// Copy memory
		rect_area_t src = {
			(int32_t)(ch.u0 * font->atlas.w),
			(int32_t)(ch.v0 * font->atlas.h),
			(int32_t)((ch.u1-ch.u0) * font->atlas.w),
			(int32_t)((ch.v1-ch.v0) * font->atlas.h) };
		for (int32_t y = 0; y < src.h; y++) { 
			memcpy(&new_data[src.x + (src.y+y)*new_w], &font->atlas_data[src.x + (src.y+y)*font->atlas.w], src.w * sizeof(uint8_t));
		}
	}
	// Update the rest of our rectangles
	for (int32_t i = 32; i < 128;                          i++) font_char_reuv(&font->characters         [i],       scale_x, scale_y);
	for (int32_t i = 0;  i < font->character_map.capacity; i++) font_char_reuv(&font->character_map.items[i].value, scale_x, scale_y);
	
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
		char* tex_id = string_append(nullptr, 2, font_get_id(font), "/atlas");
		tex_set_id(font->font_tex, tex_id);
		sk_free(tex_id);
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
	if (glyph.idx == 0) return font->character_map.set(character, font->characters['?']);

	int32_t g_index = font->glyph_map.contains(glyph);
	if (g_index >= 0) {
		if (character < 128) {
			font->characters[character] = font->glyph_map.items[g_index].value;
			index = -1;
		} else index = font->character_map.set(character, font->glyph_map.items[g_index].value);
	} else {
		font_char_t ch = font_place_glyph(font, glyph);
		if (character < 128) {
			font->characters[character] = ch;
			index = -1;
		} else index = font->character_map.set(character, ch);
		font->glyph_map   .set(glyph, ch);
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
	return &font->character_map.items[index].value;
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

///////////////////////////////////////////

font_t font_create_family(const char* font_family) {
	font_fallback_info_t* info;
	int32_t               info_count;
	fontfile_from_css(font_family, &info, &info_count);

	if (info_count == 0) {
		log_errf("No font files provided for the font_family %s.", font_family);
		return nullptr;
	}
	const char** files = sk_malloc_t(const char*, info_count);
	for (int32_t i = 0; i < info_count; i++) {
		files[i] = string_copy(info[i].filepath);
	}

	free(info);
	font_t font = font_create_files(files, info_count);
	return font;
}

} // namespace sk
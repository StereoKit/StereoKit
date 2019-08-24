#include "font.h"

#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_rect_pack.h"
#include "stb_truetype.h"

#include <stdio.h>

font_t font_find(const char *id) {
	font_t result = (font_t)assets_find(id);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}
	return nullptr;
}
font_t font_create(const char *file) {
	font_t result = font_find(file);
	if (result != nullptr)
		return result;
	result = (font_t)assets_allocate(asset_type_font, file);

	stbtt_fontinfo font;
	unsigned char *bitmap;
	int width,height,i,j,c =  'a', s = 20;

	FILE *fp;
	if (fopen_s(&fp, file, "rb") != 0 || fp == nullptr)
		return nullptr;

	// Get length of file
	fseek(fp, 0L, SEEK_END);
	size_t length = ftell(fp);
	rewind(fp);

	// Read the data
	unsigned char *data = (unsigned char *)malloc(sizeof(unsigned char) *length);
	if (data == nullptr) { fclose(fp); return false; }
	fread(data, 1, length, fp);
	fclose(fp);

	stbtt_InitFont(&font, data, stbtt_GetFontOffsetForIndex(data,0));
	//bitmap = stbtt_GetCodepointSDF(&font, 0, stbtt_ScaleForPixelHeight(&font, s), c, &width, &height, 0, 0);
	bitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, s), c, &width, &height, 0, 0);
	stbtt_bakedchar cdata[96];
	unsigned char temp_bitmap[256*256];
	stbtt_BakeFontBitmap(data, 0, 32.0, temp_bitmap, 256, 256, 32, 96, cdata);
	free(data);

	color32 *colors = (color32*)malloc(256 * 256 * sizeof(color32));
	for (size_t i = 0; i < 256*256; i++) {
		colors[i] = color32{ temp_bitmap[i], 0, 0, 0 };
	}
	result->font_tex = tex2d_create("fonttex", tex_type_image);
	tex2d_set_colors(result->font_tex, 256, 256, colors);

	return result;
}
void font_release(font_t font) {
	assets_releaseref(font->header);
}
void font_destroy(font_t font) {
	if (font->font_tex != nullptr)
		tex2d_release(font->font_tex);
	*font = {};
}
tex2d_t font_get_tex(font_t font) {
	return font->font_tex;
}
#include "font.h"

#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_rect_pack.h"
#include "stb_truetype.h"

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

}
void font_release(font_t font) {
	assets_releaseref(font->header);
}
void font_destroy(font_t font) {
	if (font->font_tex != nullptr)
		tex2d_release(font->font_tex);
	*font = {};
}
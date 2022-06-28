#include "defaults.h"
#include "../platforms/platform_utils.h"
#include "../stereokit.h"
#include "../shaders_builtin/shader_builtin.h"
#include "../asset_types/font.h"
#include "../libraries/stb_image.h"

#include <string.h>

namespace sk {

///////////////////////////////////////////

tex_t        sk_default_tex;
tex_t        sk_default_tex_black;
tex_t        sk_default_tex_gray;
tex_t        sk_default_tex_flat;
tex_t        sk_default_tex_rough;
tex_t        sk_default_tex_devtex;
tex_t        sk_default_tex_error;
tex_t        sk_default_cubemap;
mesh_t       sk_default_quad;
mesh_t       sk_default_screen_quad;
mesh_t       sk_default_sphere;
mesh_t       sk_default_cube;
shader_t     sk_default_shader;
shader_t     sk_default_shader_blit;
shader_t     sk_default_shader_pbr;
shader_t     sk_default_shader_pbr_clip;
shader_t     sk_default_shader_unlit;
shader_t     sk_default_shader_unlit_clip;
shader_t     sk_default_shader_font;
shader_t     sk_default_shader_equirect;
shader_t     sk_default_shader_ui;
shader_t     sk_default_shader_ui_box;
shader_t     sk_default_shader_ui_quadrant;
shader_t     sk_default_shader_sky;
shader_t     sk_default_shader_lines;
material_t   sk_default_material;
material_t   sk_default_material_pbr;
material_t   sk_default_material_pbr_clip;
material_t   sk_default_material_unlit;
material_t   sk_default_material_unlit_clip;
material_t   sk_default_material_equirect;
material_t   sk_default_material_font;
material_t   sk_default_material_ui;
material_t   sk_default_material_ui_box;
material_t   sk_default_material_ui_quadrant;
font_t       sk_default_font;
text_style_t sk_default_text_style;
sound_t      sk_default_click;
sound_t      sk_default_unclick;
sound_t      sk_default_grab;
sound_t      sk_default_ungrab;

///////////////////////////////////////////

tex_t defaults_texture(const char *id, color128 color) {
	tex_t result = tex_gen_color(color, 2, 2, tex_type_image_nomips, tex_format_rgba32_linear);
	tex_set_id(result, id);
	return result;
}

///////////////////////////////////////////

tex_t dev_texture(const char *id, color128 base_color, float contrast_boost) {
	tex_t result = tex_create();
	tex_set_id(result, id);

	const int32_t size          = 256;                 // Texture total size
	const int32_t slices        = 4;                   // Slice this up into 4x4 squares
	const int32_t slice_size    = size / slices;       // Size in px of each square
	const int32_t slice_half    = slice_size/2;        // precalculate half (for lines)
	const int32_t slice_quarter = slice_size/4;        // precalculate quarter (for lines)
	const int32_t checker_size  = size / (slices * 2); // Alternate core color in a checker pattern, with 2 checkers per slice

	vec3 lab = color_to_lab(base_color);
	color32 core_color  = color_to_32(base_color);
	color32 core_color2 = color_to_32(color_lab(lab.x * powf(0.9f,  contrast_boost), lab.y, lab.z, 1));
	color32 line_color  = color_to_32(color_lab(lab.x * powf(0.8f,  contrast_boost), lab.y, lab.z, 1));
	color32 line2_color = color_to_32(color_lab(lab.x * powf(0.75f, contrast_boost), lab.y, lab.z, 1));

	color32 *data   = sk_malloc_t(color32, size * size);
	for (int32_t y = 0; y < size; y++) {
		int ydist  = abs(slice_half    - ((y + slice_half   ) % slice_size));
		int ydist2 = abs(slice_quarter - ((y + slice_quarter) % slice_half));

		for (int32_t x = 0; x < size; x++) {
			int xdist  = abs(slice_half    - ((x + slice_half   ) % slice_size));
			int xdist2 = abs(slice_quarter - ((x + slice_quarter) % slice_half));

			int32_t i = x + y * size;
			if      (xdist < 2 || ydist <2) data[i] = line_color;
			else if (xdist2< 1 || ydist2<1) data[i] = line2_color;
			else                            data[i] = ((x/checker_size) + (y/checker_size)) %2 == 0 ? core_color : core_color2;
		}
	}

	tex_set_colors(result, size, size, data);
	return result;
}

///////////////////////////////////////////

bool defaults_init() {
	// Textures
	sk_default_tex       = defaults_texture(default_id_tex,       {1,1,1,1}         );
	sk_default_tex_black = defaults_texture(default_id_tex_black, {0,0,0,1}         );
	sk_default_tex_gray  = defaults_texture(default_id_tex_gray,  {0.5f,0.5f,0.5f,1});
	sk_default_tex_flat  = defaults_texture(default_id_tex_flat,  {0.5f,0.5f,1,1}   ); // Default for normal maps
	sk_default_tex_rough = defaults_texture(default_id_tex_rough, {1,1,0,1}         ); // Default for metal/roughness maps

	sk_default_tex_devtex = dev_texture(default_id_tex_devtex, { 1,1,   1,   1 }, 1);
	sk_default_tex_error  = dev_texture(default_id_tex_error,  { 1,0.7f,0.7f,1 }, 1);

	if (sk_default_tex       == nullptr ||
		sk_default_tex_black == nullptr ||
		sk_default_tex_gray  == nullptr ||
		sk_default_tex_flat  == nullptr ||
		sk_default_tex_rough == nullptr ||
		sk_default_tex_devtex== nullptr ||
		sk_default_tex_error == nullptr)
		return false;

	tex_set_loading_fallback(sk_default_tex_devtex);
	tex_set_error_fallback  (sk_default_tex_error);

	// Cubemap
	spherical_harmonics_t lighting = { {
		{ 0.74f,  0.74f,  0.73f}, 
		{ 0.24f,  0.25f,  0.26f}, 
		{ 0.09f,  0.09f,  0.09f}, 
		{ 0.05f,  0.05f,  0.06f}, 
		{-0.01f, -0.01f, -0.01f}, 
		{-0.03f, -0.03f, -0.03f}, 
		{ 0.00f,  0.00f,  0.00f}, 
		{-0.02f, -0.02f, -0.02f}, 
		{ 0.04f,  0.04f,  0.04f}, 
	} };
	render_set_skylight(lighting);
	sh_brightness(lighting, 0.75f);
	sk_default_cubemap = tex_gen_cubemap_sh(lighting, 16, 0.3f);
	tex_set_id(sk_default_cubemap, default_id_cubemap);
	render_set_skytex(sk_default_cubemap);
	render_enable_skytex(true);

	// Default quad mesh
	sk_default_quad = mesh_create();
	vert_t verts[4] = {
		{ vec3{-0.5f,-0.5f,0}, vec3{0,0,-1}, vec2{1,1}, color32{255,255,255,255} },
		{ vec3{ 0.5f,-0.5f,0}, vec3{0,0,-1}, vec2{0,1}, color32{255,255,255,255} },
		{ vec3{ 0.5f, 0.5f,0}, vec3{0,0,-1}, vec2{0,0}, color32{255,255,255,255} },
		{ vec3{-0.5f, 0.5f,0}, vec3{0,0,-1}, vec2{1,0}, color32{255,255,255,255} }, };
	vind_t inds[6] = { 2,1,0, 3,2,0 };
	mesh_set_data(sk_default_quad, verts, 4, inds, 6);
	
	// Default rendering quad
	sk_default_screen_quad = mesh_create();
	vert_t sq_verts[4] = {
		{ vec3{-1,-1,0}, vec3{0,0,1}, vec2{0,1}, color32{255,255,255,255} },
		{ vec3{ 1,-1,0}, vec3{0,0,1}, vec2{1,1}, color32{255,255,255,255} },
		{ vec3{ 1, 1,0}, vec3{0,0,1}, vec2{1,0}, color32{255,255,255,255} },
		{ vec3{-1, 1,0}, vec3{0,0,1}, vec2{0,0}, color32{255,255,255,255} }, };
	vind_t sq_inds[6] = { 0,1,2, 0,2,3 };
	mesh_set_data(sk_default_screen_quad, sq_verts, 4, sq_inds, 6);
	
	sk_default_cube   = mesh_gen_cube(vec3_one);
	sk_default_sphere = mesh_gen_sphere(1);

	mesh_set_id(sk_default_quad,        default_id_mesh_quad);
	mesh_set_id(sk_default_screen_quad, default_id_mesh_screen_quad);
	mesh_set_id(sk_default_cube,        default_id_mesh_cube);
	mesh_set_id(sk_default_sphere,      default_id_mesh_sphere);

	// Shaders
	int32_t size = 0;
	void*   data = nullptr;
#define SHADER_DECODE(shader_mem) { sk_free(data); data = stbi_zlib_decode_malloc((const char*)shader_mem, sizeof(shader_mem), &size); }
	SHADER_DECODE(sks_shader_builtin_default_hlsl_zip    ); sk_default_shader             = shader_create_mem(data, size);
	SHADER_DECODE(sks_shader_builtin_blit_hlsl_zip       ); sk_default_shader_blit        = shader_create_mem(data, size);
	SHADER_DECODE(sks_shader_builtin_unlit_hlsl_zip      ); sk_default_shader_unlit       = shader_create_mem(data, size);
	SHADER_DECODE(sks_shader_builtin_unlit_clip_hlsl_zip ); sk_default_shader_unlit_clip  = shader_create_mem(data, size);
	SHADER_DECODE(sks_shader_builtin_font_hlsl_zip       ); sk_default_shader_font        = shader_create_mem(data, size);
	SHADER_DECODE(sks_shader_builtin_equirect_hlsl_zip   ); sk_default_shader_equirect    = shader_create_mem(data, size);
	SHADER_DECODE(sks_shader_builtin_ui_hlsl_zip         ); sk_default_shader_ui          = shader_create_mem(data, size);
	SHADER_DECODE(sks_shader_builtin_ui_box_hlsl_zip     ); sk_default_shader_ui_box      = shader_create_mem(data, size);
	SHADER_DECODE(sks_shader_builtin_ui_quadrant_hlsl_zip); sk_default_shader_ui_quadrant = shader_create_mem(data, size);
	SHADER_DECODE(sks_shader_builtin_skybox_hlsl_zip     ); sk_default_shader_sky         = shader_create_mem(data, size);
	SHADER_DECODE(sks_shader_builtin_lines_hlsl_zip      ); sk_default_shader_lines       = shader_create_mem(data, size);
	SHADER_DECODE(sks_shader_builtin_pbr_hlsl_zip        ); sk_default_shader_pbr         = shader_create_mem(data, size);
	SHADER_DECODE(sks_shader_builtin_pbr_clip_hlsl_zip   ); sk_default_shader_pbr_clip    = shader_create_mem(data, size);
	sk_free(data);
#undef SHADER_DECODE
	
	// Android seems to give us a hard time about this one, so let's fall
	// back at least somewhat gently.
	if (sk_default_shader && !sk_default_shader_pbr) {
		sk_default_shader_pbr = sk_default_shader;
		shader_addref(sk_default_shader);
	}
	if (sk_default_shader && !sk_default_shader_pbr_clip) {
		sk_default_shader_pbr_clip = sk_default_shader;
		shader_addref(sk_default_shader);
	}

	if (sk_default_shader             == nullptr ||
		sk_default_shader_blit        == nullptr ||
		sk_default_shader_pbr         == nullptr ||
		sk_default_shader_pbr_clip    == nullptr ||
		sk_default_shader_unlit       == nullptr ||
		sk_default_shader_unlit_clip  == nullptr ||
		sk_default_shader_font        == nullptr ||
		sk_default_shader_equirect    == nullptr ||
		sk_default_shader_ui          == nullptr ||
		sk_default_shader_ui_box      == nullptr ||
		sk_default_shader_ui_quadrant == nullptr ||
		sk_default_shader_sky         == nullptr ||
		sk_default_shader_lines       == nullptr)
		return false;

	shader_set_id(sk_default_shader,             default_id_shader);
	shader_set_id(sk_default_shader_blit,        default_id_shader_blit);
	shader_set_id(sk_default_shader_pbr,         default_id_shader_pbr);
	shader_set_id(sk_default_shader_pbr_clip,    default_id_shader_pbr_clip);
	shader_set_id(sk_default_shader_unlit,       default_id_shader_unlit);
	shader_set_id(sk_default_shader_unlit_clip,  default_id_shader_unlit_clip);
	shader_set_id(sk_default_shader_font,        default_id_shader_font);
	shader_set_id(sk_default_shader_equirect,    default_id_shader_equirect);
	shader_set_id(sk_default_shader_ui,          default_id_shader_ui);
	shader_set_id(sk_default_shader_ui_box,      default_id_shader_ui_box);
	shader_set_id(sk_default_shader_ui_quadrant, default_id_shader_ui_quadrant);
	shader_set_id(sk_default_shader_sky,         default_id_shader_sky);
	shader_set_id(sk_default_shader_lines,       default_id_shader_lines);

	// Materials
	sk_default_material             = material_create(sk_default_shader);
	sk_default_material_pbr         = material_create(sk_default_shader_pbr);
	sk_default_material_pbr_clip    = material_create(sk_default_shader_pbr_clip);
	sk_default_material_unlit       = material_create(sk_default_shader_unlit);
	sk_default_material_unlit_clip  = material_create(sk_default_shader_unlit_clip);
	sk_default_material_equirect    = material_create(sk_default_shader_equirect);
	sk_default_material_font        = material_create(sk_default_shader_font);
	sk_default_material_ui          = material_create(sk_default_shader_ui);
	sk_default_material_ui_box      = material_create(sk_default_shader_ui_box);
	sk_default_material_ui_quadrant = material_create(sk_default_shader_ui_quadrant);

	if (sk_default_material          == nullptr ||
		sk_default_material_pbr      == nullptr ||
		sk_default_material_pbr_clip == nullptr ||
		sk_default_material_unlit    == nullptr ||
		sk_default_material_unlit_clip == nullptr ||
		sk_default_material_equirect == nullptr ||
		sk_default_material_font     == nullptr ||
		sk_default_material_ui       == nullptr ||
		sk_default_material_ui_box   == nullptr ||
		sk_default_material_ui_quadrant == nullptr)
		return false;

	material_set_id(sk_default_material,          default_id_material);
	material_set_id(sk_default_material_pbr,      default_id_material_pbr);
	material_set_id(sk_default_material_pbr_clip, default_id_material_pbr_clip);
	material_set_id(sk_default_material_unlit,    default_id_material_unlit);
	material_set_id(sk_default_material_unlit_clip, default_id_material_unlit_clip);
	material_set_id(sk_default_material_equirect, default_id_material_equirect);
	material_set_id(sk_default_material_font,     default_id_material_font);
	material_set_id(sk_default_material_ui,       default_id_material_ui);
	material_set_id(sk_default_material_ui_box,   default_id_material_ui_box);
	material_set_id(sk_default_material_ui_quadrant, default_id_material_ui_quadrant);

	material_set_texture(sk_default_material_font, "diffuse", sk_default_tex);
	material_set_cull(sk_default_material_ui_box, cull_none);

	// These can be paired with changes in the shader for antialiased edges.
	// material_set_transparency(sk_default_material_ui_box, transparency_blend);
	// material_set_depth_write(sk_default_material_ui_box, false);

	// Text!
	sk_default_font = platform_default_font();
	if (sk_default_font == nullptr)
		return false;
	font_set_id(sk_default_font, default_id_font);
	sk_default_text_style = text_make_style_mat(sk_default_font, 20 * mm2m, sk_default_material_font, color128{ 1,1,1,1 });

	// Sounds
	sk_default_click = sound_generate([](float t){
		float x = t / 0.03f;
		float band1 = sinf(t*7500) * (x * powf(1 - x, 10)) / 0.03f;
		float band2 = sinf(t*4750) * (x * powf(1 - x, 12)) / 0.03f;
		float band3 = sinf(t*2500) * (x * powf(1 - x, 12)) / 0.03f;
		float band4 = sinf(t*500)  * (x * powf(1 - x, 6))  / 0.03f;

		return (band1*0.6f + band2*0.2f + band3*0.1f + band4*0.1f) * 0.2f;
		}, .03f);
	sk_default_unclick = sound_generate([](float t){
		float x = t / 0.03f;
		float band1 = sinf(t*7500) * (x * powf(1 - x, 10)) / 0.03f;
		float band2 = sinf(t*4750) * (x * powf(1 - x, 12)) / 0.03f;
		float band3 = sinf(t*2500) * (x * powf(1 - x, 12)) / 0.03f;
		float band4 = sinf(t*500)  * (x * powf(1 - x, 6))  / 0.03f;

		return (band1*0.2f + band2*0.4f + band3*0.1f + band4*0.1f) * 0.2f;
		}, .03f);
	sk_default_grab = sound_generate([](float t){
		float x = t / 0.04f;
		float band1 = sinf(t*4000*3.14f) * (x * powf(1 - x, 10)) / 0.04f;
		float band2 = sinf(t*6000*3.14f) * (x * powf(1 - x, 12)) / 0.04f;
		float band3 = sinf(t*1000*3.14f) * (x * powf(1 - x, 12)) / 0.04f;
		float band4 = sinf(t*400*3.14f)  * (x * powf(1 - x, 6))  / 0.04f;

		return (band1*0.05f + band2*0.05f + band3*0.2f + band4*0.7f) * 0.1f;
		}, .04f);
	sk_default_ungrab = sound_generate([](float t){
		float x = t / 0.04f;
		float band1 = sinf(t*3000*3.14f) * (x * powf(1 - x, 10)) / 0.04f;
		float band2 = sinf(t*5000*3.14f) * (x * powf(1 - x, 12)) / 0.04f;
		float band3 = sinf(t*800*3.14f) * (x * powf(1 - x, 12)) / 0.04f;
		float band4 = sinf(t*300*3.14f)  * (x * powf(1 - x, 6))  / 0.04f;

		return (band1*0.05f + band2*0.05f + band3*0.1f + band4*0.8f) * 0.1f;
		}, .04f);

	sound_set_id(sk_default_click,   default_id_sound_click);
	sound_set_id(sk_default_unclick, default_id_sound_unclick);
	sound_set_id(sk_default_grab,    default_id_sound_grab);
	sound_set_id(sk_default_ungrab,  default_id_sound_ungrab);

	return true;
}

///////////////////////////////////////////

void defaults_shutdown() {
	tex_set_error_fallback  (nullptr);
	tex_set_loading_fallback(nullptr);

	sound_release   (sk_default_click);
	sound_release   (sk_default_unclick);
	sound_release   (sk_default_grab);
	sound_release   (sk_default_ungrab);
	font_release    (sk_default_font);
	material_release(sk_default_material);
	material_release(sk_default_material_pbr);
	material_release(sk_default_material_pbr_clip);
	material_release(sk_default_material_unlit);
	material_release(sk_default_material_unlit_clip);
	material_release(sk_default_material_equirect);
	material_release(sk_default_material_font);
	material_release(sk_default_material_ui);
	material_release(sk_default_material_ui_box);
	material_release(sk_default_material_ui_quadrant);
	shader_release  (sk_default_shader);
	shader_release  (sk_default_shader_blit);
	shader_release  (sk_default_shader_unlit);
	shader_release  (sk_default_shader_unlit_clip);
	shader_release  (sk_default_shader_font);
	shader_release  (sk_default_shader_equirect);
	shader_release  (sk_default_shader_ui);
	shader_release  (sk_default_shader_ui_box);
	shader_release  (sk_default_shader_ui_quadrant);
	shader_release  (sk_default_shader_sky);
	shader_release  (sk_default_shader_lines);
	shader_release  (sk_default_shader_pbr);
	shader_release  (sk_default_shader_pbr_clip);
	mesh_release    (sk_default_cube);
	mesh_release    (sk_default_sphere);
	mesh_release    (sk_default_quad);
	mesh_release    (sk_default_screen_quad);
	tex_release     (sk_default_tex);
	tex_release     (sk_default_tex_black);
	tex_release     (sk_default_tex_gray);
	tex_release     (sk_default_tex_flat);
	tex_release     (sk_default_tex_rough);
	tex_release     (sk_default_tex_devtex);
	tex_release     (sk_default_tex_error);
	tex_release     (sk_default_cubemap);
}

} // namespace sk

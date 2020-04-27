#include "defaults.h"
#include "../stereokit.h"
#include "../shaders_builtin/shader_builtin.h"

#include <string.h>

namespace sk {

///////////////////////////////////////////

tex_t        sk_default_tex;
tex_t        sk_default_tex_black;
tex_t        sk_default_tex_gray;
tex_t        sk_default_tex_flat;
tex_t        sk_default_tex_rough;
tex_t        sk_default_cubemap;
mesh_t       sk_default_quad;
mesh_t       sk_default_sphere;
mesh_t       sk_default_cube;
shader_t     sk_default_shader;
shader_t     sk_default_shader_pbr;
shader_t     sk_default_shader_unlit;
shader_t     sk_default_shader_font;
shader_t     sk_default_shader_equirect;
shader_t     sk_default_shader_ui;
shader_t     sk_default_shader_ui_quadrant;
material_t   sk_default_material;
material_t   sk_default_material_equirect;
material_t   sk_default_material_font;
material_t   sk_default_material_ui;
material_t   sk_default_material_ui_quadrant;
font_t       sk_default_font;
text_style_t sk_default_text_style;
sound_t      sk_default_click;
sound_t      sk_default_unclick;
sound_t      sk_default_grab;
sound_t      sk_default_ungrab;

///////////////////////////////////////////

tex_t defaults_texture(const char *id, color32 color) {
	tex_t result = tex_create();
	if (result == nullptr) {
		return nullptr;
	}
	color32 tex_colors[2*2];
	for (size_t i = 0; i < 2 * 2; i++) 
		tex_colors[i] = color;
	tex_set_colors(result, 2, 2, tex_colors);
	tex_set_id    (result, id);

	return result;
}

///////////////////////////////////////////

bool defaults_init() {
	// Textures
	sk_default_tex       = defaults_texture("default/tex",       {255,255,255,255});
	sk_default_tex_black = defaults_texture("default/tex_black", {0,0,0,255}      );
	sk_default_tex_gray  = defaults_texture("default/tex_gray",  {128,128,128,255});
	sk_default_tex_flat  = defaults_texture("default/tex_flat",  {128,128,255,255}); // Default for normal maps
	sk_default_tex_rough = defaults_texture("default/tex_rough", {0,0,255,255}    ); // Default for metal/roughness maps

	if (sk_default_tex       == nullptr ||
		sk_default_tex_black == nullptr ||
		sk_default_tex_gray  == nullptr ||
		sk_default_tex_flat  == nullptr ||
		sk_default_tex_rough == nullptr)
		return false;

	// Cubemap
	spherical_harmonics_t lighting = { {
		{ 0.27f,  0.26f,  0.25f},
		{ 0.07f,  0.09f,  0.11f},
		{-0.06f, -0.06f, -0.04f},
		{-0.06f, -0.04f, -0.01f},
		{-0.04f, -0.05f, -0.06f},
		{ 0.15f,  0.16f,  0.16f},
		{-0.04f, -0.05f, -0.05f},
		{ 0.05f,  0.05f,  0.04f},
		{-0.11f, -0.13f, -0.13f},
	} };
	sk_default_cubemap = tex_gen_cubemap_sh(lighting, 16);
	tex_set_id(sk_default_cubemap, "default/cubemap");
	render_set_skytex(sk_default_cubemap);
	render_set_skylight(lighting);
	render_enable_skytex(true);

	// Default rendering quad
	sk_default_quad = mesh_create();
	vert_t verts[4] = {
		vec3{-1,-1,0}, vec3{0,0,-1}, vec2{0,0}, color32{255,255,255,255},
		vec3{ 1,-1,0}, vec3{0,0,-1}, vec2{1,0}, color32{255,255,255,255},
		vec3{ 1, 1,0}, vec3{0,0,-1}, vec2{1,1}, color32{255,255,255,255},
		vec3{-1, 1,0}, vec3{0,0,-1}, vec2{0,1}, color32{255,255,255,255},
	};
	vind_t inds[6] = { 0,1,2, 0,2,3 };
	mesh_set_verts(sk_default_quad, verts, 4);
	mesh_set_inds (sk_default_quad, inds,  6);
	mesh_set_id   (sk_default_quad, "default/mesh_quad");
	sk_default_cube   = mesh_gen_cube(vec3_one);
	sk_default_sphere = mesh_gen_sphere(1);
	
	mesh_set_id(sk_default_cube,   "default/mesh_cube");
	mesh_set_id(sk_default_sphere, "default/mesh_sphere");

	// Shaders
	sk_default_shader          = shader_create_mem((void*)shader_builtin_default,  sizeof(shader_builtin_default));
	sk_default_shader_pbr      = shader_create_mem((void*)shader_builtin_pbr,      sizeof(shader_builtin_pbr));
	sk_default_shader_unlit    = shader_create_mem((void*)shader_builtin_unlit,    sizeof(shader_builtin_unlit));
	sk_default_shader_font     = shader_create_mem((void*)shader_builtin_font,     sizeof(shader_builtin_font));
	sk_default_shader_equirect = shader_create_mem((void*)shader_builtin_equirect, sizeof(shader_builtin_equirect));
	sk_default_shader_ui       = shader_create_mem((void*)shader_builtin_ui,       sizeof(shader_builtin_ui));
	sk_default_shader_ui_quadrant = shader_create_mem((void*)shader_builtin_ui_quadrant, sizeof(shader_builtin_ui_quadrant));
	
	if (sk_default_shader          == nullptr ||
		sk_default_shader_pbr      == nullptr ||
		sk_default_shader_unlit    == nullptr ||
		sk_default_shader_font     == nullptr ||
		sk_default_shader_equirect == nullptr ||
		sk_default_shader_ui       == nullptr ||
		sk_default_shader_ui_quadrant == nullptr)
		return false;

	shader_set_id(sk_default_shader,          "default/shader");
	shader_set_id(sk_default_shader_pbr,      "default/shader_pbr");
	shader_set_id(sk_default_shader_unlit,    "default/shader_unlit");
	shader_set_id(sk_default_shader_font,     "default/shader_font");
	shader_set_id(sk_default_shader_equirect, "default/shader_equirect");
	shader_set_id(sk_default_shader_ui,       "default/shader_ui");
	shader_set_id(sk_default_shader_ui_quadrant, "default/shader_ui_quadrant");
	
	// Materials
	sk_default_material          = material_create(sk_default_shader);
	sk_default_material_equirect = material_create(sk_default_shader_equirect);
	sk_default_material_font     = material_create(sk_default_shader_font);
	sk_default_material_ui       = material_create(sk_default_shader_ui);
	sk_default_material_ui_quadrant = material_create(sk_default_shader_ui_quadrant);

	if (sk_default_material          == nullptr ||
		sk_default_material_equirect == nullptr ||
		sk_default_material_font     == nullptr ||
		sk_default_material_ui       == nullptr ||
		sk_default_material_ui_quadrant == nullptr)
		return false;

	material_set_id(sk_default_material,          "default/material");
	material_set_id(sk_default_material_equirect, "default/equirect_convert");
	material_set_id(sk_default_material_font,     "default/material_font");
	material_set_id(sk_default_material_ui,       "default/material_ui");
	material_set_id(sk_default_material_ui_quadrant, "default/material_ui_quadrant");

	material_set_texture(sk_default_material_font, "diffuse", sk_default_tex);

	// Text!
	sk_default_font       = font_create("C:/Windows/Fonts/segoeui.ttf");
	sk_default_text_style = text_make_style(sk_default_font, 20 * mm2m, sk_default_material_font, color32{ 255,255,255,255 });

	font_set_id(sk_default_font, "default/font");

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

	sound_set_id(sk_default_click,   "default/sound_click");
	sound_set_id(sk_default_unclick, "default/sound_unclick");
	sound_set_id(sk_default_grab,    "default/sound_grab");
	sound_set_id(sk_default_ungrab,  "default/sound_ungrab");

	return true;
}

///////////////////////////////////////////

void defaults_shutdown() {
	sound_release   (sk_default_click);
	sound_release   (sk_default_unclick);
	font_release    (sk_default_font);
	material_release(sk_default_material_equirect);
	material_release(sk_default_material);
	material_release(sk_default_material_font);
	material_release(sk_default_material_ui);
	material_release(sk_default_material_ui_quadrant);
	shader_release  (sk_default_shader_equirect);
	shader_release  (sk_default_shader_font);
	shader_release  (sk_default_shader_unlit);
	shader_release  (sk_default_shader_pbr);
	shader_release  (sk_default_shader);
	shader_release  (sk_default_shader_ui);
	shader_release  (sk_default_shader_ui_quadrant);
	mesh_release    (sk_default_quad);
	tex_release     (sk_default_tex);
	tex_release     (sk_default_tex_black);
	tex_release     (sk_default_tex_gray);
	tex_release     (sk_default_tex_flat);
	tex_release     (sk_default_tex_rough);
}

} // namespace sk
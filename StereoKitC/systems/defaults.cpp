#include "defaults.h"
#include "../stereokit.h"
#include "../shaders_builtin/shader_builtin.h"

#include <string.h>

///////////////////////////////////////////

tex2d_t    sk_default_tex;
tex2d_t    sk_default_tex_black;
tex2d_t    sk_default_tex_gray;
tex2d_t    sk_default_tex_flat;
tex2d_t    sk_default_tex_rough;
mesh_t     sk_default_quad;
shader_t   sk_default_shader;
shader_t   sk_default_shader_pbr;
shader_t   sk_default_shader_unlit;
shader_t   sk_default_shader_font;
shader_t   sk_default_shader_equirect;
material_t sk_default_material;
material_t sk_default_material_equirect;

///////////////////////////////////////////

bool defaults_init() {
	// Default white texture
	sk_default_tex = tex2d_create();
	if (sk_default_tex == nullptr) {
		return false;
	}
	color32 tex_colors[2*2];
	memset(tex_colors, 255, sizeof(color32) * 2 * 2);
	tex2d_set_name  (sk_default_tex, "default/tex2d");
	tex2d_set_colors(sk_default_tex, 2, 2, tex_colors);

	// Default black texture, for use with shader defaults
	sk_default_tex_black = tex2d_create();
	if (sk_default_tex_black == nullptr) {
		return false;
	}
	for (size_t i = 0; i < 2 * 2; i++) 
		tex_colors[i] = { 0,0,0,255 };
	tex2d_set_name  (sk_default_tex_black, "default/tex2d_black");
	tex2d_set_colors(sk_default_tex_black, 2, 2, tex_colors);

	// Default middle gray texture, for use with shader defaults
	sk_default_tex_gray = tex2d_create();
	if (sk_default_tex_gray == nullptr) {
		return false;
	}
	for (size_t i = 0; i < 2 * 2; i++) 
		tex_colors[i] = { 128,128,128,255 };
	tex2d_set_name  (sk_default_tex_gray, "default/tex2d_gray");
	tex2d_set_colors(sk_default_tex_gray, 2, 2, tex_colors);

	// Default normal map, for use with shader defaults
	sk_default_tex_flat = tex2d_create();
	if (sk_default_tex_flat == nullptr) {
		return false;
	}
	for (size_t i = 0; i < 2 * 2; i++) 
		tex_colors[i] = { 128,128,255,255 };
	tex2d_set_name  (sk_default_tex_flat, "default/tex2d_flat");
	tex2d_set_colors(sk_default_tex_flat, 2, 2, tex_colors);

	// Default metal/roughness map, for use with shader defaults
	sk_default_tex_rough = tex2d_create();
	if (sk_default_tex_rough == nullptr) {
		return false;
	}
	for (size_t i = 0; i < 2 * 2; i++) 
		tex_colors[i] = { 0,0,255,255 };
	tex2d_set_name  (sk_default_tex_rough, "default/tex2d_rough");
	tex2d_set_colors(sk_default_tex_rough, 2, 2, tex_colors);

	// Default rendering quad
	sk_default_quad = mesh_create();
	vert_t verts[4] = {
		vec3{-1,-1,0}, vec3{0,0,-1}, vec2{0,0}, color32{255,255,255,255},
		vec3{ 1,-1,0}, vec3{0,0,-1}, vec2{1,0}, color32{255,255,255,255},
		vec3{ 1, 1,0}, vec3{0,0,-1}, vec2{1,1}, color32{255,255,255,255},
		vec3{-1, 1,0}, vec3{0,0,-1}, vec2{0,1}, color32{255,255,255,255},
	};
	vind_t inds[6] = { 0,1,2, 0,2,3 };
	mesh_set_name (sk_default_quad, "default/quad");
	mesh_set_verts(sk_default_quad, verts, 4);
	mesh_set_inds (sk_default_quad,  inds,  6);

	// Shaders
	sk_default_shader = shader_create(sk_shader_builtin_default);
	shader_set_name(sk_default_shader, "default/shader");
	if (sk_default_shader == nullptr) {
		return false;
	}
	sk_default_shader_pbr = shader_create(sk_shader_builtin_pbr);
	shader_set_name(sk_default_shader_pbr, "default/shader_pbr");
	if (sk_default_shader_pbr == nullptr) {
		return false;
	}
	sk_default_shader_unlit = shader_create(sk_shader_builtin_unlit);
	shader_set_name(sk_default_shader_unlit, "default/shader_unlit");
	if (sk_default_shader_unlit == nullptr) {
		return false;
	}
	sk_default_shader_font = shader_create(sk_shader_builtin_font);
	shader_set_name(sk_default_shader_font, "default/shader_font");
	if (sk_default_shader_font == nullptr) {
		return false;
	}
	sk_default_shader_equirect = shader_create(sk_shader_builtin_equirect);
	shader_set_name(sk_default_shader_equirect, "default/equirect_shader");
	if (sk_default_shader_equirect == nullptr) {
		return false;
	}

	sk_default_material = material_create(sk_default_shader_pbr);
	material_set_name(sk_default_material, "default/material");
	if (sk_default_material == nullptr) {
		return false;
	}
	sk_default_material_equirect = material_create(sk_default_shader_equirect);
	material_set_name(sk_default_material_equirect, "default/equirect_convert");
	if (sk_default_material_equirect == nullptr) {
		return false;
	}

	material_set_texture(sk_default_material, "diffuse", sk_default_tex);

	return true;
}

///////////////////////////////////////////

void defaults_shutdown() {
	material_release(sk_default_material_equirect);
	material_release(sk_default_material);
	shader_release  (sk_default_shader_equirect);
	shader_release  (sk_default_shader_font);
	shader_release  (sk_default_shader_unlit);
	shader_release  (sk_default_shader_pbr);
	shader_release  (sk_default_shader);
	mesh_release    (sk_default_quad);
	tex2d_release   (sk_default_tex);
	tex2d_release   (sk_default_tex_black);
	tex2d_release   (sk_default_tex_gray);
	tex2d_release   (sk_default_tex_flat);
	tex2d_release   (sk_default_tex_rough);
}
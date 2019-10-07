#include "defaults.h"
#include "../stereokit.h"
#include "../shaders_builtin/shader_builtin.h"

#include <string.h>

namespace sk {

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

tex2d_t defaults_texture(const char *id, color32 color) {
	tex2d_t result = tex2d_create();
	if (result == nullptr) {
		return nullptr;
	}
	color32 tex_colors[2*2];
	for (size_t i = 0; i < 2 * 2; i++) 
		tex_colors[i] = color;
	tex2d_set_id    (result, id);
	tex2d_set_colors(result, 2, 2, tex_colors);

	return result;
}

///////////////////////////////////////////

bool defaults_init() {
	// Textures
	sk_default_tex       = defaults_texture("default/tex2d",       {255,255,255,255});
	sk_default_tex_black = defaults_texture("default/tex2d_black", {0,0,0,255}      );
	sk_default_tex_gray  = defaults_texture("default/tex2d_gray",  {128,128,128,255});
	sk_default_tex_flat  = defaults_texture("default/tex2d_flat",  {128,128,255,255}); // Default for normal maps
	sk_default_tex_rough = defaults_texture("default/tex2d_rough", {0,0,255,255}    ); // Default for metal/roughness maps

	if (sk_default_tex       == nullptr ||
		sk_default_tex_black == nullptr ||
		sk_default_tex_gray  == nullptr ||
		sk_default_tex_flat  == nullptr ||
		sk_default_tex_rough == nullptr)
		return false;

	// Default rendering quad
	sk_default_quad = mesh_create();
	vert_t verts[4] = {
		vec3{-1,-1,0}, vec3{0,0,-1}, vec2{0,0}, color32{255,255,255,255},
		vec3{ 1,-1,0}, vec3{0,0,-1}, vec2{1,0}, color32{255,255,255,255},
		vec3{ 1, 1,0}, vec3{0,0,-1}, vec2{1,1}, color32{255,255,255,255},
		vec3{-1, 1,0}, vec3{0,0,-1}, vec2{0,1}, color32{255,255,255,255},
	};
	vind_t inds[6] = { 0,1,2, 0,2,3 };
	mesh_set_id   (sk_default_quad, "default/quad");
	mesh_set_verts(sk_default_quad, verts, 4);
	mesh_set_inds (sk_default_quad,  inds,  6);

	// Shaders
	sk_default_shader          = shader_create(sk_shader_builtin_default);
	sk_default_shader_pbr      = shader_create(sk_shader_builtin_pbr);
	sk_default_shader_unlit    = shader_create(sk_shader_builtin_unlit);
	sk_default_shader_font     = shader_create(sk_shader_builtin_font);
	sk_default_shader_equirect = shader_create(sk_shader_builtin_equirect);
	
	if (sk_default_shader          == nullptr ||
		sk_default_shader_pbr      == nullptr ||
		sk_default_shader_unlit    == nullptr ||
		sk_default_shader_font     == nullptr ||
		sk_default_shader_equirect == nullptr)
		return false;

	shader_set_id(sk_default_shader,          "default/shader");
	shader_set_id(sk_default_shader_pbr,      "default/shader_pbr");
	shader_set_id(sk_default_shader_unlit,    "default/shader_unlit");
	shader_set_id(sk_default_shader_font,     "default/shader_font");
	shader_set_id(sk_default_shader_equirect, "default/equirect_shader");
	
	// Materials
	sk_default_material          = material_create(sk_default_shader_pbr);
	sk_default_material_equirect = material_create(sk_default_shader_equirect);
	
	if (sk_default_material          == nullptr ||
		sk_default_material_equirect == nullptr)
		return false;

	material_set_id(sk_default_material,          "default/material");
	material_set_id(sk_default_material_equirect, "default/equirect_convert");

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

} // namespace sk
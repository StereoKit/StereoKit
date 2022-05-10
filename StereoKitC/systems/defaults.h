#pragma once

#include "../stereokit.h"

namespace sk {

bool defaults_init    ();
void defaults_shutdown();

extern tex_t        sk_default_tex;
extern tex_t        sk_default_tex_black;
extern tex_t        sk_default_tex_gray;
extern tex_t        sk_default_tex_flat;
extern tex_t        sk_default_tex_rough;
extern tex_t        sk_default_tex_devtex;
extern tex_t        sk_default_tex_error;
extern tex_t        sk_default_cubemap;
extern mesh_t       sk_default_quad;
extern shader_t     sk_default_shader;
extern shader_t     sk_default_shader_blit;
extern shader_t     sk_default_shader_pbr;
extern shader_t     sk_default_shader_unlit;
extern shader_t     sk_default_shader_font;
extern shader_t     sk_default_shader_equirect;
extern shader_t     sk_default_shader_ui;
extern shader_t     sk_default_shader_sky;
extern shader_t     sk_default_shader_lines;
extern material_t   sk_default_material;
extern material_t   sk_default_material_equirect;
extern material_t   sk_default_material_font;
extern material_t   sk_default_material_ui;
extern font_t       sk_default_font;
extern text_style_t sk_default_text_style;
extern sound_t      sk_default_click;
extern sound_t      sk_default_unclick;

} // namespace sk
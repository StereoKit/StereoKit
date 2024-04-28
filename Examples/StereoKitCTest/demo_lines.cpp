#include "demo_lines.h"

#include <stereokit.h>
#include <stereokit_ui.h>
using namespace sk;
#include <vector>
using namespace std;

///////////////////////////////////////////

pose_t     line_window_pose;
pose_t     line_palette_pose;
model_t    line_palette_model;
color32    line_color = {255,255,255,255};
float      line_size = 0.02f;
material_t line_hand_mat;

vec3                         line_prev_tip;
bool                         line_painting = false;
vector<line_point_t>         line_draw;
vector<vector<line_point_t>> line_list;

///////////////////////////////////////////

void demo_lines_init() {
	shader_t ui_shader = shader_find(default_id_shader_ui);
	line_palette_model = model_create_file("Palette.glb", ui_shader);
	line_window_pose   = { { 0.3f, 0, -0.3f}, quat_lookat(vec3_zero, {-1,0,1}) };
	line_palette_pose  = { {-0.3f, 0, -0.3f}, quat_lookat(vec3_zero, { 1,-1,1}) };

	line_hand_mat = material_find(default_id_material_hand);

	shader_release(ui_shader);
}

///////////////////////////////////////////

void demo_lines_set_color(color128 color)
{
	line_color = color_to_32( color );
	material_set_color(line_hand_mat, "color", color);
}

///////////////////////////////////////////

bool swatch_button(const char* id, vec2 c, vec2 size) {
	float offset;
	button_state_ state= button_state_inactive, focus=button_state_inactive;
	ui_button_behavior(vec3{ c.x+size.x/2.0f, c.y+ size.y/2.0f, 0}, size, ui_stack_hash(id),
		offset, state, focus);
	return (state & button_state_just_inactive) != 0;
}

///////////////////////////////////////////

void demo_lines_palette()
{
	ui_handle_begin("PaletteMenu", line_palette_pose, model_get_bounds( line_palette_model ), false);
	render_add_model(line_palette_model, matrix_identity);
	ui_push_surface({ {}, quat_from_angles(90,0,0) });

	ui_hslider_at("Size", line_size, 0.001f, 0.02f, 0, vec3{ 6,-1,0 } *cm2m, vec2{ 8, 2 } *cm2m);
	line_add(vec3{ 6, 1, -1 } *cm2m, vec3{ -2,1,-1 } *cm2m, line_color, line_color, line_size);
	
	if (swatch_button("White", vec2{4,  7} * cm2m, vec2{4,4} * cm2m))
		demo_lines_set_color({1,1,1,1});
	if (swatch_button("Red",   vec2{9,  3} * cm2m, vec2{4,4} * cm2m))
		demo_lines_set_color({1,0,0,1});
	if (swatch_button("Green", vec2{9, -3} * cm2m, vec2{4,4} * cm2m))
		demo_lines_set_color({0,1,0,1});
	if (swatch_button("Blue",  vec2{3, -6} * cm2m, vec2{4,4} * cm2m))
		demo_lines_set_color({0,0,1,1});

	ui_pop_surface();
	ui_handle_end();
}

///////////////////////////////////////////

void demo_lines_draw() {
	const hand_t *hand = input_hand(handed_right);
	vec3          tip  = hand->fingers[1][4].position;
	tip = line_prev_tip + (tip-line_prev_tip) * 0.3f;

	if ((hand->pinch_state & button_state_just_active) && !ui_is_interacting(handed_right)) {
		
		if (line_draw.size() > 0)
			line_list.push_back(line_draw);
		line_draw.clear();
		line_draw.push_back(line_point_t{tip, line_size, line_color});
		line_draw.push_back(line_point_t{tip, line_size, line_color});
		line_prev_tip = tip;
		line_painting = true;
	}
	if (hand->pinch_state & button_state_just_inactive)
		line_painting = false;

	if (line_painting && line_draw.size() > 1)
	{
		vec3         prev  = line_draw[line_draw.size() - 2].pt;
		vec3         dir   = vec3_normalize(prev - (line_draw.size() > 2 ? line_draw[line_draw.size() - 3].pt : line_draw[line_draw.size() - 1].pt));
		float        dist  = vec3_magnitude(prev - tip);
		float        speed = vec3_magnitude(tip - line_prev_tip) * time_stepf();
		line_point_t here  = line_point_t{tip, max(1-speed/0.0003f,0.1f) * line_size, line_color};

		if ((vec3_dot( dir, vec3_normalize(tip-prev)) < 0.2f && dist > 0.001f) || dist > 0.005f) { 
			line_draw.push_back(here);
		} else {
			line_draw[line_draw.size() - 1]  = here;
		}
	}

	line_add_listv(line_draw.data(), (int32_t)line_draw.size());
	for (size_t i = 0; i < line_list.size(); i++)
		line_add_listv(&line_list[i][0], (int32_t)line_list[i].size());
	line_prev_tip = tip;
}

///////////////////////////////////////////

void demo_lines_update() {
	ui_window_begin("Settings", line_window_pose, vec2{ 20 }*cm2m);
	if (ui_button("Clear")) {
		line_draw.clear();
		line_list.clear();
	}
	ui_window_end();

	demo_lines_palette();
	demo_lines_draw();
}

///////////////////////////////////////////

void demo_lines_shutdown() {
	model_release   (line_palette_model);
	material_release(line_hand_mat);
}
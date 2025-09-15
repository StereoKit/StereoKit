/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "_stereokit_systems.h"
#include "../stereokit.h"
#include "../_stereokit.h"

#include "render.h"
#include "input.h"
#include "system.h"
#include "text.h"
#include "audio.h"
#include "sprite_drawer.h"
#include "line_drawer.h"
#include "world.h"
#include "defaults.h"
#include "permission.h"

#include "../platforms/_platform.h"
#include "../_stereokit_ui.h"
#include "../asset_types/assets.h"
#include "../tools/tools.h"
#include "../asset_types/animation.h"

namespace sk {

bool stereokit_systems_register() {
	// Platform related systems
	system_t sys_platform         = { "Platform"    };
	system_t sys_platform_begin   = { "FrameBegin"  };
	system_t sys_platform_render  = { "FrameRender" };

	system_set_initialize_deps(sys_platform, "Assets");
	system_set_step_deps      (sys_platform_render, "App", "Text", "Sprites", "Lines", "World", "UILate", "Animation");

	sys_platform       .func_initialize = platform_init;
	sys_platform       .func_shutdown   = platform_shutdown;
	sys_platform_begin .func_step       = platform_step_begin;
	sys_platform_render.func_step       = platform_step_end;

	systems_add(&sys_platform);
	systems_add(&sys_platform_begin);
	systems_add(&sys_platform_render);

	// Rest of the systems
	system_t sys_defaults = { "Defaults" };
	system_set_initialize_deps(sys_defaults, "Platform", "Assets");
	sys_defaults.func_initialize = defaults_init;
	sys_defaults.func_shutdown   = defaults_shutdown;
	systems_add(&sys_defaults);

	system_t sys_ui = { "UI" };
	system_set_initialize_deps(sys_ui, "Defaults");
	system_set_step_deps      (sys_ui, "Input", "FrameBegin");
	sys_ui.func_initialize = ui_init;
	sys_ui.func_step       = ui_step;
	sys_ui.func_shutdown   = ui_shutdown;
	systems_add(&sys_ui);

	system_t sys_ui_late = { "UILate" };
	system_set_step_deps(sys_ui_late, "App", "Tools");
	sys_ui_late.func_step = ui_step_late;
	systems_add(&sys_ui_late);

	system_t sys_renderer = { "Renderer" };
	system_set_initialize_deps(sys_renderer, "Platform", "Defaults");
	system_set_step_deps      (sys_renderer, "FrameBegin");
	sys_renderer.func_initialize = render_init;
	sys_renderer.func_step       = render_step;
	sys_renderer.func_shutdown   = render_shutdown;
	systems_add(&sys_renderer);

	system_t sys_assets = { "Assets" };
	system_set_step_deps(sys_assets, "FrameRender");
	sys_assets.func_initialize = assets_init;
	sys_assets.func_step       = assets_step;
	sys_assets.func_shutdown   = assets_shutdown;
	systems_add(&sys_assets);

	system_t sys_audio = { "Audio" };
	system_set_initialize_deps(sys_audio, "Platform");
	system_set_step_deps      (sys_audio, "Platform");
	sys_audio.func_initialize = audio_init;
	sys_audio.func_step       = audio_step;
	sys_audio.func_shutdown   = audio_shutdown;
	systems_add(&sys_audio);

	system_t sys_input = { "Input" };
	system_set_initialize_deps(sys_input, "Platform", "Defaults");
	system_set_step_deps      (sys_input, "FrameBegin");
	sys_input.func_initialize = input_init;
	//sys_input.func_step       = input_step; // Handled by the platform, not my fav solution
	sys_input.func_shutdown   = input_shutdown;
	systems_add(&sys_input);

	system_t sys_text = { "Text" };
	system_set_initialize_deps(sys_text, "Defaults");
	system_set_step_deps      (sys_text, "App");
	sys_text.func_step     = text_step;
	sys_text.func_shutdown = text_shutdown;
	systems_add(&sys_text);

	system_t sys_sprite = { "Sprites" };
	system_set_initialize_deps(sys_sprite, "Defaults");
	system_set_step_deps      (sys_sprite, "App", "Tools");
	sys_sprite.func_initialize = sprite_drawer_init;
	sys_sprite.func_step       = sprite_drawer_step;
	sys_sprite.func_shutdown   = sprite_drawer_shutdown;
	systems_add(&sys_sprite);

	system_t sys_lines = { "Lines" };
	system_set_initialize_deps(sys_lines, "Defaults");
	system_set_step_deps      (sys_lines, "App");
	sys_lines.func_initialize = line_drawer_init;
	sys_lines.func_step       = line_drawer_step;
	sys_lines.func_shutdown   = line_drawer_shutdown;
	systems_add(&sys_lines);

	system_t sys_world = { "World" };
	system_set_initialize_deps(sys_world, "Platform", "Defaults", "Renderer");
	system_set_step_deps      (sys_world, "Platform", "App");
	sys_world.func_initialize = world_init;
	sys_world.func_step       = world_step;
	sys_world.func_shutdown   = world_shutdown;
	systems_add(&sys_world);

	system_t sys_tools = { "Tools" };
	system_set_initialize_deps(sys_tools, "Platform", "Defaults", "UI");
	system_set_step_deps      (sys_tools, "App");
	sys_tools.func_initialize = tools_init;
	sys_tools.func_step       = tools_step;
	sys_tools.func_shutdown   = tools_shutdown;
	systems_add(&sys_tools);

	system_t sys_anim = { "Animation" };
	system_set_step_deps(sys_anim, "App");
	sys_anim.func_step     = anim_step;
	sys_anim.func_shutdown = anim_shutdown;
	systems_add(&sys_anim);

	system_t sys_permission = { "Permission" };
	system_set_initialize_deps(sys_permission, "Platform");
	sys_permission.func_initialize = permission_init;
	sys_permission.func_shutdown   = permission_shutdown;
	systems_add(&sys_permission);

	system_t sys_app = { "App" };
	system_set_step_deps(sys_app, "Input", "Defaults", "FrameBegin", "Platform", "Renderer", "UI");
	sys_app.func_step = sk_app_step;
	systems_add(&sys_app);

	return systems_initialize();
}

}
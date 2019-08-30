#pragma once

struct scene_t {
	void (*init)(void);
	void (*update)(void);
	void (*shutdown)(void);
};

void scene_set_active(scene_t scene);
void scene_shutdown();
void scene_update();
void scene_init();
#pragma once

struct app_t {
	void (*init)(void);
	void (*update)(void);
	void (*shutdown)(void);
};

void app_set_active(app_t app);
void app_shutdown();
void app_update();
void app_init();
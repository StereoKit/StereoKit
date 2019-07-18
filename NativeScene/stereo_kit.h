#pragma once

#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "transform.h"
#include "camera.h"
#include "render.h"

enum sk_runtime_ {
	sk_runtime_win32  = 0,
	sk_runtime_openxr = 1
};

extern float  sk_timef;
extern double sk_time;
extern double sk_time_elapsed;
extern float  sk_time_elapsedf;

extern sk_runtime_ sk_runtime;
extern bool sk_focused;
extern bool sk_run;

bool sk_init      (const char *app_name, sk_runtime_ runtime);
void sk_shutdown  ();
bool sk_step      (void (*app_update)(void));
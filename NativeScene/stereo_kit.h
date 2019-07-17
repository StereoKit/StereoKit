#pragma once

#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "transform.h"
#include "camera.h"
#include "render.h"

extern float  sk_timef;
extern double sk_time;
extern double sk_time_elapsed;
extern float  sk_time_elapsedf;

bool sk_init      (const char *app_name);
void sk_shutdown  ();
bool sk_step      (void (*app_update)(void), void (*app_draw)(void));
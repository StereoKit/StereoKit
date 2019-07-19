#pragma once

#if defined(_DLL)
#define SK_EXPORT __declspec(dllexport)
#else
#define SK_EXPORT __declspec(dllimport)
#endif

#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "transform.h"
#include "camera.h"
#include "render.h"

enum sk_runtime_ {
	sk_runtime_flatscreen   = 0,
	sk_runtime_mixedreality = 1
};

extern float  sk_timef;
extern double sk_time;
extern double sk_time_elapsed;
extern float  sk_time_elapsedf;
extern long long sk_time_raw;

extern sk_runtime_ sk_runtime;
extern bool sk_focused;
extern bool sk_run;

SK_EXPORT bool sk_init      (const char *app_name, sk_runtime_ runtime);
SK_EXPORT void sk_shutdown  ();
SK_EXPORT bool sk_step      (void (*app_update)(void));
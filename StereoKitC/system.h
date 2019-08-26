#pragma once

#include <stdint.h>

struct system_t {
	const char  *name;
	const char **init_dependencies;
	int32_t      init_dependency_count;
	const char **update_dependencies;
	int32_t      update_dependency_count;

	int64_t profile_frame_start;
	int64_t profile_frame_duration;

	int64_t profile_update_count;
	int64_t profile_update_duration;
	int64_t profile_start_duration;
	int64_t profile_shutdown_duration;

	bool (*func_initialize)(void);
	void (*func_update)(void);
	void (*func_shutdown)(void);
};

void    systems_add (const char *name, const char **init_dependencies, int32_t init_dependency_count, const char **update_dependencies, int32_t update_dependency_count, bool (*func_initialize)(void), void (*func_update)(void), void (*func_shutdown)(void));

bool    systems_initialize();
void    systems_update();
void    systems_shutdown();
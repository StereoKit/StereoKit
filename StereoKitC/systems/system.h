#pragma once

#include <stdint.h>

namespace sk {

struct system_t {
	const char  *name;
	const char **init_dependencies;
	int32_t      init_dependency_count;
	const char **update_dependencies;
	int32_t      update_dependency_count;

	uint64_t profile_frame_start;
	uint64_t profile_frame_duration;

	uint64_t profile_update_count;
	uint64_t profile_update_duration;
	uint64_t profile_start_duration;
	uint64_t profile_shutdown_duration;

	bool (*func_initialize)(void);
	void (*func_update    )(void);
	void (*func_shutdown  )(void);
};

void systems_add       (const system_t *system);
bool systems_initialize();
void systems_update    ();
void systems_shutdown  ();
system_t *systems_find (const char *name);

} // namespace sk
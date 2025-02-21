// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#pragma once

#include <stdint.h>

namespace sk {

enum system_run_ {
	system_run_before,
	system_run_from,
};

struct system_t {
	const char  *name;
	const char **init_dependencies;
	int32_t      init_dependency_count;
	const char **step_dependencies;
	int32_t      step_dependency_count;

	uint64_t profile_frame_start;
	uint64_t profile_frame_duration;

	uint64_t profile_step_count;
	uint64_t profile_step_duration;
	uint64_t profile_start_duration;
	uint64_t profile_shutdown_duration;

	bool (*func_initialize)(void);
	void (*func_step      )(void);
	void (*func_shutdown  )(void);
};

// Some macros to make assigning dependencies easier and more readable
#define system_set_initialize_deps(sys, ...) \
	const char *_##sys##_init_deps[] = { __VA_ARGS__ }; \
	sys.init_dependencies     = _##sys##_init_deps; \
	sys.init_dependency_count = sizeof(_##sys##_init_deps) / sizeof(const char *);
#define system_set_step_deps(sys, ...) \
	const char *_##sys##_step_deps[] = { __VA_ARGS__ }; \
	sys.step_dependencies     = _##sys##_step_deps; \
	sys.step_dependency_count = sizeof(_##sys##_step_deps) / sizeof(const char *);

void      systems_add         (const system_t *system);
bool      systems_initialize  ();
void      systems_step_partial(system_run_ run_section, int32_t system_idx);
void      systems_shutdown    ();
system_t* systems_find        (const char *name);
int32_t   systems_find_idx    (const char *name);

} // namespace sk
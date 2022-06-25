#include "system.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../libraries/stref.h"
#include "../libraries/array.h"
#include "../libraries/sokol_time.h"
#include "../stereokit.h"
#include "../sk_memory.h"

namespace sk {

///////////////////////////////////////////

struct sort_dependency_t {
	int32_t *ids;
	int32_t  count;
};

array_t<system_t> systems             = {};
int32_t          *system_init_order   = nullptr;
bool              systems_initialized = false;

///////////////////////////////////////////

int32_t systems_find_id(const char *name);
bool    systems_sort   ();

int32_t topological_sort      (sort_dependency_t *dependencies, int32_t count, int32_t *ref_order);
int32_t topological_sort_visit(sort_dependency_t *dependencies, int32_t count, int32_t index, uint8_t *marks, int32_t *sorted_curr, int32_t *out_order);

///////////////////////////////////////////

void systems_add(const system_t *system) {
	systems.add(*system);
}

///////////////////////////////////////////

int32_t systems_find_id(const char *name) {
	for (int32_t i = 0; i < systems.count; i++) {
		if (string_eq(name, systems[i].name))
			return (int32_t)i;
	}
	return -1;
}

///////////////////////////////////////////

system_t *systems_find(const char *name) {
	for (int32_t i = 0; i < systems.count; i++) {
		if (string_eq(name, systems[i].name))
			return &systems[i];
	}
	return nullptr;
}

///////////////////////////////////////////

bool systems_sort() {
	
	int32_t result = 0;
	
	// Turn dependency names into indices for update
	sort_dependency_t *update_ids = sk_malloc_t(sort_dependency_t, systems.count);
	for (int32_t i = 0; i < systems.count; i++) {
		update_ids[i].count = systems[i].update_dependency_count;
		update_ids[i].ids   = sk_malloc_t(int32_t, systems[i].update_dependency_count);

		for (int32_t d = 0; d < systems[i].update_dependency_count; d++) {
			update_ids[i].ids[d] = systems_find_id(systems[i].update_dependencies[d]);
			if (update_ids[i].ids[d] == -1) {
				log_errf("Can't find system update dependency by the name of %s!", systems[i].update_dependencies[d]);
				result = 1;
			}
		}
	}

	// Sort sort the update order
	if (result == 0) {
		int32_t *update_order = sk_malloc_t(int32_t, systems.count);

		result = topological_sort(update_ids, systems.count, update_order);
		if (result != 0) log_errf("Invalid update dependencies! Cyclic dependency detected at %s!", systems[result].name);
		else systems.reorder(update_order);

		sk_free(update_order);
	}

	// Turn dependency names into indices for init
	sort_dependency_t *init_ids = sk_malloc_t(sort_dependency_t, systems.count);
	for (int32_t i = 0; i < systems.count; i++) {
		init_ids[i].count = systems[i].init_dependency_count;
		init_ids[i].ids   = sk_malloc_t(int32_t, systems[i].init_dependency_count);

		for (int32_t d = 0; d < systems[i].init_dependency_count; d++) {
			init_ids[i].ids[d] = systems_find_id(systems[i].init_dependencies[d]);
			if (init_ids[i].ids[d] == -1) {
				log_errf("Can't find system init dependency by the name of %s!", systems[i].init_dependencies[d]);
				result = 1;
			}
		}
	}

	// Sort the init order
	if (result == 0) {
		system_init_order = sk_malloc_t(int32_t, systems.count);
		result = topological_sort(init_ids, systems.count, system_init_order);
		if (result != 0) log_errf("Invalid initialization dependencies! Cyclic dependency detected at %s!", systems[result].name);
	}

	// Release memory
	for (int32_t i = 0; i < systems.count; i++) {
		sk_free(init_ids  [i].ids);
		sk_free(update_ids[i].ids);
	}
	sk_free(init_ids);
	sk_free(update_ids);

	return result == 0;
}

///////////////////////////////////////////

bool systems_initialize() {
	if (!systems_sort())
		return false;

	for (int32_t i = 0; i < systems.count; i++) {
		int32_t index = system_init_order[i];
		if (systems[index].func_initialize != nullptr) {
			log_diagf("Initializing %s", systems[index].name);

			// start timing
			uint64_t start = stm_now();

			if (!systems[index].func_initialize()) {
				log_errf("System %s failed to initialize!", systems[index].name);
				return false;
			}

			// end timing
			systems[index].profile_start_duration = stm_since(start);
		}
	}
	systems_initialized = true;
	log_info("Initialization successful");
	return true;
}

///////////////////////////////////////////

void systems_update() {
	for (int32_t i = 0; i < systems.count; i++) {
		if (systems[i].func_update != nullptr) {
			// start timing
			systems[i].profile_frame_start = stm_now();

			systems[i].func_update();

			// end timing
			if (systems[i].profile_frame_duration == 0)
				systems[i].profile_frame_duration = stm_since(systems[i].profile_frame_start);
			systems[i].profile_update_duration += systems[i].profile_frame_duration;
			systems[i].profile_update_count    += 1;
			systems[i].profile_frame_duration   = 0;
		}
	}
}

///////////////////////////////////////////

void systems_shutdown() {
	for (int32_t i = systems.count-1; i >= 0; i--) {
		int32_t index = system_init_order[i];
		if (systems[index].func_shutdown != nullptr) {
			// start timing
			uint64_t start = stm_now();

			systems[index].func_shutdown();

			// end timing
			systems[index].profile_shutdown_duration = stm_since(start);
		}
	}

	log_info("Session Performance Report:");
	log_info("<~BLK>______________________________________________________<~clr>");
	log_info("<~BLK>|<~clr>         <~YLW>System <~BLK>|<~clr> <~YLW>Initialize <~BLK>|<~clr>   <~YLW>Update <~BLK>|<~clr>  <~YLW>Shutdown <~BLK>|<~clr>");
	log_info("<~BLK>|________________|____________|__________|___________|<~clr>");
	for (int32_t i = 0; i < systems.count; i++) {
		char start_time   [24];
		char update_time  [24];
		char shutdown_time[24];

		if (systems[i].profile_start_duration != 0) {
			double ms = stm_ms(systems[i].profile_start_duration);
			snprintf(start_time, sizeof(start_time), "%s%8.2f<~BLK>ms", ms>500?"<~RED>":"", ms);
		} else snprintf(start_time, sizeof(start_time), "          ");

		if (systems[i].profile_update_duration != 0) {
			double ms = stm_ms(systems[i].profile_update_duration / systems[i].profile_update_count);
			// Exception for FramePresent, since it includes vsync time
			snprintf(update_time, sizeof(update_time), "%s%6.3f<~BLK>ms", ms>8 && !string_eq(systems[i].name, "FramePresent") ? "<~RED>":"", ms);
		} else snprintf(update_time, sizeof(update_time), "        ");

		if (systems[i].profile_shutdown_duration != 0) {
			double ms = stm_ms(systems[i].profile_shutdown_duration);
			snprintf(shutdown_time, sizeof(shutdown_time), "%s%7.2f<~BLK>ms", ms>500?"<~RED>":"", ms);
		} else snprintf(shutdown_time, sizeof(shutdown_time), "         ");
		
		log_infof("<~BLK>|<~CYN>%15s <~BLK>|<~clr> %s <~BLK>|<~clr> %s <~BLK>|<~clr> %s <~BLK>|<~clr>", systems[i].name, start_time, update_time, shutdown_time);
	}
	log_info("<~BLK>|________________|____________|__________|___________|<~clr>");
	
	systems.free();
	sk_free(system_init_order);
}

///////////////////////////////////////////

int32_t topological_sort(sort_dependency_t *dependencies, int32_t count, int32_t *ref_order) {
	// Topological sort, Depth-first algorithm:
	// https://en.wikipedia.org/wiki/Topological_sorting

	uint8_t *marks       = sk_malloc_t(uint8_t, count);
	int32_t  sorted_curr = count-1;
	memset(marks, 0, sizeof(uint8_t) * count);

	while (sorted_curr > 0) {
		for (int32_t i = 0; i < count; i++) {
			if (marks[i] != 0)
				continue;
			int result = topological_sort_visit(dependencies, count, i, marks, &sorted_curr, ref_order);
			// If we found a cyclic dependency, ditch out!
			if (result != 0) {
				sk_free(marks);
				return result;
			}
		}
	}

	sk_free(marks);
	return 0;
}

///////////////////////////////////////////

#define MARK_TEMP 1
#define MARK_PERM 2
int32_t topological_sort_visit(sort_dependency_t *dependencies, int32_t count, int32_t index, uint8_t *marks, int32_t *sorted_curr, int32_t *out_order) {
	if (marks[index] == MARK_PERM) return 0;
	if (marks[index] == MARK_TEMP) return index;
	marks[index] = MARK_TEMP;
	for (int32_t i = 0; i < count; i++) {
		for (int32_t d = 0; d < dependencies[i].count; d++) {
			if (dependencies[i].ids[d] == index) {
				int result = topological_sort_visit(dependencies, count, i, marks, sorted_curr, out_order);
				if (result != 0)
					return result;
			}
		}
	}
	marks[index] = MARK_PERM;
	out_order[*sorted_curr] = index;
	*sorted_curr = *sorted_curr-1;
	return 0;
}


} // namespace sk
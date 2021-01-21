#include "system.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../libraries/stref.h"
#include "../libraries/array.h"
#include "../libraries/sokol_time.h"
#include "../stereokit.h"

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

int32_t systems_find(const char *name);
bool    systems_sort();

void    array_reorder(void **list, size_t item_size, int32_t count, int32_t *sort_order);
int32_t topological_sort      (sort_dependency_t *dependencies, int32_t count, int32_t **out_order);
int32_t topological_sort_visit(sort_dependency_t *dependencies, int32_t count, int32_t index, uint8_t *marks, int32_t *sorted_curr, int32_t *out_order);

///////////////////////////////////////////

void systems_add(const system_t *system) {
	systems.add(*system);
}

///////////////////////////////////////////

int32_t systems_find(const char *name) {
	for (int32_t i = 0; i < systems.count; i++) {
		if (string_eq(name, systems[i].name))
			return i;
	}
	return -1;
}

///////////////////////////////////////////

bool systems_sort() {
	
	int32_t result = 0;
	
	// Turn dependency names into indices for update
	sort_dependency_t *update_ids = (sort_dependency_t *)malloc(sizeof(sort_dependency_t) * systems.count);
	for (size_t i = 0; i < systems.count; i++) {
		update_ids[i].count = systems[i].update_dependency_count;
		update_ids[i].ids   = (int32_t*)malloc(sizeof(int32_t) * systems[i].update_dependency_count);

		for (size_t d = 0; d < systems[i].update_dependency_count; d++) {
			update_ids[i].ids[d] = systems_find(systems[i].update_dependencies[d]);
			if (update_ids[i].ids[d] == -1) {
				log_errf("Can't find system update dependency by the name of %s!", systems[i].update_dependencies[d]);
				result = 1;
			}
		}
	}

	// Sort sort the update order
	if (result == 0) {
		int32_t *update_order = nullptr;

		result = topological_sort(update_ids, (int32_t)systems.count, &update_order);
		if (result != 0) log_errf("Invalid update dependencies! Cyclic dependency detected at %s!", systems[result].name);
		else array_reorder((void**)&systems, sizeof(system_t), (int32_t)systems.count, update_order);

		free(update_order);
	}

	// Turn dependency names into indices for init
	sort_dependency_t *init_ids = (sort_dependency_t *)malloc(sizeof(sort_dependency_t) * systems.count);
	for (size_t i = 0; i < systems.count; i++) {
		init_ids[i].count = systems[i].init_dependency_count;
		init_ids[i].ids   = (int32_t*)malloc(sizeof(int32_t) * systems[i].init_dependency_count);

		for (size_t d = 0; d < systems[i].init_dependency_count; d++) {
			init_ids[i].ids[d] = systems_find(systems[i].init_dependencies[d]);
			if (init_ids[i].ids[d] == -1) {
				log_errf("Can't find system init dependency by the name of %s!", systems[i].init_dependencies[d]);
				result = 1;
			}
		}
	}

	// Sort the init order
	if (result == 0) {
		free(system_init_order);
		result = topological_sort(init_ids, (int32_t)systems.count, &system_init_order);
		if (result != 0) log_errf("Invalid initialization dependencies! Cyclic dependency detected at %s!", systems[result].name);
	}

	// Release memory
	for (size_t i = 0; i < systems.count; i++) {
		free(init_ids  [i].ids);
		free(update_ids[i].ids);
	}
	free(init_ids);
	free(update_ids);

	return result == 0;
}

///////////////////////////////////////////

bool systems_initialize() {
	if (!systems_sort())
		return false;

	for (int32_t i = 0; i < systems.count; i++) {
		int32_t index = system_init_order[i];
		if (systems[index].func_initialize != nullptr) {
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
			uint64_t start = stm_now();

			systems[i].func_update();

			// end timing
			systems[i].profile_frame_duration   = stm_since(start);
			systems[i].profile_update_duration += systems[i].profile_frame_duration;
			systems[i].profile_update_count    += 1;
		}
	}
}

///////////////////////////////////////////

void systems_shutdown() {
	for (int32_t i = (int32_t)systems.count-1; i >= 0; i--) {
		int32_t index = system_init_order[i];
		if (systems[index].func_shutdown != nullptr) {
			// start timing
			uint64_t start = stm_now();

			systems[index].func_shutdown();

			// end timing
			systems[i].profile_shutdown_duration = stm_since(start);
		}
	}

	log_info("Session Performance Report:");
	log_info("<~BLK>\xDA\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC2\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC2\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC2\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xBF<~clr>");
	log_info("<~BLK>\xB3<~clr>         <~YLW>System <~BLK>\xB3<~clr> <~YLW>Initialize <~BLK>\xB3<~clr>   <~YLW>Update <~BLK>\xB3<~clr>  <~YLW>Shutdown <~BLK>\xB3<~clr>");
	log_info("<~BLK>\xC3\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC5\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC5\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC5\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xB4<~clr>");
	for (int32_t i = 0; i < systems.count; i++) {
		int32_t index = i;

		char start_time[24];
		char update_time[24];
		char shutdown_time[24];

		if (systems[index].profile_start_duration != 0) {
			double ms = stm_ms(systems[index].profile_start_duration);
			snprintf(start_time, sizeof(start_time), "%s%8.2f<~BLK>ms", ms>500?"<~RED>":"", ms);
		} else snprintf(start_time, sizeof(start_time), "          ");

		if (systems[index].profile_update_duration != 0) {
			double ms = stm_ms(systems[index].profile_update_duration / systems[index].profile_update_count);
			// Exception for FramePresent, since it includes vsync time
			snprintf(update_time, sizeof(update_time), "%s%6.3f<~BLK>ms", ms>8 && !string_eq(systems[index].name, "FramePresent") ? "<~RED>":"", ms);
		} else snprintf(update_time, sizeof(update_time), "        ");

		if (systems[index].profile_shutdown_duration != 0) {
			double ms = stm_ms(systems[index].profile_shutdown_duration);
			snprintf(shutdown_time, sizeof(shutdown_time), "%s%7.2f<~BLK>ms", ms>500?"<~RED>":"", ms);
		} else snprintf(shutdown_time, sizeof(shutdown_time), "         ");
		
		log_infof("<~BLK>\xB3<~CYN>%15s <~BLK>\xB3<~clr> %s <~BLK>\xB3<~clr> %s <~BLK>\xB3<~clr> %s <~BLK>\xB3<~clr>", systems[index].name, start_time, update_time, shutdown_time);
	}
	log_info("<~BLK>\xC0\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC1\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC1\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC1\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xD9<~clr>");

	systems.free();
	free(system_init_order);
}

///////////////////////////////////////////

int32_t topological_sort(sort_dependency_t *dependencies, int32_t count, int32_t **out_order) {
	// Topological sort, Depth-first algorithm:
	// https://en.wikipedia.org/wiki/Topological_sorting

	*out_order           = (int32_t *)malloc(sizeof(int32_t) * count);
	uint8_t *marks       = (uint8_t *)malloc(sizeof(uint8_t) * count);
	int32_t  sorted_curr = count-1;
	memset(marks, 0, sizeof(uint8_t) * count);

	while (sorted_curr > 0) {
		for (int32_t i = 0; i < count; i++) {
			if (marks[i] != 0)
				continue;
			int result = topological_sort_visit(dependencies, count, i, marks, &sorted_curr, *out_order);
			// If we found a cyclic dependency, ditch out!
			if (result != 0) {
				free(marks);
				free(*out_order);
				*out_order = nullptr;
				return result;
			}
		}
	}

	free(marks);
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

///////////////////////////////////////////

void array_reorder(void **list, size_t item_size, int32_t count, int32_t *sort_order) {
	uint8_t *src    = (uint8_t*)*list;
	uint8_t *result = (uint8_t*)malloc(item_size * count);

	for (int32_t i = 0; i < count; i++) {
		memcpy(&result[i * item_size], &src[sort_order[i] * item_size], item_size);
	}

	*list = result;
	free(src);
}

} // namespace sk
#include "system.h"

#include <stdlib.h>
#include <string.h>
#include <windows.h> // GetSystemTimePreciseAsFileTime
#include <stdio.h>
#include "../stref.h"
#include "../stereokit.h"

///////////////////////////////////////////

struct sort_dependency_t {
	int32_t *ids;
	int32_t  count;
};

system_t *systems           = nullptr;
int32_t  *system_init_order = nullptr;
int32_t   system_count = 0;
int32_t   system_cap   = 0;

///////////////////////////////////////////

int32_t systems_find(const char *name);

void    array_reorder(void **list, size_t item_size, int32_t count, int32_t *sort_order);
int32_t topological_sort      (sort_dependency_t *dependencies, int32_t count, int32_t **out_order);
int32_t topological_sort_visit(sort_dependency_t *dependencies, int32_t count, int32_t index, uint8_t *marks, int32_t *sorted_curr, int32_t *out_order);

///////////////////////////////////////////

void systems_add(const char *name, const char **init_dependencies, int32_t init_dependency_count, const char **update_dependencies, int32_t update_dependency_count, bool (*func_initialize)(void), void (*func_update)(void), void (*func_shutdown)(void)) {
	if (system_count + 1 > system_cap) {
		system_cap = system_cap < 1 ? 1 : system_cap;
		system_cap = system_cap * 2;
		systems = (system_t*)realloc(systems, sizeof(system_t) * system_cap);
	}

	systems[system_count] = {};
	systems[system_count].init_dependencies       = init_dependencies;
	systems[system_count].init_dependency_count   = init_dependency_count;
	systems[system_count].update_dependencies     = update_dependencies;
	systems[system_count].update_dependency_count = update_dependency_count;
	systems[system_count].name             = name;
	systems[system_count].func_initialize  = func_initialize;
	systems[system_count].func_update      = func_update;
	systems[system_count].func_shutdown    = func_shutdown;
	system_count += 1;
}

///////////////////////////////////////////

int32_t systems_find(const char *name) {
	for (size_t i = 0; i < system_count; i++) {
		if (string_eq(name, systems[i].name))
			return i;
	}
	return -1;
}

///////////////////////////////////////////

bool systems_sort() {
	
	int32_t result = 0;
	
	// Turn dependency names into indices for update
	sort_dependency_t *update_ids = (sort_dependency_t *)malloc(sizeof(sort_dependency_t) * system_count);
	for (size_t i = 0; i < system_count; i++) {
		update_ids[i].count = systems[i].update_dependency_count;
		update_ids[i].ids   = (int32_t*)malloc(sizeof(int32_t) * systems[i].update_dependency_count);

		for (size_t d = 0; d < systems[i].update_dependency_count; d++) {
			update_ids[i].ids[d] = systems_find(systems[i].update_dependencies[d]);
			if (update_ids[i].ids[d] == -1) {
				log_writef(log_error, "Can't find system update dependency by the name of %s!", systems[i].update_dependencies[d]);
				result = 1;
			}
		}
	}

	// Sort sort the update order
	if (result == 0) {
		int32_t *update_order = nullptr;

		result = topological_sort(update_ids, system_count, &update_order);
		if (result != 0) log_writef(log_error, "Invalid update dependencies! Cyclic dependency detected at %s!", systems[result].name);
		else array_reorder((void**)&systems, sizeof(system_t), system_count, update_order);

		free(update_order);
	}

	// Turn dependency names into indices for init
	sort_dependency_t *init_ids = (sort_dependency_t *)malloc(sizeof(sort_dependency_t) * system_count);
	for (size_t i = 0; i < system_count; i++) {
		init_ids[i].count = systems[i].init_dependency_count;
		init_ids[i].ids   = (int32_t*)malloc(sizeof(int32_t) * systems[i].init_dependency_count);

		for (size_t d = 0; d < systems[i].init_dependency_count; d++) {
			init_ids[i].ids[d] = systems_find(systems[i].init_dependencies[d]);
			if (init_ids[i].ids[d] == -1) {
				log_writef(log_error, "Can't find system init dependency by the name of %s!", systems[i].init_dependencies[d]);
				result = 1;
			}
		}
	}

	// Sort the init order
	if (result == 0) {
		result = topological_sort(init_ids, system_count, &system_init_order);
		if (result != 0) log_writef(log_error, "Invalid initialization dependencies! Cyclic dependency detected at %s!", systems[result].name);
	}

	// Release memory
	for (size_t i = 0; i < system_count; i++) {
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

	FILETIME time;
	for (int32_t i = 0; i < system_count; i++) {
		int32_t index = system_init_order[i];
		if (systems[index].func_initialize != nullptr) {
			// start timing
			GetSystemTimePreciseAsFileTime(&time);
			systems[index].profile_start_duration = (int64_t)time.dwLowDateTime + ((int64_t)(time.dwHighDateTime) << 32LL);

			if (!systems[index].func_initialize())
				return false;

			// end timing
			GetSystemTimePreciseAsFileTime(&time);
			int64_t end = (int64_t)time.dwLowDateTime + ((int64_t)(time.dwHighDateTime) << 32LL);
			systems[index].profile_start_duration = end - systems[index].profile_start_duration;
		}
	}
	return true;
}

///////////////////////////////////////////

void systems_update() {
	FILETIME time;
	for (int32_t i = 0; i < system_count; i++) {
		if (systems[i].func_update != nullptr) {
			// start timing
			GetSystemTimePreciseAsFileTime(&time);
			systems[i].profile_frame_duration = (int64_t)time.dwLowDateTime + ((int64_t)(time.dwHighDateTime) << 32LL);

			systems[i].func_update();

			// end timing
			GetSystemTimePreciseAsFileTime(&time);
			int64_t end = (int64_t)time.dwLowDateTime + ((int64_t)(time.dwHighDateTime) << 32LL);
			systems[i].profile_frame_duration = end - systems[i].profile_frame_duration;
			systems[i].profile_update_duration += systems[i].profile_frame_duration;
			systems[i].profile_update_count += 1;
		}
	}
}

///////////////////////////////////////////

void systems_shutdown() {
	FILETIME time;
	for (int32_t i = system_count-1; i >= 0; i--) {
		int32_t index = system_init_order[i];
		if (systems[index].func_shutdown != nullptr) {
			// start timing
			GetSystemTimePreciseAsFileTime(&time);
			systems[i].profile_shutdown_duration = (int64_t)time.dwLowDateTime + ((int64_t)(time.dwHighDateTime) << 32LL);

			systems[index].func_shutdown();

			// end timing
			GetSystemTimePreciseAsFileTime(&time);
			int64_t end = (int64_t)time.dwLowDateTime + ((int64_t)(time.dwHighDateTime) << 32LL);
			systems[i].profile_shutdown_duration = end - systems[i].profile_shutdown_duration;
		}
	}

	log_write(log_info, "Session Performance Report:");
	log_write(log_info, "<~BLK>|----------------|------------|----------|-----------|<~clr>");
	log_write(log_info, "<~BLK>|<~clr>         <~YLW>System <~BLK>|<~clr> <~YLW>Initialize <~BLK>|<~clr>   <~YLW>Update <~BLK>|<~clr>  <~YLW>Shutdown <~BLK>|<~clr>");
	log_write(log_info, "<~BLK>|----------------|------------|----------|-----------|<~clr>");
	for (int32_t i = 0; i < system_count; i++) {
		int32_t index = i;

		char start_time[24];
		char update_time[24];
		char shutdown_time[24];

		if (systems[index].func_initialize != nullptr)
			 sprintf_s(start_time, 24, "%8.2f<~BLK>ms", (float)((double)systems[index].profile_start_duration / 10000.0));
		else sprintf_s(start_time, 24, "          ");

		if (systems[index].func_update != nullptr)
			 sprintf_s(update_time, 24, "%6.3f<~BLK>ms", (float)(((double)systems[index].profile_update_duration / (double)systems[index].profile_update_count) / 10000.0));
		else sprintf_s(update_time, 24, "        ");

		if (systems[index].func_shutdown != nullptr)
			 sprintf_s(shutdown_time, 24, "%7.2f<~BLK>ms", (float)((double)systems[index].profile_shutdown_duration / 10000.0));
		else sprintf_s(shutdown_time, 24, "         ");
		
		log_writef(log_info, "<~BLK>|<~CYN>%15s <~BLK>|<~clr> %s <~BLK>|<~clr> %s <~BLK>|<~clr> %s <~BLK>|<~clr>", systems[index].name, start_time, update_time, shutdown_time);
	}
	log_write(log_info, "<~BLK>|----------------|------------|----------|-----------|<~clr>");

	free(systems);
	free(system_init_order);
	systems = nullptr;
}

///////////////////////////////////////////

int32_t topological_sort(sort_dependency_t *dependencies, int32_t count, int32_t **out_order) {
	// Topological sort, Depth-first algorithm:
	// https://en.wikipedia.org/wiki/Topological_sorting

	*out_order      = (int32_t *)malloc(sizeof(int32_t) * count);
	uint8_t *marks  = (uint8_t *)malloc(sizeof(uint8_t) * count);
	int32_t  sorted_curr = count-1;
	memset(marks, 0, sizeof(uint8_t) * count);

	while (sorted_curr > 0) {
		for (size_t i = 0; i < count; i++) {
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
	for (size_t i = 0; i < count; i++) {
		for (size_t d = 0; d < dependencies[i].count; d++) {
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

	for (size_t i = 0; i < count; i++) {
		memcpy(&result[i * item_size], &src[sort_order[i] * item_size], item_size);
	}

	*list = result;
	free(src);
}
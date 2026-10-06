/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 */

#include "spatial_names.h"
#include "../platforms/platform.h"
#include "../sk_memory.h"
#include "../libraries/array.h"
#include "../libraries/stref.h"

#include <sk_app.h>
#include <stdio.h>
#include <string.h>

namespace sk {

///////////////////////////////////////////

struct spatial_name_t {
	sk_uuid_t uuid;
	char*     name;
};

struct spatial_names_state_t {
	bool32_t                loaded;
	bool32_t                has_file; // False when there's no app data folder
	char                    path[1024];
	array_t<spatial_name_t> names;
};
static spatial_names_state_t local = {};

static const char* names_filename = "spatial_anchor_names.txt";
static const char* names_header   = "# sk_anchor_names 1";

///////////////////////////////////////////

static bool uuid_eq(const sk_uuid_t& a, const sk_uuid_t& b) { return memcmp(a.bytes, b.bytes, sizeof(a.bytes)) == 0; }

static int32_t hex_val(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static int32_t index_of_uuid(sk_uuid_t uuid) {
	for (int32_t i = 0; i < local.names.count; i++)
		if (uuid_eq(local.names[i].uuid, uuid)) return i;
	return -1;
}

static int32_t index_of_name(const char* name) {
	for (int32_t i = 0; i < local.names.count; i++)
		if (string_eq(local.names[i].name, name)) return i;
	return -1;
}

static void remove_at(int32_t index) {
	sk_free(local.names[index].name);
	local.names.remove(index);
}

///////////////////////////////////////////

// One "<32 hex uuid> <name>" pair per line, after an optional header.
static void names_load() {
	if (local.loaded) return;
	local.loaded = true;

	char folder[1024];
	local.has_file = ska_path_get(ska_path_data, folder, sizeof(folder));
	if (!local.has_file) {
		log_warn("Anchor names can't be saved, there's no app data folder");
		return;
	}
	snprintf(local.path, sizeof(local.path), "%s" platform_path_separator "%s", folder, names_filename);
	if (!ska_file_exists(local.path)) return;

	char*  file = nullptr;
	size_t size = 0;
	if (!ska_file_read(local.path, (void**)&file, &size)) return;

	stref_t data = stref_make(file);
	stref_t line = {};
	while (stref_nextline(data, line)) {
		if (line.length < 34 || line.start[32] != ' ') continue;

		spatial_name_t entry = {};
		bool           valid = true;
		for (int32_t b = 0; b < 16 && valid; b++) {
			int32_t hi = hex_val(line.start[b*2]);
			int32_t lo = hex_val(line.start[b*2+1]);
			valid = hi >= 0 && lo >= 0;
			entry.uuid.bytes[b] = (uint8_t)(hi << 4 | lo);
		}
		if (!valid || index_of_uuid(entry.uuid) >= 0) continue;
		entry.name = stref_copy(stref_substr(line.start + 33, line.length - 33));
		if (index_of_name(entry.name) >= 0) { sk_free(entry.name); continue; }
		local.names.add(entry);
	}
	ska_file_free_data(file);
}

///////////////////////////////////////////

// Writes beside the file and swaps it in, so a crash mid-write can't lose names.
static void names_save() {
	if (!local.has_file) return;
	if (local.names.count == 0) {
		if (ska_file_exists(local.path)) platform_file_delete(local.path);
		return;
	}

	char* file = string_append(string_copy(names_header), 1, "\n");
	char  hex[33];
	for (int32_t i = 0; i < local.names.count; i++) {
		for (int32_t b = 0; b < 16; b++)
			snprintf(&hex[b*2], 3, "%02x", local.names[i].uuid.bytes[b]);
		file = string_append(file, 4, hex, " ", local.names[i].name, "\n");
	}

	char temp_path[1040];
	snprintf(temp_path, sizeof(temp_path), "%s.tmp", local.path);
	if (!ska_file_write_text(temp_path, file) || !platform_file_replace(temp_path, local.path))
		log_warn("Couldn't save anchor names");
	sk_free(file);
}

///////////////////////////////////////////

bool32_t spatial_names_find(const char* name, sk_uuid_t* out_uuid) {
	names_load();
	int32_t index = index_of_name(name);
	*out_uuid = index < 0 ? sk_uuid_t{} : local.names[index].uuid;
	return index >= 0;
}

///////////////////////////////////////////

const char* spatial_names_get(sk_uuid_t uuid) {
	names_load();
	int32_t index = index_of_uuid(uuid);
	return index < 0 ? nullptr : local.names[index].name;
}

///////////////////////////////////////////

void spatial_names_set(sk_uuid_t uuid, const char* name) {
	names_load();
	// The file is one entry per line, so a line break would corrupt it
	char* line_safe = string_copy(name);
	for (char* c = line_safe; *c; c++)
		if (*c == '\n' || *c == '\r') *c = ' ';

	int32_t index = index_of_uuid(uuid);
	if (index >= 0 && string_eq(local.names[index].name, line_safe)) { sk_free(line_safe); return; }

	int32_t taken = index_of_name(line_safe);
	if (taken >= 0 && taken != index) {
		remove_at(taken);
		index = index_of_uuid(uuid);
	}

	if (index >= 0) {
		sk_free(local.names[index].name);
		local.names[index].name = line_safe;
	} else {
		local.names.add({ uuid, line_safe });
	}
	names_save();
}

///////////////////////////////////////////

void spatial_names_remove(sk_uuid_t uuid) {
	names_load();
	int32_t index = index_of_uuid(uuid);
	if (index < 0) return;
	remove_at(index);
	names_save();
}

///////////////////////////////////////////

int32_t spatial_names_count() {
	names_load();
	return local.names.count;
}

sk_uuid_t spatial_names_get_index(int32_t index) {
	names_load();
	return index >= 0 && index < local.names.count ? local.names[index].uuid : sk_uuid_t{};
}

///////////////////////////////////////////

void spatial_names_clear() {
	names_load();
	for (int32_t i = 0; i < local.names.count; i++)
		sk_free(local.names[i].name);
	local.names.clear();
	names_save();
}

///////////////////////////////////////////

void spatial_names_shutdown() {
	for (int32_t i = 0; i < local.names.count; i++)
		sk_free(local.names[i].name);
	local.names.free();
	local = {};
}

}

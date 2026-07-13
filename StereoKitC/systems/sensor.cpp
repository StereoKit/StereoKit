/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

#include "sensor.h"

#include "../asset_types/texture.h"
#include "../asset_types/texture_.h"
#include "../sk_memory.h"
#include "../xr_backends/openxr.h"
#include "../xr_backends/extensions/ext_management.h"

#include <string.h>

namespace sk {

///////////////////////////////////////////

static struct {
	bool                 wants_cpu_data;
	int32_t              array_count;
	tex_t                readback_tex;
	skr_tex_readback_t   current_readback[2];
	bool                 has_current;
	skr_tex_readback_t   pending_readback[2];
	int32_t              pending_readback_count;

	// Opaque frame metadata stashed at readback-submit time
	void*                submit_meta;
	size_t               submit_meta_size;

	// Tracks the last submitted frame to avoid redundant copies
	uint64_t             last_submitted_frame_id;

	// Latest completed readback
	void*                latest_meta;
	size_t               latest_meta_size;
	void*                latest_data_ptr;
	size_t               latest_data_size;
	bool                 has_latest_data;
} readback = {};

// TODO: The single-in-flight readback is sufficient for low-rate sensors
// (~20-30 Hz depth) where the readback cycle completes before the next
// sensor frame arrives. For higher-rate sources, a circular buffer of
// in-flight readbacks would be needed to avoid dropping frames.

///////////////////////////////////////////

void sensor_readback_request() {
	readback.wants_cpu_data = true;
}

///////////////////////////////////////////

void sensor_readback_update(tex_t gpu_tex, int32_t width, int32_t height, int32_t array_count, const void* frame_meta, size_t frame_meta_size, uint64_t frame_id) {
	if (!readback.wants_cpu_data)
		return;

	readback.array_count = array_count;

	// Check if all pending readbacks completed
	if (readback.pending_readback_count == array_count) {
		bool all_done = true;
		for (int32_t i = 0; i < array_count; i++) {
			if (!skr_future_check(&readback.pending_readback[i].future)) {
				all_done = false;
				break;
			}
		}
		if (all_done) {
			// Destroy previous completed readbacks, promote pending to current
			if (readback.has_current) {
				for (int32_t i = 0; i < array_count; i++)
					skr_tex_readback_destroy(&readback.current_readback[i]);
			}
			for (int32_t i = 0; i < array_count; i++) {
				readback.current_readback[i] = readback.pending_readback[i];
				readback.pending_readback[i] = {};
			}
			readback.has_current            = true;
			readback.pending_readback_count = 0;

			// Combine layer data into contiguous buffer
			size_t layer_size = readback.current_readback[0].size;
			size_t total_size = layer_size * array_count;
			void*  old_data   = readback.latest_data_ptr;
			size_t old_size   = readback.latest_data_size;
			if (old_data == nullptr || old_size != total_size) {
				sk_free(old_data);
				old_data = sk_malloc(total_size);
			}
			for (int32_t i = 0; i < array_count; i++)
				memcpy((uint8_t*)old_data + layer_size * i, readback.current_readback[i].data, layer_size);

			readback.latest_data_ptr  = old_data;
			readback.latest_data_size = total_size;

			// Promote stashed metadata to latest
			if (readback.submit_meta != nullptr && readback.submit_meta_size > 0) {
				if (readback.latest_meta == nullptr || readback.latest_meta_size != readback.submit_meta_size) {
					sk_free(readback.latest_meta);
					readback.latest_meta      = sk_malloc(readback.submit_meta_size);
					readback.latest_meta_size = readback.submit_meta_size;
				}
				memcpy(readback.latest_meta, readback.submit_meta, readback.submit_meta_size);
			}
			readback.has_latest_data = true;
		}
	}

	// Create readback texture lazily, deriving format from the GPU texture
	if (readback.readback_tex == nullptr) {
		readback.readback_tex = tex_create(tex_type_image_nomips, gpu_tex->format);
		tex_set_color_arr(readback.readback_tex, width, height, nullptr, array_count, 1, nullptr);
	}

	// Start new readback if none in-flight and source has new data
	if (readback.pending_readback_count == 0 && frame_id != readback.last_submitted_frame_id) {
		skr_tex_copy(&gpu_tex->gpu_tex, &readback.readback_tex->gpu_tex, 0, 0, 0, 0, array_count);
		int32_t submitted = 0;
		for (int32_t i = 0; i < array_count; i++) {
			if (skr_tex_readback(&readback.readback_tex->gpu_tex, 0, i, &readback.pending_readback[i]) == skr_err_success) {
				submitted++;
			} else {
				break;
			}
		}
		if (submitted == array_count) {
			readback.pending_readback_count  = array_count;
			readback.last_submitted_frame_id = frame_id;
			// Stash metadata for when this readback completes
			if (frame_meta != nullptr && frame_meta_size > 0) {
				if (readback.submit_meta == nullptr || readback.submit_meta_size != frame_meta_size) {
					sk_free(readback.submit_meta);
					readback.submit_meta      = sk_malloc(frame_meta_size);
					readback.submit_meta_size = frame_meta_size;
				}
				memcpy(readback.submit_meta, frame_meta, frame_meta_size);
			}
			skr_cmd_flush();
		} else {
			// Partial failure: destroy any already-submitted readbacks
			for (int32_t i = 0; i < submitted; i++) {
				skr_tex_readback_destroy(&readback.pending_readback[i]);
				readback.pending_readback[i] = {};
			}
		}
	}
}

///////////////////////////////////////////

bool sensor_readback_get(void* out_frame_meta, size_t frame_meta_size, void* out_data, size_t* out_data_size, int32_t array_count, int32_t view_index) {
	if (!readback.has_latest_data)
		return false;

	// Copy out frame metadata
	if (out_frame_meta != nullptr && readback.latest_meta != nullptr) {
		size_t copy_meta = frame_meta_size < readback.latest_meta_size ? frame_meta_size : readback.latest_meta_size;
		memcpy(out_frame_meta, readback.latest_meta, copy_meta);
	}

	// Copy out pixel data
	if (readback.latest_data_ptr != nullptr) {
		size_t         layer_size = readback.latest_data_size / (array_count > 0 ? array_count : 1);
		const uint8_t* src;
		size_t         src_size;

		if (view_index >= 0 && view_index < array_count) {
			src      = (const uint8_t*)readback.latest_data_ptr + layer_size * view_index;
			src_size = layer_size;
		} else {
			src      = (const uint8_t*)readback.latest_data_ptr;
			src_size = readback.latest_data_size;
		}

		if (out_data != nullptr) {
			size_t buf_size  = (out_data_size != nullptr && *out_data_size > 0) ? *out_data_size : src_size;
			size_t copy_size = buf_size < src_size ? buf_size : src_size;
			memcpy(out_data, src, copy_size);
		}

		if (out_data_size != nullptr)
			*out_data_size = src_size;
	} else {
		if (out_data_size != nullptr)
			*out_data_size = 0;
	}
	return true;
}

///////////////////////////////////////////

void sensor_readback_cleanup() {
	int32_t pending_count = readback.pending_readback_count < 2 ? readback.pending_readback_count : 2;
	for (int32_t i = 0; i < pending_count; i++) {
		skr_tex_readback_destroy(&readback.pending_readback[i]);
		readback.pending_readback[i] = {};
	}
	readback.pending_readback_count = 0;
	if (readback.has_current) {
		int32_t arr_count = readback.array_count < 2 ? readback.array_count : 2;
		for (int32_t i = 0; i < arr_count; i++) {
			skr_tex_readback_destroy(&readback.current_readback[i]);
			readback.current_readback[i] = {};
		}
		readback.has_current = false;
	}
	sk_free(readback.latest_data_ptr);
	readback.latest_data_ptr  = nullptr;
	readback.latest_data_size = 0;
	readback.has_latest_data  = false;
	sk_free(readback.submit_meta);
	readback.submit_meta      = nullptr;
	readback.submit_meta_size = 0;
	sk_free(readback.latest_meta);
	readback.latest_meta      = nullptr;
	readback.latest_meta_size = 0;
	readback.wants_cpu_data   = false;
	if (readback.readback_tex != nullptr) {
		tex_release(readback.readback_tex);
		readback.readback_tex = nullptr;
	}
}

///////////////////////////////////////////

static void sensor_shutdown(void*) {
	sensor_readback_cleanup();
}

///////////////////////////////////////////

void sensor_register() {
	xr_system_t system = {};
	system.evt_shutdown = { sensor_shutdown };
	ext_management_sys_register(system);
}

///////////////////////////////////////////

} // namespace sk
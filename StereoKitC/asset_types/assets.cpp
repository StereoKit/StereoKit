// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2023-2024 Qualcomm Technologies, Inc.

#include "assets.h"
#include "../_stereokit.h"
#include "../sk_memory.h"

#include "mesh.h"
#include "texture.h"
#include "texture_compression.h"
#include "shader.h"
#include "material.h"
#include "model.h"
#include "font.h"
#include "sprite.h"
#include "sound.h"
#include "anchor.h"
#include "../platforms/platform.h"
#include "../libraries/stref.h"
#include "../libraries/array.h"
#include "../libraries/sokol_time.h"
#include "../libraries/atomic_util.h"
#include "../libraries/ferr_thread.h"
#include "../libraries/profiler.h"
#include "../systems/render_.h"

#include <stdio.h>
#include <assert.h>
#include <limits.h>

#if defined(SK_OS_WEB)
#include <emscripten/threading.h>
#endif

namespace sk {

///////////////////////////////////////////

struct asset_load_callback_t {
	asset_header_t *asset;
	void (*on_load)(asset_header_t *asset, void *context);
	void *context;
};

struct asset_thread_t {
	ft_id_t  id;
	bool32_t running;
};

///////////////////////////////////////////

array_t<asset_header_t *>      assets = {};
ft_mutex_t                     assets_lock = {};
array_t<asset_header_t *>      assets_multithread_destroy = {};
ft_mutex_t                     assets_multithread_destroy_lock = {};
ft_mutex_t                     assets_job_lock = {};
array_t<asset_job_t *>         assets_gpu_jobs = {};
ft_mutex_t                     assets_load_event_lock = {};
array_t<asset_load_callback_t> assets_load_callbacks = {};
array_t<asset_header_t *>      assets_load_events = {};

///////////////////////////////////////////

array_t<asset_thread_t>asset_threads         = {};
bool32_t               asset_thread_enabled  = false;
array_t<asset_task_t*> asset_thread_tasks    = {};
ft_mutex_t             asset_thread_task_mtx = {};
int32_t                asset_tasks_finished  = 0;
int32_t                asset_tasks_processing= 0;
int32_t                asset_tasks_priority  = INT_MAX;
ft_condition_t         asset_tasks_available = {};
array_t<asset_task_t*> asset_active_tasks    = {};

int32_t         asset_thread          (void *);
void            asset_step_task       ();
asset_header_t* assets_allocate_no_add(asset_type_ type, const char** out_type_str);

///////////////////////////////////////////

void *assets_find(const char *id, asset_type_ type) {
	return assets_find(hash_string(id), type);
}

///////////////////////////////////////////

void *assets_find(id_hash_t id, asset_type_ type) {
	void* result = nullptr;
	ft_mutex_lock(assets_lock);
	for (int32_t i = 0; i < assets.count; i++) {
		if (assets[i]->id == id && assets[i]->type == type && assets[i]->refs > 0) {
			result = assets[i];
			break;
		}
	}
	ft_mutex_unlock(assets_lock);
	return result;
}

///////////////////////////////////////////

asset_find_ assets_find_or_create(const char* id, asset_type_ type, void** out_asset) {
	id_hash_t   hash   = hash_string(id);
	asset_find_ result = asset_find_created;
	ft_mutex_lock(assets_lock);
	for (int32_t i = 0; i < assets.count; i++) {
		if (assets[i]->id == hash && assets[i]->type == type && assets[i]->refs > 0) {
			result     = asset_find_found;
			assets_addref(assets[i]);
			*out_asset = assets[i];
			break;
		}
	}
	if (result == asset_find_created) {
		asset_header_t* header = assets_allocate_no_add(type, nullptr);
		header->id      = hash;
		header->id_text = string_copy(id);
		header->index   = assets.count;
		assets.add(header);

		*out_asset = header;
	}
	ft_mutex_unlock(assets_lock);
	return result;
}

///////////////////////////////////////////

void assets_unique_name(asset_type_ type, const char *root_name, char *dest, int dest_size) {
	snprintf(dest, dest_size, "%s", root_name);
	id_hash_t id    = hash_string(dest);
	int       count = 1;
	while (assets_find(dest, type) != nullptr) {
		snprintf(dest, dest_size, "%s%d", root_name, count);
		id = hash_string(dest);
		count += 1;
	}
}

///////////////////////////////////////////

asset_header_t* assets_allocate_no_add(asset_type_ type, const char** out_type_str) {
	size_t      size      = sizeof(asset_header_t);
	const char* type_name = "asset";
	switch(type) {
	case asset_type_mesh:        size = sizeof(_mesh_t );       type_name = "mesh";        break;
	case asset_type_tex:         size = sizeof(_tex_t);         type_name = "tex";         break;
	case asset_type_shader:      size = sizeof(_shader_t);      type_name = "shader";      break;
	case asset_type_material:    size = sizeof(_material_t);    type_name = "material";    break;
	case asset_type_model:       size = sizeof(_model_t);       type_name = "model";       break;
	case asset_type_font:        size = sizeof(_font_t);        type_name = "font";        break;
	case asset_type_sprite:      size = sizeof(_sprite_t);      type_name = "sprite";      break;
	case asset_type_sound:       size = sizeof(_sound_t);       type_name = "sound";       break;
	case asset_type_anchor:      size = sizeof(_anchor_t);      type_name = "anchor";      break;
	case asset_type_render_list: size = sizeof(_render_list_t); type_name = "render_list"; break;
	default: log_err("Unimplemented asset type!"); abort();
	}

	asset_header_t *header = (asset_header_t *)sk_malloc(size);
	memset(header, 0, size);
	header->type    = type;
	header->state   = asset_state_none;
	assets_addref(header);

	if (out_type_str)
		*out_type_str = type_name;

	return header;
}

///////////////////////////////////////////

void *assets_allocate(asset_type_ type) {
	const char*     type_name;
	asset_header_t* header = assets_allocate_no_add(type, &type_name);

	ft_mutex_lock(assets_lock);
	char name[64];
	snprintf(name, sizeof(name), "auto/%s_%d", type_name, assets.count);
	header->id      = hash_string(name);
	header->id_text = string_copy(name);
	header->index   = assets.count;

	assets.add(header);
	ft_mutex_unlock(assets_lock);
	return header;
}

///////////////////////////////////////////

void assets_set_id(asset_header_t *header, const char *id) {
	assets_set_id(header, hash_string(id));
	char* old_text = header->id_text;
	header->id_text = string_copy(id);
	sk_free(old_text);
}

///////////////////////////////////////////

void assets_set_id(asset_header_t *header, id_hash_t id) {
#if defined(SK_DEBUG)
	asset_header_t *other = (asset_header_t *)assets_find(id, header->type);
	if (other != nullptr) {
		log_errf("Attempted to assign a pre-existing id to an asset! '%s'", header->id_text);
	}
	assert(other == nullptr);
#endif
	header->id = id;
}

///////////////////////////////////////////

void assets_addref(asset_header_t *asset) {
	atomic_increment(&asset->refs);
}

///////////////////////////////////////////

void assets_releaseref(asset_header_t *asset) {
	// Manage the reference count
	if (atomic_decrement(&asset->refs) == 0) {
		assets_destroy(asset);
	} else if (asset->refs < 0) {
		log_errf("Released too many references to asset[%d]%s%s",
			asset->type, 
			asset->id_text!=nullptr?": "          :"",
			asset->id_text!=nullptr?asset->id_text:"");
		abort();
	}
}

///////////////////////////////////////////

void assets_releaseref_threadsafe(void *asset) {
	asset_header_t *asset_header = (asset_header_t *)asset;

	if (!asset_thread_enabled)
		return;

	// Manage the reference count
	if (atomic_decrement(&asset_header->refs) == 0) {
		ft_mutex_lock(assets_multithread_destroy_lock);
		assets_multithread_destroy.add(asset_header);
		ft_mutex_unlock(assets_multithread_destroy_lock);
	} else if (asset_header->refs < 0) {
		log_errf("Released too many references to asset[%d]%s%s",
			asset_header->type, 
			asset_header->id_text!=nullptr?": "                 :"",
			asset_header->id_text!=nullptr?asset_header->id_text:"");
		abort();
	}
}

///////////////////////////////////////////

void assets_destroy(asset_header_t *asset) {
	if (asset->refs != 0) {
		// If something else picked up a reference to this between submission
		// for destruction and now, that's actually just fine! We can just
		// break out of here.
		return;
	}

	// destroy functions will often zero out their contents for safety, but we
	// still need to free the id text. It's nice for debugging to have the name
	// around, so we'll cache it here and free it after destruction.
	char* id_text = asset->id_text;

	// Call asset specific destroy function
	switch(asset->type) {
	case asset_type_mesh:        mesh_destroy       ((mesh_t       )asset); break;
	case asset_type_tex:         tex_destroy        ((tex_t        )asset); break;
	case asset_type_shader:      shader_destroy     ((shader_t     )asset); break;
	case asset_type_material:    material_destroy   ((material_t   )asset); break;
	case asset_type_model:       model_destroy      ((model_t      )asset); break;
	case asset_type_font:        font_destroy       ((font_t       )asset); break;
	case asset_type_sprite:      sprite_destroy     ((sprite_t     )asset); break;
	case asset_type_sound:       sound_destroy      ((sound_t      )asset); break;
	case asset_type_anchor:      anchor_destroy     ((anchor_t     )asset); break;
	case asset_type_render_list: render_list_destroy((render_list_t)asset); break;
	default: log_err("Unimplemented asset type!"); abort();
	}

	sk_free(id_text);

	// Remove it from our list of assets
	ft_mutex_lock(assets_lock);
	for (int32_t i = 0; i < assets.count; i++) {
		if (assets[i] == asset) {
			assets.remove(i);
			break;
		}
	}
	ft_mutex_unlock(assets_lock);

	// And at last, free the memory we allocated for it!
	sk_free(asset);
}

///////////////////////////////////////////

void assets_safeswap_ref(asset_header_t **asset_link, asset_header_t *asset) {
	// Swap references by adding a reference first, then removing. If the asset
	// is the same, then this prevents the asset from getting destroyed.
	assets_addref    ( asset);
	assets_releaseref(*asset_link);
	*asset_link = asset;
}

///////////////////////////////////////////

void assets_on_load(asset_header_t *asset, void (*on_load)(asset_header_t *asset, void *context), void *context) {
	assets_load_callbacks.add({
		asset,
		on_load,
		context
	});

	// If it was loaded previously, we want to call this right away
	if (asset->state >= asset_state_loaded) {
		// If it's already in the event queue, we don't want to call this twice
		if (assets_load_events.index_of(asset) == -1)
			on_load(asset, context);
	}
}

///////////////////////////////////////////

void assets_on_load_remove(asset_header_t *asset, void (*on_load)(asset_header_t *asset, void *context)) {
	for (int32_t i = 0; i < assets_load_callbacks.count; i++) {
		if ( assets_load_callbacks[i].asset   == asset &&
			(assets_load_callbacks[i].on_load == on_load || on_load == nullptr)) {
			assets_load_callbacks.remove(i);
			return;
		}
	}
}

///////////////////////////////////////////

void  assets_shutdown_check() {
	if (assets.count > 0) {
		log_errf("%d unreleased assets still found in the asset manager!", assets.count);
#if defined(SK_DEBUG)
		for (int32_t i = 0; i < assets.count; i++) {
			const char *type_name = "[unimplemented type name]";
			switch(assets[i]->type) {
			case asset_type_mesh:     type_name = "mesh_t";     break;
			case asset_type_tex:      type_name = "tex_t";      break;
			case asset_type_shader:   type_name = "shader_t";   break;
			case asset_type_material: type_name = "material_t"; break;
			case asset_type_model:    type_name = "model_t";    break;
			case asset_type_font:     type_name = "font_t";     break;
			case asset_type_sprite:   type_name = "sprite_t";   break;
			case asset_type_sound:    type_name = "sound_t";    break;
			case asset_type_solid:    type_name = "solid_t";    break;
			case asset_type_anchor:   type_name = "anchor_t";   break;
			default: break;
			}
			log_infof("\t%s (%d): %s", type_name, assets[i]->refs, assets[i]->id_text);
		}
#endif
	} else {
		log_info("All assets were released properly!");
	}
}

///////////////////////////////////////////

char *assets_file(const char *file_name) {
	const sk_settings_t* settings = sk_get_settings_ref();

	if (file_name == nullptr || settings->assets_folder == nullptr || settings->assets_folder[0] == '\0')
		return string_copy(file_name);

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	const char *ch = file_name;
	while (*ch != '\0') {
		if (*ch == ':') {
			return string_copy(file_name);
		}
		ch++;
	}
#elif defined(SK_OS_ANDROID)
	return string_copy(file_name);
#else
	if (file_name[0] == platform_path_separator_c)
		return string_copy(file_name);
#endif

	int   count  = snprintf(nullptr, 0, "%s/%s", settings->assets_folder, file_name);
	char *result = sk_malloc_t(char, count + 1);
	snprintf(result, count+1, "%s/%s", settings->assets_folder, file_name);
	return result;
}

///////////////////////////////////////////

bool assets_init() {
	profiler_zone();

	assets_lock                     = ft_mutex_create();
	assets_multithread_destroy_lock = ft_mutex_create();
	assets_job_lock                 = ft_mutex_create();
	asset_thread_task_mtx           = ft_mutex_create();
	assets_load_event_lock          = ft_mutex_create();
	asset_tasks_available           = ft_condition_create();

	texture_compression_init();

#if !defined(__EMSCRIPTEN__)
	asset_threads.resize(3);
#endif
	asset_thread_enabled = true;
	for (int32_t i = 0; i < asset_threads.capacity; i++)
	{
		asset_threads.add({});
		asset_thread_t* th = &asset_threads.last();
		ft_thread_create(asset_thread, th);
	}

	return true;
}

///////////////////////////////////////////

array_t<asset_load_callback_t> assets_load_call_list = {};
void assets_step() {
	profiler_zone();

	// If we have no asset threads for some reason (like WASM), then we'll need
	// to make sure assets still get loaded here!
	if (asset_threads.count <= 0) {
		asset_step_task();
	}

	// destroy objects where the request came from another thread
	ft_mutex_lock(assets_multithread_destroy_lock);
	for (int32_t i = 0; i < assets_multithread_destroy.count; i++) {
		assets_destroy(assets_multithread_destroy[i]);
	}
	assets_multithread_destroy.clear();
	ft_mutex_unlock(assets_multithread_destroy_lock);

	// Do any jobs the assets need on the main thread, like GPU buffer uploads
	ft_mutex_lock(assets_job_lock);
	for (int32_t i = 0; i < assets_gpu_jobs.count; i++) {
		assets_gpu_jobs[i]->success  = assets_gpu_jobs[i]->asset_job(assets_gpu_jobs[i]->data);
		assets_gpu_jobs[i]->finished = true;
	}
	assets_gpu_jobs.clear();
	ft_mutex_unlock(assets_job_lock);

	// Update any on_load event callbacks
	ft_mutex_lock(assets_load_event_lock);
	for (int32_t i = 0; i < assets_load_events.count; i++) {
		for (int32_t c = 0; c < assets_load_callbacks.count; c++) {
			asset_load_callback_t *callback = &assets_load_callbacks[c];
			if (assets_load_events[i] == callback->asset) {
				// If the callback removes itself when it's called, this loop
				// becomes problematic. So we're storing the items we need to
				// call, and calling them outside this loop.
				assets_load_call_list.add(*callback);
			}
		}
	}
	assets_load_events.clear();
	ft_mutex_unlock(assets_load_event_lock);
	assets_load_call_list.each([](const asset_load_callback_t &c) { c.on_load(c.asset, c.context); });
	assets_load_call_list.clear();

#if defined(SK_DEBUG_MEM)
	if (input_key(key_p) & button_state_just_active) {
		sk_mem_log_allocations();
	}
#endif
}

///////////////////////////////////////////

void assets_shutdown() {
	asset_thread_enabled = false;
	ft_condition_broadcast(asset_tasks_available);
	for (int32_t i = 0; i < asset_threads.count; i++) {
		while (asset_threads[i].running) {
			assets_step();
			ft_yield();
		}
	}
	asset_threads.free();

#if defined(SK_DEBUG_MEM)
	assets_shutdown_check();
#endif

	ft_mutex_destroy(&asset_thread_task_mtx);
	asset_thread_tasks.free();
	asset_active_tasks.free();

	assets_multithread_destroy.free();
	assets_gpu_jobs           .free();
	ft_mutex_destroy(&assets_multithread_destroy_lock);
	ft_mutex_destroy(&assets_job_lock);
	ft_mutex_destroy(&assets_load_event_lock);
	ft_mutex_destroy(&assets_lock);
	ft_condition_destroy(&asset_tasks_available);

	assets_load_call_list.free();
	assets_load_callbacks.free();
	assets_load_events   .free();
	assets               .free();

	asset_tasks_processing = 0;
	asset_tasks_finished   = 0;
	asset_tasks_priority   = INT_MAX;
}

///////////////////////////////////////////

bool32_t assets_execute_gpu(bool32_t(*asset_job)(void *data), void *data) {
	if (ft_id_matches(sk_main_thread())) {
		return asset_job(data);
	} else {
		asset_job_t *job = sk_malloc_t(asset_job_t, 1);
		*job = {};
		job->asset_job = asset_job;
		job->data      = data;

		ft_mutex_lock(assets_job_lock);
		assets_gpu_jobs.add(job);
		ft_mutex_unlock(assets_job_lock);

		// Block until the GPU thread has had a chance to take care of the job.
		uint64_t start      = stm_now();
		bool     has_warned = false;
		while (job->finished == false) {
			ft_yield();

			// if the app hasn't started stepping yet and this takes too long,
			// the application may be off the gpu thread unintentionally.
			if (sk_has_stepped() == false && has_warned == false && stm_ms(stm_since(start)) > 4000) {
				log_warn("A GPU asset is blocking its thread until the main thread is available, has async code accidentally shifted execution to a different thread since SK.Initialize?");
				has_warned = true;
			}
		}

		bool32_t result = job->success;
		sk_free(job);
		return result;
	}
}

///////////////////////////////////////////

int32_t assets_current_task() {
	return asset_tasks_finished;
}

///////////////////////////////////////////

int32_t assets_total_tasks() {
	return asset_tasks_processing + asset_tasks_finished;
}

///////////////////////////////////////////

int32_t assets_current_task_priority() {
	return asset_tasks_priority;
}

///////////////////////////////////////////

int32_t assets_count() {
	return assets.count;
}

///////////////////////////////////////////

asset_t assets_get_index(int32_t index) {
	if (index < 0 || index >= assets.count) return nullptr;
	assets_addref(assets[index]);
	return assets[index];
}

///////////////////////////////////////////

asset_type_ assets_get_type(int32_t index) {
	if (index < 0 || index >= assets.count) return asset_type_none;
	return assets[index]->type;
}

///////////////////////////////////////////
// Asset type                            //
///////////////////////////////////////////

asset_type_ asset_get_type(asset_t asset) {
	return ((asset_header_t*)asset)->type;
}

///////////////////////////////////////////

void asset_set_id(asset_t asset, const char* id) {
	assets_set_id((asset_header_t*)asset, hash_string(id));
}

///////////////////////////////////////////

const char* asset_get_id(const asset_t asset) {
	return ((asset_header_t*)asset)->id_text;
}

///////////////////////////////////////////

void asset_addref(asset_t asset) {
	assets_addref((asset_header_t*)asset);
}

///////////////////////////////////////////

void asset_release(asset_t asset) {
	assets_releaseref((asset_header_t*)asset);
}

///////////////////////////////////////////
// Asset thread                          //
///////////////////////////////////////////

void assets_add_task(asset_task_t src_task) {
	asset_task_t *task = sk_malloc_t(asset_task_t, 1);
	memcpy(task, &src_task, sizeof(asset_task_t));
	assets_addref(task->asset);

	ft_mutex_lock(asset_thread_task_mtx);

	// This array_t function has some strange behavior on 32 bit builds related
	// to render sort items. We're duplicating it here without templating to
	// avoid the issue for now.
	int32_t idx = -1;
	int32_t l = 0, r = asset_thread_tasks.count - 1;
	while (l <= r) {
		int32_t mid     = (l + r) / 2;
		int64_t mid_val = asset_thread_tasks[mid]->sort;
		if      (mid_val < task->sort) l = mid + 1;
		else if (mid_val > task->sort) r = mid - 1;
		else { idx = mid; break; };
	}
	if (idx == -1)
		idx = r < 0 ? r : -(r + 2);

	if (idx < 0) idx = ~idx;
	asset_thread_tasks.insert(idx, task);
	asset_tasks_processing += 1;
	ft_mutex_unlock(asset_thread_task_mtx);

	if (asset_thread_tasks.count > 1) ft_condition_broadcast(asset_tasks_available);
	else                              ft_condition_signal   (asset_tasks_available);
}

///////////////////////////////////////////

int32_t assets_calculate_current_priority() {
	int32_t result = INT_MAX;
	for (int32_t i = 0; i < asset_active_tasks.count; i++) {
		if (result > asset_active_tasks[i]->priority)
			result = asset_active_tasks[i]->priority;
	}
	if (asset_thread_tasks.count > 0 && result > asset_thread_tasks[0]->priority) {
		result = asset_thread_tasks[0]->priority;
	}
	return result;
}

///////////////////////////////////////////

asset_task_t* assets_acquire_task() {
	// Pop out the task we want to work on
	ft_mutex_lock(asset_thread_task_mtx);
	if (asset_thread_tasks.count <= 0) { ft_mutex_unlock(asset_thread_task_mtx); return nullptr; }
	asset_task_t* result = nullptr;

	// Find a task that's ready for work
	for (int32_t i = 0; i < asset_thread_tasks.count; i++) {
		asset_task_t*        task   = asset_thread_tasks[i];
		asset_load_action_t* action = &task->actions[task->action_curr];
		if (action->thread_affinity != asset_thread_gpu || task->gpu_started == false || task->gpu_job.finished) {
			result = task;
			asset_thread_tasks.remove(i);
			asset_active_tasks.add(result);
			asset_tasks_priority = assets_calculate_current_priority();
			break;
		}
	}
	ft_mutex_unlock(asset_thread_task_mtx);

	return result;
}

///////////////////////////////////////////

void assets_return_task(asset_task_t *task) {
	ft_mutex_lock(asset_thread_task_mtx);
	asset_active_tasks.remove(asset_active_tasks.index_of(task));
	asset_thread_tasks.insert(0, task);
	ft_mutex_unlock(asset_thread_task_mtx);

	if (asset_thread_tasks.count > 1) ft_condition_broadcast(asset_tasks_available);
	else                              ft_condition_signal   (asset_tasks_available);
}

///////////////////////////////////////////

void assets_complete_task(asset_task_t* task) {
	// Skip putting it back if it's complete :)

	ft_mutex_lock(asset_thread_task_mtx);
	asset_active_tasks.remove(asset_active_tasks.index_of(task));
	asset_tasks_finished   += 1;
	asset_tasks_processing -= 1;
	asset_tasks_priority    = assets_calculate_current_priority();
	ft_mutex_unlock(asset_thread_task_mtx);

	// If it was successfully loaded, we'll want to notify on_load, but we do
	// want to skip this if it was removed because of an issue during load.
	if (task->asset->state >= asset_state_loaded) {
		ft_mutex_lock(assets_load_event_lock);
		assets_load_events.add(task->asset);
		ft_mutex_unlock(assets_load_event_lock);
	}

	if (task->free_data != nullptr) task->free_data(task->asset, task->load_data);
	assets_releaseref_threadsafe(task->asset);
	sk_free(task);
}

///////////////////////////////////////////

void assets_task_set_complexity(asset_task_t *task, int32_t complexity) {
}

///////////////////////////////////////////

void asset_step_task() {
	asset_task_t* task = assets_acquire_task();
	if (task == nullptr) return;

	profiler_zone();

	asset_load_action_t* action = &task->actions[task->action_curr];
	if (action->thread_affinity == asset_thread_asset) {
		// Execute the asset loading action!
		bool result = action->action(task, task->asset, task->load_data);

		if (result == false) {
			// On failure, send an error message, and move to the end
			// of the action list.
			if (task->on_failure != nullptr) task->on_failure(task->asset, task->load_data);
			task->action_curr = task->action_count;
		}
		else {
			// On success, move to the next action in the task!
			task->action_curr += 1;
		}
	} else if (action->thread_affinity == asset_thread_gpu) {
		if (task->gpu_started == false) {
			task->gpu_started = true;

			// Set up a job for the GPU thread
			task->gpu_job.data = task;
			task->gpu_job.asset_job = [](void* data) {
				asset_task_t* task = (asset_task_t*)data;
				asset_load_action_t* action = &task->actions[task->action_curr];
				bool result = action->action(task, task->asset, task->load_data);

				return (bool32_t)result;
			};

			// Add the job to the list
			ft_mutex_lock(assets_job_lock);
			assets_gpu_jobs.add(&task->gpu_job);
			ft_mutex_unlock(assets_job_lock);
		} else if (task->gpu_job.finished) {
			if (task->gpu_job.success == false) {
				// On failure, send an error message, and move to
				// the end of the action list.
				task->asset->state = asset_state_error;
				if (task->on_failure != nullptr) task->on_failure(task->asset, task->load_data);
				task->action_curr = task->action_count;
			}
			else {
				// On success, move to the next action in the task!
				task->action_curr += 1;
			}
			task->gpu_job = {};
			task->gpu_started = false;
		}
	}

	// Put it back in when we're done!
	if (task->action_curr < task->action_count) {
		assets_return_task(task);
	} else {
		assets_complete_task(task);
	}
}

///////////////////////////////////////////

int32_t asset_thread(void *thread_inst_obj) {
	asset_thread_t* thread = (asset_thread_t*)thread_inst_obj;
	thread->id      = ft_id_current();
	thread->running = true;

	char name[64];
	snprintf(name, sizeof(name), "StereoKit Assets 0x%X", (uint32_t)(uint64_t)&thread->id);
	profiler_thread_name("StereoKit Assets", 1);

	// Don't start processing assets until initialization is finished
	while (asset_thread_enabled && !sk_is_initialized())
		platform_sleep(1);

	ft_mutex_t wait_mtx = ft_mutex_create();

	while (asset_thread_enabled || asset_thread_tasks.count>0) {
		asset_step_task();

		if (asset_thread_enabled && asset_thread_tasks.count == 0)
			ft_condition_wait(asset_tasks_available, wait_mtx);
	}

	ft_mutex_destroy(&wait_mtx);
	thread->running = false;
	return 0;
}

///////////////////////////////////////////

void assets_block_until(asset_header_t *asset, asset_state_ state) {
	// If we're past the required state already, drop out. asset_state_none and
	// below (error states) means no loading is happening, so blocking will
	// only put us in an infinite loop.
	if (asset->state >= state || asset->state <= asset_state_none)
		return;

	profiler_zone();

	ft_id_t curr_id = ft_id_current();
	for (int32_t i = 0; i < asset_threads.count; i++)
	{
		if (ft_id_equal(curr_id, asset_threads[i].id)) {
			log_err("assets_block_ should not be called on the assets thread!");
			return;
		}
	}

	while (asset->state < state && asset->state >= 0) {
		// Spin the GPU thread so the asset thread doesn't freeze up while
		// we're waiting on it.
		assets_step();
	}
}

///////////////////////////////////////////

void assets_block_for_priority(int32_t priority) {
	profiler_zone();

	ft_id_t curr_id = ft_id_current();
	for (int32_t i = 0; i < asset_threads.count; i++)
	{
		if (ft_id_equal(curr_id, asset_threads[i].id)) {
			log_err("assets_block_ should not be called on the assets thread!");
			return;
		}
	}

	// This handles if the user passes in INT_MAX
	int32_t curr_priority = assets_current_task_priority();
	while (curr_priority <= priority && curr_priority != INT_MAX) {
		// Spin the GPU thread so the asset thread doesn't freeze up while
		// we're waiting on it.
		assets_step();
		curr_priority = assets_current_task_priority();
	}
}

} // namespace sk
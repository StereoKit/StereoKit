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
#include "compute.h"
#include "compute_buffer.h"
#include "../platforms/platform.h"
#include "../libraries/stref.h"
#include "../libraries/array.h"
#include "../libraries/sokol_time.h"
#include "../libraries/atomic_util.h"
#include "../libraries/ferr_thread.h"
#include "../libraries/profiler.h"
#include "../systems/render_.h"

#include <sk_app.h>

#include <stdio.h>
#include <assert.h>
#include <limits.h>

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
array_t<asset_job_t *>         assets_blocking_jobs = {};
int32_t                        assets_blocking_count   = 0; // atomic mirror of the job count
int32_t                        assets_blocking_waiters = 0; // atomic, threads inside the wait
ft_condition_t                 assets_blocking_available = {};
ft_mutex_t                     assets_load_event_lock = {}; // guards both lists below; C# finalizers remove callbacks off-thread
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
ft_mutex_t             asset_thread_wait_mtx = {};
int32_t                assets_wake_gen       = 0; // atomic, bumped by every wake
uint64_t               assets_backstop_time  = 0;
array_t<asset_task_t*> asset_active_tasks    = {};

// Acquire masks, a bit per affinity. Workers never run main-affinity actions,
// and the budgeted main-thread step only starts heavy ones on a fresh budget.
enum {
	asset_mask_any   = 1 << asset_affinity_any,
	asset_mask_heavy = 1 << asset_affinity_heavy,
	asset_mask_main  = 1 << asset_affinity_main,
	asset_mask_worker   = asset_mask_any | asset_mask_heavy,
	asset_mask_all      = asset_mask_any | asset_mask_heavy | asset_mask_main,
	asset_mask_no_heavy = asset_mask_any | asset_mask_main,
};

typedef enum asset_step_ {
	asset_step_none = 0, // nothing runnable for this mask
	asset_step_ran,
	asset_step_parked,   // an action began a wait
} asset_step_;

array_t<asset_task_t*> asset_parked_tasks    = {}; // waiting on assets_task_signal

int32_t         asset_thread                    (void *);
void            assets_wake_workers             ();
asset_step_     asset_step_task                 (int32_t affinity_mask);
void            asset_step_blocking_job         ();
int32_t         assets_calculate_current_priority();
asset_header_t* assets_allocate_no_add          (asset_type_ type, const char** out_type_str);

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
	case asset_type_mesh:            size = sizeof(_mesh_t );           type_name = "mesh";            break;
	case asset_type_tex:             size = sizeof(_tex_t);             type_name = "tex";             break;
	case asset_type_shader:          size = sizeof(_shader_t);          type_name = "shader";          break;
	case asset_type_material:        size = sizeof(_material_t);        type_name = "material";        break;
	case asset_type_model:           size = sizeof(_model_t);           type_name = "model";           break;
	case asset_type_font:            size = sizeof(_font_t);            type_name = "font";            break;
	case asset_type_sprite:          size = sizeof(_sprite_t);          type_name = "sprite";          break;
	case asset_type_sound:           size = sizeof(_sound_t);           type_name = "sound";           break;
	case asset_type_anchor:          size = sizeof(_anchor_t);          type_name = "anchor";          break;
	case asset_type_render_list:     size = sizeof(_render_list_t);     type_name = "render_list";     break;
	case asset_type_compute:         size = sizeof(_compute_t);         type_name = "compute";         break;
	case asset_type_compute_buffer:  size = sizeof(_compute_buffer_t);  type_name = "compute_buffer";  break;
	case asset_type_material_buffer: size = sizeof(_material_buffer_t); type_name = "material_buffer"; break;
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
	// Check if we've shut down the asset system and have already destroyed any
	// lingering assets (or haven't started the asset system yet).
	if (asset_thread_enabled == false)
		return;

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

	// Check if we've shut down the asset system and have already destroyed any
	// lingering assets (or haven't started the asset system yet).
	if (asset_thread_enabled == false)
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

	// Remove any on_load callbacks associated with this asset to prevent stale
	// callbacks from firing if memory is reused for a new asset.
	assets_on_load_remove_all(asset);

	// destroy functions will often zero out their contents for safety, but we
	// still need to free the id text. It's nice for debugging to have the name
	// around, so we'll cache it here and free it after destruction.
	char* id_text = asset->id_text;

	// Call asset specific destroy function
	switch(asset->type) {
	case asset_type_mesh:            mesh_destroy           ((mesh_t           )asset); break;
	case asset_type_tex:             tex_destroy            ((tex_t            )asset); break;
	case asset_type_shader:          shader_destroy         ((shader_t         )asset); break;
	case asset_type_material:        material_destroy       ((material_t       )asset); break;
	case asset_type_model:           model_destroy          ((model_t          )asset); break;
	case asset_type_font:            font_destroy           ((font_t           )asset); break;
	case asset_type_sprite:          sprite_destroy         ((sprite_t         )asset); break;
	case asset_type_sound:           sound_destroy          ((sound_t          )asset); break;
	case asset_type_anchor:          anchor_destroy         ((anchor_t         )asset); break;
	case asset_type_render_list:     render_list_destroy    ((render_list_t    )asset); break;
	case asset_type_compute:         compute_destroy        ((compute_t        )asset); break;
	case asset_type_compute_buffer:  compute_buffer_destroy ((compute_buffer_t )asset); break;
	case asset_type_material_buffer: material_buffer_destroy((material_buffer_t)asset); break;
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
	ft_mutex_lock(assets_load_event_lock);
	assets_load_callbacks.add({
		asset,
		on_load,
		context
	});
	// If it was loaded previously, we want to call this right away, unless a
	// queued event is about to. Asset threads push events concurrently, so the
	// check stays under the lock; the call itself must not.
	bool call_now = asset->state >= asset_state_loaded &&
	                assets_load_events.index_of(asset) == -1;
	ft_mutex_unlock(assets_load_event_lock);

	if (call_now)
		on_load(asset, context);
}

///////////////////////////////////////////

void assets_on_load_remove(asset_header_t *asset, void (*on_load)(asset_header_t *asset, void *context), void *context) {
	ft_mutex_lock(assets_load_event_lock);
	for (int32_t i = 0; i < assets_load_callbacks.count; i++) {
		if (assets_load_callbacks[i].asset   == asset   &&
			assets_load_callbacks[i].on_load == on_load &&
			assets_load_callbacks[i].context == context) {
			assets_load_callbacks.remove(i);
			break;
		}
	}
	ft_mutex_unlock(assets_load_event_lock);
}

///////////////////////////////////////////

void assets_on_load_remove_all(asset_header_t *asset) {
	ft_mutex_lock(assets_load_event_lock);
	for (int32_t i = assets_load_callbacks.count - 1; i >= 0; i--) {
		if (assets_load_callbacks[i].asset == asset) {
			assets_load_callbacks.remove(i);
		}
	}
	ft_mutex_unlock(assets_load_event_lock);
}

///////////////////////////////////////////

void  assets_shutdown_check() {
	if (assets.count > 0) {
		log_errf("%d unreleased assets still found in the asset manager!", assets.count);
#if defined(SK_DEBUG)
		for (int32_t i = 0; i < assets.count; i++) {
			const char *type_name = "[unimplemented type name]";
			switch(assets[i]->type) {
			case asset_type_mesh:            type_name = "mesh_t";            break;
			case asset_type_tex:             type_name = "tex_t";             break;
			case asset_type_shader:          type_name = "shader_t";          break;
			case asset_type_material:        type_name = "material_t";        break;
			case asset_type_model:           type_name = "model_t";           break;
			case asset_type_font:            type_name = "font_t";            break;
			case asset_type_sprite:          type_name = "sprite_t";          break;
			case asset_type_sound:           type_name = "sound_t";           break;
			case asset_type_solid:           type_name = "solid_t";           break;
			case asset_type_anchor:          type_name = "anchor_t";          break;
			case asset_type_render_list:     type_name = "render_list_t";     break;
			case asset_type_compute:         type_name = "compute_t";         break;
			case asset_type_compute_buffer:  type_name = "compute_buffer_t";  break;
			case asset_type_material_buffer: type_name = "material_buffer_t"; break;
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

#if defined(SK_OS_WINDOWS)
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
	asset_thread_wait_mtx           = ft_mutex_create();
	assets_load_event_lock          = ft_mutex_create();
	asset_tasks_available           = ft_condition_create();
	assets_blocking_available       = ft_condition_create();

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

// Only meaningful on single-threaded builds, where assets_step is the only
// place tasks step.
static bool assets_step_did_work = false;
// Blocking pumps pass their own unbounded budget instead.
static const double assets_step_budget_ms = 4;

static void assets_step_tasks(double budget_ms) {
	if (asset_threads.count > 0) {
		// Workers cover everything else; the main thread only services
		// actions that must run here, and those stay cheap by contract.
		if (ft_id_equal(ft_id_current(), sk_main_thread())) {
			while (asset_step_task(asset_mask_main) != asset_step_none) { }
		}
		return;
	}

	// Without workers the main thread is the loader, so it runs as much of the
	// queue as fits the budget instead of one action per frame. Heavy work is
	// deliberately capped at one acquisition per frame: a single slice can be
	// near budget-sized, and an unsliced action can overrun it entirely.
	uint64_t start    = stm_now();
	bool     advanced = false;
	bool     first    = true;
	while (true) {
		asset_step_ stepped = asset_step_task(first ? asset_mask_all : asset_mask_no_heavy);
		first = false;
		if (stepped == asset_step_ran) advanced = true;
		if (stepped == asset_step_none    ) break;
		if (stm_ms(stm_since(start)) >= budget_ms) break;
	}
	assets_step_did_work = advanced;
}

void assets_step() {
	profiler_zone();

	// Before tasks step, so a signal that lands here runs this same frame.
	if (ft_id_equal(ft_id_current(), sk_main_thread()))
		tex_step_deferred();

	assets_step_tasks(assets_step_budget_ms);

	// Wake-up backstop for dependency state that advances outside the task
	// system. Paced, blocking loads spin this far faster than frame rate.
	uint64_t backstop_now = stm_now();
	if ((atomic_load_i32(&asset_tasks_processing) > 0  ||
	     atomic_load_i32(&assets_blocking_count ) > 0) &&
	    stm_ms(stm_diff(backstop_now, assets_backstop_time)) >= 1) {
		assets_backstop_time = backstop_now;
		assets_wake_workers();
	}

	// destroy objects where the request came from another thread
	ft_mutex_lock(assets_multithread_destroy_lock);
	for (int32_t i = 0; i < assets_multithread_destroy.count; i++) {
		assets_destroy(assets_multithread_destroy[i]);
	}
	assets_multithread_destroy.clear();
	ft_mutex_unlock(assets_multithread_destroy_lock);

	// Update any on_load event callbacks. They run from a local copy so a
	// callback can add or remove others, and because blocking loads step this
	// from more than one thread.
	array_t<asset_load_callback_t> call_list = {};
	ft_mutex_lock(assets_load_event_lock);
	for (int32_t i = 0; i < assets_load_events.count; i++) {
		for (int32_t c = 0; c < assets_load_callbacks.count; c++) {
			asset_load_callback_t *callback = &assets_load_callbacks[c];
			if (assets_load_events[i] == callback->asset)
				call_list.add(*callback);
		}
	}
	assets_load_events.clear();
	ft_mutex_unlock(assets_load_event_lock);
	call_list.each([](const asset_load_callback_t &c) { c.on_load(c.asset, c.context); });
	call_list.free();

#if defined(SK_DEBUG_MEM)
	if (input_key(key_p) & button_state_just_active) {
		sk_mem_log_allocations();
	}
#endif
}

///////////////////////////////////////////

void assets_shutdown() {
	// Signal asset threads to drain remaining tasks and exit. Use a single
	// loop for all threads so we keep calling assets_step while any thread
	// is still running — the old per-thread sequential loop could miss
	// queued work from threads that exited while waiting on another.
	asset_thread_enabled = false;

	// Parked tasks wait on signals that are never coming now; route them
	// through the failure path like an unresolved dependency gate. Their
	// external completions see a stale wait_id and no-op.
	ft_mutex_lock(asset_thread_task_mtx);
	for (int32_t i = 0; i < asset_parked_tasks.count; i++) {
		asset_task_t *task = asset_parked_tasks[i];
		task->wait_id    = 0;
		task->dep_failed = true;
		asset_thread_tasks.insert(0, task);
	}
	asset_parked_tasks.clear();
	asset_tasks_priority = assets_calculate_current_priority();
	ft_mutex_unlock(asset_thread_task_mtx);

	assets_wake_workers();
	// Threadless builds drain the queue right here, so the loop also runs
	// until it empties; the parked tasks above land in it too.
	bool any_running = true;
	while (any_running || asset_thread_tasks.count > 0) {
		assets_step();
		ft_yield();

		any_running = false;
		for (int32_t i = 0; i < asset_threads.count; i++) {
			if (asset_threads[i].running) { any_running = true; break; }
		}
	}
	asset_threads.free();

	// Fail any blocking jobs the workers never got to, then wait for their
	// foreign threads to leave before the primitives get destroyed below.
	ft_mutex_lock(assets_job_lock);
	for (int32_t i = 0; i < assets_blocking_jobs.count; i++) {
		assets_blocking_jobs[i]->success  = false;
		assets_blocking_jobs[i]->finished = true;
	}
	assets_blocking_jobs.clear();
	atomic_store_i32(&assets_blocking_count, 0);
	ft_condition_broadcast(assets_blocking_available);
	ft_mutex_unlock(assets_job_lock);
	while (atomic_load_i32(&assets_blocking_waiters) > 0)
		ft_yield();

	// Deferred readbacks hold asset references, so they resolve before the
	// remaining-asset teardown below.
	tex_shutdown_deferred();

#if defined(SK_DEBUG_MEM)
	assets_shutdown_check();
#endif

	// Explicitly destroy any assets that are remaining. C# will often have a
	// lot of these due to GC based asset releases.
	// Destroy in reverse, as assets_destroy will _remove_ the item from the
	// assets array on destroy!
	for (int32_t i = assets.count-1; i >= 0; i--) {
		// mark as no refs, or assets_destroy will not be pleased.
		assets[i]->refs = 0;
		assets_destroy(assets[i]);
	}

	ft_mutex_destroy(&asset_thread_task_mtx);
	asset_thread_tasks.free();
	asset_active_tasks.free();
	asset_parked_tasks.free();

	assets_multithread_destroy.free();
	assets_blocking_jobs      .free();
	ft_mutex_destroy(&assets_multithread_destroy_lock);
	ft_mutex_destroy(&assets_job_lock);
	ft_mutex_destroy(&asset_thread_wait_mtx);
	ft_mutex_destroy(&assets_load_event_lock);
	ft_mutex_destroy(&assets_lock);
	ft_condition_destroy(&asset_tasks_available);
	ft_condition_destroy(&assets_blocking_available);

	assets_load_callbacks.free();
	assets_load_events   .free();
	assets               .free();

	atomic_store_i32(&asset_tasks_processing, 0);
	atomic_store_i32(&asset_tasks_finished,   0);
	atomic_store_i32(&assets_blocking_count,  0);
	atomic_store_i32(&assets_wake_gen,        0);
	assets_backstop_time = 0;
	asset_tasks_priority = INT_MAX;
}

///////////////////////////////////////////

bool32_t assets_execute_blocking(bool32_t(*asset_job)(void *data), void *data) {
	// If we're on any GPU-initialized thread (main thread, asset threads),
	// we can run directly. Otherwise, queue to an asset thread and wait.
	if (skr_thread_is_initialized()) {
		return asset_job(data);
	} else {
		// Refuse rather than queue a job the drained workers will never run
		if (asset_thread_enabled == false) return false;

		// This thread outlives the job, so the stack is a fine home for it.
		// The waiter count keeps shutdown from destroying the primitives
		// here until this thread is fully out of them.
		asset_job_t job = {};
		job.asset_job = asset_job;
		job.data      = data;

		atomic_increment(&assets_blocking_waiters);
		ft_mutex_lock(assets_job_lock);
		assets_blocking_jobs.add(&job);
		atomic_increment(&assets_blocking_count);

		// Wake up asset threads to process the job
		assets_wake_workers();

		// Sleep until an asset thread signals the job is finished
		while (job.finished == false)
			ft_condition_wait(assets_blocking_available, assets_job_lock);
		ft_mutex_unlock(assets_job_lock);
		atomic_decrement(&assets_blocking_waiters);

		return job.success;
	}
}

///////////////////////////////////////////

int32_t assets_current_task() {
	return atomic_load_i32(&asset_tasks_finished);
}

///////////////////////////////////////////

int32_t assets_total_tasks() {
	return atomic_load_i32(&asset_tasks_processing) + atomic_load_i32(&asset_tasks_finished);
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
	assets_set_id((asset_header_t*)asset, id);
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

// Wakes bump the generation under the wait mutex, so a worker that decided
// to sleep on stale state re-scans instead of sleeping through the wake.
void assets_wake_workers() {
	ft_mutex_lock(asset_thread_wait_mtx);
	atomic_increment(&assets_wake_gen);
	ft_condition_broadcast(asset_tasks_available);
	ft_mutex_unlock(asset_thread_wait_mtx);
}

///////////////////////////////////////////

int64_t assets_task_sort(asset_task_t *task) { return task->sort; }

///////////////////////////////////////////

void assets_add_task(asset_task_t src_task) {
	asset_task_t *task = sk_malloc_t(asset_task_t, 1);
	memcpy(task, &src_task, sizeof(asset_task_t));
	assets_addref(task->asset);

	if (task->depends_on == task->asset) {
		log_err("Asset task can't depend on its own asset!");
		task->depends_on = nullptr;
	}
	if (task->depends_on != nullptr)
		assets_addref(task->depends_on);

	ft_mutex_lock(asset_thread_task_mtx);
	int32_t idx = asset_thread_tasks.binary_search(assets_task_sort, task->sort);
	if (idx < 0) idx = ~idx;
	asset_thread_tasks.insert(idx, task);
	atomic_increment(&asset_tasks_processing);
	asset_tasks_priority = assets_calculate_current_priority();
	ft_mutex_unlock(asset_thread_task_mtx);

	assets_wake_workers();
}

///////////////////////////////////////////

int32_t assets_calculate_current_priority() {
	int32_t result = INT_MAX;
	for (int32_t i = 0; i < asset_active_tasks.count; i++) {
		if (result > asset_active_tasks[i]->priority)
			result = asset_active_tasks[i]->priority;
	}
	// Parked tasks still count as in-flight, so priority-blocking callers
	// keep waiting for the signal they're parked on.
	for (int32_t i = 0; i < asset_parked_tasks.count; i++) {
		if (result > asset_parked_tasks[i]->priority)
			result = asset_parked_tasks[i]->priority;
	}
	if (asset_thread_tasks.count > 0 && result > asset_thread_tasks[0]->priority) {
		result = asset_thread_tasks[0]->priority;
	}
	return result;
}

///////////////////////////////////////////

typedef enum asset_dep_ {
	asset_dep_ready,
	asset_dep_blocked,
	asset_dep_failed,
} asset_dep_;

// Gates a task until its dependency reaches the requested state. Errors and
// never-started (none) assets fail the gate open, shutdown forces all open.
asset_dep_ asset_task_dep_check(const asset_task_t *task) {
	if (task->depends_on == nullptr)                  return asset_dep_ready;
	asset_state_ state = task->depends_on->state;
	if (state <= asset_state_none)                    return asset_dep_failed;
	if (state >= task->depends_state)                 return asset_dep_ready;
	return asset_thread_enabled ? asset_dep_blocked : asset_dep_failed;
}

///////////////////////////////////////////

asset_task_t* assets_acquire_task(int32_t affinity_mask) {
	// Pop out the task we want to work on
	ft_mutex_lock(asset_thread_task_mtx);
	if (asset_thread_tasks.count <= 0) { ft_mutex_unlock(asset_thread_task_mtx); return nullptr; }
	asset_task_t* result = nullptr;

	// Find a task that's ready for work. The dependency gate is evaluated
	// here only, the stamp is what the task's steps act on later.
	for (int32_t i = 0; i < asset_thread_tasks.count; i++) {
		asset_task_t* task = asset_thread_tasks[i];
		asset_dep_    dep  = asset_task_dep_check(task);
		if (dep == asset_dep_blocked) continue;
		// A failing task skips the affinity check, its failure path must be
		// able to run anywhere or shutdown could strand it in the queue.
		if (dep == asset_dep_ready && !task->dep_failed &&
		    (affinity_mask & (1 << task->actions[task->action_curr].affinity)) == 0) continue;

		task->dep_failed     = task->dep_failed || dep == asset_dep_failed;
		task->running_thread = ft_id_current();
		result = task;
		asset_thread_tasks.remove(i);
		asset_active_tasks.add(result);
		asset_tasks_priority = assets_calculate_current_priority();
		break;
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

	assets_wake_workers();
}

///////////////////////////////////////////

static int32_t assets_wait_id_next = 0; // atomic

uint64_t assets_task_wait_prepare(asset_task_t *task) {
	// Unlocked writes are safe because this runs on the action's own thread,
	// and the contract keeps every signal on that same thread until parked.
	task->wait_id  = (uint64_t)atomic_increment(&assets_wait_id_next);
	task->signal_beat_park = false;
	return task->wait_id;
}

///////////////////////////////////////////

// The task leaves the queue entirely until its signal, so waits cost nothing
// per frame. False when shutdown refuses the park; the caller fails the task
// instead, since its signal is never coming.
static bool assets_park_task(asset_task_t *task) {
	ft_mutex_lock(asset_thread_task_mtx);
	if (asset_thread_enabled == false) {
		ft_mutex_unlock(asset_thread_task_mtx);
		return false;
	}

	asset_active_tasks.remove(asset_active_tasks.index_of(task));
	bool requeue = task->signal_beat_park;
	if (requeue) {
		// The signal beat the park, skip it and go straight back to work
		task->signal_beat_park = false;
		task->wait_id  = 0;
		int32_t idx = asset_thread_tasks.binary_search(assets_task_sort, task->sort);
		if (idx < 0) idx = ~idx;
		asset_thread_tasks.insert(idx, task);
	} else {
		asset_parked_tasks.add(task);
	}
	asset_tasks_priority = assets_calculate_current_priority();
	ft_mutex_unlock(asset_thread_task_mtx);

	if (requeue) assets_wake_workers();
	return true;
}

///////////////////////////////////////////

bool32_t assets_task_signal(uint64_t wait_id, void (*deliver)(asset_task_t *task, void *load_data, void *context), void *context) {
	if (wait_id == 0) return false;

	ft_mutex_lock(asset_thread_task_mtx);
	// Parked is the expected home: deliver, then re-queue by sort.
	for (int32_t i = 0; i < asset_parked_tasks.count; i++) {
		asset_task_t *task = asset_parked_tasks[i];
		if (task->wait_id != wait_id) continue;

		// Contract check, see the header. Deliver anyway, dropping the result
		// would strand the task on top of the race being reported.
		if (!ft_id_equal(ft_id_current(), sk_main_thread()))
			log_err("assets_task_signal: a parked task may only be signaled from the main thread");

		asset_parked_tasks.remove(i);
		task->wait_id = 0;
		if (deliver != nullptr) deliver(task, task->load_data, context);
		int32_t idx = asset_thread_tasks.binary_search(assets_task_sort, task->sort);
		if (idx < 0) idx = ~idx;
		asset_thread_tasks.insert(idx, task);
		asset_tasks_priority = assets_calculate_current_priority();
		ft_mutex_unlock(asset_thread_task_mtx);
		assets_wake_workers();
		return true;
	}
	// Not parked yet: the signal came from the initiating action's own stack,
	// or beat the park's arrival. Flag the task to skip the park; delivery is
	// safe by the contract in the header.
	for (int32_t i = 0; i < asset_active_tasks.count; i++) {
		asset_task_t *task = asset_active_tasks[i];
		if (task->wait_id != wait_id) continue;

		// Contract check, as above: any other thread races the running action
		if (!ft_id_equal(ft_id_current(), task->running_thread))
			log_err("assets_task_signal: an active task may only be signaled from its own action");

		task->signal_beat_park = true;
		if (deliver != nullptr) deliver(task, task->load_data, context);
		ft_mutex_unlock(asset_thread_task_mtx);
		return true;
	}
	// Otherwise stale: the task already completed, or failed at shutdown.
	ft_mutex_unlock(asset_thread_task_mtx);
	return false;
}

///////////////////////////////////////////

// A completion can outlive its task, so it carries the wait id, and the
// stale-id check in assets_task_signal is what guards the read struct's
// lifetime: deliver only runs while the task, and so its load_data, is alive.
// When the task is already gone, the bytes have no home and are dropped.
struct asset_read_ctx_t {
	uint64_t           wait_id;
	asset_file_read_t *read;
};

static void assets_task_read_arrived(bool32_t success, void *data, size_t size, void *context) {
	asset_read_ctx_t ctx = *(asset_read_ctx_t*)context;
	sk_free(context);

	struct payload_t { asset_file_read_t* read; bool32_t success; void* data; size_t size; };
	payload_t payload = { ctx.read, success, data, size };
	bool32_t delivered = assets_task_signal(ctx.wait_id, [](asset_task_t*, void*, void* payload_ptr) {
		payload_t* p   = (payload_t*)payload_ptr;
		p->read->state = p->success ? asset_read_arrived : asset_read_failed;
		p->read->data  = p->data;
		p->read->size  = p->size;
	}, &payload);
	if (!delivered) sk_free(data);
}

asset_read_ assets_task_read_file(asset_task_t *task, const char *filename, asset_file_read_t *read) {
	if (!read->pending) {
		read->pending = true;
		read->state   = asset_read_in_flight;
		asset_read_ctx_t* ctx = sk_malloc_t(asset_read_ctx_t, 1);
		ctx->wait_id = assets_task_wait_prepare(task);
		ctx->read    = read;
		platform_read_file_async(filename, assets_task_read_arrived, ctx);
	}
	if (read->state != asset_read_in_flight)
		read->pending = false;
	return read->state;
}

///////////////////////////////////////////

void assets_notify_loaded(asset_header_t *asset) {
	ft_mutex_lock(assets_load_event_lock);
	assets_load_events.add(asset);
	ft_mutex_unlock(assets_load_event_lock);
}

///////////////////////////////////////////

void assets_complete_task(asset_task_t* task) {
	// Skip putting it back if it's complete :)

	// Notify on_load, skipping assets removed by a load issue. Queued before
	// the task count drops, so a blocking caller can't miss the event.
	if (task->asset->state >= asset_state_loaded)
		assets_notify_loaded(task->asset);

	ft_mutex_lock(asset_thread_task_mtx);
	asset_active_tasks.remove(asset_active_tasks.index_of(task));
	atomic_increment(&asset_tasks_finished);
	atomic_decrement(&asset_tasks_processing);
	asset_tasks_priority = assets_calculate_current_priority();
	ft_mutex_unlock(asset_thread_task_mtx);

	if (task->free_data != nullptr) task->free_data(task->asset, task->load_data);
	if (task->depends_on != nullptr) assets_releaseref_threadsafe(task->depends_on);
	assets_releaseref_threadsafe(task->asset);
	sk_free(task);

	// Completion advances the asset's state, including on failure. That can
	// open dependency gates, so wake the workers for a re-scan.
	assets_wake_workers();
}

///////////////////////////////////////////

asset_step_ asset_step_task(int32_t affinity_mask) {
	asset_task_t* task = assets_acquire_task(affinity_mask);
	if (task == nullptr) return asset_step_none;

	profiler_zone();

	// An errored dependency skips the action and takes the failure path.
	// on_failure owns the resulting state: a refresh may want to stay loaded.
	asset_action_result_ result = task->dep_failed
		? asset_action_fail
		: task->actions[task->action_curr].fn(task, task->asset, task->load_data);

	if (result == asset_action_wait) {
		if (assets_park_task(task))
			return asset_step_parked;
		// Shutdown refused the park, its signal is never coming
		result = asset_action_fail;
	}

	switch (result) {
	case asset_action_fail:
		if (task->on_failure != nullptr) task->on_failure(task->asset, task->load_data);
		task->action_curr = task->action_count;
		break;
	case asset_action_done:     task->action_curr += 1; break;
	case asset_action_continue: break; // same action again, a cooperative slice
	default: break;
	}

	// Put it back in when we're done!
	if (task->action_curr < task->action_count) {
		assets_return_task(task);
	} else {
		assets_complete_task(task);
	}
	return asset_step_ran;
}

///////////////////////////////////////////

void asset_step_blocking_job() {
	// Process all blocking jobs - these are synchronous waits so prioritize them
	while (true) {
		// Acquire a blocking job if one is available
		ft_mutex_lock(assets_job_lock);
		if (assets_blocking_jobs.count <= 0) {
			ft_mutex_unlock(assets_job_lock);
			return;
		}
		asset_job_t *job = assets_blocking_jobs[0];
		assets_blocking_jobs.remove(0);
		atomic_decrement(&assets_blocking_count);
		ft_mutex_unlock(assets_job_lock);

		// Results go back under the lock so the waiter can't miss the wake,
		// or free the job early. Broadcast: each waiter watches its own job.
		bool32_t success = job->asset_job(job->data);

		ft_mutex_lock(assets_job_lock);
		job->success  = success;
		job->finished = true;
		ft_condition_broadcast(assets_blocking_available);
		ft_mutex_unlock(assets_job_lock);
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
		ska_time_sleep(1);

	skr_thread_init();

	while (asset_thread_enabled || asset_thread_tasks.count>0 || atomic_load_i32(&assets_blocking_count)>0) {
		int32_t wake_gen = atomic_load_i32(&assets_wake_gen);
		asset_step_blocking_job();
		bool worked = asset_step_task(asset_mask_worker) != asset_step_none;

		// A gated task can sit queued for frames, so wait on "nothing was
		// runnable". The generation re-check makes a lost wake impossible.
		if (asset_thread_enabled && worked == false && atomic_load_i32(&assets_blocking_count) == 0) {
			ft_mutex_lock(asset_thread_wait_mtx);
			if (atomic_load_i32(&assets_wake_gen) == wake_gen)
				ft_condition_wait(asset_tasks_available, asset_thread_wait_mtx);
			ft_mutex_unlock(asset_thread_wait_mtx);
		}
	}

	skr_thread_shutdown();

	thread->running = false;
	return 0;
}

///////////////////////////////////////////

bool assets_on_asset_thread() {
	ft_id_t curr_id = ft_id_current();
	for (int32_t i = 0; i < asset_threads.count; i++) {
		if (ft_id_equal(curr_id, asset_threads[i].id))
			return true;
	}
	return false;
}

///////////////////////////////////////////

void assets_block_until(asset_t asset, asset_state_ state) {
	asset_header_t *header = (asset_header_t *)asset;
	// If we're past the required state already, drop out. asset_state_none and
	// below (error states) means no loading is happening, so blocking will
	// only put us in an infinite loop.
	if (header->state >= state || header->state <= asset_state_none)
		return;

	profiler_zone();

	if (assets_on_asset_thread()) {
		log_err("assets_block_ should not be called on the assets thread!");
		return;
	}

	while (header->state < state && header->state >= 0) {
		// Spin the GPU thread so the asset thread doesn't freeze up while
		// we're waiting on it.
		assets_step();

#if defined(SK_OS_WEB)
		// In a browser, parked work (a fetch, a GPU readback) only resolves
		// after this stack unwinds, so once that's all that remains, spinning
		// here can never finish. Bail loudly instead of hanging the tab.
		if (assets_step_did_work == false && header->state < state && header->state >= 0) {
			log_errf("Can't block for asset '%s' on the web, it's waiting on work that needs the frame to end. Load asynchronously instead.", header->id_text ? header->id_text : "[unnamed]");
			return;
		}
#endif
	}
}

///////////////////////////////////////////

void assets_block_for_priority(int32_t priority) {
	profiler_zone();

	if (assets_on_asset_thread()) {
		log_err("assets_block_ should not be called on the assets thread!");
		return;
	}

	// This handles if the user passes in INT_MAX
	int32_t curr_priority = assets_current_task_priority();
	while (curr_priority <= priority && curr_priority != INT_MAX) {
		// Spin the GPU thread so the asset thread doesn't freeze up while
		// we're waiting on it.
		assets_step();
		curr_priority = assets_current_task_priority();

#if defined(SK_OS_WEB)
		// See assets_block_until: parked work can't finish inside one frame
		// in a browser, so a pass that steps nothing never will.
		if (assets_step_did_work == false && curr_priority <= priority && curr_priority != INT_MAX) {
			log_err("Can't block for assets on the web, they're waiting on work that needs the frame to end. Load asynchronously instead.");
			return;
		}
#endif
	}

	// The last task queues its on_load event after the final step above, so
	// without this, blocking returns with the callback still pending.
	assets_step();
}

} // namespace sk
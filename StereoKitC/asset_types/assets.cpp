#include "assets.h"
#include "../_stereokit.h"
#include "../sk_memory.h"

#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "material.h"
#include "model.h"
#include "font.h"
#include "sprite.h"
#include "sound.h"
#include "../systems/physics.h"
#include "../libraries/stref.h"
#include "../libraries/ferr_hash.h"
#include "../libraries/array.h"
#include "../libraries/tinycthread.h"
#include "../libraries/sokol_time.h"

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
	thrd_id_t id;
	bool32_t  running;
};

///////////////////////////////////////////

array_t<asset_header_t *>      assets = {};
array_t<asset_header_t *>      assets_multithread_destroy = {};
mtx_t                          assets_multithread_destroy_lock;
thrd_id_t                      assets_gpu_thread = {};
mtx_t                          assets_job_lock;
array_t<asset_job_t *>         assets_gpu_jobs = {};
mtx_t                          assets_load_event_lock;
array_t<asset_load_callback_t> assets_load_callbacks = {};
array_t<asset_header_t *>      assets_load_events = {};

///////////////////////////////////////////

array_t<asset_thread_t>asset_threads         = {};
bool32_t               asset_thread_enabled  = false;
array_t<asset_task_t*> asset_thread_tasks    = {};
mtx_t                  asset_thread_task_mtx = {};
int32_t                asset_tasks_finished  = 0;

int32_t asset_thread   (void *);
void    asset_step_task();

///////////////////////////////////////////

void *assets_find(const char *id, asset_type_ type) {
	return assets_find(hash_fnv64_string(id), type);
}

///////////////////////////////////////////

void *assets_find(uint64_t id, asset_type_ type) {
	int32_t count = assets.count;
	for (int32_t i = 0; i < count; i++) {
		if (assets[i]->id == id && assets[i]->type == type && assets[i]->refs > 0)
			return assets[i];
	}
	return nullptr;
}

///////////////////////////////////////////

void assets_unique_name(asset_type_ type, const char *root_name, char *dest, int dest_size) {
	snprintf(dest, dest_size, "%s", root_name);
	uint64_t id    = hash_fnv64_string(dest);
	int      count = 1;
	while (assets_find(dest, type) != nullptr) {
		snprintf(dest, dest_size, "%s%d", root_name, count);
		id = hash_fnv64_string(dest);
		count += 1;
	}
}

///////////////////////////////////////////

void *assets_allocate(asset_type_ type) {
	size_t size = sizeof(asset_header_t);
	switch(type) {
	case asset_type_mesh:     size = sizeof(_mesh_t );    break;
	case asset_type_texture:  size = sizeof(_tex_t);      break;
	case asset_type_shader:   size = sizeof(_shader_t);   break;
	case asset_type_material: size = sizeof(_material_t); break;
	case asset_type_model:    size = sizeof(_model_t);    break;
	case asset_type_font:     size = sizeof(_font_t);     break;
	case asset_type_sprite:   size = sizeof(_sprite_t);   break;
	case asset_type_sound:    size = sizeof(_sound_t);    break;
	case asset_type_solid:    size = sizeof(_solid_t);    break;
	default: log_err("Unimplemented asset type!"); abort();
	}

	char name[64];
	snprintf(name, sizeof(name), "auto/asset_%d", assets.count);

	asset_header_t *header = (asset_header_t *)sk_malloc(size);
	memset(header, 0, size);
	header->type  = type;
	header->id    = hash_fnv64_string(name);
	header->index = assets.count;
	header->state = asset_state_none;
	assets_addref(header);
	assets.add(header);
	return header;
}

///////////////////////////////////////////

void assets_set_id(asset_header_t *header, const char *id) {
	assets_set_id(header, hash_fnv64_string(id));
#if defined(SK_DEBUG)
	header->id_text = string_copy(id);
#endif
}

///////////////////////////////////////////

void assets_set_id(asset_header_t *header, uint64_t id) {
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
	asset->refs += 1;
}

///////////////////////////////////////////

void assets_releaseref(asset_header_t *asset) {
	// Manage the reference count
	asset->refs -= 1;
	if (asset->refs < 0) {
		log_err("Released too many references to asset!");
		abort();
	}
	if (asset->refs != 0)
		return;

	assets_destroy(asset);
}

///////////////////////////////////////////

void assets_releaseref_threadsafe(void *asset) {
	asset_header_t *asset_header = (asset_header_t *)asset;

	// Manage the reference count
	asset_header->refs -= 1;
	if (asset_header->refs < 0) {
		log_err("Released too many references to asset!");
		abort();
	}
	if (asset_header->refs != 0)
		return;

	mtx_lock(&assets_multithread_destroy_lock);
	assets_multithread_destroy.add(asset_header);
	mtx_unlock(&assets_multithread_destroy_lock);
}

///////////////////////////////////////////

void assets_destroy(asset_header_t *asset) {
	if (asset->refs != 0) {
#if defined(SK_DEBUG)
		log_errf("Destroying asset '%s' that still has references!", asset->id_text);
#else
		log_err("Destroying an asset that still has references!");
#endif
		return;
	}

	// Call asset specific destroy function
	switch(asset->type) {
	case asset_type_mesh:     mesh_destroy    ((mesh_t    )asset); break;
	case asset_type_texture:  tex_destroy     ((tex_t     )asset); break;
	case asset_type_shader:   shader_destroy  ((shader_t  )asset); break;
	case asset_type_material: material_destroy((material_t)asset); break;
	case asset_type_model:    model_destroy   ((model_t   )asset); break;
	case asset_type_font:     font_destroy    ((font_t    )asset); break;
	case asset_type_sprite:   sprite_destroy  ((sprite_t  )asset); break;
	case asset_type_sound:    sound_destroy   ((sound_t   )asset); break;
	case asset_type_solid:    solid_destroy   ((solid_t   )asset); break;
	default: log_err("Unimplemented asset type!"); abort();
	}

	// Remove it from our list of assets
	for (int32_t i = 0; i < assets.count; i++) {
		if (assets[i] == asset) {
			assets.remove(i);
			break;
		}
	}

	// And at last, free the memory we allocated for it!
#if defined(SK_DEBUG)
	sk_free(asset->id_text);
#endif
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
			case asset_type_texture:  type_name = "tex_t";      break;
			case asset_type_shader:   type_name = "shader_t";   break;
			case asset_type_material: type_name = "material_t"; break;
			case asset_type_model:    type_name = "model_t";    break;
			case asset_type_font:     type_name = "font_t";     break;
			case asset_type_sprite:   type_name = "sprite_t";   break;
			case asset_type_sound:    type_name = "sound_t";    break;
			case asset_type_solid:    type_name = "solid_t";    break;
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

char assets_file_buffer[1024];
const char *assets_file(const char *file_name) {
	if (file_name == nullptr || sk_settings.assets_folder == nullptr || sk_settings.assets_folder[0] == '\0')
		return file_name;

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	const char *ch = file_name;
	while (*ch != '\0') {
		if (*ch == ':') {
			return file_name;
		}
		ch++;
	}
#elif defined(SK_OS_ANDROID)
	return file_name;
#else
	if (file_name[0] == platform_path_separator_c)
		return file_name;
#endif

	snprintf(assets_file_buffer, sizeof(assets_file_buffer), "%s/%s", sk_settings.assets_folder, file_name);
	return assets_file_buffer;
}

///////////////////////////////////////////

bool assets_init() {
	assets_gpu_thread = thrd_id_current();
	mtx_init(&assets_multithread_destroy_lock, mtx_plain);
	mtx_init(&assets_job_lock,                 mtx_plain);
	mtx_init(&asset_thread_task_mtx,           mtx_plain);
	mtx_init(&assets_load_event_lock,          mtx_plain);

#if !defined(__EMSCRIPTEN__)
	asset_threads.resize(3);
#endif
	asset_thread_enabled = true;
	for (int32_t i = 0; i < asset_threads.capacity; i++)
	{
		asset_threads.add({});
		asset_thread_t* th = &asset_threads.last();
		thrd_t thread = {};
		thrd_create(&thread, asset_thread, th);
	}

	return true;
}

///////////////////////////////////////////

array_t<asset_load_callback_t> assets_load_call_list = {};
void assets_update() {
	// If we have no asset threads for some reason (like WASM), then we'll need
	// to make sure assets still get loaded here!
	if (asset_threads.count <= 0) {
		asset_step_task();
	}

	// destroy objects where the request came from another thread
	mtx_lock(&assets_multithread_destroy_lock);
	for (int32_t i = 0; i < assets_multithread_destroy.count; i++) {
		assets_destroy(assets_multithread_destroy[i]);
	}
	assets_multithread_destroy.clear();
	mtx_unlock(&assets_multithread_destroy_lock);

	// Do any jobs the assets need on the main thread, like GPU buffer uploads
	mtx_lock(&assets_job_lock);
	for (int32_t i = 0; i < assets_gpu_jobs.count; i++) {
		assets_gpu_jobs[i]->success  = assets_gpu_jobs[i]->asset_job(assets_gpu_jobs[i]->data);
		assets_gpu_jobs[i]->finished = true;
	}
	assets_gpu_jobs.clear();
	mtx_unlock(&assets_job_lock);

	// Update any on_load event callbacks
	mtx_lock(&assets_load_event_lock);
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
	mtx_unlock(&assets_load_event_lock);
	assets_load_call_list.each([](const asset_load_callback_t &c) { c.on_load(c.asset, c.context); });
	assets_load_call_list.clear();
}

///////////////////////////////////////////

void assets_shutdown() {
	asset_thread_enabled = false;
	for (int32_t i = 0; i < asset_threads.count; i++) {
		while (asset_threads[i].running) {
			assets_update();
			thrd_yield();
		}
	}
	asset_threads.free();

	mtx_destroy(&asset_thread_task_mtx);
	asset_thread_tasks.free();

	assets_multithread_destroy.free();
	assets_gpu_jobs           .free();
	mtx_destroy(&assets_multithread_destroy_lock);
	mtx_destroy(&assets_job_lock);
	mtx_destroy(&assets_load_event_lock);

	assets_load_call_list.free();
	assets_load_callbacks.free();
	assets_load_events   .free();
}

///////////////////////////////////////////

bool32_t assets_execute_gpu(bool32_t(*asset_job)(void *data), void *data) {
	if (thrd_id_equal(thrd_id_current(), assets_gpu_thread)) {
		return asset_job(data);
	} else {
		asset_job_t *job = sk_malloc_t(asset_job_t, 1);
		*job = {};
		job->asset_job = asset_job;
		job->data      = data;

		mtx_lock(&assets_job_lock);
		assets_gpu_jobs.add(job);
		mtx_unlock(&assets_job_lock);

		// Block until the GPU thread has had a chance to take care of the job.
		uint64_t start      = stm_now();
		bool     has_warned = false;
		while (job->finished == false) {
			thrd_yield();

			// if the app hasn't started stepping yet and this takes too long,
			// the application may be off the gpu thread unintentionally.
			if (sk_first_step == false && has_warned == false && stm_ms(stm_since(start)) > 4000) {
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
	return asset_thread_tasks.count + asset_tasks_finished;
}

///////////////////////////////////////////

int32_t assets_current_task_priority() {
	if (asset_thread_tasks.count > 0)
		return (int32_t)asset_thread_tasks[0]->sort;
	return INT_MAX;
}

///////////////////////////////////////////
// Asset thread                          //
///////////////////////////////////////////

void assets_add_task(asset_task_t src_task) {
	asset_task_t *task = sk_malloc_t(asset_task_t, 1);
	memcpy(task, &src_task, sizeof(asset_task_t));
	assets_addref(task->asset);

	mtx_lock(&asset_thread_task_mtx);

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
	mtx_unlock(&asset_thread_task_mtx);
}

///////////////////////////////////////////

void assets_task_set_complexity(asset_task_t *task, int32_t complexity) {
}

///////////////////////////////////////////

void asset_step_task() {
	// Pop out the task we want to work on
	mtx_lock(&asset_thread_task_mtx);
	if (asset_thread_tasks.count <= 0) { mtx_unlock(&asset_thread_task_mtx); return; }
	asset_task_t* task = asset_thread_tasks[0];
	asset_thread_tasks.remove(0);
	mtx_unlock(&asset_thread_task_mtx);

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
			mtx_lock(&assets_job_lock);
			assets_gpu_jobs.add(&task->gpu_job);
			mtx_unlock(&assets_job_lock);
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
		mtx_lock(&asset_thread_task_mtx);
		asset_thread_tasks.insert(0, task);
		mtx_unlock(&asset_thread_task_mtx);
	} else {
		// Or skip putting it back if it's complete :)
		asset_tasks_finished += 1;

		// If it was successfully loaded, we'll want to notify on_load, but we
		// do want to skip this if it was removed because of an issue during
		// load.
		if (task->asset->state >= asset_state_loaded) {
			mtx_lock(&assets_load_event_lock);
			assets_load_events.add(task->asset);
			mtx_unlock(&assets_load_event_lock);
		}

		if (task->free_data != nullptr) task->free_data(task->asset, task->load_data);
		assets_releaseref_threadsafe(task->asset);
		sk_free(task);
	}
}

///////////////////////////////////////////

int32_t asset_thread(void *thread_inst_obj) {
	asset_thread_t* thread = (asset_thread_t*)thread_inst_obj;
	thread->id      = thrd_id_current();
	thread->running = true;

	while (asset_thread_enabled || asset_thread_tasks.count>0) {
		asset_step_task();
		thrd_yield();
	}

	thread->running = false;
	return 0;
}

///////////////////////////////////////////

void assets_block_until(asset_header_t *asset, asset_state_ state) {
	if (asset->state >= state || asset->state < 0)
		return;

	thrd_id_t curr_id = thrd_id_current();
	for (int32_t i = 0; i < asset_threads.count; i++)
	{
		if (thrd_id_equal(curr_id, asset_threads[i].id)) {
			log_err("assets_block_ should not be called on the assets thread!");
			return;
		}
	}

	while (asset->state < state && asset->state >= 0) {
		// Spin the GPU thread so the asset thread doesn't freeze up while
		// we're waiting on it.
		assets_update();
		thrd_yield();
	}
}

///////////////////////////////////////////

void assets_block_for_priority(int32_t priority) {
	thrd_id_t curr_id = thrd_id_current();
	for (int32_t i = 0; i < asset_threads.count; i++)
	{
		if (thrd_id_equal(curr_id, asset_threads[i].id)) {
			log_err("assets_block_ should not be called on the assets thread!");
			return;
		}
	}
	

	// This handles if the user passes in INT_MAX
	int32_t curr_priority = assets_current_task_priority();
	while (curr_priority <= priority && curr_priority != INT_MAX) {
		// Spin the GPU thread so the asset thread doesn't freeze up while
		// we're waiting on it.
		assets_update();
		thrd_yield();
		curr_priority = assets_current_task_priority();
	}
}

} // namespace sk
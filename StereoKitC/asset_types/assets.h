#pragma once

#include "../platforms/platform.h" // SK_DEBUG
#include "../libraries/ferr_thread.h"
#include <stdint.h>

namespace sk {

struct asset_header_t {
	asset_type_  type;
	asset_state_ state;
	id_hash_t    id;
	uint64_t     index;
	int32_t      refs;
	char        *id_text;
};

struct asset_job_t {
	bool32_t  finished;
	bool32_t  success;
	void     *data;
	bool32_t(*asset_job)(void *data);
};

typedef enum asset_find_ {
	asset_find_found,
	asset_find_created,
} asset_find_;

struct asset_task_t;

typedef enum asset_action_result_ {
	asset_action_fail = 0,
	asset_action_done,      // advance to the next action
	asset_action_continue,  // more work right now, call again; a cooperative slice
	asset_action_wait,      // parked until assets_task_signal re-queues it
} asset_action_result_;

// Where an action may run. heavy marks unsliced work that can take a long
// time, so the budgeted main-thread step only starts it on a fresh budget.
// main is for work that has to run on the main thread, like browser API
// calls or deferred GPU completions. main actions only progress while the
// main thread steps, so a foreign thread that blocks on one while also
// holding the main thread up deadlocks itself.
typedef enum asset_affinity_ {
	asset_affinity_any = 0,
	asset_affinity_heavy,
	asset_affinity_main,
} asset_affinity_;

typedef asset_action_result_ (*asset_load_action_t)(asset_task_t *task, asset_header_t *asset, void *data);

struct asset_action_t {
	asset_load_action_t fn;
	asset_affinity_     affinity;
};

struct asset_task_t {
	asset_header_t      *asset;
	void                *load_data;
	void               (*free_data )(asset_header_t *asset, void *data);
	void               (*on_failure)(asset_header_t *asset, void *data);
	asset_action_t      *actions;
	int32_t              action_count;
	int32_t              action_curr;
	int32_t              priority;
	int64_t              sort;
	asset_header_t      *depends_on;       // optional: task is gated until this
	asset_state_         depends_state;    // asset reaches depends_state, or fails
	bool32_t             dep_failed;       // stamped at acquire when depends_on failed
	uint64_t             wait_id;          // nonzero from wait_prepare until the signal lands
	bool32_t             signal_beat_park; // ...so assets_park_task re-queues instead of parking
	ft_id_t              running_thread;   // stamped at acquire, backs assets_task_signal's contract check
};

void*       assets_find               (const char* id, asset_type_ type);
void*       assets_find               (id_hash_t   id, asset_type_ type);
asset_find_ assets_find_or_create     (const char* id, asset_type_ type, void** out_asset);
void*       assets_allocate           (asset_type_ type);
void        assets_destroy            (asset_header_t *asset);
void        assets_set_id             (asset_header_t *header, const char *id);
void        assets_set_id             (asset_header_t *header, uint64_t    id);
void        assets_unique_name        (asset_type_ type, const char *root_name, char *dest, int dest_size);
void        assets_addref             (asset_header_t *asset);
void        assets_releaseref         (asset_header_t *asset);
void        assets_safeswap_ref       (asset_header_t **asset_link, asset_header_t *asset);
void        assets_shutdown_check     ();
char*       assets_file               (const char *file_name);
bool        assets_init               ();
void        assets_step               ();
void        assets_shutdown           ();
void        assets_on_load            (asset_header_t *asset, void (*on_load)(asset_header_t *asset, void *context), void *context);
void        assets_on_load_remove     (asset_header_t *asset, void (*on_load)(asset_header_t *asset, void *context), void *context);
void        assets_on_load_remove_all (asset_header_t *asset);
// For work that finishes an asset outside the task system.
void        assets_notify_loaded      (asset_header_t *asset);
// SK_API so the SKTests harness can reach it from the shared library.
SK_API bool assets_on_asset_thread    ();

// An action that starts external work (a fetch, a GPU readback) calls
// wait_prepare, hands the id to whatever completion will fire, and returns
// asset_action_wait. The signal re-queues the task and runs the same action
// again; a stale id returns false, telling the completion its result has no
// home. deliver runs with the task's load_data under the scheduler's lock, so
// keep it to short copies, and may only signal from the initiating action's
// own stack or from the main thread while parked. Other threads would race
// the running action. SK_API for the SKTests harness, as above.
SK_API uint64_t assets_task_wait_prepare(asset_task_t *task);
SK_API bool32_t assets_task_signal      (uint64_t wait_id, void (*deliver)(asset_task_t *task, void *load_data, void *context) = nullptr, void *context = nullptr);

typedef enum asset_read_ {
	asset_read_in_flight = 0, // also how a request that hasn't started reads
	asset_read_arrived,
	asset_read_failed,
} asset_read_;

// One asynchronous file read, embedded zero-initialized in a task's load
// data. On arrived, `data` is the caller's to take (and to free through the
// task's free_data if the task dies first).
struct asset_file_read_t {
	bool32_t    pending; // a request is out
	asset_read_ state;
	void*       data;
	size_t      size;
};

// The wait/signal plumbing for reading a file from a task action: starts the
// read on the first call, and the caller parks on in_flight. The arrival
// signal re-runs the action, which lands back here and sees the result.
asset_read_ assets_task_read_file(asset_task_t *task, const char *filename, asset_file_read_t *read);

// Runs the job inline on skr-initialized threads (main, asset). Foreign
// threads queue the job to the asset threads and block until it finishes.
bool32_t    assets_execute_blocking   (bool32_t (*asset_job)(void *data), void *data);
// SK_API so the SKTests harness can drive the scheduler with synthetic tasks.
SK_API void assets_add_task           (asset_task_t task);
inline int64_t asset_sort(int32_t priority, int32_t complexity) { return ((int64_t)priority << 32) | ((int64_t)complexity); }

// The priority the public tex/model/font create functions default to.
static const int32_t asset_priority_default = 10;

// Converts a byte size into the `complexity` metric used by asset_sort. The
// metric is kilobytes (rounded down), which keeps the sort within int32_t
// range even for extreme inputs and gives the scheduler a usable magnitude —
// sub-KB distinctions don't meaningfully affect load order.
inline int32_t asset_complexity_bytes(size_t bytes) {
	size_t kb = bytes >> 10;
	return kb > (size_t)INT32_MAX ? INT32_MAX : (int32_t)kb;
}

} // namespace sk
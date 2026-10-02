#pragma once

#include "../platforms/platform.h" // SK_DEBUG
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

typedef bool32_t (*asset_load_action_t)(asset_task_t *task, asset_header_t *asset, void *data);

struct asset_task_t {
	asset_header_t      *asset;
	void                *load_data;
	void               (*free_data )(asset_header_t *asset, void *data);
	void               (*on_failure)(asset_header_t *asset, void *data);
	asset_load_action_t *actions;
	int32_t              action_count;
	int32_t              action_curr;
	int32_t              priority;
	int64_t              sort;
	asset_header_t      *depends_on;    // optional: task is gated until this
	asset_state_         depends_state; // asset reaches depends_state, or fails
	bool32_t             dep_failed;    // stamped at acquire when depends_on failed
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
void        assets_on_load_remove     (asset_header_t *asset, void (*on_load)(asset_header_t *asset, void *context));
void        assets_on_load_remove_all (asset_header_t *asset);

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
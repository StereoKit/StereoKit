#pragma once

#include "../platforms/platform_utils.h" // SK_DEBUG
#include <stdint.h>

namespace sk {

struct asset_header_t {
	asset_type_  type;
	asset_state_ state;
	uint64_t     id;
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

typedef enum asset_thread_ {
	asset_thread_asset,
	asset_thread_gpu,
} asset_thread_;

struct asset_task_t;

struct asset_load_action_t {
	bool32_t    (*action)(asset_task_t *task, asset_header_t *asset, void *data);
	asset_thread_ thread_affinity;
};

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
	asset_job_t          gpu_job;
	bool32_t             gpu_started;
};

void *assets_find          (const char *id, asset_type_ type);
void *assets_find          (uint64_t    id, asset_type_ type);
void *assets_allocate      (asset_type_ type);
void  assets_destroy       (asset_header_t *asset);
void  assets_set_id        (asset_header_t *header, const char *id);
void  assets_set_id        (asset_header_t *header, uint64_t    id);
void  assets_unique_name   (asset_type_ type, const char *root_name, char *dest, int dest_size);
void  assets_addref        (asset_header_t *asset);
void  assets_releaseref    (asset_header_t *asset);
void  assets_safeswap_ref  (asset_header_t **asset_link, asset_header_t *asset);
void  assets_shutdown_check();
char *assets_file          (const char *file_name);
bool  assets_init          ();
void  assets_update        ();
void  assets_shutdown      ();
void  assets_on_load       (asset_header_t *asset, void (*on_load)(asset_header_t *asset, void *context), void *context);
void  assets_on_load_remove(asset_header_t *asset, void (*on_load)(asset_header_t *asset, void *context));

// This function will block execution until `asset_job` is finished, but will
// ensure it is run on the GPU thread.
bool32_t assets_execute_gpu        (bool32_t (*asset_job)(void *data), void *data);
void     assets_add_task           (asset_task_t task);
void     assets_task_set_complexity(asset_task_t *task, int32_t priority);
void     assets_block_until        (asset_header_t *asset, asset_state_ state);

inline int64_t asset_sort(int32_t priority, int32_t complexity) { return ((int64_t)priority << 32) | ((int64_t)complexity); }

} // namespace sk
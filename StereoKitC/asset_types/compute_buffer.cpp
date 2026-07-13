#include "../stereokit.h"
#include "../sk_memory.h"
#include "../libraries/stref.h"
#include "compute_buffer.h"
#include "assets.h"

namespace sk {

///////////////////////////////////////////

compute_buffer_t compute_buffer_create(compute_buffer_type_ type, int32_t element_count, int32_t element_size, const void *initial_data) {
	skr_use_ use;
	switch (type) {
		case compute_buffer_type_read:      use = (skr_use_)(skr_use_dynamic | skr_use_compute_read);      break;
		case compute_buffer_type_readwrite: use = (skr_use_)(skr_use_dynamic | skr_use_compute_readwrite); break;
		default:
			log_err("compute_buffer_create: invalid buffer type");
			return nullptr;
	}

	compute_buffer_t result = (compute_buffer_t)assets_allocate(asset_type_compute_buffer);
	result->type          = type;
	result->element_count = element_count;
	result->element_size  = element_size;

	if (skr_buffer_create(initial_data, (uint32_t)element_count, (uint32_t)element_size, skr_buffer_type_storage, use, &result->gpu_buffer) != skr_err_success) {
		log_err("compute_buffer_create: failed to create GPU buffer");
		assets_releaseref(&result->header);
		return nullptr;
	}

	return result;
}

///////////////////////////////////////////

void compute_buffer_set_id(compute_buffer_t buffer, const char *id) {
	assets_set_id(&buffer->header, id);
}

///////////////////////////////////////////

const char* compute_buffer_get_id(const compute_buffer_t buffer) {
	return buffer->header.id_text;
}

///////////////////////////////////////////

void compute_buffer_set_data(compute_buffer_t buffer, const void *data, int32_t element_count) {
	if (element_count > buffer->element_count) {
		log_warnf("compute_buffer_set_data: element_count %d exceeds buffer capacity %d, clamping", element_count, buffer->element_count);
		element_count = buffer->element_count;
	}
	skr_buffer_set(&buffer->gpu_buffer, data, (uint32_t)(element_count * buffer->element_size));
}

///////////////////////////////////////////

void compute_buffer_get_data(compute_buffer_t buffer, void *out_data, int32_t element_count) {
	if (element_count > buffer->element_count) {
		log_warnf("compute_buffer_get_data: element_count %d exceeds buffer capacity %d, clamping", element_count, buffer->element_count);
		element_count = buffer->element_count;
	}
	skr_buffer_get(&buffer->gpu_buffer, out_data, (uint32_t)(element_count * buffer->element_size));
}

///////////////////////////////////////////

int32_t compute_buffer_get_count(const compute_buffer_t buffer) {
	return buffer->element_count;
}

///////////////////////////////////////////

int32_t compute_buffer_get_stride(const compute_buffer_t buffer) {
	return buffer->element_size;
}

///////////////////////////////////////////

void compute_buffer_addref(compute_buffer_t buffer) {
	assets_addref(&buffer->header);
}

///////////////////////////////////////////

void compute_buffer_release(compute_buffer_t buffer) {
	if (buffer == nullptr)
		return;
	assets_releaseref(&buffer->header);
}

///////////////////////////////////////////

void compute_buffer_destroy(compute_buffer_t buffer) {
	skr_buffer_destroy(&buffer->gpu_buffer);
	*buffer = {};
}

} // namespace sk

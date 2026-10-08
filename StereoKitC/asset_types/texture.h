#pragma once

#include <sk_renderer.h>

#include "../stereokit.h"
#include "assets.h"

namespace sk {

// Internal only, an attachment resolved in-pass and never read. Drops every
// usage bit but COLOR_ATTACHMENT, and reading one is a validation error.
const tex_type_ tex_type_transient_internal = (tex_type_)(1 << 30);
// Internal only, an attachment that's never sampled, so it stays in its
// ATTACHMENT_OPTIMAL layout between passes. XR swapchain images need this.
const tex_type_ tex_type_attachment_internal = (tex_type_)(1 << 29);

struct tex_upload_t;

struct _tex_t {
	asset_header_t   header;
	tex_t            fallback;
	bool32_t         owned;

	// Metadata fields - kept for quick CPU access
	// TODO: can get rid of some maybe?
	int32_t          width;
	int32_t          height;
	int32_t          depth;
	tex_format_      format;
	uint64_t         meta_hash;
	tex_format_      data_format;     // what uploads arrive as, `format` may be its compressed form
	tex_upload_t*    upload_pending;  // atomic, newest upload not yet started
	int32_t          upload_queued;   // atomic, 1 while a task for upload_pending is queued
	int32_t          upload_priority; // atomic, priority of upload_pending
	int32_t          write_running;   // atomic, 1 while a thread writes gpu_tex, see tex_write_begin
	int32_t          write_issued;    // atomic, number of the newest write requested
	int32_t          write_landed;    // atomic, number of the newest write landed, set under write_running

	tex_type_        type;
	tex_sample_      sample_mode;
	tex_sample_comp_ sample_comp;
	tex_address_     address_mode;
	int32_t          anisotropy;
	skr_tex_t        gpu_tex;
	tex_t            depth_buffer;
	spherical_harmonics_t *light_info; // owned lighting cache, texture.cpp only
	skr_buffer_t     sh_buffer;  // SH projection result, GPU side
	skr_future_t     sh_future;  // completes when sh_buffer is readable
	bool32_t         sh_pending; // readback pending, resolved lazily on query
	bool32_t         sh_dirty;   // content changed; queries kick a refresh
};

void tex_destroy       (tex_t texture);
// GPU writes bypass the upload path, so systems that render into a texture
// call this to flag a cubemap's cached lighting for a background refresh.
void tex_lighting_dirty(tex_t texture);

} // namespace sk

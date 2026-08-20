// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#include "web.h"
#if defined(SK_OS_WEB)

#include <emscripten.h>
#include <string.h>
#include <stdlib.h>

#include "../asset_types/assets.h"
#include "../asset_types/font.h"
#include "../sk_memory.h"

namespace sk {

///////////////////////////////////////////

bool platform_impl_init() {
	return true;
}

///////////////////////////////////////////

void platform_impl_shutdown() {
}

///////////////////////////////////////////

void platform_impl_step() {
}

///////////////////////////////////////////

font_t platform_default_font() {
	// There are no font files to enumerate in a wasm filesystem, so the bundled
	// Aileron is the platform default here.
	return font_create_family("builtin");
}

///////////////////////////////////////////

struct web_fetch_t {
	platform_read_callback_t callback;
	void                    *context;
};

void platform_read_file_async(const char *filename, platform_read_callback_t callback, void *context) {
	// Files the page preloaded resolve right away
	void*  data = nullptr;
	size_t size = 0;
	if (platform_read_file(filename, &data, &size)) {
		callback(true, data, size, context);
		return;
	}

	// Everything else streams in; the asset path doubles as its URL, both are
	// rooted at the folder the page serves from. The formats loaded this way
	// are mirrored by skStreamedAsset in web/stereokit.js, keep them in step.
	char*        url   = assets_file(filename);
	web_fetch_t* fetch = sk_malloc_t(web_fetch_t, 1);
	fetch->callback = callback;
	fetch->context  = context;
	emscripten_async_wget_data(url, fetch,
		[](void* arg, void* data, int size) {
			// emscripten frees its buffer after this returns, so copy it out
			web_fetch_t* fetch = (web_fetch_t*)arg;
			void*        copy  = sk_malloc((size_t)size);
			memcpy(copy, data, (size_t)size);
			fetch->callback(true, copy, (size_t)size, fetch->context);
			sk_free(fetch);
		},
		[](void* arg) {
			web_fetch_t* fetch = (web_fetch_t*)arg;
			fetch->callback(false, nullptr, 0, fetch->context);
			sk_free(fetch);
		});
	sk_free(url);
}

///////////////////////////////////////////

// The canvas round-trip premultiplies and un-premultiplies alpha, which can
// round translucent pixels by a bit.
EM_JS(void, web_image_decode_js, (double id, const void* bytes, int size), {
	if (!Module.skImageQueue) Module.skImageQueue = [];
	// The Blob snapshots the bytes now, so wasm can free them right after
	const blob = new Blob([HEAPU8.subarray(bytes, bytes + size)]);
	createImageBitmap(blob, { premultiplyAlpha: 'none', colorSpaceConversion: 'none' })
		.then((bmp) => {
			const canvas = new OffscreenCanvas(bmp.width, bmp.height);
			const ctx    = canvas.getContext('2d', { willReadFrequently: true });
			ctx.drawImage(bmp, 0, 0);
			const img = ctx.getImageData(0, 0, bmp.width, bmp.height);
			bmp.close();
			const ptr = _malloc(img.data.length);
			HEAPU8.set(img.data, ptr);
			Module.skImageQueue.push({ id: id, ptr: ptr, w: img.width, h: img.height });
		})
		.catch((e) => {
			console.error('[sk] browser image decode failed: ' + e);
			Module.skImageQueue.push({ id: id, ptr: 0, w: 0, h: 0 });
		});
});

void web_image_decode_begin(uint64_t id, const void *bytes, size_t size) {
	// Wait ids come from a 32 bit counter, so the double is exact
	web_image_decode_js((double)id, bytes, (int)size);
}

bool web_image_decode_poll(uint64_t *out_id, void **out_rgba, int32_t *out_width, int32_t *out_height) {
	double id = EM_ASM_DOUBLE({ const q = Module.skImageQueue; return q && q.length ? q[0].id : -1; });
	if (id < 0) return false;
	// The pointer rides a double; EM_ASM_INT would sign-truncate past 2GB
	double  ptr = EM_ASM_DOUBLE({ return Module.skImageQueue[0].ptr; });
	int32_t w   = EM_ASM_INT   ({ return Module.skImageQueue[0].w;   });
	int32_t h   = EM_ASM_INT   ({ return Module.skImageQueue[0].h;   });
	EM_ASM({ Module.skImageQueue.shift(); });

	*out_id     = (uint64_t)id;
	*out_width  = w;
	*out_height = h;
	if (ptr == 0) { *out_rgba = nullptr; return true; }

	// Copy over to the sk allocator; the JS side had only plain malloc.
	size_t size = (size_t)w * h * 4;
	*out_rgba = sk_malloc(size);
	memcpy(*out_rgba, (void*)(uintptr_t)ptr, size);
	free((void*)(uintptr_t)ptr);
	return true;
}

///////////////////////////////////////////

void platform_msgbox_err(const char *text, const char *header) {
	EM_ASM({ alert(UTF8ToString($0) + "\n\n" + UTF8ToString($1)); }, header, text);
}

///////////////////////////////////////////

void platform_print_callstack() {
	emscripten_run_script("console.trace()");
}

}

#endif // defined(SK_OS_WEB)

#include "../stereokit.h"
#include <sk_renderer.h>
#include <stdint.h>
#include <stdbool.h>

namespace sk {

void texture_compression_init        ();
// The encoders live and die with skr, so these pair with skr_init/shutdown.
void texture_compression_gpu_init    ();
void texture_compression_gpu_shutdown();

bool ktx2_decode(void* data, size_t data_size, tex_type_* ref_image_type, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count, void** out_data);
bool ktx2_info  (void* data, size_t data_size, tex_type_* ref_image_type, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count);
bool ktx2_sniff (const void* data, size_t data_size);

// ktx2_decode split into a begin/step pair, so a loader can pace the
// transcode one mip level per cooperative slice. begin allocates the whole
// output into *out_data, which is the caller's to free either way; end
// releases the handle from any point. `data` must outlive the handle.
struct ktx2_slice_t;
ktx2_slice_t* ktx2_decode_begin(void* data, size_t data_size, tex_type_* ref_image_type, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count, void** out_data);
bool          ktx2_decode_step (ktx2_slice_t* slice, int32_t* out_level, bool* out_done);
void          ktx2_decode_end  (ktx2_slice_t* slice);

// Encoder families this device can both produce and sample.
struct tex_compress_caps_t {
	bool32_t astc;     // ASTC LDR, 4x4 and 6x6
	bool32_t astc_hdr; // ASTC HDR, 8x8
	bool32_t bc;       // BC1, BC7, and BC6H
};

SK_API tex_compress_caps_t tex_compress_caps  ();
// Encodes every mip the source has, adding none. Invalid on failure.
       skr_tex_t           tex_compress_gpu   (skr_tex_t* source, tex_format_ format, skr_tex_sampler_t sampler);
// The format a loaded image should end up in, or tex_format_none to keep the
// source. SK_API here and on caps is for the SKTests harness.
SK_API tex_format_         tex_compress_pick  (tex_data_ flags, tex_data_ default_policy, tex_type_ type, tex_format_ src_format, bool32_t src_alpha, int32_t width, int32_t height, int32_t array_count, tex_compress_caps_t caps);
// Whether an image file can carry alpha, from its header alone.
SK_API bool32_t            tex_image_has_alpha(const void* data, size_t data_size);

}

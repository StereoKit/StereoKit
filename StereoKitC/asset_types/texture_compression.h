#include "../stereokit.h"
#include <stdint.h>
#include <stdbool.h>

namespace sk {

void texture_compression_init();

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

}

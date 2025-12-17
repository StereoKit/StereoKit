#include "texture_compression.h"
#include "../sk_memory.h"

#include <sk_renderer.h>
#include <basisu_transcoder.h>

using namespace basist;

namespace sk {

///////////////////////////////////////////

void texture_compression_init() {
	basist::basisu_transcoder_init();
}

///////////////////////////////////////////

skr_tex_fmt_ texture_preferred_compressed_format(int32_t channels, bool is_srgb) {
	// The Adreno docs say this:
	// https://developer.qualcomm.com/sites/default/files/docs/adreno-gpu/snapdragon-game-toolkit/gdg/gpu/best_practices_texture.html#compression-strategies
	//
	// - Use ASTC compression if it is available.
	// - Otherwise, use ETC2 compression if available.
	// - Otherwise, select the compression format as follows :
	//   - ATC if using alpha
	//   - ETC if not using alpha

	if (channels == 1) {
		if      (skr_tex_fmt_is_supported(skr_tex_fmt_etc2_r11))  return skr_tex_fmt_etc2_r11;
		else if (skr_tex_fmt_is_supported(skr_tex_fmt_bc4_r   ))  return skr_tex_fmt_bc4_r;
		else                                                      return skr_tex_fmt_r8;
	} else if (channels == 2) {
		if      (skr_tex_fmt_is_supported(skr_tex_fmt_etc2_rg11)) return skr_tex_fmt_etc2_rg11;
		else if (skr_tex_fmt_is_supported(skr_tex_fmt_bc5_rg   )) return skr_tex_fmt_bc5_rg;
		else                                                      return skr_tex_fmt_r8g8;
	} else if (channels == 3) {
		if      (skr_tex_fmt_is_supported(skr_tex_fmt_astc4x4_rgba)) return (is_srgb ? skr_tex_fmt_astc4x4_rgba_srgb : skr_tex_fmt_astc4x4_rgba);
		//else if (skr_tex_fmt_is_supported(skr_tex_fmt_atc_rgb     )) return skr_tex_fmt_atc_rgb;
		else if (skr_tex_fmt_is_supported(skr_tex_fmt_bc1_rgb     )) return (is_srgb ? skr_tex_fmt_bc1_rgb_srgb      : skr_tex_fmt_bc1_rgb);
		else                                                         return (is_srgb ? skr_tex_fmt_rgba32_srgb       : skr_tex_fmt_rgba32_linear);
	} else if (channels == 4) {
		if      (skr_tex_fmt_is_supported(skr_tex_fmt_astc4x4_rgba)) return (is_srgb ? skr_tex_fmt_astc4x4_rgba_srgb : skr_tex_fmt_astc4x4_rgba);
		//else if (skr_tex_fmt_is_supported(skr_tex_fmt_atc_rgba    )) return skr_tex_fmt_atc_rgba;
		else if (skr_tex_fmt_is_supported(skr_tex_fmt_bc3_rgba    )) return (is_srgb ? skr_tex_fmt_bc3_rgba_srgb     : skr_tex_fmt_bc3_rgba);
		else                                                         return (is_srgb ? skr_tex_fmt_rgba32_srgb       : skr_tex_fmt_rgba32_linear);
	}
	log_err("Shouldn't get here!");
	return skr_tex_fmt_none;
}

///////////////////////////////////////////

transcoder_texture_format texture_transcode_format(skr_tex_fmt_ format) {
	switch (format) {
	case skr_tex_fmt_rgba32_srgb:
	case skr_tex_fmt_rgba32_linear:    return transcoder_texture_format::cTFRGBA32;
	case skr_tex_fmt_bc1_rgb:
	case skr_tex_fmt_bc1_rgb_srgb:     return transcoder_texture_format::cTFBC1_RGB;
	case skr_tex_fmt_bc3_rgba:
	case skr_tex_fmt_bc3_rgba_srgb:    return transcoder_texture_format::cTFBC3_RGBA;
	case skr_tex_fmt_bc4_r:            return transcoder_texture_format::cTFBC4_R;
	case skr_tex_fmt_bc5_rg:           return transcoder_texture_format::cTFBC5_RG;
	case skr_tex_fmt_bc7_rgba:
	case skr_tex_fmt_bc7_rgba_srgb:    return transcoder_texture_format::cTFBC7_RGBA;
	case skr_tex_fmt_atc_rgb:          return transcoder_texture_format::cTFATC_RGB;
	case skr_tex_fmt_atc_rgba:         return transcoder_texture_format::cTFATC_RGBA;
	case skr_tex_fmt_astc4x4_rgba:
	case skr_tex_fmt_astc4x4_rgba_srgb:return transcoder_texture_format::cTFASTC_4x4_RGBA;
	//case skr_tex_fmt_pvrtc2_rgb:   return transcoder_texture_format::cTFPVRTC2_4_RGB;
	case skr_tex_fmt_pvrtc2_rgba:      return transcoder_texture_format::cTFPVRTC2_4_RGBA;
	case skr_tex_fmt_etc2_r11:         return transcoder_texture_format::cTFETC2_EAC_R11;
	case skr_tex_fmt_etc2_rg11:        return transcoder_texture_format::cTFETC2_EAC_RG11;
	case skr_tex_fmt_etc2_rgba:
	case skr_tex_fmt_etc2_rgba_srgb:   return transcoder_texture_format::cTFETC2_RGBA;
	default:
		log_err("Shouldn't get here!");
		return transcoder_texture_format::cTFRGBA32;
	}
}

///////////////////////////////////////////

bool ktx2_info(void* data, size_t data_size, tex_type_* ref_image_type, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count) {
	ktx2_transcoder ktx_transcoder;
	if (!ktx_transcoder.init(data, (uint32_t)data_size)) return false;

	ktx2_header header = ktx_transcoder.get_header();
	*out_width       = ktx_transcoder.get_width ();
	*out_height      = ktx_transcoder.get_height();
	*out_mip_count   = ktx_transcoder.get_levels() == 0 ? 1 : ktx_transcoder.get_levels();
	*out_array_count = ktx_transcoder.get_layers() == 0 ? 1 : ktx_transcoder.get_layers();
	if (ktx_transcoder.get_faces() > 1) { // If it's a cubemap, it'll have a value of 6 here, otherwise it'll be 1.
		*out_array_count = ktx_transcoder.get_faces();
		*ref_image_type  = tex_type_cubemap;
	}

	int32_t channels = ktx_transcoder.get_has_alpha() ? 4 : 3;
	bool    is_srgb  = ktx_transcoder.get_dfd_transfer_func() == KTX2_KHR_DF_TRANSFER_SRGB;
	*out_format = (tex_format_)texture_preferred_compressed_format(channels, is_srgb);
	ktx_transcoder.clear();
	return true;
}

///////////////////////////////////////////

bool ktx2_decode(void* data, size_t data_size, tex_type_ *ref_image_type, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count, void** out_data_arr) {
	ktx2_transcoder ktx_transcoder;
	if (!ktx_transcoder.init(data, (uint32_t)data_size)) return false;

	ktx2_header header = ktx_transcoder.get_header();
	*out_width       = ktx_transcoder.get_width ();
	*out_height      = ktx_transcoder.get_height();
	*out_mip_count   = ktx_transcoder.get_levels();
	*out_array_count = ktx_transcoder.get_layers() == 0 ? 1 : ktx_transcoder.get_layers();
	if (ktx_transcoder.get_faces() > 1) { // If it's a cubemap, it'll have a value of 6 here, otherwise it'll be 1.
		*out_array_count = ktx_transcoder.get_faces();
		*ref_image_type  = tex_type_cubemap;
	}

	int32_t channels = ktx_transcoder.get_has_alpha() ? 4 : 3;
	bool    is_srgb  = ktx_transcoder.get_dfd_transfer_func() == KTX2_KHR_DF_TRANSFER_SRGB;
	*out_format = (tex_format_)texture_preferred_compressed_format(channels, is_srgb);
	transcoder_texture_format tc_fmt = texture_transcode_format((skr_tex_fmt_)*out_format);

	uint32_t block_width, block_height, bytes_per_block;
	skr_tex_fmt_block_info((skr_tex_fmt_)*out_format, &block_width, &block_height, &bytes_per_block);

	skr_vec3i_t base_size  = { *out_width, *out_height, 1 };
	uint64_t    layer_size = 0;
	for (int32_t mip = 0; mip < *out_mip_count; mip++) {
		layer_size += skr_tex_calc_mip_size((skr_tex_fmt_)*out_format, base_size, mip);
	}
	for (int32_t i = 0; i < *out_array_count; i++) {
		out_data_arr[i] = sk_malloc(layer_size);
	}

	ktx2_transcoder_state state;
	state.clear();
	ktx_transcoder.start_transcoding();
	bool     success    = true;
	uint64_t mip_offset = 0;
	for (uint32_t mip = 0; success && mip < ktx_transcoder.get_levels(); mip++) {
		skr_vec3i_t mip_dims         = skr_tex_calc_mip_dimensions(base_size, mip);
		int32_t     mip_block_width  = (mip_dims.x + (block_width -1)) / block_width;
		int32_t     mip_block_height = (mip_dims.y + (block_height-1)) / block_height;

		int32_t layer_count = ktx_transcoder.get_layers() == 0 ? 1 : ktx_transcoder.get_layers();
		for (int32_t layer = 0; success && layer < layer_count; layer++) {
			for (uint32_t face = 0; success && face < ktx_transcoder.get_faces(); face++) {
				int32_t layer_idx = layer + face;

				success = ktx_transcoder.transcode_image_level(mip, layer, face, ((uint8_t*)out_data_arr[layer_idx]) + mip_offset, mip_block_width * mip_block_height, tc_fmt, 0, mip_block_width, mip_block_height, 0, 0, &state);
			}
		}
		mip_offset += skr_tex_calc_mip_size((skr_tex_fmt_)*out_format, base_size, mip);
	}
	ktx_transcoder.clear();
	if (!success) {
		for (int32_t i = 0; i < *out_array_count; i++) {
			sk_free(out_data_arr[i]);
		}
	}
	return success;
}

///////////////////////////////////////////

bool basisu_info(void* data, size_t data_size, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count) {
	return false;
}

///////////////////////////////////////////

bool basisu_decode(void* data, size_t data_size, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count, void** out_data_arr) {
	basisu_transcoder transcoder;
	if (!transcoder.validate_header(data, (uint32_t)data_size)) return false;

	basisu_file_info file_info = {};
	if (!transcoder.get_file_info(data, (uint32_t)data_size, file_info))
		return false;

	if (file_info.m_tex_type != cBASISTexType2D)
		return false;

	basisu_image_info image_info = {};
	if (!transcoder.get_image_info(data, (uint32_t)data_size, image_info, 0)) {
		return false;
	}

	*out_width  = image_info.m_width;
	*out_height = image_info.m_height;
	// Transcode the .basis file to RGBA32
	/*if (!transcoder.transcode_image_level(basisFileData.data(), basisFileData.size(), 0, 0, decodedImage.data(), imageInfo.m_width * imageInfo.m_height, basist::transcoder_texture_format::cTFRGBA32)) {
		std::cerr << "Failed to transcode image!" << std::endl;
		return -1;
	}*/
	return false;
}

}
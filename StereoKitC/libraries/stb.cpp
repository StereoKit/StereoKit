#include "../sk_memory.h"

#pragma warning(push)
#pragma warning(disable : 26451 6011 6262 6308 6387 28182 26819 )
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_MALLOC sk::sk_malloc
#define STBI_REALLOC sk::sk_realloc
#define STBI_FREE sk::_sk_free
#if defined (_M_ARM)
#define STBI_NEON
#endif
#include "stb_image.h"
#pragma warning(pop)


#pragma warning(push)
#pragma warning(disable : 26451 26819 6386 6385 )
#if defined(_WIN32)
#define __STDC_LIB_EXT1__
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_WINDOWS_UTF8
#include "stb_image_write.h"
#pragma warning(pop)


#pragma warning(push)
#pragma warning(disable : 26451 26819 6387 6011 6385 )
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,y) sk::sk_malloc(x)
#define STBTT_free(x,y) sk::_sk_free(x)
#include "stb_truetype.h"
#pragma warning(pop)
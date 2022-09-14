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
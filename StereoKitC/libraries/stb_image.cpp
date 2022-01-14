#pragma warning(push)
#pragma warning(disable : 26451 6011 6262 6308 6387 28182 26819 )
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#if defined (_M_ARM)
	#define STBI_NEON
#endif
#include "stb_image.h"
#pragma warning(pop)
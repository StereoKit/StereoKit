#pragma warning(push)
#pragma warning(disable : 4100 4505 )

#define _POSIX_C_SOURCE 199309L
#define MA_NO_OPENSL
#define MA_NO_DSOUND
#define MA_NO_WINMM

#define MA_NO_FLAC

#define MA_NO_ENCODING
#define MA_NO_GENERATION

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#pragma warning(pop)
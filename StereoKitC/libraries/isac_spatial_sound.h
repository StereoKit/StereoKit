#pragma once

// This spatial sound implementation only works on Windows
#ifdef _MSC_VER

// Pull in stereokit.h for vec3 definition
#include "../stereokit.h"

// Typedef to help manage the callback that stereokit gets to fill in audio data
typedef void (*isac_callback)(float** sourceBuffers, uint32_t numSources, uint32_t numframes, sk::vec3* positions, float* volumes);

long isac_activate(uint32_t maxSources, isac_callback callback);
void isac_destroy ();

#endif
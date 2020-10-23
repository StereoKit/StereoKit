#pragma once

#include <stdint.h>
#include <malloc.h>

template <typename T>
inline T *malloc()              { return (T *)malloc(sizeof(T)); }
template <typename T>
inline T *malloc(int32_t count) { return (T *)malloc(sizeof(T)*count); }
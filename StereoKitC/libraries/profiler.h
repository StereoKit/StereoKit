#pragma once

#if defined(TRACY_ENABLE)
#include <tracy/Tracy.hpp>
#define profiler_frame_mark() FrameMark
#define profiler_zone() ZoneScoped
#define profiler_zone_name(name) ZoneScopedN(name)
#define profiler_zone_text(fmt, ...) ZoneScoped; ZoneNameF(fmt, __VA_ARGS__)
#define profiler_alloc(ptr, size) TracyAlloc(ptr, size)
#define profiler_free(ptr) TracyFree(ptr)
#define profiler_thread_name(name, groupId) tracy::SetThreadNameWithHint(name, groupId)
#else
#define profiler_frame_mark() ((void)0)
#define profiler_zone() ((void)0)
#define profiler_zone_name(name) ((void)0)
#define profiler_zone_text(fmt, ...) ((void)0)
#define profiler_alloc(ptr, size) ((void)0)
#define profiler_free(ptr) ((void)0)
#define profiler_thread_name(name, groupId) ((void)0)
#endif

#if defined(TRACY_ENABLE)
#include <tracy/Tracy.hpp>
#define profiler_frame_mark() FrameMark
#define profiler_zone() ZoneScoped
#define profiler_zone_name(name) ZoneScopedN(name)
#define profiler_zone_text(fmt, ...) ZoneScoped; ZoneNameF(fmt, __VA_ARGS__)
#else
#define profiler_frame_mark()
#define profiler_zone()
#define profiler_zone_name(name)
#define profiler_zone_text(fmt, ...)
#endif

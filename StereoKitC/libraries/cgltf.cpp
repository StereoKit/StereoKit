#include "../sk_memory.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning( disable : 26451 )
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#endif
#define CGLTF_IMPLEMENTATION
#define CGLTF_MALLOC sk::sk_malloc
#define CGLTF_FREE sk::_sk_free
#include "../libraries/cgltf.h"
#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma clang diagnostic pop
#endif
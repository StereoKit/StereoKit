#pragma once

#include <stdint.h>
#include <string.h>

#if defined(_MSC_VER)

	#include <wtypes.h>
	#include <winnt.h>
	#define atomic_increment(int_val_ref) InterlockedIncrement((LONG*)int_val_ref)
	#define atomic_decrement(int_val_ref) InterlockedDecrement((LONG*)int_val_ref)

	#define atomic_load_i32(ref)           ReadNoFence  ((volatile LONG*)(ref))
	#define atomic_store_i32(ref, val)     WriteNoFence ((volatile LONG*)(ref), (LONG)(val))
	#define atomic_load_i32_acq(ref)       ReadAcquire  ((volatile LONG*)(ref))
	#define atomic_store_i32_rel(ref, val) WriteRelease ((volatile LONG*)(ref), (LONG)(val))
	#define atomic_exchange_i32(ref, val)  InterlockedExchange((volatile LONG*)(ref), (LONG)(val))
	// True when the value was `expect`, and was swapped to `val`. A function
	// so `expect` evaluates once, matching the gcc/clang side.
	inline bool atomic_cas_i32_fn(volatile LONG* ref, LONG expect, LONG val) {
		return InterlockedCompareExchange(ref, val, expect) == expect;
	}
	#define atomic_cas_i32(ref, expect, val) atomic_cas_i32_fn((volatile LONG*)(ref), (LONG)(expect), (LONG)(val))

	#define atomic_load_u64(ref)           (uint64_t)ReadNoFence64 ((volatile LONG64*)(ref))
	#define atomic_store_u64(ref, val)     WriteNoFence64((volatile LONG64*)(ref), (LONG64)(val))
	#define atomic_load_u64_acq(ref)       (uint64_t)ReadAcquire64((volatile LONG64*)(ref))
	#define atomic_store_u64_rel(ref, val) WriteRelease64((volatile LONG64*)(ref), (LONG64)(val))
	#define atomic_exchange_u64(ref, val)  (uint64_t)InterlockedExchange64((volatile LONG64*)(ref), (LONG64)(val))

#else

	// gcc and clang both implement these at least
	#define atomic_increment(int_val_ref) __sync_add_and_fetch(int_val_ref, 1)
	#define atomic_decrement(int_val_ref) __sync_sub_and_fetch(int_val_ref, 1)

	#define atomic_load_i32(ref)           __atomic_load_n ((ref), __ATOMIC_RELAXED)
	#define atomic_store_i32(ref, val)     __atomic_store_n((ref), (val), __ATOMIC_RELAXED)
	#define atomic_load_i32_acq(ref)       __atomic_load_n ((ref), __ATOMIC_ACQUIRE)
	#define atomic_store_i32_rel(ref, val) __atomic_store_n((ref), (val), __ATOMIC_RELEASE)
	#define atomic_exchange_i32(ref, val)  __atomic_exchange_n((ref), (val), __ATOMIC_ACQ_REL)
	// True when the value was `expect`, and was swapped to `val`
	inline bool atomic_cas_i32_fn(int32_t* ref, int32_t expect, int32_t val) {
		return __atomic_compare_exchange_n(ref, &expect, val, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
	}
	#define atomic_cas_i32(ref, expect, val) atomic_cas_i32_fn((int32_t*)(ref), (int32_t)(expect), (int32_t)(val))

	#define atomic_load_u64(ref)           __atomic_load_n ((ref), __ATOMIC_RELAXED)
	#define atomic_store_u64(ref, val)     __atomic_store_n((ref), (val), __ATOMIC_RELAXED)
	#define atomic_load_u64_acq(ref)       __atomic_load_n ((ref), __ATOMIC_ACQUIRE)
	#define atomic_store_u64_rel(ref, val) __atomic_store_n((ref), (val), __ATOMIC_RELEASE)
	#define atomic_exchange_u64(ref, val)  __atomic_exchange_n((ref), (val), __ATOMIC_ACQ_REL)

#endif

// Pointers travel through the u64 atomics, all supported targets are 64
// bit. Relaxed ordering: publication ordering must come from a separate
// release/acquire, these only prevent tearing and satisfy the race rules.
#define atomic_load_ptr(ref)       ((void*)atomic_load_u64 ((uint64_t*)(ref)))
#define atomic_store_ptr(ref, val) atomic_store_u64((uint64_t*)(ref), (uint64_t)(val))

// Floats travel as their bit patterns through the i32 atomics. A relaxed
// atomic is enough for single-value parameter snapshots, tearing is what
// these prevent.
inline float atomic_load_f32(const float* ref) {
	int32_t bits = atomic_load_i32((const int32_t*)ref);
	float   result; memcpy(&result, &bits, sizeof(float));
	return result;
}
inline void atomic_store_f32(float* ref, float val) {
	int32_t bits; memcpy(&bits, &val, sizeof(float));
	atomic_store_i32((int32_t*)ref, bits);
}

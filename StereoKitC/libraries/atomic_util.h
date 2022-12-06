#pragma once

#if defined(_MSC_VER)
	#include <winnt.h>
	#define atomic_increment(int_val_ref) InterlockedIncrement((LONG*)int_val_ref)
	#define atomic_decrement(int_val_ref) InterlockedDecrement((LONG*)int_val_ref)
#else
	// gcc and clang both implement these at least
	#define atomic_increment(int_val_ref) __sync_add_and_fetch(int_val_ref, 1)
	#define atomic_decrement(int_val_ref) __sync_sub_and_fetch(int_val_ref, 1)
#endif
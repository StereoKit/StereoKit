#pragma once

#include <stdint.h>

#define GEN_INDEX_BITS 20      // 2^20 = ~1M slots
#define GEN_GENERATION_BITS 12 // 2^12 = 4096 generations per slot
#define GEN_INDEX_MASK ((1u << GEN_INDEX_BITS) - 1)
#define GEN_GENERATION_MASK ((1u << GEN_GENERATION_BITS) - 1)

static inline uint32_t gen_id_index(uint32_t id) {
	return id & GEN_INDEX_MASK;
}

static inline uint32_t gen_id_generation(uint32_t id) {
	return (id >> GEN_INDEX_BITS) & GEN_GENERATION_MASK;
}

static inline uint32_t gen_id_make(int32_t index, int32_t generation) {
	assert(index      < (1u << GEN_INDEX_BITS));
	assert(generation < (1u << GEN_GENERATION_BITS));
	assert(index      >= 0);
	assert(generation >  0);
	return (generation << GEN_INDEX_BITS) | index;
}

static inline int32_t gen_next_gen(int32_t current_generation) {
	int32_t next = current_generation < 0
		? -current_generation + 1
		:  current_generation + 1;
	return (next > GEN_GENERATION_MASK) ? 1 : next;
}
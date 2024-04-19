#include "random.h"

uint32_t rand_hash_x (int32_t x, uint32_t seed) {
	const uint32_t noise_1 = 0xD2A80A3F;
	const uint32_t noise_2 = 0xA884F197;
	const uint32_t noise_3 = 0x6C736F4B;
	const uint32_t noise_4 = 0xB79F3ABB;
	const uint32_t noise_5 = 0x1B56C4F5;

	uint32_t result = (uint32_t)x;
	result *= noise_1;
	result += seed;
	result ^= (result >> 9);
	result += noise_2;
	result ^= (result >> 11);
	result *= noise_3;
	result ^= (result >> 13);
	result += noise_4;
	result ^= (result >> 15);
	result *= noise_5;
	result ^= (result >> 17);
	return result;
}

rand_state_t _rand_state = {};

void         rand_set_seed (uint32_t seed)      { _rand_state.seed = seed; _rand_state.x = 0; }
rand_state_t rand_get_state()                   { return _rand_state; }
void         rand_set_state(rand_state_t state) { _rand_state = state; }
uint32_t     rand_x        ()                   { return rand_hash_x(_rand_state.x++, _rand_state.seed); }
#include <stdint.h>

const int32_t _hash_prime1   = 198491317;
const int32_t _hash_prime2   = 6542989;
const float   _hash_uintmaxf = 4294967295;

typedef struct rand_state_t {
	uint32_t seed;
	uint32_t x;
} rand_state_t;

       uint32_t rand_hash_x   (int32_t x,                       uint32_t seed);
inline float    rand_hash_xf  (int32_t x,                       uint32_t seed) { return rand_hash_x(x, seed) / _hash_uintmaxf; }
inline float    rand_hash_xyf (int32_t x, int32_t y,            uint32_t seed) { return rand_hash_x(x + (y * _hash_prime1), seed) / _hash_uintmaxf; }
inline float    rand_hash_xyzf(int32_t x, int32_t y, int32_t z, uint32_t seed) { return rand_hash_x(x + (y * _hash_prime1) + (z * _hash_prime2), seed) / _hash_uintmaxf; }
inline uint32_t rand_hash_xy  (int32_t x, int32_t y,            uint32_t seed) { return rand_hash_x(x + (y * _hash_prime1), seed); }
inline uint32_t rand_hash_xyz (int32_t x, int32_t y, int32_t z, uint32_t seed) { return rand_hash_x(x + (y * _hash_prime1) + (z * _hash_prime2), seed); }

       void         rand_set_seed (uint32_t seed);
       rand_state_t rand_get_state();
       void         rand_set_state(rand_state_t state);
       uint32_t     rand_x        ();
inline float        rand_xf       () { return rand_x() / _hash_uintmaxf; }
inline int32_t      rand_range    (int32_t min, int32_t max) { return min + (rand_x ()%(max-min)); }
inline int32_t      rand_rangef   (float   min, float   max) { return min + (rand_xf()*(max-min)); }
#include "../stereokit.h"

namespace sk {

float sdf_box_round (vec2 pt, float size, float radius);
float sdf_box       (vec2 pt, float size);
float sdf_diamond   (vec2 pt, vec2  size);
float sdf_rounded_x (vec2 pt, float size, float radius);
tex_t sdf_create_tex(int32_t width, int32_t height, float (*sdf)(vec2 pt), float scale);

inline float sdf_circle  (vec2 pt, float radius)              { return vec2_magnitude(pt) - radius; }
inline float sdf_subtract(float sdf, float from)              { return fmaxf(-sdf, from); }
inline float sdf_union   (float distance_1, float distance_2) { return fminf(distance_1, distance_2); }

}
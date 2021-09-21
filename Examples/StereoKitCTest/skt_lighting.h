#pragma once

#include <stereokit.h>
#include <stdint.h>

typedef int32_t skt_light_id_t;

void           skt_lighting_init        ();
void           skt_lighting_step        ();
void           skt_lighting_shutdown    ();
skt_light_id_t skt_lighting_add_light   (sk::vec3 at, sk::color128 color, float radius, float intensity);
void           skt_lighting_remove_light(skt_light_id_t id);
void           skt_lighting_clear_lights();

void           skt_lighting_set_pos   (skt_light_id_t id, sk::vec3 position);
sk::vec3       skt_lighting_get_pos   (skt_light_id_t id);
void           skt_lighting_set_color (skt_light_id_t id, sk::color128 color);
sk::color128   skt_lighting_get_color (skt_light_id_t id);
void           skt_lighting_set_radius(skt_light_id_t id, float radius);
float          skt_lighting_get_radius(skt_light_id_t id);
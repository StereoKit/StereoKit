#include "skt_lighting.h"

#include <vector>
#include <algorithm>

using namespace sk;
using namespace std;

///////////////////////////////////////////

#define SKT_MAX_LIGHTS 6

///////////////////////////////////////////

typedef enum skt_light_state_ {
	skt_light_state_lit,
	skt_light_state_fading,
	skt_light_state_unlit,
} skt_light_state_;

typedef struct skt_light_info_t {
	float            dist;
	vec3             pos;
	float            radius;
	color128         color;
	float            intensity;
	skt_light_state_ state;
	float            curr_intensity;
} skt_light_info_t;

typedef struct skt_packed_light_t {
	vec3  pos;
	float radius;
	vec3  color;
	float intensity;
} skt_packed_light_t;

typedef struct skt_light_buffer_t {
	skt_packed_light_t lights[SKT_MAX_LIGHTS];
} skt_light_buffer_t;

///////////////////////////////////////////

vector<skt_light_info_t> skt_lights;
vector<skt_light_id_t>   skt_light_ids;
bool                     skt_first_frame = true;
skt_light_buffer_t       skt_light_data  = {};
material_buffer_t        light_buffer    = {};

///////////////////////////////////////////

void skt_lighting_init() {
	light_buffer = material_buffer_create(3, sizeof(skt_light_buffer_t));
}

///////////////////////////////////////////

void skt_lighting_shutdown() {
	material_buffer_release(light_buffer);
	skt_lighting_clear_lights();
}

///////////////////////////////////////////

void skt_lighting_step() {
	vec3 pos  = input_head()->position;
	int  free = SKT_MAX_LIGHTS;
	for (int i = 0; i < skt_lights.size(); i++) {
		skt_light_info_t l = skt_lights[i];
		vec3 worldPos   = hierarchy_to_world_point(l.pos);
		vec3 worldScale = hierarchy_to_world_point(vec3{ l.radius, 0, 0 });
		l.pos    = worldPos;
		l.radius = worldScale.x;
		l.dist   = vec3_distance_sq(pos, worldPos);

		if (l.state == skt_light_state_fading) { 
			l.curr_intensity -= time_elapsedf()*8;
			if (l.curr_intensity < 0.001f) { 
				l.state          = skt_light_state_unlit;
				l.curr_intensity = 0;
			} else {
				free -= 1;
			}
		} else if (l.state == skt_light_state_lit) { 
			l.curr_intensity += time_elapsedf()*3;
			if (l.curr_intensity > 1) l.curr_intensity = 1;
			free -= 1;
		}
		l.intensity = l.curr_intensity;

		skt_lights[i] = l;
	}
	sort(skt_light_ids.begin(), skt_light_ids.end(), [](skt_light_id_t a, skt_light_id_t b) { return skt_lights[a].dist < skt_lights[b].dist; });

	for (int i = 0; i < skt_lights.size(); i++) {
		skt_light_info_t l = skt_lights[skt_light_ids[i]];
		if (free > 0 && l.state == skt_light_state_unlit) {
			l.state = skt_light_state_lit;
			if (skt_first_frame) {
				l.curr_intensity = 1;
				l.intensity      = 1;
			}
			free--;
		} else if (i >= SKT_MAX_LIGHTS && l.state == skt_light_state_lit) {
			l.state = skt_light_state_fading;
		}
		skt_lights[skt_light_ids[i]] = l;
	}
	sort(skt_light_ids.begin(), skt_light_ids.end(), [](skt_light_id_t a, skt_light_id_t b) { return skt_lights[a].state < skt_lights[b].state; });

	for (int t = 0; t < SKT_MAX_LIGHTS; t++) {
		if (t < skt_lights.size()) {
			skt_light_info_t light = skt_lights[skt_light_ids[t]];
			skt_light_data.lights[t].pos       = light.pos;
			skt_light_data.lights[t].radius    = light.radius;
			skt_light_data.lights[t].color     = { light.color.r,light.color.g,light.color.b };
			skt_light_data.lights[t].intensity = light.intensity;
		} else {
			skt_light_data.lights[t] = {};
		}
	}
	material_buffer_set_data(light_buffer, &skt_light_data);

	skt_first_frame = false;
}

///////////////////////////////////////////

skt_light_id_t skt_lighting_add_light(sk::vec3 at, sk::color128 color, float radius, float intensity) {
	skt_light_info_t light = {};
	light.state     = skt_light_state_unlit;
	light.pos       = at;
	light.radius    = radius;
	light.color     = color;
	light.intensity = intensity;
	skt_lights   .push_back(light);
	skt_light_ids.push_back(skt_light_ids.size());
	return skt_light_ids.size()-1;
}

///////////////////////////////////////////

void skt_lighting_remove_light(skt_light_id_t id) {
	for (int i = 0; i < skt_light_ids.size(); i++) {
		if (skt_light_ids[i] == id) {
			skt_light_ids.erase(skt_light_ids.begin() + id);
			i--;
			continue;
		}

		if (skt_light_ids[i] > id)
			skt_light_ids[i] --;
	}
	skt_lights.erase(skt_lights.begin()+id);
}

///////////////////////////////////////////

void skt_lighting_clear_lights() {
	skt_lights   .clear();
	skt_light_ids.clear();
	skt_first_frame = true;
}

///////////////////////////////////////////

void           skt_lighting_set_pos   (skt_light_id_t id, sk::vec3 position) { skt_lights[id].pos = position; }
sk::vec3       skt_lighting_get_pos   (skt_light_id_t id)                    { return skt_lights[id].pos; }
void           skt_lighting_set_color (skt_light_id_t id, sk::color128 color){ skt_lights[id].color = color;}
sk::color128   skt_lighting_get_color (skt_light_id_t id)                    { return skt_lights[id].color; }
void           skt_lighting_set_radius(skt_light_id_t id, float radius)      { skt_lights[id].radius = radius; }
float          skt_lighting_get_radius(skt_light_id_t id)                    { return skt_lights[id].radius; }
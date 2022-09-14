#include "scene.h"

scene_t scene_curr = {};
scene_t scene_next = {};

void scene_set_active(scene_t scene) {
	if (scene_curr.init == nullptr) {
		scene_curr = scene;
		scene_init();
	} else {
		scene_next = scene;
	}
}
void scene_shutdown() {
	if (scene_curr.shutdown != nullptr)
		scene_curr.shutdown();
	scene_curr = {};
}
void scene_update() {
	if (scene_next.init != nullptr) {
		scene_shutdown();
		scene_curr      = scene_next;
		scene_next = {};
		scene_init();
	}

	if (scene_curr.update != nullptr)
		scene_curr.update();
}
void scene_init() {
	if (scene_curr.init != nullptr)
		scene_curr.init();
}
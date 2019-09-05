#include "../../StereoKitC/stereokit.h"

#include "scene.h"
#include "demo_basics.h"
#include "demo_ui.h"

int main() {
	if (!sk_init("StereoKit C", sk_runtime_flatscreen))
		return 1;

	scene_t demo_basics = {
		demo_basics_init,
		demo_basics_update,
		demo_basics_shutdown,
	};
	scene_t demo_ui = {
		demo_ui_init,
		demo_ui_update,
		demo_ui_shutdown,
	};
	scene_set_active(demo_ui);

	while (sk_step( []() {
		scene_update();
	}));

	scene_shutdown();
	sk_shutdown();
	return 0;
}
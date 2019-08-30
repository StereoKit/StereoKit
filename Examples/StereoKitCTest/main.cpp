#include "../../StereoKitC/stereokit.h"

#include "scene.h"
#include "demo_basics.h"

int main() {
	if (!sk_init("StereoKit C", sk_runtime_flatscreen))
		return 1;

	scene_t demo_basics = {
		demo_basics_init,
		demo_basics_update,
		demo_basics_shutdown,
	};
	scene_set_active(demo_basics);

	while (sk_step( []() {
		scene_update();
	}));

	scene_shutdown();
	sk_shutdown();
	return 0;
}
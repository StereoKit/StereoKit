#include "../../StereoKitC/stereokit.h"

#include "app.h"
#include "demo_basics.h"

int main() {
	if (!sk_init("StereoKit C", sk_runtime_flatscreen))
		return 1;

	app_t demo_basics = {
		demo_basics_init,
		demo_basics_update,
		demo_basics_shutdown,
	};
	app_set_active(demo_basics);

	while (sk_step( []() {
		app_update();
	}));

	app_shutdown();
	sk_shutdown();
	return 0;
}
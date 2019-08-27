#include "app.h"

app_t sk_app = {};
app_t sk_next_app = {};

void app_set_active(app_t app) {
	if (sk_app.init == nullptr) {
		sk_app = app;
		app_init();
	} else {
		sk_next_app = app;
	}
}
void app_shutdown() {
	if (sk_app.shutdown != nullptr)
		sk_app.shutdown();
}
void app_update() {
	if (sk_next_app.init != nullptr) {
		app_shutdown();
		sk_app      = sk_next_app;
		sk_next_app = {};
		app_init();
	}

	if (sk_app.update != nullptr)
		sk_app.update();
}
void app_init() {
	if (sk_app.init != nullptr)
		sk_app.init();
}
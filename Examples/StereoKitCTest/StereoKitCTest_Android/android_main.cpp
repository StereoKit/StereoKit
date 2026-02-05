#if defined(__ANDROID__)

#include <android_native_app_glue.h>
#include <pthread.h>

#include <stereokit.h>
using namespace sk;

// This allows us to set Android variables in SK's initialization, as well as
// invoke the normal "main" entrypoint.
extern sk_settings_t settings;
extern int main(int argc, char **argv);

// Our own variables for tracking state.
static bool      android_initialized;
static bool      android_finish;
static pthread_t sk_thread_id;

extern "C" {

void* sk_thread(void* arg) {
	// Invoke the app's "main" function.
	main(0, NULL);

	android_finish = true;
	return NULL;
}

static void android_on_cmd(android_app* state, int32_t cmd) {
	switch (cmd) {
	case APP_CMD_INIT_WINDOW:
		if (state->window != NULL && android_initialized == false) {
			android_initialized = true;

			// Set up StereoKit settings with the Android Activity variables.
			settings.android_activity = state->activity->clazz;
			settings.android_java_vm  = state->activity->vm;

			// Kick off the StereoKit thread.
			pthread_create(&sk_thread_id, NULL, sk_thread, NULL);
		}
		break;
	case APP_CMD_DESTROY:
		if (sk_is_stepping())
			sk_quit(quit_reason_user);
		break;
	default: break;
	}
}

void android_main(struct android_app* state) {
	// Register our event callback
	state->onAppCmd = android_on_cmd;
	android_initialized = false;
	android_finish      = false;
	sk_thread_id        = {};

	// The main Android event loop
	while (state->destroyRequested == 0) {
		// Process system events
		int32_t              events;
		android_poll_source* source;
		if (ALooper_pollOnce(android_initialized ? 0 : -1, NULL, &events, (void**)&source) >= 0)
			if (source) source->process(state, source);

		if (android_finish) {
			android_finish = false;
			ANativeActivity_finish(state->activity);
		}
	}

	// If we finished before the SK app has, send it a quit message
	if (sk_is_stepping())
		sk_quit(quit_reason_user);
	ANativeActivity_finish(state->activity);

	// Wait until StereoKit thread has finished cleaning up
	pthread_join(sk_thread_id, NULL);
	exit(0);
}

}

#endif
#include "xr_backend_simxr.h"
#include "stage_anchor.h"

namespace sk {

///////////////////////////////////////////

bool xr_backend_simxr_init() {
	stage_anchor_init();

	return true;
}

///////////////////////////////////////////

void xr_backend_simxr_shutdown() {
	stage_anchor_shutdown();
}

///////////////////////////////////////////

void xr_backend_simxr_step_begin() {

}

///////////////////////////////////////////

void xr_backend_simxr_step_end() {

}

} // namespace sk
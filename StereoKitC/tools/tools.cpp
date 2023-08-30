#include "tools.h"

#include "../tools/file_picker.h"
#include "../tools/virtual_keyboard.h"

namespace sk {

///////////////////////////////////////////

bool tools_init() {
	virtualkeyboard_initialize();
	return true;
}

///////////////////////////////////////////

void tools_step() {
	file_picker_update();
	virtualkeyboard_update();
}

///////////////////////////////////////////

void tools_shutdown() {
	file_picker_shutdown();
}

}
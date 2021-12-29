#pragma once
#include "stereokit.h"

#ifdef __cplusplus
namespace sk {
#endif


	void     virtualkeyboard_initialize();

	void     virtualkeyboard_update();
	void     virtualkeyboard_open(bool open, keyboard_input_type_ type);
#ifdef __cplusplus
}
#endif

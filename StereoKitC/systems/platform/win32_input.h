#pragma once

#ifndef SK_NO_FLATSCREEN

namespace sk {

void win32_input_init();
void win32_input_shutdown();
void win32_input_update();

} // namespace sk

#endif // SK_NO_FLATSCREEN
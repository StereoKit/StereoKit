#pragma once

#include "../stereokit.h"

#include <sk_app.h>

namespace sk {

void file_picker_update            ();
void file_picker_shutdown          ();
void file_picker_handle_dialog_event(ska_event_file_dialog_t* evt);

}
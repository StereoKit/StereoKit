#pragma once

#include "../../stereokit.h"

namespace sk {

bool openxr_views_create ();
void openxr_views_destroy();
void openxr_views_render ();

} // namespace sk
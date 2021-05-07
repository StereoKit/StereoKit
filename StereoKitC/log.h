#pragma once

#include "stereokit.h"

namespace sk {

void log_fail_reason (int32_t confidence, log_ log_as, const char *fail_reason);
void log_fail_reasonf(int32_t confidence, log_ log_as, const char *fail_reason, ...);
void log_show_any_fail_reason();
void log_clear_any_fail_reason();

}
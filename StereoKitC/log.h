#pragma once

#include <stdint.h>

namespace sk {

void log_fail_reason(int32_t confidence, const char *fail_reason);
void log_show_any_fail_reason();

}
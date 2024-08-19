// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

#pragma once

#include "stereokit.h"

namespace sk {

void log_fail_reason          (int32_t confidence, log_ log_as, const char *fail_reason);
void log_fail_reasonf         (int32_t confidence, log_ log_as, const char *fail_reason, ...);
void log_show_any_fail_reason ();
void log_clear_any_fail_reason();
void log_clear_subscribers    ();
void log_set_name             (const char* name);

}
/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 * Copyright (c) 2026 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../stereokit.h"

namespace sk {

// Runtimes only store a uuid for persisted anchors, so this keeps an app
// chosen name for each one in the app's data folder. Names are unique, and
// the file loads on first use, since the data folder needs sk_app running.

bool32_t    spatial_names_find     (const char* name, sk_uuid_t* out_uuid);
const char* spatial_names_get      (sk_uuid_t uuid); // Null when unnamed
void        spatial_names_set      (sk_uuid_t uuid, const char* name); // Takes the name from any other uuid
void        spatial_names_remove   (sk_uuid_t uuid);
int32_t     spatial_names_count    ();
sk_uuid_t   spatial_names_get_index(int32_t index);
void        spatial_names_clear    ();
void        spatial_names_shutdown ();

}

/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

namespace sk {

typedef enum {
	interactor_default_default = 0,
	interactor_default_none,
} interactor_default_;

void interactor_modes_init    ();
void interactor_modes_shutdown();
void interactor_modes_update  ();

void interactors_default_set  (interactor_default_ default_interactors);

}
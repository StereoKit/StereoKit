/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#include "platform.h"
#if defined (SK_OS_ANDROID) || defined(SK_OS_LINUX)

#include "../sk_memory.h"
#include <unistd.h>

namespace sk {

///////////////////////////////////////////

char *platform_working_dir() {
	int32_t len    = 260;
	char   *result = sk_malloc_t(char, len);
	result[0] = '\0';
	while (len < 5000 && getcwd(result, len) == nullptr) {
		sk_free(result);
		len       = len * 2;
		result    = sk_malloc_t(char, len);
		result[0] = '\0';
	}
	return result;
}

} // namespace sk
#endif // defined (SK_OS_ANDROID) || defined(SK_OS_LINUX)

// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

namespace StereoKit
{
	public static class Interaction
	{
		public static DefaultInteractors DefaultInteractors {
			get => NativeAPI.interaction_get_default_interactors();
			set => NativeAPI.interaction_set_default_interactors(value);
		}
		public static bool DefaultDraw
		{
			get => NativeAPI.interaction_get_default_draw();
			set => NativeAPI.interaction_set_default_draw(value);
		}
	}
}

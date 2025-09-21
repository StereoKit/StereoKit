// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

namespace StereoKit
{
	/// <summary>Controls for the interaction system, and the interactors that
	/// StereoKit provides by default.</summary>
	public static class Interaction
	{
		/// <summary>This allows you to  control what kind of interactors
		/// StereoKit will provide for you. This also allows you to entirely
		/// disable StereoKit's interactors so you can just use custom ones!
		/// </summary>
		public static DefaultInteractors DefaultInteractors {
			get => NativeAPI.interaction_get_default_interactors();
			set => NativeAPI.interaction_set_default_interactors(value);
		}

		/// <summary>By default, StereoKit will draw indicators for some of the
		/// default interactors, such as the far interaction / aiming rays.
		/// This doesn't affect custom interactors. Setting this to false will
		/// prevent StereoKit from drawing any of these indicators.</summary>
		public static bool DefaultDraw
		{
			get => NB.Bool(NativeAPI.interaction_get_default_draw());
			set => NativeAPI.interaction_set_default_draw(NB.Int(value));
		}
	}
}

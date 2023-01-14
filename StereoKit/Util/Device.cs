using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
	public static class Device
	{
		public static DisplayType  DisplayType  => NativeAPI.device_display_get_type ();
		public static DisplayBlend DisplayBlend => NativeAPI.device_display_get_blend();
		public static int DisplayWidth  => NativeAPI.device_display_get_width ();
		public static int DisplayHeight => NativeAPI.device_display_get_height();
		public static FovInfo DisplayFOV => NativeAPI.device_display_get_fov();

		public static bool ValidBlend(DisplayBlend blend)
			=> NativeAPI.device_display_valid_blend(blend) > 0 ? true : false;
	}
}

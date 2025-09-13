using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
	public static class Permissions
	{
		public static PermissionState State(Permission permission)
			=> NativeAPI.permissions_state(permission);
		public static bool IsInteractive(Permission permission)
			=> NativeAPI.permissions_is_interactive(permission);
		public static void Request(Permission permission)
			=> NativeAPI.permissions_request(permission);
	}
}

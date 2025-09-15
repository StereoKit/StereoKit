using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
	public static class Permission
	{
		public static PermissionState State(PermissionType permission)
			=> NativeAPI.permission_state(permission);
		public static bool IsInteractive(PermissionType permission)
			=> NativeAPI.permission_is_interactive(permission);
		public static void Request(PermissionType permission)
			=> NativeAPI.permission_request(permission);
	}
}

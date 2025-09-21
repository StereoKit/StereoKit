using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
	/// <summary>Certain features in XR require explicit permissions from the
	/// operating system and user! This is typically for feature that surface
	/// sensitive data like eye gaze, or objects in the user's room. This is
	/// often complicated by the fact that permissions aren't standardized
	/// across XR runtimes, making these permissions fragile and a pain to work
	/// with.
	/// 
	/// This class attempts to manage feature permissions in a nice
	/// cross-platform manner that handles runtime specific differences. You
	/// will still need to add permission strings to your app's metadata file
	/// (like AndroidManifest.xml), but this class will handle figuring out
	/// which strings in the metadata to actually use.
	/// 
	/// On platforms that don't use permissions, like Win32 or Linux, these
	/// functions will behave as though everything is granted automatically.
	/// </summary>
	public static class Permission
	{
		/// <summary>Retreives the current state of a particular permission.
		/// This is a fast check, so it's fine to call frequently.</summary>
		/// <param name="permission">The permission you're interested in.</param>
		/// <returns>Check `PermissionState` for details.</returns>
		public static PermissionState State(PermissionType permission)
			=> NativeAPI.permission_state(permission);

		/// <summary>Does this permission need the user to approve it? This
		/// typically means a popup window will come up when you Request this
		/// permission, and the user has a chance to decline it.</summary>
		/// <param name="permission">The permission you're interested in.</param>
		/// <returns>True if the permission requires user interaction, false
		/// otherwise.</returns>
		public static bool IsInteractive(PermissionType permission)
			=> NB.Bool(NativeAPI.permission_is_interactive(permission));

		/// <summary>This sends off a request to the OS for a particular
		/// permission! If the permission IsInteractive, then this will bring
		/// up a popup that the user may need to interact with. Otherwise, this
		/// will silently approve the permission. This means that the
		/// permission may take an arbitrary amount of time before it's
		/// approved, or declined.</summary>
		/// <param name="permission">The permission to request.</param>
		public static void Request(PermissionType permission)
			=> NativeAPI.permission_request(permission);
	}
}

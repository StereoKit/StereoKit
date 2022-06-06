using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace StereoKit
{
	internal struct FileFilter
	{
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)] public string ext;

		public static FileFilter[] List(params string[] list)
			=> list.Select(i => new FileFilter { ext = i }).ToArray();
	}

	/// <summary>This class provides some platform related code that runs
	/// cross-platform. You might be able to do many of these things with C#,
	/// but you might not be able to do them in as portable a manner as these
	/// methods do!</summary>
	public static class Platform
	{
		#region Keyboard
		/// <summary>Force the use of StereoKit's built-in fallback keyboard
		/// instead of the system keyboard. This may be great for testing or
		/// look and feel matching, but the system keyboard should generally be
		/// preferred for accessibility reasons.</summary>
		public static bool ForceFallbackKeyboard
		{
			get => NativeAPI.platform_keyboard_get_force_fallback() > 0;
			set => NativeAPI.platform_keyboard_set_force_fallback(value?1:0);
		}

		/// <summary>Check if a soft keyboard is currently visible. This may be
		/// an OS provided keyboard or StereoKit's fallback keyboard, but will
		/// not indicate the presence of a physical keyboard.</summary>
		public static bool KeyboardVisible
			=> NativeAPI.platform_keyboard_visible() > 0;

		/// <summary>Request or hide a soft keyboard for the user to type on.
		/// StereoKit will surface OS provided soft keyboards where available,
		/// and use a fallback keyboard when not. On systems with physical
		/// keyboards, soft keyboards generally will not be shown if the user
		/// has interacted with their physical keyboard recently.</summary>
		/// <param name="show">Tells whether or not to open or close the soft
		/// keyboard.</param>
		/// <param name="inputType">Soft keyboards can change layout to
		/// optimize for the type of text that's required. StereoKit will
		/// request the soft keyboard layout that most closely represents the
		/// TextContext provided.</param>
		public static void KeyboardShow(bool show, TextContext inputType = TextContext.Text)
			=> NativeAPI.platform_keyboard_show(show?1:0, inputType);
		#endregion

		#region File Picker
		static Action<string>       _filePickerOnSelect;
		static Action               _filePickerOnCancel;
		static Action<bool, string> _filePickerOnComplete;
		static PickerCallback       _filePickerCallback;
		private static void FilePickerCallback(IntPtr data, int confirmed, IntPtr file, int fileLength)
		{
			string filename = NativeHelper.FromUtf8(file, fileLength);

			if (confirmed > 0) {
				_filePickerOnSelect?.Invoke(filename);
			} else {
				_filePickerOnCancel?.Invoke();
			}
			_filePickerOnComplete?.Invoke(confirmed>0, filename);
		}

		/// <summary>Starts a file picker window! This will create a native
		/// file picker window if one is available in the current setup, and
		/// if it is not, it'll create a fallback filepicker build using
		/// StereoKit's UI.
		/// 
		/// Flatscreen apps will show traditional file pickers, and UWP has 
		/// an OS provided file picker that works in MR. All others currently
		/// use the fallback system.
		/// 
		/// A note for UWP apps, UWP generally does not have permission to 
		/// access random files, unless the user has chosen them with the 
		/// picker! This picker properly handles permissions for individual
		/// files on UWP, but may have issues with files that reference other
		/// files, such as .gltf files with external textures. See 
		/// Platform.WriteFile and Platform.ReadFile for manually reading and
		/// writing files in a cross-platfom manner.</summary>
		/// <param name="mode">Are we trying to Open a file, or Save a file?
		/// This changes the appearance and behavior of the picker to support
		/// the specified action.</param>
		/// <param name="onSelectFile">This Action will be called with the
		/// proper filename when the picker has successfully completed! On a
		/// cancel or close event, this Action is not called.</param>
		/// <param name="onCancel">If the user cancels the file picker, or 
		/// the picker is closed via FilePickerClose, this Action is called.
		/// </param>
		/// <param name="filters">A list of file extensions that the picker
		/// should filter for. This is in the format of ".glb" and is case
		/// insensitive.</param>
		public static void FilePicker(PickerMode mode, Action<string> onSelectFile, Action onCancel, params string[] filters)
		{
			_filePickerCallback   = FilePickerCallback;
			_filePickerOnSelect   = onSelectFile;
			_filePickerOnCancel   = onCancel;
			_filePickerOnComplete = null;
			NativeAPI.platform_file_picker_sz(mode, IntPtr.Zero, _filePickerCallback, FileFilter.List(filters), filters.Length);
		}
		/// <summary>Starts a file picker window! This will create a native
		/// file picker window if one is available in the current setup, and
		/// if it is not, it'll create a fallback filepicker build using
		/// StereoKit's UI.
		/// 
		/// Flatscreen apps will show traditional file pickers, and UWP has 
		/// an OS provided file picker that works in MR. All others currently
		/// use the fallback system. Some pickers will block the system and
		/// return right away, but others will stick around and let users
		/// continue to interact with the app.
		/// 
		/// A note for UWP apps, UWP generally does not have permission to 
		/// access random files, unless the user has chosen them with the 
		/// picker! This picker properly handles permissions for individual
		/// files on UWP, but may have issues with files that reference other
		/// files, such as .gltf files with external textures. See 
		/// Platform.WriteFile and Platform.ReadFile for manually reading and
		/// writing files in a cross-platfom manner.</summary>
		/// <param name="mode">Are we trying to Open a file, or Save a file?
		/// This changes the appearance and behavior of the picker to support
		/// the specified action.</param>
		/// <param name="onComplete">This action will be called when the file
		/// picker has finished, either via a cancel event, or from a confirm
		/// event. First parameter is a bool, where true indicates the 
		/// presence of a valid filename, and false indicates a failure or 
		/// cancel event.</param>
		/// <param name="filters">A list of file extensions that the picker
		/// should filter for. This is in the format of ".glb" and is case
		/// insensitive.</param>
		public static void FilePicker(PickerMode mode, Action<bool, string> onComplete, params string[] filters)
		{
			_filePickerCallback = FilePickerCallback;
			_filePickerOnSelect = null;
			_filePickerOnCancel = null;
			_filePickerOnComplete = onComplete;
			NativeAPI.platform_file_picker_sz(mode, IntPtr.Zero, _filePickerCallback, FileFilter.List(filters), filters.Length);
		}
		/// <summary>If the picker is visible, this will close it and 
		/// immediately trigger a cancel event for the active picker.</summary>
		public static void FilePickerClose() => NativeAPI.platform_file_picker_close();
		/// <summary>This will check if the file picker interface is
		/// currently visible. Some pickers will never show this, as they
		/// block the application until the picker has completed.</summary>
		public static bool FilePickerVisible => NativeAPI.platform_file_picker_visible();
		#endregion

		#region File Save & Load
		/// <summary>Writes a UTF-8 text file to the filesystem, taking
		/// advantage of any permissions that may have been granted by
		/// Platform.FilePicker.</summary>
		/// <param name="filename">Path to the new file. Not affected by
		/// Assets folder path.</param>
		/// <param name="data">A string to write to the file. This gets
		/// converted to a UTF-8 encoding.</param>
		/// <returns>True on success, False on failure.</returns>
		public static bool WriteFile(string filename, string data)
		{ 
			byte[] bytes = NativeHelper.ToUtf8(data); 
			return NativeAPI.platform_write_file(NativeHelper.ToUtf8(filename), bytes, (UIntPtr)bytes.Length);
		}

		/// <summary>Writes an array of bytes to the filesystem, taking
		/// advantage of any permissions that may have been granted by
		/// Platform.FilePicker.</summary>
		/// <param name="filename">Path to the new file. Not affected by
		/// Assets folder path.</param>
		/// <param name="data">An array of bytes to write to the file.</param>
		/// <returns>True on success, False on failure.</returns>
		public static bool WriteFile(string filename, byte[] data)
			=> NativeAPI.platform_write_file(NativeHelper.ToUtf8(filename), data, (UIntPtr)data.Length);

		/// <summary>Reads the entire contents of the file as a UTF-8 string,
		/// taking advantage of any permissions that may have been granted by
		/// Platform.FilePicker.</summary>
		/// <param name="filename">Path to the file. Not affected by Assets
		/// folder path.</param>
		/// <param name="data">A UTF-8 decoded string representing the
		/// contents of the file. Will be null on failure.</param>
		/// <returns>True on success, False on failure.</returns>
		public static bool ReadFile (string filename, out string data) {
			data = null;
			if (!NativeAPI.platform_read_file(NativeHelper.ToUtf8(filename), out IntPtr fileData, out UIntPtr length))
				return false;

			data = NativeHelper.FromUtf8(fileData, (int)length);
			return true;
		}

		/// <summary>Reads the entire contents of the file as a UTF-8 string,
		/// taking advantage of any permissions that may have been granted by
		/// Platform.FilePicker. Returns null on failure.</summary>
		/// <param name="filename">Path to the file. Not affected by Assets
		/// folder path.</param>
		/// <returns>A UTF-8 decoded string if successful, null if not.</returns>
		public static string ReadFileText(string filename)
		{
			ReadFile(filename, out string data);
			return data;
		}

		/// <summary>Reads the entire contents of the file as a byte array,
		/// taking advantage of any permissions that may have been granted by
		/// Platform.FilePicker.</summary>
		/// <param name="filename">Path to the file. Not affected by Assets
		/// folder path.</param>
		/// <param name="data">A raw byte array representing the contents of
		/// the file. Will be null on failure.</param>
		/// <returns>True on success, False on failure.</returns>
		public static bool ReadFile (string filename, out byte[] data)
		{
			data = null;
			if (!NativeAPI.platform_read_file(NativeHelper.ToUtf8(filename), out IntPtr fileData, out UIntPtr length))
				return false;

			data = new byte[(uint)length];
			Marshal.Copy(fileData, data, 0, data.Length);
			return true;
		}

		/// <summary>Reads the entire contents of the file as a byte array,
		/// taking advantage of any permissions that may have been granted by
		/// Platform.FilePicker. Returns null on failure.</summary>
		/// <param name="filename">Path to the file. Not affected by Assets
		/// folder path.</param>
		/// <returns>A raw byte array if successful, null if not.</returns>
		public static byte[] ReadFileBytes(string filename)
		{
			ReadFile(filename, out byte[] data);
			return data;
		}
		#endregion
	}
}

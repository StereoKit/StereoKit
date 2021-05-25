using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
	public static class Platform
	{
		static Action<string> _filePickerOnSelect;
		static Action         _filePickerOnCancel;
		static PickerCallback _filePickerCallback;
		private static void FilePickerCallback(IntPtr data, int confirmed, string file)
		{
			if (confirmed > 0) { 
				if (_filePickerOnSelect != null) _filePickerOnSelect(file);
			} else {
				if (_filePickerOnCancel != null) _filePickerOnCancel();
			}
		}
		public static void FilePicker(PickerMode mode, FileFilter[] filters, Action<string> onSelectFile, Action onCancel)
		{
			_filePickerCallback = FilePickerCallback;
			_filePickerOnSelect = onSelectFile;
			_filePickerOnCancel = onCancel;
			NativeAPI.platform_file_picker(mode, filters, filters.Length, IntPtr.Zero, _filePickerCallback);
		}
		public static void FilePickerHide() { }
	}
}

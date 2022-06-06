using System;
using System.Runtime.InteropServices;
using System.Text;

namespace StereoKit
{
	internal class NativeHelper
	{
		public static string FromUtf8(IntPtr str, int strLength)
		{
			string result = string.Empty;
			if (str != IntPtr.Zero)
			{
				byte[] filenameUtf8 = new byte[strLength];
				Marshal.Copy(str, filenameUtf8, 0, strLength);

				int terminator = Array.IndexOf(filenameUtf8, (byte)0);
				result = Encoding.UTF8.GetString(filenameUtf8, 0, terminator == -1 ? strLength : terminator);
			}
			return result;
		}

		public static byte[] ToUtf8(string str)
			=> Encoding.UTF8.GetBytes(str + '\0');
	}
}

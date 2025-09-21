using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
using System.Text;

namespace StereoKit
{
	internal class NativeHelper
	{
		public static string FromUtf8(IntPtr str, int strLength)
		{
			string result = null;
			if (str != IntPtr.Zero)
			{
				byte[] filenameUtf8 = new byte[strLength];
				Marshal.Copy(str, filenameUtf8, 0, strLength);

				int terminator = Array.IndexOf(filenameUtf8, (byte)0);
				result = Encoding.UTF8.GetString(filenameUtf8, 0, terminator == -1 ? strLength : terminator);
			}
			return result;
		}

		public static string FromUtf8(IntPtr str)
			=> FromUtf8(str, Marshal.PtrToStringAnsi(str).Length);

		public static byte[] ToUtf8(string str)
			=> str == null ? null : Encoding.UTF8.GetBytes(str + '\0');
	}
	internal static class NB
	{
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public unsafe static int Int(bool b) => *(byte*)(&b);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public unsafe static bool Bool(int i) => *(bool*)(&i);
	}
	internal static class NU8
	{
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe string Str(byte* str) => Utf8StringMarshaller.ConvertToManaged(str);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe byte* Bytes(string str) => Utf8StringMarshaller.ConvertToUnmanaged(str);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe void Free(byte* str) => Utf8StringMarshaller.Free(str);
	}
	internal static class NU16
	{
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe string Str(ushort* str) => Utf16StringMarshaller.ConvertToManaged(str);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe ushort* Bytes(string str) => Utf16StringMarshaller.ConvertToUnmanaged(str);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe void Free(ushort* str) => Utf16StringMarshaller.Free(str);
	}
}

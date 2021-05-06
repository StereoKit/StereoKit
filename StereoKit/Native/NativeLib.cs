using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	static class NativeLib
	{
		public static bool LoadLib()
		{
			// Mono uses a different strategy for linking the DLL
			if (RuntimeInformation.FrameworkDescription.StartsWith("Mono "))
				return true;

			string arch = "";
			switch (RuntimeInformation.OSArchitecture)
			{
				case Architecture.X86:   arch = "x86";   break;
				case Architecture.X64:   arch = "x64";   break;
				case Architecture.Arm64: arch = "arm64"; break;
			}
			return RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
				? LoadWindows(arch)
				: LoadUnix   (arch);
		}

		[DllImport("kernel32")]
		static extern IntPtr LoadLibrary(string fileName);
		static bool LoadWindows(string arch)
		{
			if (LoadLibrary("StereoKitC") != IntPtr.Zero) return true;
			if (LoadLibrary($"runtimes/win-{arch}/native/StereoKitC.dll") != IntPtr.Zero) return true;
			return false;
		}

		[DllImport("libdl")]
		static extern IntPtr dlopen(string fileName, int flags);
		static bool LoadUnix(string arch)
		{
			const int RTLD_NOW = 2;
			if (dlopen("libStereoKitC.so", RTLD_NOW) != IntPtr.Zero) return true;
			if (dlopen($"./runtimes/linux-{arch}/native/libStereoKitC.so", RTLD_NOW) != IntPtr.Zero) return true;
			if (dlopen($"{AppDomain.CurrentDomain.BaseDirectory}/runtimes/linux-{arch}/native/libStereoKitC.so", RTLD_NOW) != IntPtr.Zero) return true;
			return false;
		}
	}
}

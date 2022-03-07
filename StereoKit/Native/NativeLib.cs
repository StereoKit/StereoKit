using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	static class NativeLib
	{
		static bool _loaded = false;
		internal static bool Load()
		{
			if (_loaded)
				return true;

			// Mono uses a different strategy for linking the DLL
			if (RuntimeInformation.FrameworkDescription.StartsWith("Mono "))
				return true;
			// Browsers should also have their own strategy for linking
			if (RuntimeInformation.OSDescription == "Browser")
				return true;

			string arch = RuntimeInformation.OSArchitecture == Architecture.Arm64
				? "arm64"
				: "x64";
			_loaded = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
				? LoadWindows(arch)
				: LoadUnix   (arch);
			return _loaded;
		}

		[DllImport("kernel32", CharSet = CharSet.Unicode)]
		static extern IntPtr LoadLibraryW(string fileName);
		static bool LoadWindows(string arch)
		{
			if (LoadLibraryW($"runtimes/win-{arch}/native/StereoKitC.dll") != IntPtr.Zero) return true;
			if (LoadLibraryW("StereoKitC") != IntPtr.Zero) return true;
			return false;
		}


		[DllImport("dl", CharSet = CharSet.Ansi)]
		static extern IntPtr dlopen(string fileName, int flags);
		static bool LoadUnix(string arch)
		{
			const int RTLD_NOW = 2;
			if (dlopen($"./runtimes/linux-{arch}/native/libStereoKitC.so", RTLD_NOW) != IntPtr.Zero) return true;
			if (dlopen($"{AppDomain.CurrentDomain.BaseDirectory}/runtimes/linux-{arch}/native/libStereoKitC.so", RTLD_NOW) != IntPtr.Zero) return true;
			if (dlopen("libStereoKitC.so", RTLD_NOW) != IntPtr.Zero) return true;
			return false;
		}
	}
}

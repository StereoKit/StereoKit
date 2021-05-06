using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	static class NativeLib
	{
		public static bool LoadLib(string name)
		{
			return true;

			return RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
				? LoadWindows(name)
				: LoadUnix   (name);
		}


		[DllImport("kernel32")]
		static extern IntPtr LoadLibrary(string fileName);
		static bool LoadWindows(string name)
		{
			foreach(string file in PotentialFiles(name))
			{
				if (LoadLibrary(file) != IntPtr.Zero) return true;
			}
			return false;
		}

		[DllImport("libdl")]
		static extern IntPtr dlopen(string fileName, int flags);
		static bool LoadUnix(string name)
		{
			foreach (string file in PotentialFiles(name))
			{
				// 2 is RTLD_NOW
				if (dlopen(file, 2) != IntPtr.Zero) return true;
			}
			return false;
		}

		static string[] PotentialFiles(string name)
		{
			string plat    = "";
			string arch    = "";
			string prefix  = "";
			string postfix = "";
			switch (RuntimeInformation.OSArchitecture)
			{
				case Architecture.X86:   arch = "x86";   break;
				case Architecture.X64:   arch = "x64";   break;
				case Architecture.Arm64: arch = "arm64"; break;
			}
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
			{
				plat    = "win";
				postfix = ".dll";
			}
			else
			{
				plat    = "linux";
				prefix  = "lib";
				postfix = ".so";
			}

			return new string[] { 
				name,
				$"runtimes/{plat}-{arch}/native/{prefix}{name}{postfix}"};
		}
	}
}

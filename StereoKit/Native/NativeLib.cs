using System;
using System.ComponentModel;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace StereoKit
{
    internal static class NativeLib
    {
        [DllImport("kernel32", SetLastError = true)]
        static extern IntPtr LoadLibraryEx(string lpFileName, IntPtr hFile, uint dwFlags);
        [DllImport("kernel32")]
        static extern bool FreeLibrary(IntPtr hModule);

        public const string DllName = "StereoKitC.dll";

        static IntPtr library;

        internal static void LoadDll()
        {
            if (library != IntPtr.Zero)
                return;

            string folder = "";
            switch (RuntimeInformation.ProcessArchitecture)
            {
                case Architecture.Arm:   folder = "ARM";   break;
                case Architecture.Arm64: folder = "ARM64"; break;
                case Architecture.X64:   folder = "x64";   break;
                case Architecture.X86:   folder = "Win32"; break;
                default: throw new Exception("What crazy architecture is this?!");
            }

            #if DEBUG
            folder += "_Debug";
            #else
            folder += "_Release";
            #endif

            string location = Assembly.GetExecutingAssembly().Location;
            string path     = Path.Combine(Path.GetDirectoryName(location), folder, DllName);
            library = LoadLibraryEx(path, IntPtr.Zero, 0);

            if (library == IntPtr.Zero) { 
                int err = Marshal.GetLastWin32Error();
                throw new Exception("Error loading StereoKit DLL at " + path, new Win32Exception(err));
            } else {
                Console.WriteLine("Using {0} build.", folder);
            }
        }
        public static void UnloadDLL()
        {
            FreeLibrary(library);
        }
    }
}

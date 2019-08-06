using System;
using System.IO;
using System.Runtime.InteropServices;

namespace StereoKit
{
    internal static class NativeLib
    {
        [DllImport("kernel32")]
        static extern IntPtr LoadLibraryEx(string lpFileName, IntPtr hFile, uint dwFlags);
        [DllImport("kernel32")]
        static extern bool FreeLibrary(IntPtr hModule);

        public const string DllName = "StereoKitC.dll";

        static IntPtr library;

        internal static void LoadDll()
        {
            if (library != IntPtr.Zero)
                return;

            string folder = Environment.Is64BitProcess ?
                "x64_" :
                "Win32_";

            #if DEBUG
            folder += "Debug";
            #else
            folder += "Release";
            #endif

            string location = System.Reflection.Assembly.GetExecutingAssembly().Location;
            string path     = Path.Combine(Path.GetDirectoryName(location), folder, DllName);
            library = LoadLibraryEx(path, IntPtr.Zero, 0);

            if (library == IntPtr.Zero) { 
                throw new Exception("Missing StereoKit DLL, should be at " + path);
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

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

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
            string folder;
            if (System.Environment.Is64BitProcess)
                folder = "x64_Release";
            else
                folder = "Win32_Release";

            string path = Path.Combine(Directory.GetCurrentDirectory(), folder, DllName);
            library = LoadLibraryEx(path, IntPtr.Zero, 0);
        }
        public static void UnloadDLL()
        {
            FreeLibrary(library);
        }
    }
}

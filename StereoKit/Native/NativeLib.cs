using System;
using System.ComponentModel;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace StereoKit
{
    internal static class NativeLib
    {
        [DllImport("kernel32", SetLastError = true)]
        static extern IntPtr LoadLibraryEx(string lpFileName, IntPtr hFile, uint dwFlags);
        [DllImport("kernel32")]
        static extern bool FreeLibrary(IntPtr hModule);

        //[DllImport("kernel32", CharSet = CharSet.Unicode, SetLastError = true)]
        //static extern int GetCurrentPackageFullName(ref int packageFullNameLength, StringBuilder packageFullName);

        public const string DllName = "StereoKitC.dll";

        static IntPtr library;

        /*static bool IsUWP()
        {
            int           length = 0;
            StringBuilder sb     = new StringBuilder(0);
            int           result = GetCurrentPackageFullName(ref length, sb);

            sb     = new StringBuilder(length);
            result = GetCurrentPackageFullName(ref length, sb);

            return result != 15700L; // APPMODEL_ERROR_NO_PACKAGE;
        }*/

        internal static void LoadDll()
        {
           /* if (library != IntPtr.Zero)
                return;

            // .NET Native compiles DLLs into the application, no need to load it.
            string framework = RuntimeInformation.FrameworkDescription;
            framework = framework.Substring(0, framework.LastIndexOf(' '));
            if (framework == ".NET Native")
                return;

            // Find out which native DLL architecture we need to load
            string folder = "";
            switch (RuntimeInformation.ProcessArchitecture)
            {
                case Architecture.Arm:   folder = "ARM";   break;
                case Architecture.Arm64: folder = "ARM64"; break;
                case Architecture.X64:   folder = "x64";   break;
                case Architecture.X86:   folder = "Win32"; break;
                default: throw new Exception("What crazy architecture is this?!");
            }

            // Debug vs. release uses different DLLs too
            #if DEBUG
            folder += "_Debug";
            #else
            folder += "_Release";
#endif
            //if (IsUWP())
            //    folder += "_UWP";

            
            string location = Assembly.GetExecutingAssembly().Location;
            string path     = Path.Combine(Path.GetDirectoryName(location), folder, DllName);
            library = LoadLibraryEx(path, IntPtr.Zero, 0);

            if (library == IntPtr.Zero) { 
                int err = Marshal.GetLastWin32Error();
                throw new Exception("Error loading StereoKit DLL at " + path, new Win32Exception(err));
            } else {
                Console.WriteLine("Using {0} build.", folder);
            }*/
        }
        public static void UnloadDLL()
        {
            //FreeLibrary(library);
        }
    }
}

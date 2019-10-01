using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace StereoKit
{
    public static class Log
    {
        #region Console Color Setup
        const int  STD_OUTPUT_HANDLE = -11;
        const uint ENABLE_VIRTUAL_TERMINAL_PROCESSING = 4;

        [DllImport("kernel32.dll", SetLastError = true)]
        static extern IntPtr GetStdHandle(int nStdHandle);

        [DllImport("kernel32.dll")]
        static extern bool GetConsoleMode(IntPtr hConsoleHandle, out uint lpMode);

        [DllImport("kernel32.dll")]
        static extern bool SetConsoleMode(IntPtr hConsoleHandle, uint dwMode);
        internal static void SetupConsole()
        {
            if (!Environment.OSVersion.Platform.ToString().ToLower().Contains("win"))
                return;
            
            IntPtr handle = GetStdHandle(STD_OUTPUT_HANDLE);
            uint mode;
            GetConsoleMode(handle, out mode);
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(handle, mode);
        }
        #endregion


        public static LogLevel Filter { set{ NativeAPI.log_set_filter(value); } }

        public static void Write(LogLevel level, string text)
        {
            NativeAPI.log_write(level, text);
        }
        public static void Write(LogLevel level, string text, params object[] items)
        {
            NativeAPI.log_write(level, string.Format(text, items));
        }
    }
}

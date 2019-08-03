using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace StereoKit
{
    public enum LogLevel
    {
        Info = 0,
        Warning,
        Error
    };

    public static class Log
    {
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern void log_write(LogLevel level, string text);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void log_set_filter(LogLevel level);

        public static LogLevel Filter { set{ log_set_filter(value); } }

        public static void Write(LogLevel level, string text)
        {
            log_write(level, text);
        }
        public static void Write(LogLevel level, string text, params object[] items)
        {
            log_write(level, string.Format(text, items));
        }
    }
}

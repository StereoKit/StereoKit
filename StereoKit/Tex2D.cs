using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace StereoKit
{
    public class Tex2D
    {
        #region Imports
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr tex2d_create();
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr tex2d_create_file(string file);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void tex2d_release(IntPtr tex);
        #endregion

        internal IntPtr _texInst;
        public Tex2D()
        {
            _texInst = tex2d_create();
        }
        public Tex2D(string file)
        {
            _texInst = tex2d_create_file(file);
        }
        ~Tex2D()
        {
            if (_texInst != IntPtr.Zero)
                tex2d_release(_texInst);
        }
    }
}

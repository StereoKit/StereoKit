using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    [Flags]
    enum TexType
    {
        ImageNomips  = 1 << 0,
        Cubemap      = 1 << 1,
        Rendertarget = 1 << 2,
        Depth        = 1 << 3,
        Mips         = 1 << 4,
        Dynamic      = 1 << 5,
        Image        = ImageNomips | Mips,
    };

    enum TexFormat
    {
        Rgba32 = 0,
        Rgba64,
        Rgba128,
        Depthstencil,
        Depth32,
        Depth16,
    };

    enum TexSample
    {
        Linear = 0,
        Point,
        Anisotropic
    };

    enum TexAddress
    {
        Wrap = 0,
        Clamp,
        Mirror,
    };

    public class Tex2D
    {
        #region Imports
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr tex2d_find(string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr tex2d_create(string id, TexType type, TexFormat format);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr tex2d_create_file(string file);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr tex2d_create_cubemap_files(string[] cube_face_file_xxyyzz);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr tex2d_create_cubemap_file(string equirectangular_file);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void tex2d_release(IntPtr tex);
        #endregion

        internal IntPtr _texInst;
        static   int    index = 0;
        public Tex2D()
        {
            index += 1;
            _texInst = tex2d_create("auto/tex"+index, TexType.Image, TexFormat.Rgba32);
        }
        private Tex2D(IntPtr tex)
        {
            _texInst = tex;
            if (_texInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Received an empty texture!");
        }
        public Tex2D(string file)
        {
            _texInst = tex2d_create_file(file);
        }
        public Tex2D(string[] cubeFaceFiles_xxyyzz)
        {
            if (cubeFaceFiles_xxyyzz.Length != 6)
                Log.Write(LogLevel.Error, "To create a cubemap, you must have exactly 6 images!");
            _texInst = tex2d_create_cubemap_files(cubeFaceFiles_xxyyzz);
        }
        ~Tex2D()
        {
            if (_texInst != IntPtr.Zero)
                tex2d_release(_texInst);
        }

        public static Tex2D Find(string id)
        {
            IntPtr tex = tex2d_find(id);
            return tex == IntPtr.Zero ? null : new Tex2D(tex);
        }
        public static Tex2D FromEquirectangular(string equirectangularCubemap)
        {
            IntPtr tex = tex2d_create_cubemap_file(equirectangularCubemap);
            return tex == IntPtr.Zero ? null : new Tex2D(tex);
        }
    }
}

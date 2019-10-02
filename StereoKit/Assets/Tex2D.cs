using System;

namespace StereoKit
{
    public class Tex2D
    {
        internal IntPtr _texInst;
        public Tex2D()
        {
            _texInst = NativeAPI.tex2d_create(TexType.Image, TexFormat.Rgba32);
        }
        private Tex2D(IntPtr tex)
        {
            _texInst = tex;
            if (_texInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Received an empty texture!");
        }
        public Tex2D(string file)
        {
            _texInst = NativeAPI.tex2d_create_file(file);
        }
        public Tex2D(string[] cubeFaceFiles_xxyyzz)
        {
            if (cubeFaceFiles_xxyyzz.Length != 6)
                Log.Write(LogLevel.Error, "To create a cubemap, you must have exactly 6 images!");
            _texInst = NativeAPI.tex2d_create_cubemap_files(cubeFaceFiles_xxyyzz);
        }
        ~Tex2D()
        {
            if (_texInst != IntPtr.Zero)
                NativeAPI.tex2d_release(_texInst);
        }

        public static Tex2D Find(string id)
        {
            IntPtr tex = NativeAPI.tex2d_find(id);
            return tex == IntPtr.Zero ? null : new Tex2D(tex);
        }
        public static Tex2D FromEquirectangular(string equirectangularCubemap)
        {
            IntPtr tex = NativeAPI.tex2d_create_cubemap_file(equirectangularCubemap);
            return tex == IntPtr.Zero ? null : new Tex2D(tex);
        }
    }
}

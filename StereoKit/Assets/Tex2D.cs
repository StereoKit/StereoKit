using System;

namespace StereoKit
{
    public class Tex
    {
        internal IntPtr _texInst;
        public Tex()
        {
            _texInst = NativeAPI.tex_create(TexType.Image, TexFormat.Rgba32);
        }
        private Tex(IntPtr tex)
        {
            _texInst = tex;
            if (_texInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Received an empty texture!");
        }
        public Tex(string file)
        {
            _texInst = NativeAPI.tex_create_file(file);
        }
        public Tex(string[] cubeFaceFiles_xxyyzz)
        {
            if (cubeFaceFiles_xxyyzz.Length != 6)
                Log.Write(LogLevel.Error, "To create a cubemap, you must have exactly 6 images!");
            _texInst = NativeAPI.tex_create_cubemap_files(cubeFaceFiles_xxyyzz);
        }
        ~Tex()
        {
            if (_texInst != IntPtr.Zero)
                NativeAPI.tex_release(_texInst);
        }

        public static Tex Find(string id)
        {
            IntPtr tex = NativeAPI.tex_find(id);
            return tex == IntPtr.Zero ? null : new Tex(tex);
        }
        public static Tex FromEquirectangular(string equirectangularCubemap)
        {
            IntPtr tex = NativeAPI.tex_create_cubemap_file(equirectangularCubemap);
            return tex == IntPtr.Zero ? null : new Tex(tex);
        }
    }
}

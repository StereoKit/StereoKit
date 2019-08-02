using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    public class Material
    {
        #region Imports
        [DllImport(Util.DllName, CharSet = CharSet.Ansi)]
        static extern IntPtr material_find(string id);
        [DllImport(Util.DllName, CharSet = CharSet.Ansi)]
        static extern IntPtr material_create(string name, IntPtr shader);
        [DllImport(Util.DllName)]
        static extern void material_release(IntPtr material);
        [DllImport(Util.DllName, CharSet = CharSet.Ansi)]
        static extern void material_set_float  (IntPtr material, string name, float   value);
        [DllImport(Util.DllName, CharSet = CharSet.Ansi)]
        static extern void material_set_color32(IntPtr material, string name, Color32 value);
        [DllImport(Util.DllName, CharSet = CharSet.Ansi)]
        static extern void material_set_color  (IntPtr material, string name, Color   value);
        [DllImport(Util.DllName, CharSet = CharSet.Ansi)]
        static extern void material_set_vector (IntPtr material, string name, Vec4    value);
        [DllImport(Util.DllName, CharSet = CharSet.Ansi)]
        static extern void material_set_matrix (IntPtr material, string name, Matrix  value);
        [DllImport(Util.DllName, CharSet = CharSet.Ansi)]
        static extern void material_set_texture(IntPtr material, string name, IntPtr  value);
        #endregion

        internal IntPtr _materialInst;
        public Material(string name, Shader shader)
        {
            _materialInst = material_create(name, shader == null ? IntPtr.Zero : shader._shaderInst);
            if (_materialInst == IntPtr.Zero)
                Console.WriteLine("Couldn't create material!");
        }
        private Material(IntPtr material)
        {
            _materialInst = material;
            if (_materialInst == IntPtr.Zero)
                Console.WriteLine("Received an empty material!");
        }
        ~Material()
        {
            if (_materialInst != IntPtr.Zero)
                material_release(_materialInst);
        }
        public static Material Find(string id)
        {
            IntPtr material = material_find(id);
            return material == IntPtr.Zero ? null : new Material(material);
        }

        public void SetFloat(string name, float value)
        {
            material_set_float(_materialInst, name, value);
        }
        public void SetColor(string name, Color32 value)
        {
            material_set_color32(_materialInst, name, value);
        }
        public void SetColor(string name, Color value)
        {
            material_set_color(_materialInst, name, value);
        }
        public void SetVector(string name, Vec4 value)
        {
            material_set_vector(_materialInst, name, value);
        }
        public void SetMatrix(string name, float value)
        {
            material_set_float(_materialInst, name, value);
        }
        public void SetTexture(string name, Tex2D value)
        {
            material_set_texture(_materialInst, name, value._texInst);
        }
    }
}

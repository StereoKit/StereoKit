using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    public enum AlphaMode
    {
        None = 1,
        Blend,
        Test,
    };
    public enum CullMode
    {
        Ccw = 0,
        Cw,
        None,
    }

    public class Material
    {
        #region Imports
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr material_find(string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr material_create(string name, IntPtr shader);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr material_copy(string name, IntPtr material);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void material_release(IntPtr material);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void material_set_alpha_mode(IntPtr material, AlphaMode mode);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void material_set_cull(IntPtr material, CullMode mode);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void material_set_queue_offset(IntPtr material, int offset);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern void material_set_float  (IntPtr material, string name, float   value);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern void material_set_color32(IntPtr material, string name, Color32 value);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern void material_set_color  (IntPtr material, string name, Color   value);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern void material_set_vector (IntPtr material, string name, Vec4    value);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern void material_set_matrix (IntPtr material, string name, Matrix  value);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern void material_set_texture(IntPtr material, string name, IntPtr  value);
        #endregion

        internal IntPtr _materialInst;
        static int index;

        public AlphaMode Alpha       { set { material_set_alpha_mode  (_materialInst, value); } }
        public CullMode  Cull        { set { material_set_cull        (_materialInst, value); } }
        public int       QueueOffset { set { material_set_queue_offset(_materialInst, value); } }

        public Material(Shader shader)
        {
            index += 1;
            _materialInst = material_create("auto/material"+index, shader == null ? IntPtr.Zero : shader._shaderInst);
        }
        public Material(string name, Shader shader)
        {
            _materialInst = material_create(name, shader == null ? IntPtr.Zero : shader._shaderInst);
        }
        private Material(IntPtr material)
        {
            _materialInst = material;
            if (_materialInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Received an empty material!");
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

        public object this[string parameterName] { set { 
            switch(value)
            {
                case float   f:    SetFloat  (parameterName, f); break;
                case int     i:    SetFloat  (parameterName, i); break;
                case Color32 c32:  SetColor  (parameterName, c32); break;
                case Color   c128: SetColor  (parameterName, c128); break;
                case Vec4    v:    SetVector (parameterName, v); break;
                case Matrix  m:    SetMatrix (parameterName, m); break;
                case Tex2D   t:    SetTexture(parameterName, t); break;
                default: Log.Write(LogLevel.Error, "Invalid material parameter type: {0}", value.GetType().ToString()); break;
            }
        } }

        public Material Copy()
        {
            index += 1;
            material_copy("auto/material" + index, _materialInst);
            return new Material(_materialInst);
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
        public void SetMatrix(string name, Matrix value)
        {
            material_set_matrix(_materialInst, name, value);
        }
        public void SetTexture(string name, Tex2D value)
        {
            material_set_texture(_materialInst, name, value._texInst);
        }
    }
}

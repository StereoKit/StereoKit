using System;

namespace StereoKit
{
    public class Material
    {
        internal IntPtr _materialInst;

        public AlphaMode Alpha       { set { NativeAPI.material_set_alpha_mode  (_materialInst, value); } }
        public CullMode  Cull        { set { NativeAPI.material_set_cull        (_materialInst, value); } }
        public int       QueueOffset { set { NativeAPI.material_set_queue_offset(_materialInst, value); } }

        public Material(Shader shader)
        {
            _materialInst = NativeAPI.material_create(shader == null ? IntPtr.Zero : shader._shaderInst);
        }
        public Material(string name, Shader shader)
        {
            _materialInst = NativeAPI.material_create(shader == null ? IntPtr.Zero : shader._shaderInst);
            NativeAPI.material_set_id(_materialInst, name);
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
                NativeAPI.material_release(_materialInst);
        }
        public static Material Find(string id)
        {
            IntPtr material = NativeAPI.material_find(id);
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
            NativeAPI.material_copy(_materialInst);
            return new Material(_materialInst);
        }

        public void SetFloat(string name, float value)
        {
            NativeAPI.material_set_float(_materialInst, name, value);
        }
        public void SetColor(string name, Color32 value)
        {
            NativeAPI.material_set_color(_materialInst, name, new Color( value.r/255f, value.g/255f, value.b/255f, value.a/255f ));
        }
        public void SetColor(string name, Color value)
        {
            NativeAPI.material_set_color(_materialInst, name, value);
        }
        public void SetVector(string name, Vec4 value)
        {
            NativeAPI.material_set_vector(_materialInst, name, value);
        }
        public void SetMatrix(string name, Matrix value)
        {
            NativeAPI.material_set_matrix(_materialInst, name, value);
        }
        public void SetTexture(string name, Tex2D value)
        {
            NativeAPI.material_set_texture(_materialInst, name, value._texInst);
        }
    }
}

using System;

namespace StereoKit
{
    /// <summary>A Material describes the surface of anything drawn on the graphics 
    /// card! It is typically composed of a Shader, and shader properties like colors,
    /// textures, transparency info, etc.
    /// 
    /// Items drawn with the same Material can be batched together into a single, fast
    /// operation on the graphics card, so re-using materials can be extremely beneficial 
    /// for performance!</summary>
    public class Material
    {
        internal IntPtr _materialInst;

        /// <summary>What type of transparency does this Material use? Default is None. Transparency 
        /// has an impact on performance, and draw order. Check the Transparency enum for details.</summary>
        public Transparency Transparency { set { NativeAPI.material_set_transparency(_materialInst, value); } }
        /// <summary>How should this material cull faces?</summary>
        public Cull         FaceCull     { set { NativeAPI.material_set_cull        (_materialInst, value); } }
        /// <summary>This property will force this material to draw earlier or later in the draw
        /// queue. Positive values make it draw later, negative makes it earlier. This can be helpful 
        /// for tweaking performance! If you know an object is always going to be close to the user
        /// and likely to obscure lots of objects (like hands), drawing it earlier can mean objects behind it
        /// get discarded much faster! Similarly, objects that are far away (skybox!) can be pushed towards the 
        /// back of the queue, so they're more likely to be discarded early.</summary>
        public int          QueueOffset  { set { NativeAPI.material_set_queue_offset(_materialInst, value); } }
        /// <summary>The number of shader parameters available to this material.</summary>
        public int          ParamCount => NativeAPI.material_get_param_count(_materialInst);
        /// <summary>Gets a link to the Shader that the Material is currently using</summary>
        public Shader       Shader => new Shader(NativeAPI.material_get_shader(_materialInst));


        public Material(Shader shader)
        {
            _materialInst = NativeAPI.material_create(shader == null ? IntPtr.Zero : shader._shaderInst);
        }
        public Material(string name, Shader shader)
        {
            _materialInst = NativeAPI.material_create(shader == null ? IntPtr.Zero : shader._shaderInst);
            NativeAPI.material_set_id(_materialInst, name);
        }
        internal Material(IntPtr material)
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

        /// <summary>Creates a new Material asset with the same shader and properties! Draw calls with
        /// the new Material will not batch together with this one.</summary>
        /// <returns>A new Material asset with the same shader and properties.</returns>
        public Material Copy()
        {
            return new Material(NativeAPI.material_copy(_materialInst));
        }
        
        /// <summary>Sets a shader parameter with the given name to the provided value. If no parameter
        /// is found, nothing happens, and the value is not set!</summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">New value for the parameter.</param>
        public void SetFloat(string name, float value)
        {
            NativeAPI.material_set_float(_materialInst, name, value);
        }
        /// <summary>Sets a shader parameter with the given name to the provided value. If no parameter
        /// is found, nothing happens, and the value is not set!</summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">New value for the parameter.</param>
        public void SetColor(string name, Color32 value)
        {
            NativeAPI.material_set_color(_materialInst, name, new Color( value.r/255f, value.g/255f, value.b/255f, value.a/255f ));
        }
        /// <summary>Sets a shader parameter with the given name to the provided value. If no parameter
        /// is found, nothing happens, and the value is not set!</summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">New value for the parameter.</param>
        public void SetColor(string name, Color value)
        {
            NativeAPI.material_set_color(_materialInst, name, value);
        }
        /// <summary>Sets a shader parameter with the given name to the provided value. If no parameter
        /// is found, nothing happens, and the value is not set!</summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">New value for the parameter.</param>
        public void SetVector(string name, Vec4 value)
        {
            NativeAPI.material_set_vector(_materialInst, name, value);
        }
        /// <summary>Sets a shader parameter with the given name to the provided value. If no parameter
        /// is found, nothing happens, and the value is not set!</summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">New value for the parameter.</param>
        public void SetMatrix(string name, Matrix value)
        {
            NativeAPI.material_set_matrix(_materialInst, name, value);
        }
        /// <summary>Sets a shader parameter with the given name to the provided value. If no parameter
        /// is found, nothing happens, and the value is not set!</summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">New value for the parameter.</param>
        public void SetTexture(string name, Tex2D value)
        {
            NativeAPI.material_set_texture(_materialInst, name, value._texInst);
        }

        public static Material Find(string id)
        {
            IntPtr material = NativeAPI.material_find(id);
            return material == IntPtr.Zero ? null : new Material(material);
        }
        /// <summary>Creates a new Material asset with the same shader and properties! Draw calls with
        /// the new Material will not batch togethre with the source Material.</summary>
        /// <returns>A new Material asset with the same shader and properties. Returns null if no
        /// materials are found with the given id.</returns>
        public static Material Copy(string materialId)
        {
            IntPtr inst = NativeAPI.material_copy_id(materialId);
            return inst == IntPtr.Zero ? null : new Material(inst);
        }
    }
}

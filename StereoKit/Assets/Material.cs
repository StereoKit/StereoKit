using System;

namespace StereoKit
{
    /// <summary>An better way to access standard shader paramter names, instead
    /// of using just strings! If you have your own custom parameters, you can still 
    /// access them via the string methods, but this is checked and verified by the
    /// compiler!</summary>
    public enum MatParamName
    {
        /// <summary>The primary color texture for the shader! Diffuse, Albedo,
        /// 'The Texture', or whatever you want to call it, this is usually the
        /// base color that the shader works with. 
        /// This represents the texture param 'diffuse'.</summary>
        DiffuseTex = 0,
        /// <summary>This texture is unaffected by lighting, and is frequently just
        /// added in on top of the material's final color! Tends to look really glowy.
        /// This represents the texture param 'emission'.</summary>
        EmissionTex,
        /// <summary>For physically based shaders, metal is a texture that encodes
        /// metallic and roughness data into the 'B' and 'G' channels, respectively.
        /// This represents the texture param 'metal'.</summary>
        MetalTex,
        /// <summary>The 'normal map' texture for the material! This texture contains
        /// information about the direction of the material's surface, which is used
        /// to calculate lighting, and make surfaces look like they have more detail
        /// than they actually do. Normals are in Tangent Coordinate Space, and the RGB
        /// values map to XYZ values.
        /// This represents the texture param 'normal'.</summary>
        NormalTex,
        /// <summary>Used by physically based shaders, this can be used for baked ambient 
        /// occlusion lighting, or to remove specular reflections from areas that are
        /// surrounded by geometry that would likely block reflections.
        /// This represents the texture param 'occlusion'.</summary>
        OcclusionTex,
        /// <summary>A per-material color tint, behavior could vary from shader to shader,
        /// but often this is just multiplied against the diffuse texture right at the start.
        /// This represents the Color param 'color'.</summary>
        ColorTint,
        /// <summary>For physically based shader, this is a multiplier to scale the metallic
        /// properties of the material.
        /// This represents the float param 'metallic'.</summary>
        MetallicAmount,
        /// <summary>For physically based shader, this is a multiplier to scale the roughness
        /// properties of the material.
        /// This represents the float param 'roughness'.</summary>
        RoughnessAmount,
        /// <summary>Not necessarily present in all shaders, this multiplies the UV coordinates
        /// of the mesh, so that the texture will repeat. This is great for tiling textures!
        /// This represents the float param 'tex_scale'.</summary>
        TexScale
    }

    /// <summary>A Material describes the surface of anything drawn on the graphics 
    /// card! It is typically composed of a Shader, and shader properties like colors,
    /// textures, transparency info, etc.
    /// 
    /// Items drawn with the same Material can be batched together into a single, fast
    /// operation on the graphics card, so re-using materials can be extremely beneficial 
    /// for performance!</summary>
    public class Material
    {
        internal IntPtr _inst;

        /// <summary>What type of transparency does this Material use? Default is None. Transparency 
        /// has an impact on performance, and draw order. Check the Transparency enum for details.</summary>
        public Transparency Transparency { set { NativeAPI.material_set_transparency(_inst, value); } }
        /// <summary>How should this material cull faces?</summary>
        public Cull         FaceCull     { set { NativeAPI.material_set_cull        (_inst, value); } }
        /// <summary>This property will force this material to draw earlier or later in the draw
        /// queue. Positive values make it draw later, negative makes it earlier. This can be helpful 
        /// for tweaking performance! If you know an object is always going to be close to the user
        /// and likely to obscure lots of objects (like hands), drawing it earlier can mean objects behind it
        /// get discarded much faster! Similarly, objects that are far away (skybox!) can be pushed towards the 
        /// back of the queue, so they're more likely to be discarded early.</summary>
        public int          QueueOffset  { set { NativeAPI.material_set_queue_offset(_inst, value); } }
        /// <summary>The number of shader parameters available to this material.</summary>
        public int          ParamCount => NativeAPI.material_get_param_count(_inst);
        /// <summary>Gets a link to the Shader that the Material is currently using</summary>
        public Shader       Shader => new Shader(NativeAPI.material_get_shader(_inst));

        /// <summary>Creates a material from a shader, and uses the shader's default settings. Uses an auto-generated id.</summary>
        /// <param name="shader">Any valid shader.</param>
        public Material(Shader shader)
        {
            _inst = NativeAPI.material_create(shader == null ? IntPtr.Zero : shader._inst);
        }
        /// <summary>Creates a material from a shader, and uses the shader's default settings.</summary>
        /// <param name="id">Set the material's id to this.</param>
        /// <param name="shader">Any valid shader.</param>
        public Material(string id, Shader shader)
        {
            _inst = NativeAPI.material_create(shader == null ? IntPtr.Zero : shader._inst);
            NativeAPI.material_set_id(_inst, id);
        }
        internal Material(IntPtr material)
        {
            _inst = material;
            if (_inst == IntPtr.Zero)
                Log.Err("Received an empty material!");
        }
        ~Material()
        {
            if (_inst != IntPtr.Zero)
                NativeAPI.material_release(_inst);
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
                case Tex     t:    SetTexture(parameterName, t); break;
                default: Log.Err("Invalid material parameter type: {0}", value.GetType().ToString()); break;
            }
        } }
        public object this[MatParamName parameter] { set { this[MaterialParamString(parameter)] = value; } }
        private string MaterialParamString(MatParamName parameter)
        {
            switch (parameter)
            {
                case MatParamName.ColorTint:       return "color";
                case MatParamName.DiffuseTex:      return "diffuse";
                case MatParamName.EmissionTex:     return "emission";
                case MatParamName.MetallicAmount:  return "metallic";
                case MatParamName.MetalTex:        return "metal";
                case MatParamName.NormalTex:       return "normal";
                case MatParamName.OcclusionTex:    return "occlusion";
                case MatParamName.RoughnessAmount: return "roughness";
                case MatParamName.TexScale:        return "tex_scale";
                default: Log.Err("Unimplemented Material Parameter Name! " + parameter); return "";
            }
        }

        /// <summary>Creates a new Material asset with the same shader and properties! Draw calls with
        /// the new Material will not batch together with this one.</summary>
        /// <returns>A new Material asset with the same shader and properties.</returns>
        public Material Copy()
        {
            return new Material(NativeAPI.material_copy(_inst));
        }
        
        /// <summary>Sets a shader parameter with the given name to the provided value. If no parameter
        /// is found, nothing happens, and the value is not set!</summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">New value for the parameter.</param>
        public void SetFloat(string name, float value)
        {
            NativeAPI.material_set_float(_inst, name, value);
        }
        /// <summary>Sets a shader parameter with the given name to the provided value. If no parameter
        /// is found, nothing happens, and the value is not set!</summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">New value for the parameter.</param>
        public void SetColor(string name, Color32 value)
        {
            NativeAPI.material_set_color(_inst, name, new Color( value.r/255f, value.g/255f, value.b/255f, value.a/255f ));
        }
        /// <summary>Sets a shader parameter with the given name to the provided value. If no parameter
        /// is found, nothing happens, and the value is not set!</summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">New value for the parameter.</param>
        public void SetColor(string name, Color value)
        {
            NativeAPI.material_set_color(_inst, name, value);
        }
        /// <summary>Sets a shader parameter with the given name to the provided value. If no parameter
        /// is found, nothing happens, and the value is not set!</summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">New value for the parameter.</param>
        public void SetVector(string name, Vec4 value)
        {
            NativeAPI.material_set_vector(_inst, name, value);
        }
        /// <summary>Sets a shader parameter with the given name to the provided value. If no parameter
        /// is found, nothing happens, and the value is not set!</summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">New value for the parameter.</param>
        public void SetMatrix(string name, Matrix value)
        {
            NativeAPI.material_set_matrix(_inst, name, value);
        }
        /// <summary>Sets a shader parameter with the given name to the provided value. If no parameter
        /// is found, nothing happens, and the value is not set!</summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">New value for the parameter.</param>
        public void SetTexture(string name, Tex value)
        {
            NativeAPI.material_set_texture(_inst, name, value._inst);
        }

        /// <summary>Looks for a Material asset that's already loaded, matching the given id!</summary>
        /// <param name="materialId">Which Material are you looking for?</param>
        /// <returns>A link to the Material matching 'id', null if none is found.</returns>
        public static Material Find(string materialId)
        {
            IntPtr material = NativeAPI.material_find(materialId);
            return material == IntPtr.Zero ? null : new Material(material);
        }
        /// <summary>Creates a new Material asset with the same shader and properties! Draw calls with
        /// the new Material will not batch togethre with the source Material.</summary>
        /// /// <param name="materialId">Which Material are you looking for?</param>
        /// <returns>A new Material asset with the same shader and properties. Returns null if no
        /// materials are found with the given id.</returns>
        public static Material Copy(string materialId)
        {
            IntPtr inst = NativeAPI.material_copy_id(materialId);
            return inst == IntPtr.Zero ? null : new Material(inst);
        }
    }
}

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>A better way to access standard shader parameter names,
	/// instead of using just strings! If you have your own custom
	/// parameters, you can still access them via the string methods, but
	/// this is checked and verified by the compiler!</summary>
	public enum MatParamName
	{
		/// <summary>The primary color texture for the shader! Diffuse, 
		/// Albedo, 'The Texture', or whatever you want to call it, this is
		/// usually the base color that the shader works with.
		/// 
		/// This represents the texture param 'diffuse'.</summary>
		DiffuseTex = 0,
		/// <summary>This texture is unaffected by lighting, and is
		/// frequently just added in on top of the material's final color!
		/// Tends to look really glowy.
		/// 
		/// This represents the texture param 'emission'.</summary>
		EmissionTex,
		/// <summary>For physically based shaders, metal is a texture that
		/// encodes metallic and roughness data into the 'B' and 'G'
		/// channels, respectively.
		/// 
		/// This represents the texture param 'metal'.</summary>
		MetalTex,
		/// <summary>The 'normal map' texture for the material! This texture
		/// contains information about the direction of the material's
		/// surface, which is used to calculate lighting, and make surfaces
		/// look like they have more detail than they actually do. Normals
		/// are in Tangent Coordinate Space, and the RGB values map to XYZ
		/// values.
		/// 
		/// This represents the texture param 'normal'.</summary>
		NormalTex,
		/// <summary>Used by physically based shaders, this can be used for
		/// baked ambient occlusion lighting, or to remove specular
		/// reflections from areas that are surrounded by geometry that would
		/// likely block reflections.
		/// 
		/// This represents the texture param 'occlusion'.</summary>
		OcclusionTex,
		/// <summary>A per-material color tint, behavior could vary from
		/// shader to shader, but often this is just multiplied against the
		/// diffuse texture right at the start.
		/// 
		/// This represents the Color param 'color'.</summary>
		ColorTint,
		/// <summary>A multiplier for emission values sampled from the emission
		/// texture. The default emission texture in SK shaders is white, and
		/// the default value for this parameter is 0,0,0,0.
		/// 
		/// This represents the Color param 'emission_factor'.</summary>
		EmissionFactor,
		/// <summary>For physically based shader, this is a multiplier to
		/// scale the metallic properties of the material.
		/// 
		/// This represents the float param 'metallic'.</summary>
		MetallicAmount,
		/// <summary>For physically based shader, this is a multiplier to
		/// scale the roughness properties of the material.
		/// 
		/// This represents the float param 'roughness'.</summary>
		RoughnessAmount,
		/// <summary>Not necessarily present in all shaders, this multiplies
		/// the UV coordinates of the mesh, so that the texture will repeat.
		/// This is great for tiling textures!
		/// 
		/// This represents the float param 'tex_scale'.</summary>
		TexScale,
		/// <summary>In clip shaders, this is the cutoff value below which
		/// pixels are discarded. Typically, the diffuse/albedo's alpha
		/// component is sampled for comparison here.
		/// 
		/// This represents the float param 'cutoff'.</summary>
		ClipCutoff
	}

	/// <summary>Information and details about the shader parameters
	/// available on a Material. Currently only the text name and data type
	/// of the parameter are surfaced here. This contains textures as well as
	/// global constant buffer variables.</summary>
	public struct MatParamInfo
	{
		/// <summary>The name of the shader parameter, as provided inside of
		/// the shader file itself. These are the 'global' variables defined
		/// in the shader, as well as textures. The shader compiler will
		/// output a list of parameter names when compiling, so check the
		/// output window after building if you're uncertain what you'll
		/// find.
		/// 
		/// See `MatParamName` for a list of "standardized" parameter names.
		/// </summary>
		public string name;

		/// <summary>This is the data type that StereoKit recognizes the
		/// parameter to be.</summary>
		public MaterialParam type;

		internal MatParamInfo(string name, MaterialParam type)
		{
			this.name = name;
			this.type = type;
		}
	}

	/// <summary>A Material describes the surface of anything drawn on the 
	/// graphics card! It is typically composed of a Shader, and shader 
	/// properties like colors, textures, transparency info, etc.
	/// 
	/// Items drawn with the same Material can be batched together into a 
	/// single, fast operation on the graphics card, so re-using materials 
	/// can be extremely beneficial for performance!</summary>
	public class Material : IAsset
	{
		internal IntPtr _inst;

		/// <summary>What type of transparency does this Material use?
		/// Default is None. Transparency has an impact on performance, and
		/// draw order. Check the Transparency enum for details.</summary>
		public Transparency Transparency {
			get => NativeAPI.material_get_transparency(_inst); 
			set => NativeAPI.material_set_transparency(_inst, value); }
		/// <summary>How should this material cull faces?</summary>
		public Cull FaceCull {
			get => NativeAPI.material_get_cull(_inst);
			set => NativeAPI.material_set_cull(_inst, value); }
		/// <summary>Should this material draw only the edges/wires of the 
		/// mesh? This can be useful for debugging, and even some kinds of
		/// visualization work. Note that this may not work on some mobile
		/// OpenGL systems like Quest.</summary>
		public bool Wireframe {
			get => NativeAPI.material_get_wireframe(_inst)>0;
			set => NativeAPI.material_set_wireframe(_inst, value); }
		/// <summary>How does this material interact with the ZBuffer? 
		/// Generally DepthTest.Less would be normal behavior: don't draw
		/// objects that are occluded. But this can also be used to achieve
		/// some interesting effects, like you could use DepthTest.Greater 
		/// to draw a glow that indicates an object is behind something.
		/// </summary>
		public DepthTest DepthTest {
			get => NativeAPI.material_get_depth_test(_inst);
			set => NativeAPI.material_set_depth_test(_inst, value); }
		/// <summary>Should this material write to the ZBuffer? For opaque
		/// objects, this generally should be true. But transparent objects 
		/// writing to the ZBuffer can be problematic and cause draw order 
		/// issues. Note that turning this off can mean that this material
		/// won't get properly accounted for when the MR system is performing
		/// late stage reprojection.
		/// 
		/// Not writing to the buffer can also be faster! :)</summary>
		public bool DepthWrite {
			get => NativeAPI.material_get_depth_write(_inst)>0;
			set => NativeAPI.material_set_depth_write(_inst, value); }
		/// <summary>This property will force this material to draw earlier
		/// or later in the draw queue. Positive values make it draw later, 
		/// negative makes it earlier. This can be helpful for tweaking 
		/// performance! If you know an object is always going to be close to 
		/// the user and likely to obscure lots of objects (like hands), 
		/// drawing it earlier can mean objects behind it get discarded much 
		/// faster! Similarly, objects that are far away (skybox!) can be 
		/// pushed towards the back of the queue, so they're more likely to 
		/// be discarded early.</summary>
		public int QueueOffset {
			get => NativeAPI.material_get_queue_offset(_inst);
			set => NativeAPI.material_set_queue_offset(_inst, value); }
		/// <summary>Allows you to chain Materials together in a form of
		/// multi-pass rendering! Any time the Material is used, the chained
		/// Materials will also be used to draw the same item.</summary>
		public Material Chain {
			get { IntPtr mat = NativeAPI.material_get_chain(_inst); return mat == IntPtr.Zero ? null : new Material(mat); }
			set { NativeAPI.material_set_chain(_inst, value?._inst ?? IntPtr.Zero); } }
		/// <summary>The number of shader parameters available to this 
		/// material, includes global shader variables as well as textures.
		/// </summary>
		public int ParamCount => NativeAPI.material_get_param_count(_inst);
		/// <summary>Gets a link to the Shader that the Material is currently 
		/// using, or overrides the Shader this material uses.</summary>
		public Shader Shader { 
			get => new Shader(NativeAPI.material_get_shader(_inst));
			set => NativeAPI.material_set_shader(_inst, value._inst); }
		/// <summary>Gets or sets the unique identifier of this asset resource!
		/// This can be helpful for debugging, managine your assets, or finding
		/// them later on!</summary>
		public string Id { 
			get => Marshal.PtrToStringAnsi(NativeAPI.material_get_id(_inst));
			set => NativeAPI.material_set_id(_inst, value); }

		/// <summary>Creates a material from a shader, and uses the shader's 
		/// default settings. Uses an auto-generated id.</summary>
		/// <param name="shader">Any valid shader.</param>
		public Material(Shader shader)
		{
			_inst = NativeAPI.material_create(shader == null ? IntPtr.Zero : shader._inst);
		}
		/// <summary>Creates a material from a shader, and uses the shader's 
		/// default settings.</summary>
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
		/// <summary>Release reference to the StereoKit asset.</summary>
		~Material()
		{
			if (_inst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe(_inst);
		}

		public object this[string parameterName] { set { 
			switch(value)
			{
				case float   f:    SetFloat  (parameterName, f); break;
				case int     f:    SetFloat  (parameterName, f); Log.Warn($"Material param '{parameterName}' was provided as int and is being converted to float. Please use SetInt to actually set an integer value, or use the .0f postfix to indicate a float."); break;
				case Color32 c32:  SetColor  (parameterName, c32); break;
				case Color   c128: SetColor  (parameterName, c128); break;
				case Vec4    v:    SetVector (parameterName, v); break;
				case Vec3    v:    SetVector (parameterName, v); break;
				case Vec2    v:    SetVector (parameterName, v); break;
				case bool    v:    SetBool   (parameterName, v); break;
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
				case MatParamName.EmissionFactor:  return "emission_factor";
				case MatParamName.DiffuseTex:      return "diffuse";
				case MatParamName.EmissionTex:     return "emission";
				case MatParamName.MetallicAmount:  return "metallic";
				case MatParamName.MetalTex:        return "metal";
				case MatParamName.NormalTex:       return "normal";
				case MatParamName.OcclusionTex:    return "occlusion";
				case MatParamName.RoughnessAmount: return "roughness";
				case MatParamName.TexScale:        return "tex_scale";
				case MatParamName.ClipCutoff:      return "cutoff";
				default: Log.Err("Unimplemented Material Parameter Name! " + parameter); return "";
			}
		}

		/// <summary>Gets an enumerable list of all parameter information on
		/// the Material. This includes all global shader variables and
		/// textures.</summary>
		/// <returns>A pretty standard IEnumarable of MatParamInfo that can
		/// be used with foreach.</returns>
		public IEnumerable<MatParamInfo> GetAllParamInfo()
		{
			int count = ParamCount;
			for (int i = 0; i < count; i++)
			{
				NativeAPI.material_get_param_info(_inst, i, out IntPtr name, out MaterialParam type);
				yield return new MatParamInfo(Marshal.PtrToStringAnsi(name), type);
			}
		}

		/// <summary>Gets available shader parameter information for the
		/// parameter at the indicated index. Parameters are listed as
		/// variables first, then textures.</summary>
		/// <param name="index">Index of the shader parameter, bounded by
		/// ParamCount.</param>
		/// <returns>A structure that contains all the available information
		/// about the parameter.</returns>
		public MatParamInfo GetParamInfo(int index)
		{
			if (index < 0 || index >= ParamCount)
				throw new IndexOutOfRangeException();

			NativeAPI.material_get_param_info(_inst, index, out IntPtr name, out MaterialParam type);
			return new MatParamInfo(Marshal.PtrToStringAnsi(name), type);
		}

		/// <summary>Creates a new Material asset with the same shader and
		/// properties! Draw calls with the new Material will not batch
		/// together with this one.</summary>
		/// <returns>A new Material asset with the same shader and 
		/// properties.</returns>
		public Material Copy()
			=> new Material(NativeAPI.material_copy(_inst));

		/// <summary>Sets a shader parameter with the given name to the 
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value">New value for the parameter.</param>
		public void SetFloat(string name, float value)
			=> NativeAPI.material_set_float(_inst, name, value);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="colorGamma">The gamma space color for the shader
		/// to use.</param>
		public void SetColor(string name, Color32 colorGamma)
			=> NativeAPI.material_set_color(_inst, name, new Color(colorGamma.r/255f, colorGamma.g/255f, colorGamma.b/255f, colorGamma.a/255f ));

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="colorGamma">The gamma space color for the shader
		/// to use.</param>
		public void SetColor(string name, Color colorGamma)
			=> NativeAPI.material_set_color(_inst, name, colorGamma);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value">New value for the parameter.</param>
		public void SetVector(string name, Vec4 value)
			=> NativeAPI.material_set_vector4(_inst, name, value);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value">New value for the parameter.</param>
		public void SetVector(string name, Vec3 value)
			=> NativeAPI.material_set_vector3(_inst, name, value);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value">New value for the parameter.</param>
		public void SetVector(string name, Vec2 value)
			=> NativeAPI.material_set_vector2(_inst, name, value);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value">New value for the parameter.</param>
		public void SetInt(string name, int value)
			=> NativeAPI.material_set_int(_inst, name, value);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value1">New value for the parameter.</param>
		/// <param name="value2">New value for the parameter.</param>
		public void SetInt(string name, int value1, int value2)
			=> NativeAPI.material_set_int2(_inst, name, value1, value2);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value1">New value for the parameter.</param>
		/// <param name="value2">New value for the parameter.</param>
		/// <param name="value3">New value for the parameter.</param>
		public void SetInt(string name, int value1, int value2, int value3)
			=> NativeAPI.material_set_int3(_inst, name, value1, value2, value3);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value1">New value for the parameter.</param>
		/// <param name="value2">New value for the parameter.</param>
		/// <param name="value3">New value for the parameter.</param>
		/// <param name="value4">New value for the parameter.</param>
		public void SetInt(string name, int value1, int value2, int value3, int value4)
			=> NativeAPI.material_set_int4(_inst, name, value1, value2, value3, value4);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value">New value for the parameter.</param>
		public void SetUInt(string name, uint value)
			=> NativeAPI.material_set_uint(_inst, name, value);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value1">New value for the parameter.</param>
		/// <param name="value2">New value for the parameter.</param>
		public void SetUInt(string name, uint value1, uint value2)
			=> NativeAPI.material_set_uint2(_inst, name, value1, value2);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value1">New value for the parameter.</param>
		/// <param name="value2">New value for the parameter.</param>
		/// <param name="value3">New value for the parameter.</param>
		public void SetUInt(string name, uint value1, uint value2, uint value3)
			=> NativeAPI.material_set_uint3(_inst, name, value1, value2, value3);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value1">New value for the parameter.</param>
		/// <param name="value2">New value for the parameter.</param>
		/// <param name="value3">New value for the parameter.</param>
		/// <param name="value4">New value for the parameter.</param>
		public void SetUInt(string name, uint value1, uint value2, uint value3, uint value4)
			=> NativeAPI.material_set_uint4(_inst, name, value1, value2, value3, value4);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value">New value for the parameter.</param>
		public void SetBool(string name, bool value)
			=> NativeAPI.material_set_bool(_inst, name, value?1:0);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value">New value for the parameter.</param>
		public void SetMatrix(string name, Matrix value)
			=> NativeAPI.material_set_matrix(_inst, name, value);

		/// <summary>Sets a shader parameter with the given name to the
		/// provided value. If no parameter is found, nothing happens, and
		/// the value is not set!</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="value">New value for the parameter.</param>
		public void SetTexture(string name, Tex value)
			=> NativeAPI.material_set_texture(_inst, name, value._inst);

		/// <summary>This allows you to set more complex shader data types such
		/// as structs. Note the SK doesn't guard against setting data of the
		/// wrong size here, so pay extra attention to the size of your data
		/// here, and ensure it matched up with the shader!</summary>
		/// <typeparam name="T">a struct that uses the 
		/// `[StructLayout(LayoutKind.Sequential)]` attribute for proper
		/// copying.</typeparam>
		/// <param name="name">Name of the shader parameter.</param>
		/// <param name="serializableData">New value for the parameter.</param>
		public void SetData<T>(string name, in T serializableData) where T : struct
		{
			if (!(typeof(T).IsLayoutSequential || typeof(T).IsExplicitLayout))
				throw new NotSupportedException("Material.SetData's data type must have a '[StructLayout(LayoutKind.Sequential)]' attribute for proper copying! Explicit would work too.");

			int    size   = Marshal.SizeOf(typeof(T));
			IntPtr memory = Marshal.AllocHGlobal(size);
			Marshal.StructureToPtr(serializableData, memory, false);

			NativeAPI.material_set_param(_inst, name, MaterialParam.Unknown, memory);

			Marshal.FreeHGlobal(memory);
		}

		/// <summary>Gets the value of a shader parameter with the given name.
		/// If no parameter is found, a default value of '0' will be returned.
		/// </summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <returns>The parameter's value, if present, otherwise 0.</returns>
		public float GetFloat(string name) => NativeAPI.material_get_float(_inst, name);

		/// <summary>Gets the value of a shader parameter with the given name.
		/// If no parameter is found, a default value of Vec2.Zero will be
		/// returned.</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <returns>The parameter's value, if present, otherwise
		/// Vec2.Zero.</returns>
		public Vec2 GetVector2(string name) => NativeAPI.material_get_vector2(_inst, name);

		/// <summary>Gets the value of a shader parameter with the given name.
		/// If no parameter is found, a default value of Vec3.Zero will be
		/// returned.</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <returns>The parameter's value, if present, otherwise
		/// Vec3.Zero.</returns>
		public Vec3 GetVector3(string name) => NativeAPI.material_get_vector3(_inst, name);

		/// <summary>Gets the value of a shader parameter with the given name.
		/// If no parameter is found, a default value of Vec4.Zero will be
		/// returned.</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <returns>The parameter's value, if present, otherwise
		/// Vec4.Zero.</returns>
		public Vec4 GetVector4(string name) => NativeAPI.material_get_vector4(_inst, name);

		/// <summary>Gets the value of a shader parameter with the given name.
		/// If no parameter is found, a default value of Color.White will be
		/// returned.</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <returns>The parameter's value, if present, otherwise
		/// Color.White.</returns>
		public Color GetColor(string name) => NativeAPI.material_get_color(_inst, name);

		/// <summary>Gets the value of a shader parameter with the given name.
		/// If no parameter is found, a default value of '0' will be returned.
		/// </summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <returns>The parameter's value, if present, otherwise 0.</returns>
		public int GetInt(string name) => NativeAPI.material_get_int(_inst, name);

		/// <summary>Gets the value of a shader parameter with the given name.
		/// If no parameter is found, a default value of 'false' will be
		/// returned.</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <returns>The parameter's value, if present, otherwise false.
		/// </returns>
		public bool GetBool(string name) => NativeAPI.material_get_bool(_inst, name)>0;

		/// <summary>Gets the value of a shader parameter with the given name.
		/// If no parameter is found, a default value of '0' will be returned.
		/// </summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <returns>The parameter's value, if present, otherwise 0.</returns>
		public uint GetUInt(string name) => NativeAPI.material_get_uint(_inst, name);

		/// <summary>Gets the value of a shader parameter with the given name.
		/// If no parameter is found, a default value of Matrix.Identity will
		/// be returned.</summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <returns>The parameter's value, if present, otherwise 
		/// Matrix.Identity.</returns>
		public Matrix GetMatrix(string name) => NativeAPI.material_get_matrix(_inst, name);

		/// <summary>Gets the value of a shader parameter with the given name.
		/// If no parameter is found, a default value of null will be returned.
		/// </summary>
		/// <param name="name">Name of the shader parameter.</param>
		/// <returns>The parameter's value, if present, otherwise null.
		/// </returns>
		public Tex GetTexture(string name) {
			IntPtr result = NativeAPI.material_get_texture(_inst, name);
			return result == IntPtr.Zero
				? null
				: new Tex(result);
		}

		/// <summary>Looks for a Material asset that's already loaded,
		/// matching the given id!</summary>
		/// <param name="materialId">Which Material are you looking for?
		/// </param>
		/// <returns>A link to the Material matching 'id', null if none is 
		/// found.</returns>
		public static Material Find(string materialId)
		{
			IntPtr material = NativeAPI.material_find(materialId);
			return material == IntPtr.Zero ? null : new Material(material);
		}
		/// <summary>Creates a new Material asset with the same shader and
		/// properties! Draw calls with the new Material will not batch 
		/// together with the source Material.</summary>
		/// /// <param name="materialId">Which Material are you looking for?
		/// </param>
		/// <returns>A new Material asset with the same shader and 
		/// properties. Returns null if no materials are found with the given
		/// id.</returns>
		public static Material Copy(string materialId)
		{
			IntPtr inst = NativeAPI.material_copy_id(materialId);
			return inst == IntPtr.Zero ? null : new Material(inst);
		}

		/// <inheritdoc cref="StereoKit.Default.Material" />
		public static Material Default => StereoKit.Default.Material;
		/// <inheritdoc cref="StereoKit.Default.MaterialPBR" />
		public static Material PBR => StereoKit.Default.MaterialPBR;
		/// <inheritdoc cref="StereoKit.Default.MaterialPBRClip" />
		public static Material PBRClip => StereoKit.Default.MaterialPBRClip;
		/// <inheritdoc cref="StereoKit.Default.MaterialUI" />
		public static Material UI => StereoKit.Default.MaterialUI;
		/// <inheritdoc cref="StereoKit.Default.MaterialUIBox" />
		public static Material UIBox => StereoKit.Default.MaterialUIBox;
		/// <inheritdoc cref="StereoKit.Default.MaterialUIQuadrant" />
		public static Material UIQuadrant => StereoKit.Default.MaterialUIQuadrant;
		/// <inheritdoc cref="StereoKit.Default.MaterialUnlit" />
		public static Material Unlit => StereoKit.Default.MaterialUnlit;
		/// <inheritdoc cref="StereoKit.Default.MaterialUnlitClip" />
		public static Material UnlitClip => StereoKit.Default.MaterialUnlitClip;
	}
}

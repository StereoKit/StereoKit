using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>Compute shaders allow you to run code on the GPU in a
	/// massively parallel way! This is great for accelerating complex work, or
	/// simply for working inline with the graphics pipeline with easy access
	/// to GPU memory.
	/// 
	/// This behaves very much like Materials do! You can set parameters, and
	/// attach buffers or textures! Unlike Materials, you need to dispatch the
	/// compute shader to make it run. You may need to be a bit cautious about
	/// compute data, since the GPU can be picky about what and when it reads
	/// and writes to!</summary>
	public class Compute : IAsset
	{
		internal IntPtr _inst;

		/// <summary>Gets or sets the unique identifier of this asset resource!
		/// This can be helpful for debugging, managing your assets, or finding
		/// them later on!</summary>
		public string Id
		{
			get => Marshal.PtrToStringAnsi(NativeAPI.compute_get_id(_inst));
			set => NativeAPI.compute_set_id(_inst, value);
		}

		/// <summary>The shader associated with this compute object. Each
		/// access here creates a new reference!</summary>
		public Shader Shader => new Shader(NativeAPI.compute_get_shader(_inst));

		/// <summary>Create a Compute dispatch from a shader that has a
		/// compute stage! If the shader doesn't have a compute stage,
		/// this will fail.</summary>
		/// <param name="computeShader">A shader containing a compute
		/// stage.</param>
		public Compute(Shader computeShader)
		{
			_inst = NativeAPI.compute_create(computeShader == null ? IntPtr.Zero : computeShader._inst);
			if (_inst == IntPtr.Zero)
				Log.Err("Failed to create Compute!");
		}

		/// <summary>Create a Compute dispatch from a shader file! The
		/// file should be a compiled .sks shader with a compute stage.
		/// </summary>
		/// <param name="shaderFilename">The filename of a compiled
		/// shader asset containing a compute stage.</param>
		public Compute(string shaderFilename)
		{
			Shader shader = Shader.FromFile(shaderFilename);
			_inst = NativeAPI.compute_create(shader == null ? IntPtr.Zero : shader._inst);
			if (_inst == IntPtr.Zero)
				Log.Err("Failed to create Compute!");
		}

		internal Compute(IntPtr inst)
		{
			_inst = inst;
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty compute!");
		}

		/// <summary>Release reference to the StereoKit asset.</summary>
		~Compute()
		{
			if (_inst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe(_inst);
		}

		/// <summary>This is the same as calling the type-specific Set
		/// method, but the type is inferred from the value! This is
		/// set-only, and works with float, int, uint, Color, Color32,
		/// Vec2, Vec3, Vec4, bool, Matrix, and Tex.</summary>
		/// <param name="parameterName">Name of the parameter in HLSL,
		/// must match exactly!</param>
		/// <returns>This is set only.</returns>
		public object this[string parameterName] { set {
			switch (value)
			{
				case float   f:   SetFloat  (parameterName, f); break;
				case int     i:   SetInt    (parameterName, i); break;
				case uint    u:   SetUInt   (parameterName, u); break;
				case Color32 c32: SetColor  (parameterName, c32); break;
				case Color   c:   SetColor  (parameterName, c); break;
				case Vec4    v:   SetVector (parameterName, v); break;
				case Vec3    v:   SetVector (parameterName, v); break;
				case Vec2    v:   SetVector (parameterName, v); break;
				case bool    b:   SetBool   (parameterName, b); break;
				case Matrix  m:   SetMatrix (parameterName, m); break;
				case Tex     t:   SetTexture(parameterName, t); break;
				default: Log.Err($"Invalid compute parameter type: {value.GetType()}"); break;
			}
		} }

		/// <summary>Set a shader parameter by name! The name must match
		/// a variable in the HLSL compute shader exactly, and if no
		/// match is found, nothing happens. Same as Material!</summary>
		public void SetFloat(string name, float value)
			=> NativeAPI.compute_set_float(_inst, name, value);
		/// <inheritdoc cref="SetFloat"/>
		public void SetInt(string name, int value)
			=> NativeAPI.compute_set_int(_inst, name, value);
		/// <inheritdoc cref="SetFloat"/>
		public void SetUInt(string name, uint value)
			=> NativeAPI.compute_set_uint(_inst, name, value);
		/// <inheritdoc cref="SetFloat"/>
		public void SetVector(string name, Vec2 value)
			=> NativeAPI.compute_set_vector2(_inst, name, value);
		/// <inheritdoc cref="SetFloat"/>
		public void SetVector(string name, Vec3 value)
			=> NativeAPI.compute_set_vector3(_inst, name, value);
		/// <inheritdoc cref="SetFloat"/>
		public void SetVector(string name, Vec4 value)
			=> NativeAPI.compute_set_vector4(_inst, name, value);
		/// <summary>Set a color parameter by name! Color is converted
		/// from gamma to linear space, so what the shader receives will
		/// be in linear. If you're working in linear already, use
		/// SetVector with a Vec4 instead!</summary>
		public void SetColor(string name, Color colorGamma)
			=> NativeAPI.compute_set_color(_inst, name, colorGamma);
		/// <inheritdoc cref="SetColor(string, Color)"/>
		public void SetColor(string name, Color32 colorGamma)
			=> NativeAPI.compute_set_color(_inst, name, new Color(colorGamma.r/255f, colorGamma.g/255f, colorGamma.b/255f, colorGamma.a/255f));
		/// <inheritdoc cref="SetFloat"/>
		public void SetBool(string name, bool value)
			=> NativeAPI.compute_set_bool(_inst, name, value);
		/// <inheritdoc cref="SetFloat"/>
		public void SetMatrix(string name, Matrix value)
			=> NativeAPI.compute_set_matrix(_inst, name, value);

		/// <summary>Gets the value of a shader parameter by name! If
		/// the name isn't found, you'll get a default value back.
		/// </summary>
		public float GetFloat(string name)
			=> NativeAPI.compute_get_float(_inst, name);
		/// <inheritdoc cref="GetFloat"/>
		public int GetInt(string name)
			=> NativeAPI.compute_get_int(_inst, name);
		/// <inheritdoc cref="GetFloat"/>
		public uint GetUInt(string name)
			=> NativeAPI.compute_get_uint(_inst, name);
		/// <inheritdoc cref="GetFloat"/>
		public Vec2 GetVector2(string name)
			=> NativeAPI.compute_get_vector2(_inst, name);
		/// <inheritdoc cref="GetFloat"/>
		public Vec3 GetVector3(string name)
			=> NativeAPI.compute_get_vector3(_inst, name);
		/// <inheritdoc cref="GetFloat"/>
		public Vec4 GetVector4(string name)
			=> NativeAPI.compute_get_vector4(_inst, name);
		/// <inheritdoc cref="GetFloat"/>
		public bool GetBool(string name)
			=> NativeAPI.compute_get_bool(_inst, name);
		/// <summary>Gets a color parameter by name! Note that SetColor
		/// converts gamma to linear, so this returns the _linear_ value
		/// the shader is actually using.</summary>
		public Color GetColor(string name)
			=> NativeAPI.compute_get_color(_inst, name);
		/// <inheritdoc cref="GetFloat"/>
		public Matrix GetMatrix(string name)
			=> NativeAPI.compute_get_matrix(_inst, name);

		/// <summary>Bind a texture to a named resource in the shader!
		/// If you're writing to it (RWTexture2D), the texture _must_
		/// have TexType.Compute set, and use a format like
		/// TexFormat.Rgba128. Read-only Texture2D bindings work with
		/// any texture. Fallbacks are resolved at Dispatch time, so
		/// textures that are still loading will Just Work.</summary>
		/// <param name="name">The texture name in the HLSL shader.
		/// Must match exactly!</param>
		/// <param name="texture">The texture to bind.</param>
		/// <returns>True if a matching resource was found in the
		/// shader, false if the name didn't match anything.</returns>
		public bool SetTexture(string name, Tex texture)
			=> NativeAPI.compute_set_texture(_inst, name, texture?._inst ?? IntPtr.Zero);

		/// <summary>Sets a RW/StructuredBuffer or ByteAddressBuffer on the
		/// shader. This is used to provide BIG arrays of data to the
		/// GPU, for both reading and writing! These perform very similarly
		/// to textures, and can be thought of as big textures of just data!
		/// </summary>
		/// <typeparam name="T">The element type of the buffer.
		/// </typeparam>
		/// <param name="name">Name of the shader parameter in the HLSL.</param>
		/// <param name="buffer">The buffer to assign, or null to clear.
		/// </param>
		/// <returns>True if a matching resource was found in the
		/// shader.</returns>
		public bool SetStorage<T>(string name, ComputeBuffer<T> buffer) where T : unmanaged
			=> NativeAPI.compute_set_storage(_inst, name, buffer?._inst ?? IntPtr.Zero);

		/// <summary>Sets a constant/uniform buffer (cbuffer) on the shader.
		/// This is for smaller chunks of data (16kb max) that can be read from
		/// faster than textures or StructuredBuffers.</summary>
		/// <typeparam name="T">The element type of the buffer.
		/// </typeparam>
		/// <param name="name">Name of the shader parameter in the HLSL.</param>
		/// <param name="buffer">The buffer to assign, or null to clear.
		/// </param>
		/// <returns>True if a matching resource was found in the
		/// shader.</returns>
		public bool SetConstant<T>(string name, MaterialBuffer<T> buffer) where T : struct
			=> NativeAPI.compute_set_constant(_inst, name, buffer?._inst ?? IntPtr.Zero);

		/// <summary>Queue this compute dispatch into the render pipeline.
		/// It will run during the next frame's render setup phase, in
		/// source order with other queued render actions
		/// (Renderer.RenderTo, Renderer.SetGlobal*). This is the
		/// recommended path for compute work that participates in the
		/// frame's rendering pipeline (e.g. populating a texture that a
		/// later RenderTo or the main pass will sample), since
		/// sk_renderer can manage the necessary GPU barriers between
		/// queued items.
		///
		/// IMPORTANT: bindings (textures, buffers, constants, scalar
		/// parameters) are NOT snapshotted when Dispatch is called —
		/// they are read at execute time, which happens later in the
		/// frame. If you change a binding between two queued Dispatch
		/// calls on the same Compute, both dispatches will see the
		/// final binding state, not the state at their respective
		/// Dispatch times. To dispatch the same Compute with different
		/// bindings, either issue each Dispatch with DispatchNow, or
		/// use a separate Compute instance per binding set.
		///
		/// The parameters are the number of thread _groups_, not
		/// individual threads. Total thread count = groupCount *
		/// numthreads (as defined in your HLSL). So if your shader says
		/// [numthreads(8,8,1)] and you dispatch (64,64,1), you'll get
		/// 512*512 threads.</summary>
		/// <param name="groupCountX">Thread groups in X.</param>
		/// <param name="groupCountY">Thread groups in Y.</param>
		/// <param name="groupCountZ">Thread groups in Z.</param>
		public void Dispatch(uint groupCountX, uint groupCountY = 1, uint groupCountZ = 1)
			=> NativeAPI.compute_dispatch(_inst, groupCountX, groupCountY, groupCountZ);

		/// <summary>Run this compute dispatch synchronously, right now,
		/// on the calling thread. Use this for ad-hoc work that doesn't
		/// belong in the per-frame render pipeline (debugging, one-shot
		/// tasks, immediate readbacks). For compute work that feeds into
		/// later render passes within the same frame, prefer Dispatch,
		/// which queues into the pipeline and lets sk_renderer handle
		/// ordering and barriers automatically.</summary>
		/// <param name="groupCountX">Thread groups in X.</param>
		/// <param name="groupCountY">Thread groups in Y.</param>
		/// <param name="groupCountZ">Thread groups in Z.</param>
		public void DispatchNow(uint groupCountX, uint groupCountY = 1, uint groupCountZ = 1)
			=> NativeAPI.compute_dispatch_now(_inst, groupCountX, groupCountY, groupCountZ);

		/// <summary>The number of shader parameters available on this
		/// Compute! This includes both variables and textures/buffers,
		/// great for building a GUI that can inspect any shader.
		/// </summary>
		public int ParamCount => NativeAPI.compute_get_param_count(_inst);

		/// <summary>Gets an enumerable list of all parameter info on
		/// this Compute! Handy for building auto-generated shader
		/// GUIs or inspectors.</summary>
		/// <returns>An IEnumerable of MatParamInfo, usable with
		/// foreach.</returns>
		public IEnumerable<MatParamInfo> GetAllParamInfo()
		{
			int count = ParamCount;
			for (int i = 0; i < count; i++)
			{
				NativeAPI.compute_get_param_info(_inst, i, out IntPtr name, out MaterialParam type);
				yield return new MatParamInfo(Marshal.PtrToStringAnsi(name), type);
			}
		}

		/// <summary>Gets parameter info at a specific index! Parameters
		/// are listed as variables first, then textures and buffers.
		/// </summary>
		/// <param name="index">Index of the parameter, bounded by
		/// ParamCount.</param>
		/// <returns>Name and type info for this parameter.</returns>
		public MatParamInfo GetParamInfo(int index)
		{
			if (index < 0 || index >= ParamCount)
				throw new IndexOutOfRangeException();

			NativeAPI.compute_get_param_info(_inst, index, out IntPtr name, out MaterialParam type);
			return new MatParamInfo(Marshal.PtrToStringAnsi(name), type);
		}

		/// <summary>Looks for a Compute object that has already been
		/// created with a matching id!</summary>
		/// <param name="computeId">The id to search for.</param>
		/// <returns>A Compute with a matching id, or null if none
		/// exists.</returns>
		public static Compute Find(string computeId)
		{
			IntPtr inst = NativeAPI.compute_find(computeId);
			return inst == IntPtr.Zero ? null : new Compute(inst);
		}
	}
}

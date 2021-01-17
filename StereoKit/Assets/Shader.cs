using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>A shader is a piece of code that runs on the GPU, and 
	/// determines how model data gets transformed into pixels on screen! 
	/// It's more likely that you'll work more directly with Materials, which 
	/// shaders are a subset of.
	/// 
	/// With this particular class, you can mostly just look at it. It doesn't
	/// do a whole lot. Maybe you can swap out the shader code or something 
	/// sometimes!</summary>
	public class Shader
	{
		internal IntPtr _inst;

		/// <summary>The name of the shader, provided in the shader file 
		/// itself. Not the filename or id.</summary>
		public string Name => NativeAPI.shader_get_name(_inst);

		internal Shader(IntPtr shader)
		{
			_inst = shader;
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty shader!");
		}
		~Shader()
		{
			if (_inst != IntPtr.Zero)
				SK.ExecuteOnMain(()=>NativeAPI.shader_release(_inst));
		}

		/// <summary>Creates a shader asset from a precompiled StereoKit 
		/// Shader file stored as bytes! If you don't have a precompiled
		/// shader file, you can make one with the Shader.Compile method.
		/// The Id will be the shader's internal name.</summary>
		/// <param name="data">A precompiled StereoKit Shader file as bytes,
		/// you can get these bytes from Shader.Compile!</param>
		/// <returns>A shader from the given data, or null if it failed to 
		/// load/compile.</returns>
		public static Shader FromMemory(in byte[] data)
		{
			IntPtr inst = NativeAPI.shader_create_mem(data, (ulong)data.Length);
			return inst == IntPtr.Zero ? null : new Shader(inst);
		}

		/// <summary>Loads and compiles a shader from an hlsl, or precompiled 
		/// StereoKit Shader file! After loading an hlsl file, StereoKit will
		/// hash it, and check to see if it has changed since the last time 
		/// it cached a compiled version. If there is no cache for the hash, 
		/// it'll compile it, and save the compiled shader to a cache folder 
		/// in the asset path! The Id will be the shader's internal name.
		/// </summary>
		/// <param name="file">Path to a StereoKit Shader file, or hlsl code. 
		/// This gets prefixed with the asset path in StereoKitApp.settings.
		/// </param>
		/// <returns>A shader from the given file, or null if it failed to 
		/// load/compile.</returns>
		public static Shader FromFile(string file)
		{
			IntPtr inst = NativeAPI.shader_create_file(file);
			return inst == IntPtr.Zero ? null : new Shader(inst);
		}
		/// <summary>Looks for a Shader asset that's already loaded, matching 
		/// the given id! Unless the id has been set manually, the id will be 
		/// the same as the shader's name provided in the metadata.</summary>
		/// <param name="shaderId">For shaders loaded from file, this'll be 
		/// the shader's metadata name!</param>
		/// <returns>Link to a shader asset!</returns>
		public static Shader Find(string shaderId)
		{
			IntPtr inst = NativeAPI.shader_find(shaderId);
			return inst == IntPtr.Zero ? null : new Shader(inst);
		}
	}
}

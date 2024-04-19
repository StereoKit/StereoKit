using System;
using System.Runtime.InteropServices;
using System.Text;

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
	public class Shader : IAsset
	{
		internal IntPtr _inst;

		/// <summary>Gets or sets the unique identifier of this asset resource!
		/// This can be helpful for debugging, managine your assets, or finding
		/// them later on!</summary>
		public string Id
		{
			get => Marshal.PtrToStringAnsi(NativeAPI.shader_get_id(_inst));
			set => NativeAPI.shader_set_id(_inst, value);
		}

		/// <summary>The name of the shader, provided in the shader file 
		/// itself. Not the filename or id.</summary>
		public string Name => Marshal.PtrToStringAnsi(NativeAPI.shader_get_name(_inst));

		internal Shader(IntPtr shader)
		{
			_inst = shader;
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty shader!");
		}
		/// <summary>Release reference to the StereoKit asset.</summary>
		~Shader()
		{
			if (_inst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe(_inst);
		}

		/// <summary>Creates a shader asset from a precompiled StereoKit
		/// Shader file stored as bytes!</summary>
		/// <param name="data">A precompiled StereoKit Shader file as bytes.
		/// </param>
		/// <returns>A shader from the given data, or null if it failed to 
		/// load/compile.</returns>
		public static Shader FromMemory(in byte[] data)
		{
			IntPtr inst = NativeAPI.shader_create_mem(data, (UIntPtr)data.Length);
			return inst == IntPtr.Zero ? null : new Shader(inst);
		}

		/// <summary>Loads a shader from a precompiled StereoKit Shader 
		/// (.sks) file! HLSL files can be compiled using the skshaderc tool
		/// included in the NuGet package. This should be taken care of by
		/// MsBuild automatically, but you may need to ensure your HLSL file
		/// is a &lt;SKShader /&gt; item type in the .csproj for this to 
		/// work. You can also compile with the command line app manually if
		/// you're compiling/distributing a shader some other way! </summary>
		/// <param name="file">Path to a precompiled StereoKit Shader file!
		/// If no .sks extension is part of this path, StereoKit will 
		/// automatically add it and check that first.</param>
		/// <returns>A shader from the given file, or null if it failed to 
		/// load/compile.</returns>
		public static Shader FromFile(string file)
		{
			IntPtr inst = NativeAPI.shader_create_file(NativeHelper.ToUtf8(file));
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

		/// <inheritdoc cref="StereoKit.Default.Shader" />
		public static Shader Default => StereoKit.Default.Shader;
		/// <inheritdoc cref="StereoKit.Default.ShaderPbr" />
		public static Shader PBR => StereoKit.Default.ShaderPbr;
		/// <inheritdoc cref="StereoKit.Default.ShaderPbrClip" />
		public static Shader PBRClip => StereoKit.Default.ShaderPbrClip;
		/// <inheritdoc cref="StereoKit.Default.ShaderUI" />
		public static Shader UI => StereoKit.Default.ShaderUI;
		/// <inheritdoc cref="StereoKit.Default.ShaderUIBox" />
		public static Shader UIBox => StereoKit.Default.ShaderUIBox;
		/// <inheritdoc cref="StereoKit.Default.ShaderUnlit" />
		public static Shader Unlit => StereoKit.Default.ShaderUnlit;
		/// <inheritdoc cref="StereoKit.Default.ShaderUnlitClip" />
		public static Shader UnlitClip => StereoKit.Default.ShaderUnlitClip;
	}
}

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
                NativeAPI.shader_release(_inst);
        }

        /// <summary>This will compile an HLSL shader into a StereoKit binary
        /// storage format. StereoKit does not need to be initialized for 
        /// this method to work! The binary format includes data about shader
        /// parameters and textures, as well as pre-compiled shaders for 
        /// different platforms. Currently only supports HLSL, but can support
        /// more. The results can be passed into Shader.FromMemory to get a 
        /// final shader asset, or it can be saved to file!</summary>
        /// <param name="hlsl">HLSL code, including metadata comments.</param>
        /// <returns>Binary data representing a StereoKit compiled shader data
        /// file. This can be passed into Shader.FromMemory to get a final
        /// shader asset, or it can be saved to file!</returns>
        public static byte[] Compile(string hlsl)
        {
            if (!NativeAPI.shader_compile(hlsl, out IntPtr ptr, out ulong size))
                return new byte[]{ };

            byte[] result = new byte[size];
            Marshal.Copy(ptr, result, 0, (int)size);
            return result;
        }

        /// <summary>Creates a shader from a piece of HLSL code! Shader stuff
        /// like this may change in the future, since HLSL may not be all that
        /// portable. Also, before compiling the shader code, StereoKit hashes 
        /// the contents, and looks to see if it has that shader cached. If 
        /// so, it'll just load that instead of compiling it again. The Id 
        /// will be the shader's internal name.</summary>
        /// <param name="hlsl">A vertex and pixel shader written in HLSL, 
        /// check the shader guides for more on this later!</param>
        /// <returns>A shader from the given code, or null if it failed to 
        /// load/compile.</returns>
        public static Shader FromHLSL(string hlsl) { 
            IntPtr inst = NativeAPI.shader_create_hlsl(hlsl);
            return inst == IntPtr.Zero ? null : new Shader(inst);
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

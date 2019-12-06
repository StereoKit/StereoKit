using System;

namespace StereoKit
{
    /// <summary>A shader is a piece of code that runs on the GPU, and determines
    /// how model data gets transformed into pixels on screen! It's more likely 
    /// that you'll work more directly with Materials, which shaders are a subset of.
    /// 
    /// With this particular class, you can mostly just look at it. It doesn't do
    /// a whole lot. Maybe you can swap out the shader code or something sometimes!</summary>
    public class Shader
    {
        internal IntPtr _inst;

        /// <summary>The name of the shader, provided in the shader file itself. Not the filename or id.</summary>
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

        /// <summary>Creates a shader from a piece of HLSL code! Shader stuff like this may 
        /// change in the future, since HLSL may not be all that portable. Also, before
        /// compiling the shader code, StereoKit hashes the contents, and looks to see if
        /// it has that shader cached. If so, it'll just load that instead of compiling it
        /// again.</summary>
        /// <param name="hlsl">A vertex and pixel shader written in HLSL, check the shader
        /// guides for more on this later!</param>
        /// <returns></returns>
        public static Shader FromHLSL(string hlsl)
        {
            IntPtr inst = NativeAPI.shader_create(hlsl);
            return inst == IntPtr.Zero ? null : new Shader(inst);
        }
        /// <summary>Loads and compiles a shader from an hlsl file! Technically, after loading the file,
        /// StereoKit will hash it, and check to see if it has changed since the last time it cached a
        /// compiled version. If there is no cache for the hash, it'll compile it, and save the compiled
        /// shader to a cache folder in the asset path!</summary>
        /// <param name="file">Path to a file with hlsl code in it. This gets prefixed with the asset path
        /// in StereoKitApp.settings.</param>
        public static Shader FromFile(string file)
        {
            IntPtr inst = NativeAPI.shader_create_file(file);
            return inst == IntPtr.Zero ? null : new Shader(inst);
        }
        /// <summary>Looks for a Material asset that's already loaded, matching the given id! Unless
        /// the id has been set manually, the id will be the same as the filename provided for
        /// loading the shader.</summary>
        /// <param name="shaderId">For shaders loaded from file, this'll be the file name!</param>
        /// <returns>Link to a shader asset!</returns>
        public static Shader Find(string shaderId)
        {
            IntPtr inst = NativeAPI.shader_find(shaderId);
            return inst == IntPtr.Zero ? null : new Shader(inst);
        }
    }
}

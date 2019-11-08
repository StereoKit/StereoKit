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
        internal IntPtr _shaderInst;

        /// <summary>The name of the shader, provided in the shader file itself. Not the filename or id.</summary>
        public string Name => NativeAPI.shader_get_name(_shaderInst);

        internal Shader(IntPtr shader)
        {
            _shaderInst = shader;
            if (_shaderInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Received an empty shader!");
        }
        public Shader(string file)
        {
            _shaderInst = NativeAPI.shader_create_file(file);
            if (_shaderInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Couldn't load shader file {0}!", file);
        }
        ~Shader()
        {
            if (_shaderInst != IntPtr.Zero)
                NativeAPI.shader_release(_shaderInst);
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
            return new Shader(NativeAPI.shader_create(hlsl)); ;
        }
        /// <summary>Finds a shader asset with a given id.</summary>
        /// <param name="id">For shaders loaded from file, this'll be the file name!</param>
        /// <returns>Link to a shader asset!</returns>
        public static Shader Find(string id)
        {
            IntPtr shader = NativeAPI.shader_find(id);
            return shader == IntPtr.Zero ? null : new Shader(shader);
        }
    }
}

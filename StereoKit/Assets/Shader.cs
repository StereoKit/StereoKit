using System;

namespace StereoKit
{
    public class Shader
    {
        internal IntPtr _shaderInst;
        private Shader()
        {
        }
        private Shader(IntPtr shader)
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

        public static Shader FromHLSL(string hlsl)
        {
            Shader result = new Shader();
            result._shaderInst = NativeAPI.shader_create(hlsl);
            return result;
        }
        public static Shader Find(string id)
        {
            IntPtr shader = NativeAPI.shader_find(id);
            return shader == IntPtr.Zero ? null : new Shader(shader);
        }
    }
}

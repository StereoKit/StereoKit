using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct GradientKey
    {
        public Color color;
        public float position;
    }

    public class Gradient
    {
        internal IntPtr _inst;

        public int Count => NativeAPI.gradient_count(_inst);

        public Gradient() => _inst = NativeAPI.gradient_create();
        public Gradient(GradientKey[] keys) => _inst = NativeAPI.gradient_create_keys(keys, keys.Length);
        ~Gradient() => NativeAPI.gradient_release(_inst);

        public void    Add  (Color color, float position) => NativeAPI.gradient_add(_inst, color, position);
        public void    Set  (int index, Color color, float position) => NativeAPI.gradient_set(_inst, index, color, position);
        public void    Remove(int index) => NativeAPI.gradient_remove(_inst, index);
        public Color   Get  (float at) => NativeAPI.gradient_get  (_inst, at);
        public Color32 Get32(float at) => NativeAPI.gradient_get32(_inst, at);

    }
}

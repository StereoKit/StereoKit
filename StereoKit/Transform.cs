using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    internal struct transform_t
    {
        Vec3 _position;
        Vec3 _scale;
        Quat _rotation;

        bool _dirty;
        Matrix _transform;
    }

    public class Transform
    {
        #region Imports
        [DllImport(NativeLib.DllName)] static extern void transform_initialize  (IntPtr transform);
        [DllImport(NativeLib.DllName)] static extern void transform_set         (IntPtr transform, Vec3 position, Vec3 scale, Quat rotation );
        [DllImport(NativeLib.DllName)] static extern void transform_set_position(IntPtr transform, Vec3 position);
        [DllImport(NativeLib.DllName)] static extern void transform_set_scale   (IntPtr transform, Vec3 scale);
        [DllImport(NativeLib.DllName)] static extern void transform_set_rotation(IntPtr transform, Quat rotation);
        [DllImport(NativeLib.DllName)] static extern void transform_lookat      (IntPtr transform, Vec3 at);
        #endregion

        internal IntPtr _transformInst;
        public Transform()
        {
            _transformInst = Marshal.AllocCoTaskMem(Marshal.SizeOf(typeof(transform_t)));
            transform_initialize(_transformInst);
        }
        public Transform(Vec3 position, Vec3? scale = null, Quat? rotation = null)
        {
            _transformInst = Marshal.AllocCoTaskMem(Marshal.SizeOf(typeof(transform_t)));
            transform_initialize(_transformInst);
            transform_set(_transformInst, position, scale.HasValue ? scale.Value : Vec3.One, rotation.HasValue ? rotation.Value : Quat.Identity);
        }
        ~Transform()
        {
            if (_transformInst != IntPtr.Zero)
                Marshal.FreeCoTaskMem(_transformInst);
        }

        public Vec3 Position { set { transform_set_position(_transformInst, value); } }
        public Quat Rotation { set { transform_set_rotation(_transformInst, value); } }
        public Vec3 Scale    { set { transform_set_scale   (_transformInst, value); } }

        public void LookAt(Vec3 position) { transform_lookat(_transformInst, position); }
    }
}

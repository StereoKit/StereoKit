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

        int _dirty;
        Matrix _transform;
    }

    public class Transform
    {
        #region Imports
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)] static extern void transform_initialize  (IntPtr transform);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)] static extern void transform_set         (IntPtr transform, ref Vec3 position, ref Vec3 scale, ref Quat rotation );
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)] static extern void transform_set_position(IntPtr transform, ref Vec3 position);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)] static extern Vec3 transform_get_position(IntPtr transform);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)] static extern void transform_set_scale   (IntPtr transform, ref Vec3 scale);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)] static extern Vec3 transform_get_scale   (IntPtr transform);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)] static extern void transform_set_rotation(IntPtr transform, ref Quat rotation);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)] static extern Quat transform_get_rotation(IntPtr transform);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)] static extern void transform_lookat      (IntPtr transform, ref Vec3 at);
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
            Vec3 argScale = scale    ?? Vec3.One;
            Quat argRot   = rotation ?? Quat.Identity;
            transform_set(_transformInst, ref position, ref argScale, ref argRot);
        }
        ~Transform()
        {
            if (_transformInst != IntPtr.Zero)
                Marshal.FreeCoTaskMem(_transformInst);
        }

        public Vec3 Position { set { transform_set_position(_transformInst, ref value); } get { return transform_get_position(_transformInst); } }
        public Quat Rotation { set { transform_set_rotation(_transformInst, ref value); } get { return transform_get_rotation(_transformInst); } }
        public Vec3 Scale    { set { transform_set_scale   (_transformInst, ref value); } get { return transform_get_scale   (_transformInst); } }

        public void LookAt(Vec3 position) { transform_lookat(_transformInst, ref position); }
    }
}

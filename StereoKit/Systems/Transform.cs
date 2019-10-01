using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    public class Transform
    {
        internal IntPtr _transformInst;
        public Transform()
        {
            _transformInst = Marshal.AllocCoTaskMem(Marshal.SizeOf(typeof(transform_t)));
            NativeAPI.transform_initialize(_transformInst);
        }
        public Transform(Vec3 position, Vec3? scale = null, Quat? rotation = null)
        {
            _transformInst = Marshal.AllocCoTaskMem(Marshal.SizeOf(typeof(transform_t)));
            NativeAPI.transform_initialize(_transformInst);
            Vec3 argScale = scale    ?? Vec3.One;
            Quat argRot   = rotation ?? Quat.Identity;
            NativeAPI.transform_set(_transformInst, ref position, ref argScale, ref argRot);
        }
        ~Transform()
        {
            if (_transformInst != IntPtr.Zero)
                Marshal.FreeCoTaskMem(_transformInst);
        }

        public Vec3 Position { set { NativeAPI.transform_set_position(_transformInst, ref value); } get { return NativeAPI.transform_get_position(_transformInst); } }
        public Quat Rotation { set { NativeAPI.transform_set_rotation(_transformInst, ref value); } get { return NativeAPI.transform_get_rotation(_transformInst); } }
        public Vec3 Scale    { set { NativeAPI.transform_set_scale   (_transformInst, ref value); } get { return NativeAPI.transform_get_scale   (_transformInst); } }

        public void LookAt(Vec3 position) { NativeAPI.transform_lookat(_transformInst, ref position); }
    }
}

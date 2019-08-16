using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    public enum SolidType
    {
        Normal = 0,
        Immovable,
        Unaffected,
    }

    public class Solid
    {
        #region Imports
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr  solid_create    (ref Vec3 position, ref Quat rotation, SolidType type);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void solid_release      (IntPtr solid);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void solid_add_sphere   (IntPtr solid, float diameter,               float kilograms, ref Vec3 offset);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void solid_add_box      (IntPtr solid, ref Vec3 dimensions,          float kilograms, ref Vec3 offset);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void solid_add_capsule  (IntPtr solid, float diameter, float height, float kilograms, ref Vec3 offset);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void solid_set_type     (IntPtr solid, SolidType type);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void solid_set_enabled  (IntPtr solid, int enabled);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void solid_move         (IntPtr solid, ref Vec3 position, ref Quat rotation);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void solid_teleport     (IntPtr solid, ref Vec3 position, ref Quat rotation);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void solid_set_velocity    (IntPtr solid, ref Vec3 meters_per_second);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void solid_set_velocity_ang(IntPtr solid, ref Vec3 radians_per_second);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void solid_get_transform   (IntPtr solid, IntPtr out_transform);
        #endregion

        internal IntPtr _solidInst;
        public Solid(Vec3 position, Quat rotation, SolidType type = SolidType.Normal)
        {
            _solidInst = solid_create(ref position, ref rotation, type);
            if (_solidInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Couldn't create solid!");
        }
        private Solid(IntPtr solid)
        {
            _solidInst = solid;
            if (_solidInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Received an empty solid!");
        }
        ~Solid()
        {
            if (_solidInst == IntPtr.Zero)
                solid_release(_solidInst);
        }

        public bool Enabled { set{ SetEnabled(value); } }

        public void AddSphere(float diameter, float kilograms = 1, Vec3 ?offset = null)
        {
            Vec3 off = offset ?? Vec3.Zero;
            solid_add_sphere(_solidInst, diameter, kilograms, ref off);
        }
        public void AddBox(Vec3 dimensions, float kilograms = 1, Vec3? offset = null)
        {
            Vec3 off = offset ?? Vec3.Zero;
            solid_add_box(_solidInst, ref dimensions, kilograms, ref off);
        }
        public void AddCapsule(float diameter, float height, float kilograms = 1, Vec3? offset = null)
        {
            Vec3 off = offset ?? Vec3.Zero;
            solid_add_capsule(_solidInst, diameter, height, kilograms, ref off);
        }
        public void SetType(SolidType type)
        {
            solid_set_type(_solidInst, type);
        }
        public void SetEnabled(bool enabled)
        {
            solid_set_enabled(_solidInst, enabled?1:0);
        }
        public void Move(Vec3 position, Quat rotation)
        {
            solid_move(_solidInst, ref position, ref rotation);
        }
        public void Teleport(Vec3 position, Quat rotation)
        {
            solid_teleport(_solidInst, ref position, ref rotation);
        }
        public void SetVelocity(Vec3 metersPerSecond)
        {
            solid_set_velocity(_solidInst, ref metersPerSecond);
        }
        public void SetAngularVelocity(Vec3 radiansPerSecond)
        {
            solid_set_velocity_ang(_solidInst, ref radiansPerSecond);
        }
        public void GetTransform(ref Transform transform)
        {
            solid_get_transform(_solidInst, transform._transformInst);
        }
    }
}

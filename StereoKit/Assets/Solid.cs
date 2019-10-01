using System;

namespace StereoKit
{
    public class Solid
    {
        internal IntPtr _solidInst;
        public Solid(Vec3 position, Quat rotation, SolidType type = SolidType.Normal)
        {
            _solidInst = NativeAPI.solid_create(ref position, ref rotation, type);
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
                NativeAPI.solid_release(_solidInst);
        }

        public bool Enabled { set{ SetEnabled(value); } }

        public void AddSphere(float diameter, float kilograms = 1, Vec3 ?offset = null)
        {
            Vec3 off = offset ?? Vec3.Zero;
            NativeAPI.solid_add_sphere(_solidInst, diameter, kilograms, ref off);
        }
        public void AddBox(Vec3 dimensions, float kilograms = 1, Vec3? offset = null)
        {
            Vec3 off = offset ?? Vec3.Zero;
            NativeAPI.solid_add_box(_solidInst, ref dimensions, kilograms, ref off);
        }
        public void AddCapsule(float diameter, float height, float kilograms = 1, Vec3? offset = null)
        {
            Vec3 off = offset ?? Vec3.Zero;
            NativeAPI.solid_add_capsule(_solidInst, diameter, height, kilograms, ref off);
        }
        public void SetType(SolidType type)
        {
            NativeAPI.solid_set_type(_solidInst, type);
        }
        public void SetEnabled(bool enabled)
        {
            NativeAPI.solid_set_enabled(_solidInst, enabled?1:0);
        }
        public void Move(Vec3 position, Quat rotation)
        {
            NativeAPI.solid_move(_solidInst, ref position, ref rotation);
        }
        public void Teleport(Vec3 position, Quat rotation)
        {
            NativeAPI.solid_teleport(_solidInst, ref position, ref rotation);
        }
        public void SetVelocity(Vec3 metersPerSecond)
        {
            NativeAPI.solid_set_velocity(_solidInst, ref metersPerSecond);
        }
        public void SetAngularVelocity(Vec3 radiansPerSecond)
        {
            NativeAPI.solid_set_velocity_ang(_solidInst, ref radiansPerSecond);
        }
        public void GetTransform(ref Transform transform)
        {
            NativeAPI.solid_get_transform(_solidInst, transform._transformInst);
        }
    }
}

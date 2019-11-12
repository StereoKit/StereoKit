using System;

namespace StereoKit
{
    /// <summary>A Solid is an object that gets simulated with physics! Once you
    /// create a solid, it will continuously be acted upon by forces like gravity
    /// and other objects. Solid does -not- draw anything on its own, but you can 
    /// ask a Solid for its current pose, and draw an object at that pose!
    /// 
    /// Once you create a Solid, you need to define its shape using geometric
    /// primitives, this is the AddSphere, AddCube, AddCapsule functions. You can 
    /// add more than one to a single Solid to get a more complex shape!
    /// 
    /// If you want to directly move a Solid, note the difference between the Move
    /// function and the Teleport function. Move will change the velocity for a 
    /// single frame to travel through space to get to its destination, while teleport
    /// will simply appear at its destination without touching anything between.</summary>
    public class Solid
    {
        internal IntPtr _solidInst;

        /// <summary>Creates a Solid physics object and adds it to the physics system.</summary>
        /// <param name="position">Initial location of the Solid.</param>
        /// <param name="rotation">Initial rotation of the Solid.</param>
        /// <param name="type">What sort of physics properties does this object exhibit?</param>
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
            NativeAPI.solid_add_sphere(_solidInst, diameter, kilograms, off);
        }
        public void AddBox(Vec3 dimensions, float kilograms = 1, Vec3? offset = null)
        {
            Vec3 off = offset ?? Vec3.Zero;
            NativeAPI.solid_add_box(_solidInst, dimensions, kilograms, off);
        }
        public void AddCapsule(float diameter, float height, float kilograms = 1, Vec3? offset = null)
        {
            Vec3 off = offset ?? Vec3.Zero;
            NativeAPI.solid_add_capsule(_solidInst, diameter, height, kilograms, off);
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
            NativeAPI.solid_move(_solidInst, position, rotation);
        }
        public void Teleport(Vec3 position, Quat rotation)
        {
            NativeAPI.solid_teleport(_solidInst, position, rotation);
        }
        public void SetVelocity(Vec3 metersPerSecond)
        {
            NativeAPI.solid_set_velocity(_solidInst, metersPerSecond);
        }
        public void SetAngularVelocity(Vec3 radiansPerSecond)
        {
            NativeAPI.solid_set_velocity_ang(_solidInst, radiansPerSecond);
        }
        public Pose GetPose()
        {
            Pose result;
            NativeAPI.solid_get_pose(_solidInst, out result);
            return result;
        }
        public void GetPose(out Pose pose)
        {
            NativeAPI.solid_get_pose(_solidInst, out pose);
        }
    }
}

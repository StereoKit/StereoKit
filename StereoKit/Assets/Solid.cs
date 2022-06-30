using System;

namespace StereoKit
{
	/// <summary>A Solid is an object that gets simulated with physics! Once
	/// you create a solid, it will continuously be acted upon by forces like
	/// gravity and other objects. Solid does -not- draw anything on its own, 
	/// but you can ask a Solid for its current pose, and draw an object at 
	/// that pose!
	/// 
	/// Once you create a Solid, you need to define its shape using geometric
	/// primitives, this is the AddSphere, AddCube, AddCapsule functions. You
	/// can add more than one to a single Solid to get a more complex shape!
	/// 
	/// If you want to directly move a Solid, note the difference between the
	/// Move function and the Teleport function. Move will change the 
	/// velocity for a single frame to travel through space to get to its
	/// destination, while teleport will simply appear at its destination 
	/// without touching anything between.</summary>
	public class Solid
	{
		internal IntPtr _inst;

		/// <summary>Creates a Solid physics object and adds it to the 
		/// physics system.</summary>
		/// <param name="position">Initial location of the Solid.</param>
		/// <param name="rotation">Initial rotation of the Solid.</param>
		/// <param name="type">What sort of physics properties does this 
		/// object exhibit?</param>
		public Solid(Vec3 position, Quat rotation, SolidType type = SolidType.Normal)
		{
			_inst = NativeAPI.solid_create(ref position, ref rotation, type);
			if (_inst == IntPtr.Zero)
				Log.Err("Couldn't create solid!");
		}
		private Solid(IntPtr solid)
		{
			_inst = solid;
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty solid!");
		}
		~Solid()
		{
			if (_inst != IntPtr.Zero)
				NativeAPI.solid_release(_inst);
		}

		/// <summary>Is the Solid enabled in the physics simulation? Set this 
		/// to false if you want to prevent physics from influencing this 
		/// solid!</summary>
		public bool Enabled { set{ SetEnabled(value); } }

		/// <summary>This adds a sphere to this solid's physical shape! This 
		/// is in addition to any other shapes you've added to this solid.
		/// </summary>
		/// <param name="diameter">Twice the radius of the sphere! The total
		/// size, in meters!</param>
		/// <param name="kilograms">How many kilograms does this shape weigh?
		/// </param>
		/// <param name="offset">Offset to this shape from the center of the
		/// solid.</param>
		public void AddSphere(float diameter, float kilograms = 1, Vec3 ?offset = null)
			=> NativeAPI.solid_add_sphere(_inst, diameter, kilograms, offset ?? Vec3.Zero);

		/// <summary>This adds a box to this solid's physical shape! This is
		/// in addition to any other shapes you've added to this solid.
		/// </summary>
		/// <param name="dimensions">The total width, height, and depth of
		/// the box in meters!</param>
		/// <param name="kilograms">How many kilograms does this shape weigh?
		/// </param>
		/// <param name="offset">Offset of this shape from the center of the
		/// solid.</param>
		public void AddBox(Vec3 dimensions, float kilograms = 1, Vec3? offset = null)
			=> NativeAPI.solid_add_box(_inst, dimensions, kilograms, offset ?? Vec3.Zero);

		/// <summary>This adds a capsule, a cylinder with rounded ends, to
		/// this solid's physical shape! This is in addition to any other 
		/// shapes you've added to this solid.</summary>
		/// <param name="diameter">The diameter of the cylinder, as well as
		/// the capsule ends, in meters.</param>
		/// <param name="height">The total width, height, and depth of the
		/// box in meters!</param>
		/// <param name="kilograms">How many kilograms does this shape weigh?
		/// </param>
		/// <param name="offset">Offset of this shape from the center of the 
		/// solid.</param>
		public void AddCapsule(float diameter, float height, float kilograms = 1, Vec3? offset = null)
			=> NativeAPI.solid_add_capsule(_inst, diameter, height, kilograms, offset ?? Vec3.Zero);

		/// <summary>Changes the behavior type of the solid after it's 
		/// created.</summary>
		/// <param name="type">The new solid type!</param>
		public void SetType(SolidType type)
			=> NativeAPI.solid_set_type(_inst, type);

		/// <summary>Same as Enabled. Is the Solid enabled in the physics 
		/// simulation? Set this to false if you want to prevent physics from
		/// influencing this solid!</summary>
		/// <param name="enabled">False to disable physics on this object, 
		/// true to enable it.</param>
		public void SetEnabled(bool enabled)
			=> NativeAPI.solid_set_enabled(_inst, enabled?1:0);

		/// <summary>This moves the Solid from its current location through 
		/// space to the new location provided, colliding with things along
		/// the way. This is achieved by applying the velocity and angular 
		/// velocity necessary to get to the destination in a single frame
		/// during the next physics step, then restoring the previous 
		/// velocity info afterwards! See also Teleport for movement without
		/// collision.</summary>
		/// <param name="position">The destination position!</param>
		/// <param name="rotation">The destination rotation!</param>
		public void Move(Vec3 position, Quat rotation)
			=> NativeAPI.solid_move(_inst, position, rotation);

		/// <summary>Moves the Solid to the new pose, without colliding with
		/// objects on the way there.</summary>
		/// <param name="position">The destination position!</param>
		/// <param name="rotation">The destination rotation!</param>
		public void Teleport(Vec3 position, Quat rotation)
			=> NativeAPI.solid_teleport(_inst, position, rotation);

		/// <summary>Sets the velocity of this Solid.</summary>
		/// <param name="metersPerSecond">How fast should it be going, along
		/// what vector?</param>
		public void SetVelocity(Vec3 metersPerSecond)
			=> NativeAPI.solid_set_velocity(_inst, metersPerSecond);

		/// <summary>I wish I knew what angular velocity actually meant, 
		/// perhaps you know? It's rotation velocity of some kind or another!
		/// </summary>
		/// <param name="radiansPerSecond">I think it's in radians. Maybe?
		/// </param>
		public void SetAngularVelocity(Vec3 radiansPerSecond)
			=> NativeAPI.solid_set_velocity_ang(_inst, radiansPerSecond);

		/// <summary>Retrieves the current pose of the Solid from the physics
		/// simulation.</summary>
		/// <returns>The Solid's current pose.</returns>
		public Pose GetPose()
		{ 
			NativeAPI.solid_get_pose(_inst, out Pose result);
			return result;
		}

		/// <summary>Retrieves the current pose of the Solid from the physics
		/// simulation.</summary>
		/// <param name="pose">Out param for the Solid's current pose.</param>
		public void GetPose(out Pose pose)
			=> NativeAPI.solid_get_pose(_inst, out pose);
	}
}

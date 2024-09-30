namespace StereoKit
{
	/// <summary>This class represents a stack of transform matrices that 
	/// build up a transform hierarchy! This can be used like an object-less 
	/// parent-child system, where you push a parent's transform onto the 
	/// stack, render child objects relative to that parent transform and 
	/// then pop it off the stack.
	/// 
	/// Performance note: if any matrices are on the hierarchy stack, any 
	/// render will cause a matrix multiplication to occur! So if you have a
	/// collection of objects with their transforms baked and cached into 
	/// matrices for performance reasons, you'll want to ensure there are no 
	/// matrices in the hierarchy stack, or that the hierarchy is disabled! 
	/// It'll save you a matrix multiplication in that case :)</summary>
	public static class Hierarchy
	{
		/// <summary>Pushes a transform Matrix onto the stack, and combines 
		/// it with the Matrix below it. Any draw operation's Matrix will now
		/// be combined with this Matrix to make it relative to the current 
		/// hierarchy. Use Hierarchy.Pop to remove it from the Hierarchy 
		/// stack! All Push calls must have an accompanying Pop call.</summary>
		/// <param name="parentTransform">The transform Matrix you want to 
		/// apply to all following draw calls.</param>
		/// <param name="parentBehavior">This determines how this matrix
		/// combines with the parent matrix below it. Normal behavior is to
		/// "inherit" the parent matrix, but there are cases where you may wish
		/// to entirely ignore the parent transform. For example, if you're in
		/// UI space, and wish to do some world space rendering.</param>
		public static void Push(in Matrix parentTransform, HierarchyParent parentBehavior = HierarchyParent.Inherit)
			=> NativeAPI.hierarchy_push(parentTransform, parentBehavior);

		/// <summary>Removes the top Matrix from the stack!</summary>
		public static void Pop()
			=> NativeAPI.hierarchy_pop();

		/// <summary>This is enabled by default. Disabling this will cause
		/// any draw call to ignore any Matrices that are on the Hierarchy
		/// stack.</summary>
		public static bool Enabled {
			get => NativeAPI.hierarchy_is_enabled();
			set => NativeAPI.hierarchy_set_enabled(value);
		}

		/// <summary>Converts a world space point into the local space of the
		/// current Hierarchy stack!</summary>
		/// <param name="worldPoint">A point in world space.</param>
		/// <returns>The provided point now in local hierarchy space!</returns>
		public static Vec3 ToLocal(Vec3 worldPoint) 
			=> NativeAPI.hierarchy_to_local_point(worldPoint);

		/// <summary>Converts a world space direction into the local space of
		/// the current Hierarchy stack! This excludes the translation
		/// component normally applied to vectors, so it's still a valid
		/// direction.</summary>
		/// <param name="worldDirection">A direction in world space.</param>
		/// <returns>The provided direction now in local hierarchy space!
		/// </returns>
		public static Vec3 ToLocalDirection(Vec3 worldDirection)
			=> NativeAPI.hierarchy_to_local_direction(worldDirection);

		/// <summary>Converts a world space rotation into the local space of
		/// the current Hierarchy stack!</summary>
		/// <param name="worldOrientation">A rotation in world space.</param>
		/// <returns>The provided rotation now in local hierarchy space!
		/// </returns>
		public static Quat ToLocal(Quat worldOrientation)
			=> NativeAPI.hierarchy_to_local_rotation(worldOrientation);


		/// <summary>Converts a world pose relative to the current
		/// hierarchy stack into local space!</summary>
		/// <param name="worldPose">A pose in world space.</param>
		/// <returns>The provided pose now in local hierarchy space!</returns>
		public static Pose ToLocal(Pose worldPose)
			=> NativeAPI.hierarchy_to_local_pose(worldPose);

		/// <summary>Converts a world ray relative to the current
		/// hierarchy stack into local space!</summary>
		/// <param name="worldRay">A ray in world space.</param>
		/// <returns>The provided ray now in local hierarchy space!</returns>
		public static Ray ToLocal(Ray worldRay)
			=> NativeAPI.hierarchy_to_local_ray(worldRay);

		/// <summary>Converts a local point relative to the current hierarchy
		/// stack into world space!</summary>
		/// <param name="localPoint">A point in local space.</param>
		/// <returns>The provided point now in world space!</returns>
		public static Vec3 ToWorld(Vec3 localPoint)
			=> NativeAPI.hierarchy_to_world_point(localPoint);

		/// <summary>Converts a local direction relative to the current 
		/// hierarchy stack into world space! This excludes the translation 
		/// component normally applied to vectors, so it's still a valid 
		/// direction.</summary>
		/// <param name="localDirection">A direction in local space.</param>
		/// <returns>The provided direction now in world space!</returns>
		public static Vec3 ToWorldDirection(Vec3 localDirection)
			=> NativeAPI.hierarchy_to_world_direction(localDirection);

		/// <summary>Converts a local rotation relative to the current 
		/// hierarchy stack into world space!</summary>
		/// <param name="localOrientation">A rotation in local space.</param>
		/// <returns>The provided rotation now in world space!</returns>
		public static Quat ToWorld(Quat localOrientation)
			=> NativeAPI.hierarchy_to_world_rotation(localOrientation);

		/// <summary>Converts a local pose relative to the current
		/// hierarchy stack into world space!</summary>
		/// <param name="localPose">A pose in local space.</param>
		/// <returns>The provided pose now in world space!</returns>
		public static Pose ToWorld(Pose localPose)
			=> NativeAPI.hierarchy_to_world_pose(localPose);

		/// <summary>Converts a local ray relative to the current
		/// hierarchy stack into world space!</summary>
		/// <param name="localRay">A ray in local space.</param>
		/// <returns>The provided ray now in world space!</returns>
		public static Ray ToWorld(Ray localRay)
			=> NativeAPI.hierarchy_to_world_ray(localRay);
	}
}

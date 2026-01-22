// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

namespace StereoKit
{
	/// <summary>Interactors are essentially capsules that allow interaction
	/// with StereoKit's interaction primitives used by the UI system. While
	/// StereoKit does provide a number of interactors by default, you can
	/// replace StereoKit's defaults, add additional interactors, or generally
	/// just customize your interactions!</summary>
	public struct Interactor
	{
		private int _inst;

		private Interactor(int inst)
		{
			_inst = inst;
		}

		/// <summary>The distance at which a ray starts being interactive. For
		/// pointing rays, you may not want them to interact right at their
		/// start, or you may want the start to move depending on how
		/// outstretched the hand is! This allows you to change that start
		/// location without affecting the movement caused by the ray, and
		/// still capturing occlusion from blocking elements too close to the
		/// start. By default, this is a large negative value.</summary>
		public float MinDistance {
			get => NativeAPI.interactor_get_min_distance(_inst);
			set => NativeAPI.interactor_set_min_distance(_inst, value);
		}
		/// <summary>The world space radius of the interactor capsule, in
		/// meters.</summary>
		public float Radius {
			get => NativeAPI.interactor_get_radius(_inst);
			set => NativeAPI.interactor_set_radius(_inst, value);
		}
		/// <summary>The world space start of the interactor capsule. Some
		/// interactions can be directional, especially for `Line` type
		/// interactors, so if you think of the interactor as an "oriented"
		/// capsule, this would be the origin which points towards the capsule
		/// `End`.</summary>
		public Vec3     Start => NativeAPI.interactor_get_capsule_start(_inst);

		/// <summary>The world space end of the interactor capsule. Some
		/// interactions can be directional, especially for `Line` type
		/// interactors, so if you think of the interactor as an "oriented"
		/// capsule, this would be the end which the `Start`/origin points
		/// towards.</summary>
		public Vec3     End => NativeAPI.interactor_get_capsule_end(_inst);
		/// <summary>The tracking state of this interactor.</summary>
		public BtnState Tracked => NativeAPI.interactor_get_tracked(_inst);
		/// <summary>The id of the interaction element that is currently
		/// focused, this will be `IdHash.None` if this interactor has nothing
		/// focused.</summary>
		public IdHash   Focused => NativeAPI.interactor_get_focused(_inst);
		/// <summary>The id of the interaction element that is currently
		/// active, this will be `IdHash.None` if this interactor has nothing
		/// active. This will always be the same id as `Focused` when not
		/// `None`.</summary>
		public IdHash   Active  => NativeAPI.interactor_get_active (_inst);
		/// <summary>This pose is the source of translation and rotation motion
		/// caused by the interactor. In most cases it will be the same as your
		/// Start with the orientation of your interactor, but in some instance
		/// may be something else!</summary>
		public Pose     Motion  => NativeAPI.interactor_get_motion (_inst);

		/// <summary>Update the interactor with data for the current frame!
		/// This should be called as soon as possible at the start of the frame
		/// before any UI is done, otherwise the UI will not properly react.</summary>
		/// <param name="capsuleStart">World space location of the collision
		/// capsule's start. For Line interactors, this should be the 'origin'
		/// of the capsule's orientation.</param>
		/// <param name="capsuleEnd">World space location of the collision
		/// capsule's end. For Line interactors, this should be in the
		/// direction the Start/origin is facing.</param>
		/// <param name="motion">This pose is the source of translation and
		/// rotation motion caused by the interactor. In most cases it will be
		/// the same as your capsuleStart with the orientation of your
		/// interactor, but in some instance may be something else!</param>
		/// <param name="motionAnchor">Some motion, like that of amplified
		/// motion, needs some anchor point with which to determine the
		/// amplification from. This might be a shoulder, or a head, or some
		/// other point that the interactor will push from / pull towards.</param>
		/// <param name="secondaryMotion">This is motion that comes from
		/// somewhere other than the interactor itself! This can be something
		/// like an analog stick on a controller, or the scroll wheel of a
		/// mouse.</param>
		/// <param name="active">The activation state of the Interactor.</param>
		/// <param name="tracked">The tracking state of the Interactor.</param>
		public void Update(Vec3 capsuleStart, Vec3 capsuleEnd, Pose motion, Vec3 motionAnchor, Vec3 secondaryMotion, BtnState active, BtnState tracked)
			=> NativeAPI.interactor_update(_inst, capsuleStart, capsuleEnd, motion, motionAnchor, secondaryMotion, active, tracked);

		/// <summary>Interactors, unlike Assets, don't destroy themselves! You
		/// must explicitly Destroy an Interactor if you're finished with it,
		/// otherwise it will continue to interact with StereoKit's 
		/// interactors. This function immediately removes the interactor from
		/// the interactor list.</summary>
		public void Destroy()
			=> NativeAPI.interactor_destroy(_inst);

		/// <summary>If this interactor has an element focused, this will
		/// output information about the location of that element, as well as
		/// the interactor's intersection point with that element.</summary>
		/// <param name="poseWorld">The world space Pose of the element's hierarchy space. This is typically the Pose of the Window/Handle/Surface the element belongs to.</param>
		/// <param name="boundsLocal">The bounds of the UI element relative to the Pose. Note that the `center` should always be accounted for here!</param>
		/// <param name="atLocal">The intersection point relative to the Bounds, NOT relative to the Pose!</param>
		/// <returns>True if bounds data is available.</returns>
		public bool TryGetFocusBounds(out Pose poseWorld, out Bounds boundsLocal, out Vec3 atLocal)
			=> NativeAPI.interactor_get_focus_bounds(_inst, out poseWorld, out boundsLocal, out atLocal);

		/// <summary>Create a new custom Interactor.</summary>
		/// <param name="shapeType">A line, or a point? These interactors
		/// behave slightly differently with respect to distance checks and
		/// directionality. See `InteractorType` for mor details.</param>
		/// <param name="events">What type of interaction events should this
		/// interactor fire? Interaction elements use this bitflag as a filter
		/// to avoid interacting with certain interactors.</param>
		/// <param name="activationType"></param>
		/// <param name="inputSourceId">An identifier that uniquely indicates a
		/// shared source for inputs. This will deactivate other interactors
		/// with a shared source if one is already active. For example, 3
		/// interactors for poke, pinch, and aim on a hand would all come from
		/// a single hand, and if one is actively interacting, then the whole
		/// hand source is considered busy.</param>
		/// <param name="capsuleRadius">The radius of the interactor's capsule,
		/// in meters.</param>
		/// <param name="secondaryMotionDimensions">How many axes of secondary
		/// motion can this interactor provide? This should be 0-3.</param>
		/// <returns>The Interactor that was just created.</returns>
		public static Interactor Create(InteractorType shapeType, InteractorEvent events, InteractorActivation activationType, int inputSourceId, float capsuleRadius, int secondaryMotionDimensions)
			=> new Interactor(NativeAPI.interactor_create(shapeType, events, activationType, inputSourceId, capsuleRadius, secondaryMotionDimensions));

		/// <summary>The number of interactors currently in the system. Can be used with `Get`.</summary>
		public static int Count => NativeAPI.interactor_count();
		/// <summary>Returns the `Interactor` at the given index. Should be used with `Count`.</summary>
		/// <param name="index">The index.</param>
		/// <returns>An Interactor.</returns>
		public static Interactor Get(int index) => new Interactor(NativeAPI.interactor_get(index));
	}
}

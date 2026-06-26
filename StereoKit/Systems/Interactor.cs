// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

using System;
using System.Collections;
using System.Collections.Generic;

namespace StereoKit
{
	/// <summary>Interactors are essentially capsules that allow interaction
	/// with StereoKit's interaction primitives used by the UI system. While
	/// StereoKit does provide a number of interactors by default, you can
	/// replace StereoKit's defaults, add additional interactors, or generally
	/// just customize your interactions!</summary>
	public struct Interactor : IEquatable<Interactor>
	{
		internal int _inst;

		internal Interactor(int inst)
		{
			_inst = inst;
		}

		/// <summary>An empty Interactor that represents "no interactor". UI
		/// building blocks like `UI.ButtonBehavior` and `UI.VolumeAt` report
		/// this when nothing is interacting with them, so you can test their
		/// result against `Interactor.None`.</summary>
		public static Interactor None => new Interactor(-1);

		/// <summary>The distance at which a ray starts being interactive. For
		/// pointing rays, you may not want them to interact right at their
		/// start, or you may want the start to move depending on how
		/// outstretched the hand is! This allows you to change that start
		/// location without affecting the movement caused by the ray, and
		/// still capturing occlusion from blocking elements too close to the
		/// start. By default, this is a large negative value.</summary>
		public float MinDistance {
			get => NativeAPI.interactor_get_min_distance(this);
			set => NativeAPI.interactor_set_min_distance(this, value);
		}
		/// <summary>The world space radius of the interactor capsule, in
		/// meters.</summary>
		public float Radius {
			get => NativeAPI.interactor_get_radius(this);
			set => NativeAPI.interactor_set_radius(this, value);
		}
		/// <summary>The world space start of the interactor capsule. Some
		/// interactions can be directional, especially for `Line` type
		/// interactors, so if you think of the interactor as an "oriented"
		/// capsule, this would be the origin which points towards the capsule
		/// `End`.</summary>
		public Vec3     Start => NativeAPI.interactor_get_capsule_start(this);

		/// <summary>The world space end of the interactor capsule. Some
		/// interactions can be directional, especially for `Line` type
		/// interactors, so if you think of the interactor as an "oriented"
		/// capsule, this would be the end which the `Start`/origin points
		/// towards.</summary>
		public Vec3     End => NativeAPI.interactor_get_capsule_end(this);
		/// <summary>The tracking state of this interactor.</summary>
		public BtnState Tracked => NativeAPI.interactor_get_tracked(this);
		/// <summary>The id of the interaction element that is currently
		/// focused, this will be `IdHash.None` if this interactor has nothing
		/// focused.</summary>
		public IdHash   Focused => NativeAPI.interactor_get_focused(this);
		/// <summary>The id of the interaction element that is currently
		/// active, this will be `IdHash.None` if this interactor has nothing
		/// active. This will always be the same id as `Focused` when not
		/// `None`.</summary>
		public IdHash   Active  => NativeAPI.interactor_get_active (this);
		/// <summary>This pose is the source of translation and rotation motion
		/// caused by the interactor. In most cases it will be the same as your
		/// Start with the orientation of your interactor, but in some instance
		/// may be something else!</summary>
		public Pose     Motion  => NativeAPI.interactor_get_motion (this);

		/// <summary>A line, or a point? These interactors behave slightly
		/// differently with respect to distance checks and directionality. See
		/// `InteractorType` for more details. This is set at creation time and
		/// does not change.</summary>
		public InteractorType       Type       => NativeAPI.interactor_get_type(this);
		/// <summary>What type of interaction events does this interactor fire?
		/// Interaction elements use this bitflag as a filter to avoid
		/// interacting with certain interactors. This is set at creation time
		/// and does not change.</summary>
		public InteractorEvent      Events     => NativeAPI.interactor_get_events(this);
		/// <summary>How does this interactor activate elements? Does it use the
		/// physical position of the interactor, or its activation state? This is
		/// set at creation time and does not change.</summary>
		public InteractorActivation Activation => NativeAPI.interactor_get_activation(this);
		/// <summary>The physical source this interactor's input comes from, such
		/// as a specific hand, controller, or the mouse. Interactors that share
		/// a source will deactivate each other when one becomes active, for
		/// example the poke, pinch, and aim interactors of a single hand.
		/// `InteractorSource.Unique` indicates a source that never groups with
		/// other interactors. This is set at creation time and does not change.
		/// </summary>
		public InteractorSource     Source => NativeAPI.interactor_get_source(this);
		/// <summary>How many axes of secondary motion can this interactor
		/// provide? Secondary motion is input that comes from somewhere other
		/// than the interactor's own movement through space. For example, a
		/// mouse's scroll wheel is 1 axis, and a controller's analog thumbstick
		/// is 2 axes (X/Y). This should be 0-3.</summary>
		public int                  SecondaryDims => NativeAPI.interactor_get_secondary_dims(this);

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
			=> NativeAPI.interactor_update(this, capsuleStart, capsuleEnd, motion, motionAnchor, secondaryMotion, active, tracked);

		/// <summary>Interactors, unlike Assets, don't destroy themselves! You
		/// must explicitly Destroy an Interactor if you're finished with it,
		/// otherwise it will continue to interact with StereoKit's 
		/// interactors. This function immediately removes the interactor from
		/// the interactor list.</summary>
		public void Destroy()
			=> NativeAPI.interactor_destroy(this);

		/// <summary>If this interactor has an element focused, this will
		/// output information about the location of that element, as well as
		/// the interactor's intersection point with that element.</summary>
		/// <param name="poseWorld">The world space Pose of the element's hierarchy space. This is typically the Pose of the Window/Handle/Surface the element belongs to.</param>
		/// <param name="boundsLocal">The bounds of the UI element relative to the Pose. Note that the `center` should always be accounted for here!</param>
		/// <param name="atLocal">The intersection point relative to the Bounds, NOT relative to the Pose!</param>
		/// <returns>True if bounds data is available.</returns>
		public bool TryGetFocusBounds(out Pose poseWorld, out Bounds boundsLocal, out Vec3 atLocal)
			=> NativeAPI.interactor_get_focus_bounds(this, out poseWorld, out boundsLocal, out atLocal);

		/// <summary>Create a new custom Interactor.</summary>
		/// <param name="shapeType">A line, or a point? These interactors
		/// behave slightly differently with respect to distance checks and
		/// directionality. See `InteractorType` for mor details.</param>
		/// <param name="events">What type of interaction events should this
		/// interactor fire? Interaction elements use this bitflag as a filter
		/// to avoid interacting with certain interactors.</param>
		/// <param name="activationType"></param>
		/// <param name="source">The physical source this interactor's input
		/// comes from. Interactors that share a source will deactivate each
		/// other if one is already active. For example, the poke, pinch, and
		/// aim interactors of a single hand all share that hand's source, so if
		/// one is actively interacting the whole hand is considered busy. Use
		/// `InteractorSource.Unique` for a source that never groups with others,
		/// or a custom value at or above `InteractorSource.Max` for your own
		/// sources.</param>
		/// <param name="capsuleRadius">The radius of the interactor's capsule,
		/// in meters.</param>
		/// <param name="secondaryMotionDimensions">How many axes of secondary
		/// motion can this interactor provide? Secondary motion is input from a
		/// source other than the interactor's own movement, such as a mouse's
		/// scroll wheel (1 axis) or a controller's analog thumbstick (2 axes,
		/// X/Y). This should be 0-3.</param>
		/// <returns>The Interactor that was just created.</returns>
		public static Interactor Create(InteractorType shapeType, InteractorEvent events, InteractorActivation activationType, InteractorSource source, float capsuleRadius, int secondaryMotionDimensions)
			=> NativeAPI.interactor_create(shapeType, events, activationType, source, capsuleRadius, secondaryMotionDimensions);

		/// <summary>Is any interactor from the given source currently
		/// interacting with an element, that is, actively pressing or focusing
		/// it? Sources can be combined as a bit-flag to ask about several at
		/// once, e.g. `InteractorSource.HandLeft | InteractorSource.HandRight`.
		/// </summary>
		/// <param name="source">The source, or combination of sources, to check.
		/// </param>
		/// <returns>True if a matching interactor has an active element.</returns>
		public static bool IsInteracting(InteractorSource source) => NativeAPI.interactor_is_interacting(source);

		/// <summary>An enumerable collection of all the Interactors currently
		/// in the system. Use this to inspect or visualize every Interactor,
		/// including any custom ones you've added.</summary>
		public static InteractorCollection All => new InteractorCollection();

		/// <summary>An equality test. Two Interactors are equal when they refer
		/// to the same interactor instance.</summary>
		/// <param name="b">Another Interactor.</param>
		/// <returns>True if equal, false otherwise.</returns>
		public override bool Equals(object b) => (b is Interactor i) && _inst == i._inst;
		/// <inheritdoc cref="Equals(object)"/>
		public bool Equals(Interactor b) => _inst == b._inst;
		/// <summary>A hash code based on the interactor's id.</summary>
		/// <returns>A hash code.</returns>
		public override int GetHashCode() => _inst.GetHashCode();
		/// <summary>An equality test.</summary>
		/// <param name="a">An Interactor.</param>
		/// <param name="b">An Interactor.</param>
		/// <returns>True if equal, false otherwise.</returns>
		public static bool operator ==(Interactor a, Interactor b) => a._inst == b._inst;
		/// <summary>An inequality test.</summary>
		/// <param name="a">An Interactor.</param>
		/// <param name="b">An Interactor.</param>
		/// <returns>True if unequal, false otherwise.</returns>
		public static bool operator !=(Interactor a, Interactor b) => a._inst != b._inst;
	}

	/// <summary>An enumerable collection of all the Interactors in the system.
	/// Retrieved via `Interactor.All`.</summary>
	public struct InteractorCollection : IEnumerable<Interactor>
	{
		/// <summary>The number of Interactors currently in the system.</summary>
		public int Count => NativeAPI.interactor_count();
		/// <summary>Retrieves the Interactor at the given index.</summary>
		/// <param name="index">This should be in the range of [0, Count).</param>
		/// <returns>The Interactor at the given index.</returns>
		public Interactor this[int index] => NativeAPI.interactor_get(index);

		/// <summary>Gets an enumerator for the collection. This returns a
		/// concrete struct enumerator so that `foreach` over the collection
		/// stays allocation-free.</summary>
		/// <returns>An enumerator.</returns>
		public Enumerator GetEnumerator() => new Enumerator(NativeAPI.interactor_count());
		// These keep the collection usable as an IEnumerable (LINQ, etc.), but
		// they box the enumerator. A plain `foreach` binds to the concrete
		// GetEnumerator above and avoids that.
		IEnumerator<Interactor> IEnumerable<Interactor>.GetEnumerator() => GetEnumerator();
		IEnumerator             IEnumerable.GetEnumerator()             => GetEnumerator();

		/// <summary>An allocation-free struct enumerator for an
		/// `InteractorCollection`.</summary>
		public struct Enumerator : IEnumerator<Interactor>
		{
			int _index;
			int _count;
			internal Enumerator(int count) { _index = -1; _count = count; }

			/// <summary>The Interactor at the enumerator's current position.</summary>
			public Interactor Current => NativeAPI.interactor_get(_index);
			object IEnumerator.Current => Current;

			/// <summary>Advances to the next Interactor.</summary>
			/// <returns>False once the end of the collection is reached.</returns>
			public bool MoveNext() => ++_index < _count;
			/// <summary>Resets the enumerator to before the first element.</summary>
			public void Reset() => _index = -1;
			/// <summary>Nothing to dispose, this is here to satisfy the
			/// IEnumerator interface.</summary>
			public void Dispose() { }
		}
	}
}

// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace StereoKit {
	/// <summary> An Anchor in StereoKit is a completely virtual pose that is
	/// pinned to a real-world location. They are creatable via code, generally
	/// can persist across sessions, may provide additional stability beyond
	/// the system's 6dof tracking, and are not physical objects!
	/// 
	/// This functionality is backed by extensions like the Microsoft Spatial
	/// Anchor, or the Facebook Spatial Entity. If a proper anchoring system
	/// isn't present on the device, StereoKit will fall back to a stage-
	/// relative anchor. Stage-relative anchors may be a good solution for
	/// devices with a consistent stage, but may be troublesome if the user
	/// adjusts their stage frequently.
	/// 
	/// A conceptual guide to Anchors:
	/// - A cloud anchor is an Anchor
	/// - A QR code is _not_ an Anchor (it's physical)
	/// - That spot around where your coffee usually sits can be an Anchor
	/// - A semantically labeled floor plane is _not_ an Anchor (it's physical)
	/// </summary>
	public class Anchor : IAsset
	{
		internal IntPtr _inst;

		/// <summary>Gets or sets the unique identifier of this asset resource!
		/// This can be helpful for debugging, managing your assets, or finding
		/// them later on! This is StereoKit's asset ID, and not the system's
		/// unique Name for the anchor.</summary>
		public string Id
		{
			get { unsafe { return NU8.Str(NativeAPI.anchor_get_id(_inst)); } }
			set { unsafe { byte* val = NU8.Bytes(value); NativeAPI.anchor_set_id(_inst, val); NU8.Free(val); } }
		}
		/// <summary>The most recently identified Pose of the Anchor. While an
		/// Anchor will generally be in the same position once discovered, it
		/// may shift slightly to compensate for drift in the device's 6dof
		/// tracking. Anchor Poses when tracked are more accurate than
		/// world-space positions.</summary>
		public Pose Pose => NativeAPI.anchor_get_pose(_inst);
		/// <summary>Does the device consider this Anchor to be tracked? This
		/// doesn't require the Anchor to be visible, just that the device
		/// knows where this Anchor is located.</summary>
		public BtnState Tracked => NativeAPI.anchor_get_tracked(_inst);
		/// <summary>Will this Anchor persist across multiple app sessions? You
		/// can use `TrySetPersistent` to change this value.</summary>
		public bool Persistent => NativeAPI.anchor_get_persistent(_inst)>0;
		/// <summary>A unique system provided name identifying this anchor.
		/// This will be the same across sessions for persistent Anchors.
		/// </summary>
		public string Name { get { unsafe { return NU8.Str(NativeAPI.anchor_get_name(_inst)); } } }
		
		/// <summary>Tries to get the underlying perception spatial anchor 
		/// for platforms using Microsoft spatial anchors.</summary>
		/// <typeparam name="T">The type of the spatial anchor. Must be an instance 
		/// of Windows.Perception.Spatial.SpatialAnchor.</typeparam>
		/// <param name="spatialAnchor">The spatial anchor.</param>
		/// <returns>True if the perception spatial anchor was successfully obtained, false otherwise.</returns>
		public bool TryGetPerceptionAnchor<T>(out T spatialAnchor) where T : class
		{
			spatialAnchor = null;
			if (!RuntimeInformation.IsOSPlatform(OSPlatform.Windows)) return false;

			unsafe
			{
				void* anchor;
				bool  result = NB.Bool(NativeAPI.anchor_get_perception_anchor(_inst, &anchor));
				if (result)
					spatialAnchor = Marshal.GetObjectForIUnknown((nint)anchor) as T;
				return result;
			}
		}

		/// <summary>Tries to get the underlying perception spatial anchor as a COM pointer.
		/// Use this when you need the raw IntPtr for interop or custom marshalling.</summary>
		/// <param name="spatialAnchor">The raw COM pointer to the spatial anchor.</param>
		/// <returns>True if the pointer was successfully obtained, false otherwise.</returns>
		public bool TryGetPerceptionAnchor(out IntPtr spatialAnchor)
		{
			spatialAnchor = IntPtr.Zero;
			if (!RuntimeInformation.IsOSPlatform(OSPlatform.Windows)) return false;

			unsafe
			{
				void* anchor;
				bool  result = NB.Bool(NativeAPI.anchor_get_perception_anchor(_inst, &anchor));
				if (result)
					spatialAnchor = (IntPtr)anchor;
				return result;
			}
		}
		
		internal Anchor(IntPtr anchor)
		{
			_inst = anchor;
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty Anchor!");
		}
		/// <summary>Release reference to the StereoKit asset.</summary>
		~Anchor()
		{ unsafe {
			if (_inst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe((void*)_inst);
		} }

		/// <summary>This will attempt to make or prevent this Anchor from
		/// persisting across app sessions. You may want to check if the system
		/// is capable of persisting anchors via Anchors.Capabilities, but it's
		/// possible for this to fail on the OpenXR runtime's side as well.
		/// </summary>
		/// <param name="persistent">Whether this should or shouldn't persist
		/// across sessions.</param>
		/// <returns>Success or failure of setting persistence.</returns>
		public bool TrySetPersistent(bool persistent) => NB.Bool(NativeAPI.anchor_try_set_persistent(_inst, NB.Int(persistent)));

		/// <summary>This creates a new Anchor from a world space pose.</summary>
		/// <param name="pose">A world space pose for the new Anchor.</param>
		/// <returns>A new Anchor at the Pose provided. This Anchor is not
		/// guaranteed to be tracked, but will start at the Pose provided.
		/// </returns>
		public static Anchor FromPose(Pose pose)
		{
			IntPtr anchor = NativeAPI.anchor_create(pose);
			return anchor == IntPtr.Zero ? null : new Anchor(anchor);
		}

		/// <summary>This describes the anchoring capabilities of the current
		/// XR anchoring backend. Some systems like a HoloLens can create
		/// Anchors that provide stability, and can persist across multiple
		/// sessions. Some like SteamVR might be able to make a persistent
		/// Anchor that's relative to the stage, but doesn't provide any
		/// stability benefits.</summary>
		public static AnchorCaps Capabilities => NativeAPI.anchor_get_capabilities();

		/// <summary>This will remove persistence from all Anchors the app
		/// knows about, even if they aren't tracked.</summary>
		public static void ClearStored() => NativeAPI.anchor_clear_stored();

		/// <summary>This is an enumeration of all Anchors that exist in
		/// StereoKit at the current moment.</summary>
		public static IEnumerable<Anchor> Anchors { get
		{
			int count = NativeAPI.anchor_get_count();
			for (int i = 0; i < count; i++)
			{
				IntPtr inst   = NativeAPI.anchor_get_index(i);
				Anchor anchor = new Anchor(inst);
				if (anchor != null)
					yield return anchor;
			}
		}}

		/// <summary>This is an enumeration of all Anchors that are new to
		/// StereoKit this frame.</summary>
		public static IEnumerable<Anchor> NewAnchors { get
		{
			int count = NativeAPI.anchor_get_new_count();
			for (int i = 0; i < count; i++)
			{
				IntPtr inst   = NativeAPI.anchor_get_new_index(i);
				Anchor anchor = new Anchor(inst);
				if (anchor != null)
					yield return anchor;
			}
		}}
	}
}

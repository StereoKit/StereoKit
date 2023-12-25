using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace StereoKit {
	public class Anchor : IAsset
	{
		internal IntPtr _inst;

		public string Id
		{
			get => NativeHelper.FromUtf8(NativeAPI.anchor_get_id(_inst));
			set => NativeAPI.anchor_set_id(_inst, NativeHelper.ToUtf8(value));
		}
		public Pose Pose => NativeAPI.anchor_get_pose(_inst);
		public BtnState Tracked => NativeAPI.anchor_get_tracked(_inst);
		public bool Persistent => NativeAPI.anchor_get_persistent(_inst);
		public string Name => NativeHelper.FromUtf8(NativeAPI.anchor_get_name(_inst));
		public bool TryGetPerceptionAnchor<T>(out T spatialAnchor) where T : class
		{
			spatialAnchor = null;
			bool result = NativeAPI.anchor_get_perception_anchor(_inst, out IntPtr pointer);
			if (result)
			{
				spatialAnchor = (T)Marshal.GetObjectForIUnknown(pointer);
			}
			return result;
		}
		
		internal Anchor(IntPtr anchor)
		{
			_inst = anchor;
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty Anchor!");
		}
		/// <summary>Release reference to the StereoKit asset.</summary>
		~Anchor()
		{
			if (_inst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe(_inst);
		}

		public bool TrySetPersistent(bool persistent) => NativeAPI.anchor_try_set_persistent(_inst, persistent);

		public static Anchor FromPose(Pose pose)
		{
			IntPtr anchor = NativeAPI.anchor_create(pose);
			return anchor == IntPtr.Zero ? null : new Anchor(anchor);
		}


		public static AnchorCaps Capabilities => NativeAPI.anchor_get_capabilities();
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

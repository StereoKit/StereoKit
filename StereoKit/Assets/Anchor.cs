using System;

namespace StereoKit {
	public class Anchor : IAsset
	{
		internal IntPtr _inst;
		public string Id
		{
			get => NativeHelper.FromUtf8(NativeAPI.anchor_get_id(_inst));
			set => NativeAPI.anchor_set_id(_inst, NativeHelper.ToUtf8(value));
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

		public static Anchor FromPose(string uniqueName, Pose pose)
		{
			IntPtr anchor = NativeAPI.anchor_create(NativeHelper.ToUtf8(uniqueName), pose);
			return anchor == IntPtr.Zero ? null : new Anchor(anchor);
		}
	}
}

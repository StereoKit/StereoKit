using System;

namespace StereoKit.Framework
{
	class XrRefreshRate : IStepper
	{
		int _rate;
		public bool Enabled { get; private set; }

		public XrRefreshRate() : this(0) { }
		public XrRefreshRate(int rate)
		{
			_rate = rate;

			if (SK.IsInitialized)
				Log.Err("XrRefreshRate must be Setup before StereoKit is initialized!");

			Backend.OpenXR.RequestExt("XR_FB_display_refresh_rate");
		}

		public bool Initialize()
		{
			Enabled =
				Backend.XRType == BackendXRType.OpenXR &&
				Backend.OpenXR.ExtEnabled("XR_FB_display_refresh_rate") &&
				LoadBindings();

			if (Enabled && _rate != 0)
				SetRefreshRate(_rate);

			return true;
		}

		public void SetRefreshRate(int rate)
		{
			if (Enabled)
				xrRequestDisplayRefreshRateFB(Backend.OpenXR.Session, rate);
		}

		public void Shutdown() { Enabled = false; }
		public void Step() { }

		#region OpenXR Bindings
		enum XrResult : Int32 { Success = 0 }

		delegate XrResult del_xrRequestDisplayRefreshRateFB( ulong session, float displayRefreshRate);
		del_xrRequestDisplayRefreshRateFB xrRequestDisplayRefreshRateFB;

		bool LoadBindings()
		{
			xrRequestDisplayRefreshRateFB = Backend.OpenXR.GetFunction<del_xrRequestDisplayRefreshRateFB>("xrRequestDisplayRefreshRateFB");

			return xrRequestDisplayRefreshRateFB != null;
		}
		#endregion
	}
}

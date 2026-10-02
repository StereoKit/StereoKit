using System;

namespace StereoKit
{
	/// <summary>A desktop OS window belonging to the app, like the one the
	/// Simulator and Window app modes render into. There is no window
	/// available in XR mode, so if you're working with this class, be prepared
	/// to gate against `AppWindow` objects being null!</summary>
	public class AppWindow
	{
		internal IntPtr _inst;
		AppWindow(IntPtr inst) => _inst = inst;

		static AppWindow _main;
		/// <summary>The app's main window! Only the Simulator and Window app
		/// modes have one, everywhere else this is null. This handle belongs
		/// to StereoKit, so don't hold onto it across SK.Shutdown.</summary>
		public static AppWindow Main { get {
			IntPtr inst = NativeAPI.window_get_main();
			if (inst == IntPtr.Zero) return null;
			if (_main == null || _main._inst != inst) _main = new AppWindow(inst);
			return _main;
		} }

		/// <summary>Is this window currently covering its whole display? This
		/// is always the window's real state, so it also picks up fullscreen
		/// changes the user made through the window manager. Use
		/// RequestFullscreen to change it.</summary>
		public bool Fullscreen => NativeAPI.window_get_fullscreen(_inst);

		/// <summary>The width of the window's drawable area, in physical
		/// pixels. This is the size the swapchain renders at, and it changes
		/// whenever the window is resized.</summary>
		public int Width { get {
			NativeAPI.window_get_size(_inst, out int width, out int height);
			return width;
		} }

		/// <summary>The height of the window's drawable area, in physical
		/// pixels. This is the size the swapchain renders at, and it changes
		/// whenever the window is resized.</summary>
		public int Height { get {
			NativeAPI.window_get_size(_inst, out int width, out int height);
			return height;
		} }

		/// <summary>Asks for this window to cover its whole display, or to go
		/// back to a normal window. This is only ever a request, and it's
		/// never immediate! Window managers can refuse it, browsers wait for
		/// a user gesture, and some platforms don't implement it at all. None
		/// of those report back a refusal, so watch the Fullscreen property
		/// to see if and when it takes effect. Going fullscreen resizes the
		/// window, so expect the render surface to follow along.</summary>
		/// <param name="fullscreen">True to ask for fullscreen, false to ask
		/// for a normal window.</param>
		public void RequestFullscreen(bool fullscreen) => NativeAPI.window_request_fullscreen(_inst, fullscreen);
	}
}

namespace StereoKit.Framework
{
	/// <summary>This is a lightweight standard interface for fire-and-forget
	/// systems that can be attached to StereoKit! This is particularly handy
	/// for extensions/plugins that need to run in the background of your
	/// application, or even for managing some of your own simpler systems.
	/// 
	/// `IStepper`s can be added before or after the call to `SK.Initialize`,
	/// and this does affect when the `IStepper.Initialize` call will be made!
	/// `IStepper.Initialize` is always called _after_ `SK.Initialize`. This
	/// can be important to note when writing code that uses SK functions that
	/// are dependant on initialization, you'll want to avoid putting this code
	/// in the constructor, and add them to `Initialize` instead.
	/// 
	/// `IStepper`s also pay attention to threading! `Initialize` and `Step`
	/// always happen on the main thread, even if the constructor is called on
	/// a different one.</summary>
	public interface IStepper
	{
		/// <summary>This is called by StereoKit at the start of the next
		/// frame, and on the main thread. This happens before StereoKit's main
		/// `Step` callback, and always after `SK.Initialize`.</summary>
		/// <returns>If false is returned here, this `IStepper` will be
		/// immediately removed from the list of `IStepper`s, and neither
		/// `Step` nor `Shutdown` will be called.</returns>
		bool Initialize();

		/// <summary>This Step method will be called every frame of the
		/// application, as long as `Enabled` is `true`. This happens
		/// immediately before the main application's `Step` callback.
		/// </summary>
		void Step();

		/// <summary>This is called when the `IStepper` is removed, or the
		/// application shuts down. This is always called on the main thread,
		/// and happens at the start of the next frame, before the main
		/// application's `Step` callback.</summary>
		void Shutdown();
	}
}

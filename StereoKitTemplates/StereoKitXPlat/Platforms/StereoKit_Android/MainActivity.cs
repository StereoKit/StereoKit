using Android.App;
using Android.OS;
using Android.Support.V7.App;
using Android.Runtime;
using Android.Views;
using Android.Content;
using StereoKit;
using System;
using Android.Graphics;
using Java.Lang;
using System.Threading.Tasks;
using StereoKitApp;

namespace StereoKit_Android
{
	[Activity(Label = "@string/app_name", Theme = "@style/AppTheme", MainLauncher = true)]
	[IntentFilter(new[] { Intent.ActionMain }, Categories = new[] { "com.oculus.intent.category.VR", Intent.CategoryLauncher })]
	public class MainActivity : AppCompatActivity, ISurfaceHolderCallback2
	{
		App  app;
		View surface;

		protected override void OnCreate(Bundle savedInstanceState)
		{
			JavaSystem.LoadLibrary("openxr_loader");
			JavaSystem.LoadLibrary("StereoKitC");

			// Set up a surface for StereoKit to draw on
			Window.TakeSurface(this);
			Window.SetFormat(Format.Unknown);
			surface = new View(this);
			SetContentView(surface);
			surface.RequestFocus();

			base.OnCreate(savedInstanceState);
			Xamarin.Essentials.Platform.Init(this, savedInstanceState);

			Run(Handle);
		}
		public override void OnRequestPermissionsResult(int requestCode, string[] permissions, [GeneratedEnum] Android.Content.PM.Permission[] grantResults)
		{
			Xamarin.Essentials.Platform.OnRequestPermissionsResult(requestCode, permissions, grantResults);
			base.OnRequestPermissionsResult(requestCode, permissions, grantResults);
		}

		static bool running = false;
		void Run(IntPtr activityHandle)
		{
			if (running)
				return;
			running = true;

			Task.Run(() => {
				// If the app has a constructor that takes a string array, then
				// we'll use that, and pass the command line arguments into it on
				// creation
				Type appType = typeof(App);
				app = appType.GetConstructor(new Type[] { typeof(string[]) }) != null
					? (App)Activator.CreateInstance(appType, new object[] { new string[0] { } })
					: (App)Activator.CreateInstance(appType);
				if (app == null)
					throw new System.Exception("StereoKit loader couldn't construct an instance of the App!");

				// Initialize StereoKit, and the app
				SKSettings settings = app.Settings;
				settings.androidActivity = activityHandle;
				if (!SK.Initialize(settings))
					return;
				app.Init();

				// Now loop until finished, and then shut down
				while (SK.Step(app.Step)) { }
				SK.Shutdown();
				Android.OS.Process.KillProcess(Android.OS.Process.MyPid());
			});
		}

		// Events related to surface state changes
		public void SurfaceChanged     (ISurfaceHolder holder, [GeneratedEnum] Format format, int width, int height) => SK.SetWindow(holder.Surface.Handle);
		public void SurfaceCreated     (ISurfaceHolder holder) => SK.SetWindow(holder.Surface.Handle);
		public void SurfaceDestroyed   (ISurfaceHolder holder) => SK.SetWindow(IntPtr.Zero);
		public void SurfaceRedrawNeeded(ISurfaceHolder holder) { }
	}
}
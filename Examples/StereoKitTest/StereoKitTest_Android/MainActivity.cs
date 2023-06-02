using Android;
using Android.App;
using Android.Content;
using Android.Graphics;
using Android.OS;
using Android.Runtime;
using Android.Views;
using AndroidX.AppCompat.App;
using AndroidX.Core.App;
using AndroidX.Core.Content;
using StereoKit;
using System;
using System.Threading.Tasks;

namespace StereoKitTest_Android
{
	// NativeActivity seems to work fine, so here's a link to that code
	// https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/app/NativeActivity.java
	// Which also calls loadNativeCode_native over here:
	// https://android.googlesource.com/platform/frameworks/base.git/+/android-4.3_r3.1/core/jni/android_app_NativeActivity.cpp
	// Additional ref here:
	// https://github.com/spurious/SDL-mirror/blob/6fe5bd1536beb197de493c9b55d16e516219c58f/android-project/app/src/main/java/org/libsdl/app/SDLActivity.java#L1663
	// https://github.com/MonoGame/MonoGame/blob/31dca640482bc0c27aec8e51d6369612ce8577a2/MonoGame.Framework/Platform/Android/MonoGameAndroidGameView.cs
	[Activity(Label = "@string/app_name", Theme = "@style/AppTheme", MainLauncher = true)]
	[IntentFilter(new[] { Intent.ActionMain }, Categories = new[] { "org.khronos.openxr.intent.category.IMMERSIVE_HMD", "com.oculus.intent.category.VR", Intent.CategoryLauncher })]
	public class MainActivity : AppCompatActivity, ISurfaceHolderCallback2
	{
		App  app;
		View surface;

		protected override void OnCreate(Bundle savedInstanceState)
		{
			base.OnCreate(savedInstanceState);
			Xamarin.Essentials.Platform.Init(this, savedInstanceState);
			Run();
		}
		protected override void OnDestroy()
		{
			SK.Quit();
			base.OnDestroy();
		}
		public override void OnRequestPermissionsResult(int requestCode, string[] permissions, [GeneratedEnum] Android.Content.PM.Permission[] grantResults)
		{
			Xamarin.Essentials.Platform.OnRequestPermissionsResult(requestCode, permissions, grantResults);
			base.OnRequestPermissionsResult(requestCode, permissions, grantResults);
		}

		static bool running = false;
		void Run()
		{
			if (running) return;
			running = true;

			// Start up a thread for StereoKit to run in
			Task.Run(() => {
				// If the app has a constructor that takes a string array, then
				// we'll use that, and pass the command line arguments into it
				// on creation
				Type appType = typeof(App);
				app = appType.GetConstructor(new Type[] { typeof(string[]) }) != null
					? (App)Activator.CreateInstance(appType, new object[] { new string[0] { } })
					: (App)Activator.CreateInstance(appType);
				if (app == null)
					throw new Exception("StereoKit loader couldn't construct an instance of the App!");

				// Set up a surface for StereoKit to draw on, this is only really
				// important for flatscreen experiences.
				if (app.Settings.displayPreference == DisplayMode.Flatscreen)
				{
					Window.TakeSurface(this);
					Window.SetFormat(Format.Unknown);
					surface = new View(this);
					SetContentView(surface);
					surface.RequestFocus();
				}

				// For requesting permission to use the microphone and eye
				// tracking
				if (ContextCompat.CheckSelfPermission(this, Manifest.Permission.RecordAudio) != Android.Content.PM.Permission.Granted)
					ActivityCompat.RequestPermissions(this, new string[] { Manifest.Permission.RecordAudio }, 1);
				if (ContextCompat.CheckSelfPermission(this, "com.oculus.permission.EYE_TRACKING") != Android.Content.PM.Permission.Granted)
					ActivityCompat.RequestPermissions(this, new string[] { "com.oculus.permission.EYE_TRACKING" }, 1);

				// Initialize StereoKit, and the app
				if (!SK.Initialize(app.Settings))
					return;
				app.Init();

				// Now loop until finished, and then shut down
				SK.Run(app.Step);

				Process.KillProcess(Process.MyPid());
			});
		}

		// Events related to surface state changes
		public void SurfaceChanged     (ISurfaceHolder holder, [GeneratedEnum] Format format, int width, int height) => SK.SetWindow(holder.Surface.Handle);
		public void SurfaceCreated     (ISurfaceHolder holder) => SK.SetWindow(holder.Surface.Handle);
		public void SurfaceDestroyed   (ISurfaceHolder holder) => SK.SetWindow(IntPtr.Zero);
		public void SurfaceRedrawNeeded(ISurfaceHolder holder) { }
	}
}
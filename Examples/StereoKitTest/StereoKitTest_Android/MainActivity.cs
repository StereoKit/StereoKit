using Android.App;
using Android.Content;
using Android.Graphics;
using Android.OS;
using Android.Runtime;
using Android.Views;
using Java.Lang;
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
	[Activity(Label = "@string/app_name", Theme = "@style/AppTheme")]
	[IntentFilter(new[] { Intent.ActionMain }, Categories = new[] { "com.oculus.intent.category.VR", Intent.CategoryLauncher })]
	public class MainActivity : Activity, ISurfaceHolderCallback2
	{
		View surface;

		protected override void OnCreate(Bundle savedInstanceState)
		{
			JavaSystem.LoadLibrary("StereoKitC");
			JavaSystem.LoadLibrary("openxr_loader");

			Window.TakeSurface(this);
			Window.SetFormat  (Format.Unknown);

			surface       = new View(this);
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
		static void Run(IntPtr activityHandle)
		{
			if (running)
				return;
			running = true;

			Android.Util.Log.Info("StereoKitTest", "Starting StereoKit thread...");

			Task.Run(() => {
				StereoKit.Log.Filter = LogLevel.Diagnostic;
				SK.settings.androidActivity = activityHandle;
				if (!SK.Initialize("StereoKitTemplate"))
					return;

				// Create assets used by the app
				Pose  cubePose = new Pose(0, 0, -0.5f, Quat.Identity);
				Model cube     = Model.FromMesh(
					Mesh.GenerateRoundedCube(Vec3.One * 0.1f, 0.02f),
					Default.MaterialUI);

				// Core application loop
				while (SK.Step(() =>
				{
					Text.Add(Time.Elapsed + "s", StereoKit.Matrix.Identity);
					UI.Handle("Cube", ref cubePose, cube.Bounds);
					cube.Draw(cubePose.ToMatrix());
				})) ;
				SK.Shutdown();
			});
		}

		public void SurfaceChanged     (ISurfaceHolder holder, [GeneratedEnum] Format format, int width, int height) { SK.SetWindow(holder.Surface.Handle); Android.Util.Log.Info("StereoKitTest", "OnChanged: " + width + "x" + height); }
		public void SurfaceCreated     (ISurfaceHolder holder) { SK.SetWindow(holder.Surface.Handle); }
		public void SurfaceDestroyed   (ISurfaceHolder holder) { SK.SetWindow(IntPtr.Zero); Android.Util.Log.Info("StereoKitTest", "Destroyed"); }
		public void SurfaceRedrawNeeded(ISurfaceHolder holder) { }
	}
}
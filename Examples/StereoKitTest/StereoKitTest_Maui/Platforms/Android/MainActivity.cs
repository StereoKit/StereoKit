using Android.App;
using Android.Content;
using Android.Graphics;
using Android.OS;
using Android.Runtime;
using Android.Views;
using AndroidX.AppCompat.App;
using StereoKit;
using System;
using System.Threading.Tasks;

namespace SKTemplate_Maui;

[Activity(Theme = "@style/Maui.SplashTheme", MainLauncher = true, Exported = true)]
[IntentFilter(new[] { Intent.ActionMain }, Categories = new[] { "org.khronos.openxr.intent.category.IMMERSIVE_HMD", "com.oculus.intent.category.VR", Intent.CategoryLauncher })]
public class MainActivity : AppCompatActivity, ISurfaceHolderCallback2
{
	App  app;
	View surface;

	protected override void OnCreate(Bundle savedInstanceState)
	{
		base.OnCreate(savedInstanceState);
		Microsoft.Maui.ApplicationModel.Platform.Init(this, savedInstanceState);
		Run();
	}
	protected override void OnDestroy()
	{
		SK.Quit();
		base.OnDestroy();
	}
	public override void OnRequestPermissionsResult(int requestCode, string[] permissions, [GeneratedEnum] Android.Content.PM.Permission[] grantResults)
	{
		Microsoft.Maui.ApplicationModel.Platform.OnRequestPermissionsResult(requestCode, permissions, grantResults);
		base.OnRequestPermissionsResult(requestCode, permissions, grantResults);
	}

	static bool running = false;
	void Run()
	{
		if (running) return;
		running = true;

		// Before anything else, give StereoKit the Activity. This should
		// be set before any other SK calls, otherwise native library
		// loading may fail.
		SK.AndroidActivity = this;

		Task.Run(() => {
			// If the app has a constructor that takes a string array, then
			// we'll use that, and pass the command line arguments into it on
			// creation
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
using Android.App;
using Android.Content;
using Android.Graphics;
using Android.OS;
using Android.Runtime;
using Android.Views;
using StereoKit;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Threading;

[Activity(Label = "@string/app_name", MainLauncher = true, Exported = true)]
[IntentFilter(new[] { Intent.ActionMain }, Categories = new[] { "org.khronos.openxr.intent.category.IMMERSIVE_HMD", "com.oculus.intent.category.VR", Intent.CategoryLauncher })]
public class MainActivity : Activity, ISurfaceHolderCallback2
{
	View surface;

	protected override void OnCreate(Bundle savedInstanceState)
	{
		base.OnCreate(savedInstanceState);

		// Set up a surface for StereoKit to draw on, this is only really
		// important for flatscreen experiences.
		Window.TakeSurface(this);
		Window.SetFormat  (Format.Unknown);
		surface = new View(this);
		SetContentView(surface);
		surface.RequestFocus();

		Run();
		SetContentView(new View(this));
	}

	protected override void OnDestroy()
	{
		// Quit, but not if Destroy is just a rotation or resize
		if (IsChangingConfigurations == false)
			SK.Quit();

		base.OnDestroy();
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

		SK.ExecuteOnMain(() =>
		{
			// For requesting permission to use the microphone, eye tracking, and spatial mapping
			List<string> permissions = new();
			if (CheckSelfPermission(Android.Manifest.Permission.RecordAudio) != Android.Content.PM.Permission.Granted)
				permissions.Add(Android.Manifest.Permission.RecordAudio);
			if (CheckSelfPermission("com.magicleap.permission.EYE_TRACKING") != Android.Content.PM.Permission.Granted)
				permissions.Add("com.magicleap.permission.EYE_TRACKING");
			if (CheckSelfPermission("com.magicleap.permission.SPATIAL_MAPPING") != Android.Content.PM.Permission.Granted)
				permissions.Add("com.magicleap.permission.SPATIAL_MAPPING");
			if (permissions.Count > 0)
				RequestPermissions(permissions.ToArray(), permissions.Count);
		});

		// Task.Run will eat exceptions, but Thread.Start doesn't seem to.
		new Thread(InvokeStereoKit).Start();
	}

	static void InvokeStereoKit()
	{
		Type       entryClass = typeof(Program);
		MethodInfo entryPoint = entryClass?.GetMethod("Main", BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic);
		
		// There are a number of potential method signatures for Main, so
		// we need to check each one, and give it the correct values.
		//
		// Converting MethodInfo into an Action instead of calling Invoke on
		// it allows for exceptions to properly bubble up to the IDE.
		ParameterInfo[] entryParams = entryPoint?.GetParameters();
		if (entryParams == null || entryParams.Length == 0)
		{
			Action Program_Main = (Action)Delegate.CreateDelegate(typeof(Action), entryPoint);
			Program_Main();
		}
		else if (entryParams?.Length == 1 && entryParams[0].ParameterType == typeof(string[]))
		{
			Action<string[]> Program_Main = (Action<string[]>)Delegate.CreateDelegate(typeof(Action<string[]>), entryPoint);
			Program_Main(new string[] { });
		}
		else throw new Exception("Couldn't invoke Program.Main!");

		Process.KillProcess(Process.MyPid());
	}

	// Events related to surface state changes
	public void SurfaceChanged     (ISurfaceHolder holder, [GeneratedEnum] Format format, int width, int height) => SK.SetWindow(holder.Surface.Handle);
	public void SurfaceCreated     (ISurfaceHolder holder) => SK.SetWindow(holder.Surface.Handle);
	public void SurfaceDestroyed   (ISurfaceHolder holder) => SK.SetWindow(IntPtr.Zero);
	public void SurfaceRedrawNeeded(ISurfaceHolder holder) { }
}
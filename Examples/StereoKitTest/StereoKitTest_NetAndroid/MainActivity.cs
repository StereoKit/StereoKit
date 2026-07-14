using Android.App;
using Android.Content;
using Android.Content.PM;
using Android.Graphics;
using Android.OS;
using Android.Runtime;
using Android.Views;
using StereoKit;
using System;
using System.Reflection;
using System.Threading;

[Activity(Label = "@string/app_name", MainLauncher = true, Exported = true, LaunchMode = LaunchMode.SingleTask)]
[IntentFilter(new[] { Intent.ActionMain }, Categories = new[] { "org.khronos.openxr.intent.category.IMMERSIVE_HMD", "com.oculus.intent.category.VR", Intent.CategoryLauncher })]
public class MainActivity : Activity, ISurfaceHolderCallback2
{
	View   surface;
	Thread skThread;

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
	}

	protected override void OnDestroy()
	{
		// Quit, but not if Destroy is just a rotation or resize
		if (IsChangingConfigurations == false)
		{
			// SK.Quit only signals; wait for the SK thread to finish its OpenXR
			// teardown before Android tears us down.
			SK.Quit();
			skThread?.Join();
		}

		base.OnDestroy();
	}

	static bool running = false;
	void Run()
	{
		if (running) return;
		running = true;

		// Before anything else, give StereoKit the Activity and VM. These
		// should be set before any other SK calls, otherwise native
		// library loading may fail.
		SK.AndroidActivity = this;
		// This is optional, but helps with compatibility on older devices,
		// Android API 30 and older.
		SK.AndroidJavaVM   = Java.Interop.JniEnvironment.Runtime.InvocationPointer;

		// Task.Run will eat exceptions, but Thread.Start doesn't seem to.
		skThread = new Thread(InvokeStereoKit);
		skThread.Start();
	}

	void InvokeStereoKit()
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

		// SK has fully shut down. Finish the Activity so Android's task state is
		// tidy, then kill the process so the next launch re-inits SK from scratch.
		Finish();
		Process.KillProcess(Process.MyPid());
	}

	// Events related to surface state changes
	public void SurfaceChanged     (ISurfaceHolder holder, [GeneratedEnum] Format format, int width, int height) => SK.SetWindow(holder.Surface.Handle);
	public void SurfaceCreated     (ISurfaceHolder holder) => SK.SetWindow(holder.Surface.Handle);
	public void SurfaceDestroyed   (ISurfaceHolder holder) => SK.SetWindow(IntPtr.Zero);
	public void SurfaceRedrawNeeded(ISurfaceHolder holder) { }
}
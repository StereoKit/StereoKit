using Android.App;
using Android.Content;
using Android.OS;
using StereoKit;
using System;
using System.Reflection;
using System.Threading.Tasks;

[Activity(Label = "@string/app_name", MainLauncher = true, Exported = true)]
[IntentFilter(new[] { Intent.ActionMain }, Categories = new[] { "org.khronos.openxr.intent.category.IMMERSIVE_HMD", "com.oculus.intent.category.VR", Intent.CategoryLauncher })]
public class MainActivity : Activity
{
	protected override void OnCreate(Bundle savedInstanceState)
	{
		base.OnCreate(savedInstanceState);
		Run();
		SetContentView(StereoKitTest_NetAndroid.Resource.Layout.activity_main);
	}

	protected override void OnDestroy()
	{
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
		
		Task.Run(() => {
			Type       entryClass = typeof(Program);
			MethodInfo entryPoint = entryClass?.GetMethod("Main", BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic);

			// There are a number of potential method signatures for Main, so
			// we need to check each one, and give it the correct values.
			ParameterInfo[] entryParams = entryPoint?.GetParameters();
			if (entryParams == null || entryParams.Length == 0)
				entryPoint.Invoke(null, null);
			else if (entryParams?.Length == 1 && entryParams[0].ParameterType == typeof(string[]))
				entryPoint.Invoke(null, new object[] { new string[0] });
			else
				throw new Exception("Couldn't invoke Program.Main!");

			Process.KillProcess(Process.MyPid());
		});
	}
}
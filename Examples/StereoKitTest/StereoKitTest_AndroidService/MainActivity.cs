using Android.App;
using Android.Content;
using Android.Content.PM;
using Android.OS;
using Android.Runtime;
using StereoKit;
using System;
using System.Reflection;
using System.Threading;

// Some SK users run StereoKit inside a Service rather than an Activity.
// Services provide a different Android Context that StereoKit treats
// differently (e.g. no interactive permission requests), so this project
// exists to exercise that code path.
//
// The launcher Activity stays alive because the Meta Quest OpenXR runtime
// ties XR session lifecycle to the Activity lifecycle. Without a resumed
// Activity, the session stays in XR_SESSION_STATE_IDLE and never renders.
// The Activity exists only to satisfy this requirement — all SK work
// happens in the Service using a Service context.

[Activity(Label = "@string/app_name", MainLauncher = true, Exported = true, LaunchMode = LaunchMode.SingleTask)]
[IntentFilter(new[] { Intent.ActionMain }, Categories = new[] { "org.khronos.openxr.intent.category.IMMERSIVE_HMD", "com.oculus.intent.category.VR", Intent.CategoryLauncher })]
public class MainActivity : Activity
{
	protected override void OnCreate(Bundle savedInstanceState)
	{
		base.OnCreate(savedInstanceState);
		var intent = new Intent(this, typeof(SKService));
		StartForegroundService(intent);
	}

	protected override void OnDestroy()
	{
		SK.Quit();
		base.OnDestroy();
	}
}

[Service(Label = "@string/app_name", Exported = true, ForegroundServiceType = ForegroundService.TypeConnectedDevice)]
public class SKService : Service
{
	const int    NotificationId = 1;
	const string ChannelId      = "sk_service_channel";

	public override IBinder OnBind(Intent intent) => null;

	public override StartCommandResult OnStartCommand(Intent intent, StartCommandFlags flags, int startId)
	{
		StartInForeground();
		Run();
		return StartCommandResult.Sticky;
	}

	public override void OnDestroy()
	{
		base.OnDestroy();
	}

	void StartInForeground()
	{
		// Android 8+ requires a notification channel for foreground
		// services.
		if (Build.VERSION.SdkInt >= BuildVersionCodes.O)
		{
			var channel = new NotificationChannel(
				ChannelId,
				"StereoKit Service",
				NotificationImportance.Low);
			GetSystemService(NotificationService)
				.JavaCast<NotificationManager>()
				.CreateNotificationChannel(channel);
		}

		var notification = new Notification.Builder(this, ChannelId)
			.SetContentTitle("StereoKit")
			.SetContentText ("Running")
			.SetSmallIcon   (Android.Resource.Drawable.IcDialogInfo)
			.Build();

		StartForeground(NotificationId, notification);
	}

	static bool running = false;
	void Run()
	{
		if (running) return;
		running = true;

		// Before anything else, give StereoKit the Service context and
		// VM. These should be set before any other SK calls, otherwise
		// native library loading may fail.
		SK.AndroidActivity = this;
		// This is optional, but helps with compatibility on older
		// devices, Android API 30 and older.
		SK.AndroidJavaVM   = Java.Interop.JniEnvironment.Runtime.InvocationPointer;

		// Task.Run will eat exceptions, but Thread.Start doesn't seem
		// to.
		new Thread(InvokeStereoKit).Start();
	}

	static void InvokeStereoKit()
	{
		Type       entryClass = typeof(Program);
		MethodInfo entryPoint = entryClass?.GetMethod("Main", BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic);

		// There are a number of potential method signatures for Main, so
		// we need to check each one, and give it the correct values.
		//
		// Converting MethodInfo into an Action instead of calling Invoke
		// on it allows for exceptions to properly bubble up to the IDE.
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
}

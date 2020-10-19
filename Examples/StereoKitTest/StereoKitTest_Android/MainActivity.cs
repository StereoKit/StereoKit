using Android.App;
using Android.Content;
using Android.Graphics;
using Android.OS;
using Android.Runtime;
using Android.Support.V7.App;
using Android.Views;
using Java.Lang;
using StereoKit;
using System;
using System.Threading.Tasks;

namespace StereoKitTest_Android
{
	[Activity(Label = "@string/app_name", Theme = "@style/AppTheme", MainLauncher = true)]
	[IntentFilter(new[] { Intent.ActionMain }, Categories = new[] { "com.oculus.intent.category.VR", Intent.CategoryLauncher })]
    public class MainActivity : AppCompatActivity
    {
		SKAndroidSurface surface;
		bool running = false;
		protected override void OnCreate(Bundle savedInstanceState)
        {
			JavaSystem.LoadLibrary("StereoKitC");

			base.OnCreate(savedInstanceState);
            Xamarin.Essentials.Platform.Init(this, savedInstanceState);
            // Set our view from the "main" layout resource
            SetContentView(Resource.Layout.activity_main);
			surface = new SKAndroidSurface(this);
			surface.OnChanged   += (v) => {
				Android.Util.Log.Info("StereoKitTest", "Changed: " + v.Width + "x" + v.Height);
				if (!running)
				{
					running = true;
					Start(v.WindowHandle);
				}
			};
			surface.OnDestroyed += (v) => Android.Util.Log.Info("StereoKitTest", "Destroyed");
			SetContentView(surface);
		}
        public override void OnRequestPermissionsResult(int requestCode, string[] permissions, [GeneratedEnum] Android.Content.PM.Permission[] grantResults)
        {
            Xamarin.Essentials.Platform.OnRequestPermissionsResult(requestCode, permissions, grantResults);

            base.OnRequestPermissionsResult(requestCode, permissions, grantResults);
        }

		void Start(IntPtr window)
		{
			Task.Run(()=>{
				StereoKit.Log.Filter = LogLevel.Diagnostic;
				if (!StereoKitApp.InitializeAndroid("StereoKitTemplate", window, StereoKit.Runtime.MixedReality))
					return;

				// Create assets used by the app
				Pose cubePose = new Pose(0, 0, -0.5f, Quat.Identity);
				Model cube = Model.FromMesh(
					Mesh.GenerateRoundedCube(Vec3.One * 0.1f, 0.02f),
					Default.MaterialUI);

				StereoKit.Matrix floorTransform = StereoKit.Matrix.TS(new Vec3(0, -1.5f, 0), new Vec3(30, 0.1f, 30));
				//Material floorMaterial = new Material(StereoKit.Shader.FromFile("floor.hlsl"));
				//floorMaterial.Transparency = Transparency.Blend;


				// Core application loop
				while (StereoKitApp.Step(() =>
				{
					//if (StereoKitApp.System.displayType == Display.Opaque)
					//	Default.MeshCube.Draw(floorMaterial, floorTransform);
					UI.Handle("Cube", ref cubePose, cube.Bounds);
					cube.Draw(cubePose.ToMatrix());
				})) ;
				StereoKitApp.Shutdown();
			});
		}
    }

	// Ref here:
	// https://github.com/spurious/SDL-mirror/blob/6fe5bd1536beb197de493c9b55d16e516219c58f/android-project/app/src/main/java/org/libsdl/app/SDLActivity.java#L1663
	// https://github.com/MonoGame/MonoGame/blob/31dca640482bc0c27aec8e51d6369612ce8577a2/MonoGame.Framework/Platform/Android/MonoGameAndroidGameView.cs
	class SKAndroidSurface : SurfaceView, ISurfaceHolderCallback
	{
		public event Action<SKAndroidSurface> OnChanged;
		public event Action<SKAndroidSurface> OnCreated;
		public event Action<SKAndroidSurface> OnDestroyed;

		/// <summary>A native pointer to the window surface required by JNIs 
		/// ANativeWindow_fromSurface.</summary>
		public IntPtr WindowHandle => Holder.Surface.Handle;

		public SKAndroidSurface(Context context) : base(context) => Holder.AddCallback(this);
		
		public void SurfaceChanged  (ISurfaceHolder holder, [GeneratedEnum] Format format, int width, int height) => OnChanged?.Invoke(this);
		public void SurfaceCreated  (ISurfaceHolder holder) => OnCreated?.Invoke(this);
		public void SurfaceDestroyed(ISurfaceHolder holder) => OnDestroyed?.Invoke(this);
	}
}
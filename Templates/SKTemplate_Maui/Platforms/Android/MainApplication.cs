using Android.App;
using Android.Runtime;

namespace SKTemplate_Maui;

[Application]
public class MainApplication : Android.App.Application
{
	public MainApplication(IntPtr handle, JniHandleOwnership ownership)
		: base(handle, ownership)
	{
	}
}

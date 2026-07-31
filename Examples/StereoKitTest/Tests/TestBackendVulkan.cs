using StereoKit;
using System;

// Verifies the Backend.Vulkan extension/feature request surface: the request
// registered before init (see Program.Main) enabled, extension/request queries
// behave, and Vulkan function pointers resolve. Vulkan is StereoKit's only
// graphics backend, so this runs on every supported platform.
class TestBackendVulkan : ITest
{
	public void Initialize()
	{
		Tests.Test(() => Backend.Graphics == BackendGraphics.Vulkan);

		// The empty request registered in Program.Main enables vacuously - no
		// extensions or features to miss.
		Tests.Test(() => Backend.Vulkan.RequestEnabled("sk_test_request") == true);
		// An unregistered request name is simply not enabled.
		Tests.Test(() => Backend.Vulkan.RequestEnabled("not_a_registered_request") == false);

		// A made-up extension is never enabled.
		Tests.Test(() => Backend.Vulkan.ExtEnabled("VK_KHR_not_a_real_extension") == false);

		// Core Vulkan entry points always resolve, missing ones return zero.
		Tests.Test(() => Backend.Vulkan.GetFunctionPtr("vkCreateBuffer")   != IntPtr.Zero);
		Tests.Test(() => Backend.Vulkan.GetFunctionPtr("vkNotARealVkFunc") == IntPtr.Zero);
	}

	public void Shutdown() { }
	public void Step()     { }
}

#ifndef SK_NO_FLATSCREEN
#ifdef WINDOWS_UWP

#include <dxgi1_2.h>
#include <process.h>

#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../../asset_types/texture.h"
#include "../d3d.h"
#include "../render.h"
#include "../input.h"
#include "flatscreen_input.h"

namespace sk {

void            *uwp_window    = nullptr;
tex_t            uwp_target    = {};
IDXGISwapChain1 *uwp_swapchain = {};

}

// The WinRT/UWP mess comes from:
// https://github.com/walbourn/directx-vs-templates/tree/master/d3d11game_uwp_cppwinrt

#include <wrl/client.h>

#include "winrt/Windows.ApplicationModel.h" 
#include "winrt/Windows.ApplicationModel.Core.h"
#include "winrt/Windows.ApplicationModel.Activation.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.Graphics.Display.h"
#include "winrt/Windows.System.h"
#include "winrt/Windows.UI.Core.h"
#include "winrt/Windows.UI.Input.h"
#include "winrt/Windows.UI.ViewManagement.h"

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::UI::Core;
//using namespace winrt::Windows::UI::Input;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::System;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Display;
//using namespace DirectX;

using namespace sk;

inline float dips_to_pixels(float dips, float DPI) {
	return dips * DPI / 96.f + 0.5f;
}

inline float pixels_to_dips(float pixels, float DPI) {
	return (float(pixels) * 96.f / DPI);
}

class ViewProvider : public winrt::implements<ViewProvider, IFrameworkView> {
public:
	static ViewProvider *inst;
	bool initialized = false;
	bool valid       = false;
	const CoreWindow *core_window;
	vec2  mouse_point;
	float mouse_scroll = 0;
	bool  mouse_down[2];
	uint8_t key_state[255];
	float m_DPI;

	ViewProvider() :
		m_exit(false),
		m_visible(true),
		m_in_sizemove(false),
		m_DPI(96.f),
		m_logicalWidth(800.f),
		m_logicalHeight(600.f),
		m_nativeOrientation(DisplayOrientations::None),
		m_currentOrientation(DisplayOrientations::None)
	{
		inst = this;
	}
	~ViewProvider() {
	}

	// IFrameworkView methods
	void Initialize(const CoreApplicationView &applicationView) {
		applicationView.Activated  ({ this, &ViewProvider::OnActivated });
		CoreApplication::Suspending({ this, &ViewProvider::OnSuspending });
		CoreApplication::Resuming  ({ this, &ViewProvider::OnResuming   });
	}

	void Uninitialize() { 
	}

	void SetWindow(CoreWindow const & window)
	{
		core_window = &window;

		auto dispatcher                = CoreWindow::GetForCurrentThread().Dispatcher();
		auto navigation                = SystemNavigationManager::GetForCurrentView();
		auto currentDisplayInformation = DisplayInformation::GetForCurrentView();

		window.SizeChanged({ this, &ViewProvider::OnWindowSizeChanged });

#if defined(NTDDI_WIN10_RS2) && (NTDDI_VERSION >= NTDDI_WIN10_RS2)
		try {
			window.ResizeStarted  ([this](auto&&, auto&&) { m_in_sizemove = true; });
			window.ResizeCompleted([this](auto&&, auto&&) { m_in_sizemove = false; HandleWindowSizeChanged(); });
		} catch (...) {
			// Requires Windows 10 Creators Update (10.0.15063) or later
		}
#endif
		window.KeyDown                                ([this](auto &&, auto &&args) { key_state[(int)args.VirtualKey()] = true; });
		window.KeyUp                                  ([this](auto &&, auto &&args) { key_state[(int)args.VirtualKey()] = false; });
		window.PointerPressed                         ({ this, &ViewProvider::OnMouseButtonChanged });
		window.PointerReleased                        ({ this, &ViewProvider::OnMouseButtonChanged });
		window.PointerMoved                           ({ this, &ViewProvider::OnMouseChanged       });
		window.PointerWheelChanged                    ({ this, &ViewProvider::OnWheelChanged       });
		window.VisibilityChanged                      ({ this, &ViewProvider::OnVisibilityChanged  });
		dispatcher.AcceleratorKeyActivated            ({ this, &ViewProvider::OnAcceleratorKeyActivated });
		currentDisplayInformation.DpiChanged          ({ this, &ViewProvider::OnDpiChanged         });
		currentDisplayInformation.OrientationChanged  ({ this, &ViewProvider::OnOrientationChanged });
		
		window.Closed([this](auto &&, auto &&) { m_exit = true; sk_run = false; log_info("OnClosed!"); });
		// UWP on Xbox One triggers a back request whenever the B button is pressed
		// which can result in the app being suspended if unhandled
		navigation.BackRequested([](const winrt::Windows::Foundation::IInspectable&, const BackRequestedEventArgs& args)
			{ args.Handled(true); });

		m_DPI                = currentDisplayInformation.LogicalDpi();
		m_logicalWidth       = window.Bounds().Width;
		m_logicalHeight      = window.Bounds().Height;
		m_nativeOrientation  = currentDisplayInformation.NativeOrientation ();
		m_currentOrientation = currentDisplayInformation.CurrentOrientation();
		d3d_screen_width  = dips_to_pixels(m_logicalWidth,  m_DPI);
		d3d_screen_height = dips_to_pixels(m_logicalHeight, m_DPI);

		DXGI_MODE_ROTATION rotation = ComputeDisplayRotation();
		if (rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270) {
			std::swap(d3d_screen_width, d3d_screen_height);
		}

		auto windowPtr = static_cast<::IUnknown*>(winrt::get_abi(window));
		//uwp_window = (void*)windowPtr;

		// Create a swapchain for the window
		DXGI_SWAP_CHAIN_DESC1 sd = { };
		sd.BufferCount = 2;
		sd.Width       = d3d_screen_width;
		sd.Height      = d3d_screen_height;
		sd.Format      = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.SampleDesc.Count = 1;

		IDXGIDevice2  *dxgi_device;  d3d_device  ->QueryInterface(__uuidof(IDXGIDevice2),  (void **)&dxgi_device);
		IDXGIAdapter  *dxgi_adapter; dxgi_device ->GetParent     (__uuidof(IDXGIAdapter),  (void **)&dxgi_adapter);
		IDXGIFactory2 *dxgi_factory; dxgi_adapter->GetParent     (__uuidof(IDXGIFactory2), (void **)&dxgi_factory);

		dxgi_factory->CreateSwapChainForCoreWindow(d3d_device, windowPtr, &sd, nullptr, &uwp_swapchain);
		ID3D11Texture2D *back_buffer;
		uwp_swapchain->GetBuffer(0, __uuidof(**(&back_buffer)), reinterpret_cast<void **>(&back_buffer));

		uwp_target = tex_create(tex_type_rendertarget, tex_format_rgba32_linear);
		tex_set_id     (uwp_target, "stereokit/system/rendertarget");
		tex_setsurface (uwp_target, back_buffer, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		tex_add_zbuffer(uwp_target, tex_format_depth16);

		back_buffer ->Release();
		dxgi_factory->Release();
		dxgi_adapter->Release();
		dxgi_device ->Release();

		initialized = true;
		valid = true;
	}

	void Load(winrt::hstring const &) { }

	void Run() { 
		while (!m_exit) {
			if (m_visible) {
				CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			} else {
				CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
			}
			Sleep(10);
		}
	}

protected:
	// Event handlers
	void OnActivated(CoreApplicationView const & /*applicationView*/, IActivatedEventArgs const & args) {
		if (args.Kind() == ActivationKind::Launch) {
			auto launchArgs = (const LaunchActivatedEventArgs*)(&args);

			if (launchArgs->PrelaunchActivated())
			{
				// Opt-out of Prelaunch
				CoreApplication::Exit();
				return;
			}
		}

		m_DPI = DisplayInformation::GetForCurrentView().LogicalDpi();

		ApplicationView::PreferredLaunchWindowingMode(ApplicationViewWindowingMode::PreferredLaunchViewSize);
		// Change to ApplicationViewWindowingMode::FullScreen to default to full screen

		auto desiredSize = Size(pixels_to_dips(1280, m_DPI), pixels_to_dips(720, m_DPI));

		ApplicationView::PreferredLaunchViewSize(desiredSize);

		auto view    = ApplicationView::GetForCurrentView();
		auto minSize = Size(pixels_to_dips(320, m_DPI), pixels_to_dips(200, m_DPI));

		view.SetPreferredMinSize(minSize);

		CoreWindow::GetForCurrentThread().Activate();

		view.FullScreenSystemOverlayMode(FullScreenSystemOverlayMode::Minimal);
		view.TryResizeView(desiredSize);
	}

	void OnSuspending(IInspectable const & /*sender*/, SuspendingEventArgs const &) {
		/*auto deferral = args.SuspendingOperation().GetDeferral();

		auto f = std::async(std::launch::async, [this, deferral]()
			{
				m_game->OnSuspending();

				deferral.Complete();
			});*/
	}

	void OnResuming(IInspectable const & /*sender*/, IInspectable const & /*args*/)
	{
	}

	void OnWindowSizeChanged(CoreWindow const & sender, WindowSizeChangedEventArgs const & /*args*/)
	{
		m_logicalWidth  = sender.Bounds().Width;
		m_logicalHeight = sender.Bounds().Height;

		if (m_in_sizemove)
			return;

		HandleWindowSizeChanged();
	}

	void OnMouseButtonChanged(CoreWindow const & /*sender*/, PointerEventArgs const &args) {
		key_state[0x01] = args.CurrentPoint().Properties().IsLeftButtonPressed();
		key_state[0x02] = args.CurrentPoint().Properties().IsRightButtonPressed();
	}
	void OnMouseChanged(CoreWindow const & /*sender*/, PointerEventArgs const &args) {
		Point p = args.CurrentPoint().RawPosition();
		mouse_point = { 
			dips_to_pixels(p.X, m_DPI),
			dips_to_pixels(p.Y, m_DPI) };
	}
	void OnWheelChanged(CoreWindow const & /*sender*/, PointerEventArgs const &args) {
		mouse_scroll += args.CurrentPoint().Properties().MouseWheelDelta();
	}

	void OnVisibilityChanged(CoreWindow const & /*sender*/, VisibilityChangedEventArgs const & args) {
		m_visible = args.Visible();
		sk_focused = m_visible;
	}

	void OnAcceleratorKeyActivated(CoreDispatcher const &, AcceleratorKeyEventArgs const & args)
	{
		if (args.EventType() == CoreAcceleratorKeyEventType::SystemKeyDown
			&& args.VirtualKey() == VirtualKey::Enter
			&& args.KeyStatus().IsMenuKeyDown
			&& !args.KeyStatus().WasKeyDown) {
			// Implements the classic ALT+ENTER fullscreen toggle
			auto view = ApplicationView::GetForCurrentView();

			if (view.IsFullScreenMode())
				view.ExitFullScreenMode();
			else
				view.TryEnterFullScreenMode();

			args.Handled(true);
		}
	}

	void OnDpiChanged(DisplayInformation const & sender, IInspectable const & /*args*/) {
		m_DPI = sender.LogicalDpi();
		HandleWindowSizeChanged();
	}

	void OnOrientationChanged(DisplayInformation const & sender, IInspectable const & /*args*/) {
		auto resizeManager = CoreWindowResizeManager::GetForCurrentView();
		resizeManager.ShouldWaitForLayoutCompletion(true);

		m_currentOrientation = sender.CurrentOrientation();

		HandleWindowSizeChanged();

		resizeManager.NotifyLayoutCompleted();
	}

private:
	bool  m_exit;
	bool  m_visible;
	bool  m_in_sizemove;
	float m_logicalWidth;
	float m_logicalHeight;

	winrt::Windows::Graphics::Display::DisplayOrientations	m_nativeOrientation;
	winrt::Windows::Graphics::Display::DisplayOrientations	m_currentOrientation;

	DXGI_MODE_ROTATION ComputeDisplayRotation() const {
		DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

		switch (m_nativeOrientation) {
		case DisplayOrientations::Landscape:
			switch (m_currentOrientation) {
			case DisplayOrientations::Landscape:        rotation = DXGI_MODE_ROTATION_IDENTITY;  break;
			case DisplayOrientations::Portrait:         rotation = DXGI_MODE_ROTATION_ROTATE270; break;
			case DisplayOrientations::LandscapeFlipped: rotation = DXGI_MODE_ROTATION_ROTATE180; break;
			case DisplayOrientations::PortraitFlipped:  rotation = DXGI_MODE_ROTATION_ROTATE90;  break;
			} break;

		case DisplayOrientations::Portrait:
			switch (m_currentOrientation) {
			case DisplayOrientations::Landscape:        rotation = DXGI_MODE_ROTATION_ROTATE90;  break;
			case DisplayOrientations::Portrait:         rotation = DXGI_MODE_ROTATION_IDENTITY;  break;
			case DisplayOrientations::LandscapeFlipped: rotation = DXGI_MODE_ROTATION_ROTATE270; break;
			case DisplayOrientations::PortraitFlipped:  rotation = DXGI_MODE_ROTATION_ROTATE180; break;
			} break;
		}
		return rotation;
	}

	void HandleWindowSizeChanged() {
		int outputWidth  = dips_to_pixels(m_logicalWidth, m_DPI);
		int outputHeight = dips_to_pixels(m_logicalHeight, m_DPI);

		DXGI_MODE_ROTATION rotation = ComputeDisplayRotation();

		if (rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270)
			std::swap(outputWidth, outputHeight);

		if (outputWidth == d3d_screen_width && outputHeight == d3d_screen_height)
			return;
		d3d_screen_width  = outputWidth;
		d3d_screen_height = outputHeight;
		log_infof("Resized to: %d<~BLK>x<~clr>%d", outputWidth, outputHeight);

		if (uwp_swapchain != nullptr) {
			tex_releasesurface(uwp_target);
			uwp_swapchain->ResizeBuffers(0, (UINT)d3d_screen_width, (UINT)d3d_screen_height, DXGI_FORMAT_UNKNOWN, 0);
			ID3D11Texture2D *back_buffer;
			uwp_swapchain->GetBuffer(0, __uuidof(**(&back_buffer)), reinterpret_cast<void **>(&back_buffer));
			tex_setsurface(uwp_target, back_buffer, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
			back_buffer->Release();
		}

		render_update_projection();
	}
};
ViewProvider *ViewProvider::inst = nullptr;

class ViewProviderFactory : public winrt::implements<ViewProviderFactory, IFrameworkViewSource> {
public:
	IFrameworkView CreateView() {
		return winrt::make<ViewProvider>();
	}
};

namespace sk {

IFrameworkViewSource viewProviderFactory;
void window_thread(void *) {
	viewProviderFactory = winrt::make<ViewProviderFactory>();
	CoreApplication::Run(viewProviderFactory);
}

bool uwp_get_mouse(vec2 &out_pos) {
	out_pos = ViewProvider::inst->mouse_point;
	return true;
}
void uwp_set_mouse(vec2 window_pos) {
	ViewProvider::inst->mouse_point = window_pos;

	winrt::Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(
		winrt::Windows::UI::Core::CoreDispatcherPriority::High, [window_pos]() {
			Rect pos = CoreWindow::GetForCurrentThread().Bounds();
			CoreWindow::GetForCurrentThread().PointerPosition(Point(
				pixels_to_dips(window_pos.x, ViewProvider::inst->m_DPI) + pos.X,
				pixels_to_dips(window_pos.y, ViewProvider::inst->m_DPI) + pos.Y));
		});
}

float uwp_get_scroll() {
	return ViewProvider::inst->mouse_scroll;
}

bool uwp_mouse_button(int button) {
	return ViewProvider::inst->mouse_down[button];
}
bool uwp_key_down(int vk) {
	return ViewProvider::inst->key_state[vk];
}

bool uwp_init(const char *) {
	d3d_screen_width  = sk_settings.flatscreen_width;
	d3d_screen_height = sk_settings.flatscreen_height;
	if (!d3d_init(nullptr))
		return false;

	sk_info.display_type = display_opaque;

	_beginthread(window_thread, 0, nullptr);
	
	while (ViewProvider::inst == nullptr || !ViewProvider::inst->initialized) {
		Sleep(1);
	}

	flatscreen_input_init();
	return ViewProvider::inst->valid;
}

void uwp_step_begin() {
	d3d_update();
	flatscreen_input_update();
}
void uwp_step_end() { 
	// Set up where on the render target we want to draw, the view has a 
	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.f, 0.f, (float)d3d_screen_width, (float)d3d_screen_height);
	d3d_context->RSSetViewports(1, &viewport);

	// Wipe our swapchain color and depth target clean, and then set them up for rendering!
	tex_rtarget_clear(uwp_target, {0,0,0,255});
	tex_rtarget_set_active(uwp_target);

	input_update_predicted();

	render_draw();
	render_clear();
}
void uwp_vsync() {
	uwp_swapchain->Present(1, 0);
}

void uwp_shutdown() {
	flatscreen_input_shutdown();

	winrt::Windows::ApplicationModel::Core::CoreApplication::Exit();
	tex_release(uwp_target);
	uwp_swapchain->Release();

	d3d_shutdown();
}

}

#endif // WINDOWS_UWP
#endif // SK_NO_FLATSCREEN
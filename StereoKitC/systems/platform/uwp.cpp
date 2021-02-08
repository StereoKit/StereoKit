#include "platform_utils.h"
#if defined(SK_OS_WINDOWS_UWP)

#include <dxgi1_2.h>
#include <process.h>

#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../../asset_types/texture.h"
#include "../render.h"
#include "../input.h"
#include "flatscreen_input.h"

namespace sk {

HWND            uwp_window    = nullptr;
skg_swapchain_t uwp_swapchain = {};

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

		key_state[key_caps_lock] = window.GetKeyState(VirtualKey::CapitalLock) == CoreVirtualKeyStates::Locked ? true : false;

		window.SizeChanged({ this, &ViewProvider::OnWindowSizeChanged });

#if defined(NTDDI_WIN10_RS2) && (NTDDI_VERSION >= NTDDI_WIN10_RS2)
		try {
			window.ResizeStarted  ([this](auto&&, auto&&) { m_in_sizemove = true; });
			window.ResizeCompleted([this](auto&&, auto&&) { m_in_sizemove = false; HandleWindowSizeChanged(); });
		} catch (...) {
			// Requires Windows 10 Creators Update (10.0.15063) or later
		}
#endif
		window.KeyDown([this](auto &&, auto &&args) {
			key_ key = (key_)args.VirtualKey();
			key_state[key] = key == key_caps_lock ? !key_state[key] : true;
		});
		window.KeyUp([this](auto &&, auto &&args) { 
			key_ key = (key_)args.VirtualKey();
			if (key != key_caps_lock)
				key_state[key] = false; 
		});
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
		sk_info.display_width  = (int32_t)dips_to_pixels(m_logicalWidth,  m_DPI);
		sk_info.display_height = (int32_t)dips_to_pixels(m_logicalHeight, m_DPI);

		DXGI_MODE_ROTATION rotation = ComputeDisplayRotation();
		if (rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270) {
			std::swap(sk_info.display_width, sk_info.display_height);
		}

		// Get the HWND of the UWP window
		// https://kennykerr.ca/2012/11/09/windows-8-whered-you-put-my-hwnd/
		struct 
		__declspec(uuid("45D64A29-A63E-4CB6-B498-5781D298CB4F")) 
		__declspec(novtable)
		ICoreWindowInterop : ::IUnknown
		{
			virtual HRESULT __stdcall get_WindowHandle(HWND * hwnd) = 0;
			virtual HRESULT __stdcall put_MessageHandled(unsigned char) = 0;
		};
		winrt::com_ptr<ICoreWindowInterop> interop {};
		winrt::check_hresult(winrt::get_unknown(window)->QueryInterface(interop.put()));
		winrt::check_hresult(interop->get_WindowHandle(&uwp_window));

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
		key_state[key_mouse_left ] = args.CurrentPoint().Properties().IsLeftButtonPressed();
		key_state[key_mouse_right] = args.CurrentPoint().Properties().IsRightButtonPressed();
	}

	void OnMouseChanged(CoreWindow const & /*sender*/, PointerEventArgs const &args) {
		key_state[key_mouse_left ] = args.CurrentPoint().Properties().IsLeftButtonPressed();
		key_state[key_mouse_right] = args.CurrentPoint().Properties().IsRightButtonPressed();

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
		int outputWidth  = (int)dips_to_pixels(m_logicalWidth, m_DPI);
		int outputHeight = (int)dips_to_pixels(m_logicalHeight, m_DPI);

		DXGI_MODE_ROTATION rotation = ComputeDisplayRotation();

		if (rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270)
			std::swap(outputWidth, outputHeight);

		if (outputWidth == sk_info.display_width && outputHeight == sk_info.display_height)
			return;
		sk_info.display_width  = outputWidth;
		sk_info.display_height = outputHeight;
		log_infof("Resized to: %d<~BLK>x<~clr>%d", outputWidth, outputHeight);

		skg_swapchain_resize(&uwp_swapchain, sk_info.display_width, sk_info.display_height);
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

bool uwp_init() {
	return true;
}

void uwp_shutdown() {
}

bool uwp_start() {
	sk_info.display_width  = sk_settings.flatscreen_width;
	sk_info.display_height = sk_settings.flatscreen_height;
	sk_info.display_type   = display_opaque;

	_beginthread(window_thread, 0, nullptr);
	
	while (ViewProvider::inst == nullptr || !ViewProvider::inst->initialized) {
		Sleep(1);
	}

	skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
	skg_tex_fmt_ depth_fmt = render_preferred_depth_fmt();
	uwp_swapchain = skg_swapchain_create(uwp_window, color_fmt, depth_fmt, sk_settings.flatscreen_width, sk_settings.flatscreen_height);
	sk_info.display_width  = uwp_swapchain.width;
	sk_info.display_height = uwp_swapchain.height;
	log_diagf("Created swapchain: %dx%d color:%s depth:%s", uwp_swapchain.width, uwp_swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));

	flatscreen_input_init();
	return ViewProvider::inst->valid;
}

void uwp_step_begin() {
	flatscreen_input_update();
}
void uwp_step_end() { 
	skg_draw_begin();

	// Wipe our swapchain color and depth target clean, and then set them up for rendering!
	color128 color = render_get_clear_color();
	skg_swapchain_bind(&uwp_swapchain, true, &color.r);

	input_update_predicted();

	matrix view = render_get_cam_root  ();
	matrix proj = render_get_projection();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1);
	render_clear();
}
void uwp_vsync() {
	skg_swapchain_present(&uwp_swapchain);
}

void uwp_stop() {
	flatscreen_input_shutdown();

	winrt::Windows::ApplicationModel::Core::CoreApplication::Exit();
	skg_swapchain_destroy(&uwp_swapchain);
}

}

#endif // defined(SK_OS_WINDOWS_UWP)
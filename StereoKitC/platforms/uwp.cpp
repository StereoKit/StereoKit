#include "platform_utils.h"
#if defined(SK_OS_WINDOWS_UWP)

#include <dxgi1_2.h>
#include <process.h>

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../asset_types/texture.h"
#include "../libraries/sokol_time.h"
#include "../systems/system.h"
#include "../systems/render.h"
#include "../systems/input.h"
#include "../systems/input_keyboard.h"
#include "flatscreen_input.h"

namespace sk {

HWND            uwp_window           = nullptr;
skg_swapchain_t uwp_swapchain        = {};
tex_t           uwp_target           = {};
system_t       *uwp_render_sys       = nullptr;
bool            uwp_keyboard_showing = false;
bool            uwp_keyboard_show_evts = false;

bool uwp_mouse_set;
vec2 uwp_mouse_set_delta;
vec2 uwp_mouse_frame_get;

bool    uwp_resize_pending = false;
int32_t uwp_resize_width   = 0;
int32_t uwp_resize_height  = 0;

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
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::System;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Display;

using namespace sk;

void uwp_on_corewindow_character(CoreWindow const &, CharacterReceivedEventArgs const &args) {
	input_text_inject_char((uint32_t)args.KeyCode());
}

void uwp_on_corewindow_keypress(CoreDispatcher const &, AcceleratorKeyEventArgs const & args) {
	// https://github.com/microsoft/DirectXTK/blob/master/Src/Keyboard.cpp#L466
	CorePhysicalKeyStatus status = args.KeyStatus();
	int32_t               vk     = (int32_t)args.VirtualKey();

	bool down;
	switch (args.EventType()) {
	case CoreAcceleratorKeyEventType::KeyDown:
	case CoreAcceleratorKeyEventType::SystemKeyDown: down = true; break;
	case CoreAcceleratorKeyEventType::KeyUp:
	case CoreAcceleratorKeyEventType::SystemKeyUp:   down = false; break;
	default:                                         return;
	}

	/*switch (vk) {
	case VK_SHIFT:   vk = status.ScanCode == 0x36 ? VK_RSHIFT   : VK_LSHIFT;   break;
	case VK_CONTROL: vk = status.IsExtendedKey    ? VK_RCONTROL : VK_LCONTROL; break;
	case VK_MENU:    vk = status.IsExtendedKey    ? VK_RMENU    : VK_LMENU;    break;
	}*/
	if (down) input_keyboard_inject_press  ((key_)vk);
	else      input_keyboard_inject_release((key_)vk);
}

inline float dips_to_pixels(float dips, float DPI) {
	return dips * DPI / 96.f + 0.5f;
}

inline float pixels_to_dips(float pixels, float DPI) {
	return (float(pixels) * 96.f / DPI);
}

class ViewProvider : public winrt::implements<ViewProvider, IFrameworkView> {
public:
	static ViewProvider *inst;
	bool  initialized  = false;
	bool  valid        = false;
	vec2  mouse_point  = {};
	float mouse_scroll = 0;
	
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

	void SetWindow(CoreWindow const & window) {
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
		window.PointerPressed                         ({ this, &ViewProvider::OnMouseButtonDown    });
		window.PointerReleased                        ({ this, &ViewProvider::OnMouseButtonUp      });
		window.PointerWheelChanged                    ({ this, &ViewProvider::OnWheelChanged       });
		window.VisibilityChanged                      ({ this, &ViewProvider::OnVisibilityChanged  });
		window.Activated                              ({ this, &ViewProvider::OnActivation         });
		window.CharacterReceived                      (uwp_on_corewindow_character);
		dispatcher.AcceleratorKeyActivated            (uwp_on_corewindow_keypress);
		currentDisplayInformation.DpiChanged          ({ this, &ViewProvider::OnDpiChanged         });
		currentDisplayInformation.OrientationChanged  ({ this, &ViewProvider::OnOrientationChanged });


		window   .Closed ([this](auto &&, auto &&) { m_exit = true; sk_running = false; log_info("OnClosed!"); });

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
			int32_t swap_tmp = sk_info.display_width;
			sk_info.display_width  = sk_info.display_height;
			sk_info.display_height = swap_tmp;
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

			if (uwp_mouse_set) {
				Point pos = CoreWindow::GetForCurrentThread().PointerPosition();
				Rect  win = CoreWindow::GetForCurrentThread().Bounds();

				vec2 new_point = uwp_mouse_set_delta + vec2{ 
					dips_to_pixels(pos.X, m_DPI) - dips_to_pixels(win.X, m_DPI),
					dips_to_pixels(pos.Y, m_DPI) - dips_to_pixels(win.Y, m_DPI)};

				CoreWindow::GetForCurrentThread().PointerPosition(Point(
					pixels_to_dips(new_point.x, m_DPI) + win.X,
					pixels_to_dips(new_point.y, m_DPI) + win.Y));

				ViewProvider::inst->mouse_point = new_point;
				uwp_mouse_set = false;
			} else {
				Point pos = CoreWindow::GetForCurrentThread().PointerPosition();
				Rect  win = CoreWindow::GetForCurrentThread().Bounds();
				mouse_point = { 
					dips_to_pixels(pos.X, m_DPI) - dips_to_pixels(win.X, m_DPI),
					dips_to_pixels(pos.Y, m_DPI) - dips_to_pixels(win.Y, m_DPI)};
			}

			Sleep(1);
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

	void OnMouseButtonDown(CoreWindow const & /*sender*/, PointerEventArgs const &args) {
		if (sk_focus != app_focus_active) return;
		if (args.CurrentPoint().Properties().IsLeftButtonPressed  () && input_key(key_mouse_left)    == button_state_inactive) input_keyboard_inject_press(key_mouse_left);
		if (args.CurrentPoint().Properties().IsRightButtonPressed () && input_key(key_mouse_right)   == button_state_inactive) input_keyboard_inject_press(key_mouse_right);
		if (args.CurrentPoint().Properties().IsMiddleButtonPressed() && input_key(key_mouse_center)  == button_state_inactive) input_keyboard_inject_press(key_mouse_center);
		if (args.CurrentPoint().Properties().IsXButton1Pressed    () && input_key(key_mouse_back)    == button_state_inactive) input_keyboard_inject_press(key_mouse_back);
		if (args.CurrentPoint().Properties().IsXButton2Pressed    () && input_key(key_mouse_forward) == button_state_inactive) input_keyboard_inject_press(key_mouse_forward);
	}
	void OnMouseButtonUp(CoreWindow const & /*sender*/, PointerEventArgs const &args) {
		if (sk_focus != app_focus_active) return;
		if (!args.CurrentPoint().Properties().IsLeftButtonPressed  () && input_key(key_mouse_left)    == button_state_active) input_keyboard_inject_release(key_mouse_left);
		if (!args.CurrentPoint().Properties().IsRightButtonPressed () && input_key(key_mouse_right)   == button_state_active) input_keyboard_inject_release(key_mouse_right);
		if (!args.CurrentPoint().Properties().IsMiddleButtonPressed() && input_key(key_mouse_center)  == button_state_active) input_keyboard_inject_release(key_mouse_center);
		if (!args.CurrentPoint().Properties().IsXButton1Pressed    () && input_key(key_mouse_back)    == button_state_active) input_keyboard_inject_release(key_mouse_back);
		if (!args.CurrentPoint().Properties().IsXButton2Pressed    () && input_key(key_mouse_forward) == button_state_active) input_keyboard_inject_release(key_mouse_forward);
	}

	void OnWheelChanged(CoreWindow const & /*sender*/, PointerEventArgs const &args) {
		if (sk_focus == app_focus_active) mouse_scroll += args.CurrentPoint().Properties().MouseWheelDelta();
	}

	void OnVisibilityChanged(CoreWindow const & /*sender*/, VisibilityChangedEventArgs const & args) {
		m_visible = args.Visible();
		sk_focus = m_visible? app_focus_active : app_focus_background;
	}

	void OnActivation(CoreWindow const & /*sender*/, WindowActivatedEventArgs const & args) {
		sk_focus = args.WindowActivationState() != CoreWindowActivationState::Deactivated 
			? app_focus_active
			: app_focus_background;
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

		if (rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270) {
			int swap_tmp = outputWidth;
			outputWidth  = outputHeight;
			outputHeight = swap_tmp;
		}

		if (outputWidth == sk_info.display_width && outputHeight == sk_info.display_height)
			return;

		uwp_resize_pending = true;
		uwp_resize_width   = outputWidth;
		uwp_resize_height  = outputHeight;
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
	out_pos = uwp_mouse_frame_get;
	return true;
}

///////////////////////////////////////////

void uwp_set_mouse(vec2 window_pos) {
	uwp_mouse_set_delta = window_pos - uwp_mouse_frame_get;
	uwp_mouse_frame_get = window_pos;
	uwp_mouse_set       = true;
}

///////////////////////////////////////////

float uwp_get_scroll() {
	return ViewProvider::inst->mouse_scroll;
}

///////////////////////////////////////////

void uwp_show_keyboard(bool show) {
	// For future improvements, see here:
	// https://github.com/microsoft/Windows-universal-samples/blob/main/Samples/CustomEditControl/cs/CustomEditControl.xaml.cs
	CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(CoreDispatcherPriority::Normal, [show]() {
		auto inputPane = InputPane::GetForCurrentView();

		// Registering these callbacks up in the class doesn't work, and I
		// don't know why. Likely not worth the pain of figuring it out either.
		if (!uwp_keyboard_show_evts) {
			inputPane.Showing([](auto &&, auto &&) { uwp_keyboard_showing = true;  });
			inputPane.Hiding ([](auto &&, auto &&) { uwp_keyboard_showing = false; });
			uwp_keyboard_show_evts = true;
		}

		if (show) {
			uwp_keyboard_showing = inputPane.TryShow();
		} else {
			if (inputPane.TryHide()) {
				uwp_keyboard_showing = false;
			}
		}
	});
}

///////////////////////////////////////////

bool uwp_keyboard_visible() {
	return uwp_keyboard_showing;
}

///////////////////////////////////////////

bool uwp_init() {
	uwp_render_sys = systems_find("FrameRender");
	return true;
}

///////////////////////////////////////////

bool uwp_start_pre_xr() {
	return true;
}

///////////////////////////////////////////

bool uwp_start_post_xr() {
	// This only works with WMR, and will crash elsewhere
	try {
		CoreApplication::MainView().CoreWindow().Dispatcher().AcceleratorKeyActivated(uwp_on_corewindow_keypress);
		CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(CoreDispatcherPriority::Normal, []() {
			CoreApplication::MainView().CoreWindow().CharacterReceived(uwp_on_corewindow_character);
		});
	} catch (...) {
		log_warn("Keyboard input not available through this runtime.");
	}
	return true;
}

///////////////////////////////////////////

void uwp_shutdown() {
}

///////////////////////////////////////////

bool uwp_start_flat() {
	sk_info.display_width  = sk_settings.flatscreen_width;
	sk_info.display_height = sk_settings.flatscreen_height;
	sk_info.display_type   = display_opaque;

	_beginthread(window_thread, 0, nullptr);
	
	while (ViewProvider::inst == nullptr || !ViewProvider::inst->initialized) {
		Sleep(1);
	}

	skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
	skg_tex_fmt_ depth_fmt = (skg_tex_fmt_)render_preferred_depth_fmt();
	uwp_swapchain = skg_swapchain_create(uwp_window, color_fmt, skg_tex_fmt_none, sk_settings.flatscreen_width, sk_settings.flatscreen_height);
	sk_info.display_width  = uwp_swapchain.width;
	sk_info.display_height = uwp_swapchain.height;
	uwp_target = tex_create(tex_type_rendertarget, tex_format_rgba32);
	tex_set_color_arr(uwp_target, sk_info.display_width, sk_info.display_height, nullptr, 1, nullptr, 8);
	tex_add_zbuffer  (uwp_target, (tex_format_)depth_fmt);

	log_diagf("Created swapchain: %dx%d color:%s depth:%s", uwp_swapchain.width, uwp_swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));

	flatscreen_input_init();
	return ViewProvider::inst->valid;
}

///////////////////////////////////////////

void uwp_step_begin_xr() {
}

///////////////////////////////////////////

void uwp_step_begin_flat() {
	if (uwp_resize_pending) {
		uwp_resize_pending = false;

		sk_info.display_width  = uwp_resize_width;
		sk_info.display_height = uwp_resize_height;
		log_infof("Resized to: %d<~BLK>x<~clr>%d", uwp_resize_width, uwp_resize_height);

		skg_swapchain_resize(&uwp_swapchain, sk_info.display_width, sk_info.display_height);
		tex_set_color_arr   (uwp_target,     sk_info.display_width, sk_info.display_height, nullptr, 1, nullptr, 8);
		render_update_projection();
	}

	uwp_mouse_frame_get = ViewProvider::inst->mouse_point;
	flatscreen_input_update();
}

///////////////////////////////////////////

void uwp_step_end_flat() { 
	skg_draw_begin();

	// Wipe our swapchain color and depth target clean, and then set them up for rendering!
	color128 color = render_get_clear_color_ln();
	skg_tex_target_bind(&uwp_target->tex);
	skg_target_clear(true, &color.r);

	input_update_poses(true);

	matrix view = render_get_cam_final        ();
	matrix proj = render_get_projection_matrix();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1, render_get_filter());
	render_clear();

	// This copies the color data over to the swapchain, and resolves any
	// multisampling on the primary target texture.
	skg_tex_copy_to_swapchain(&uwp_target->tex, &uwp_swapchain);

	uwp_render_sys->profile_frame_duration = stm_since(uwp_render_sys->profile_frame_start);
	skg_swapchain_present(&uwp_swapchain);
}

///////////////////////////////////////////

void uwp_stop_flat() {
	flatscreen_input_shutdown();

	winrt::Windows::ApplicationModel::Core::CoreApplication::Exit();
	tex_release          (uwp_target);
	skg_swapchain_destroy(&uwp_swapchain);
}

}

#endif // defined(SK_OS_WINDOWS_UWP)
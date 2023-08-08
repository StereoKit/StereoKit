#include "platform_utils.h"
#include "platform.h"
#if defined(SK_OS_WINDOWS_UWP)

#include <dxgi1_2.h>
#include <process.h>

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../device.h"
#include "../sk_math.h"
#include "../asset_types/texture.h"
#include "../libraries/sokol_time.h"
#include "../libraries/stref.h"
#include "../libraries/tinycthread.h"
#include "../systems/system.h"
#include "../systems/render.h"
#include "../systems/input.h"
#include "../systems/input_keyboard.h"

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

namespace sk {

///////////////////////////////////////////

struct window_event_t {
	platform_evt_           type;
	platform_evt_data_t     data;
};

struct window_t {
	wchar_t*                title;
	HWND                    handle;
	bool                    run_thread;
	bool                    initialized;
	bool                    valid;
	array_t<window_event_t> events;
	mtx_t                   event_mtx;

	skg_swapchain_t         swapchain;
	bool                    has_swapchain;

	vec2                    mouse_point;
	bool                    resize_pending;
	int32_t                 resize_x, resize_y;
	RECT                    save_rect;
};

///////////////////////////////////////////

array_t<window_t> uwp_windows            = {};
platform_win_t    uwp_platform_win       = -1;

tex_t             uwp_target             = {};
system_t         *uwp_render_sys         = nullptr;
bool              uwp_keyboard_showing   = false;
bool              uwp_keyboard_show_evts = false;

bool              uwp_mouse_set;
vec2              uwp_mouse_set_delta;
vec2              uwp_mouse_frame_get;
float             uwp_mouse_scroll = 0;

///////////////////////////////////////////

void uwp_target_resize          (int32_t width, int32_t height);
void uwp_physical_key_interact  ();
void uwp_on_corewindow_character(CoreWindow     const&, CharacterReceivedEventArgs const& args);
void uwp_on_corewindow_keypress (CoreDispatcher const&, AcceleratorKeyEventArgs    const& args);

bool platform_win_resize        (platform_win_t window_id, int32_t width, int32_t height);
void platform_win_add_event     (platform_win_t win_id, window_event_t evt);

///////////////////////////////////////////

bool uwp_init() {
	uwp_render_sys = systems_find("FrameRender");
	return true;
}

///////////////////////////////////////////

void uwp_shutdown() {
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

void uwp_step_begin_xr() {
}


///////////////////////////////////////////

bool uwp_start_flat() {
	const sk_settings_t* settings = sk_get_settings_ref();
	device_data.display_blend = display_blend_opaque;

	uwp_platform_win = platform_win_make(settings->app_name, { settings->flatscreen_pos_x, settings->flatscreen_pos_y, settings->flatscreen_width, settings->flatscreen_height }, platform_surface_swapchain);
	if (uwp_platform_win == -1)
		return false;

	skg_swapchain_t *swapchain = platform_win_get_swapchain(uwp_platform_win);
	device_data.display_width  = swapchain->width;
	device_data.display_height = swapchain->height;

	tex_format_ color_fmt = tex_format_rgba32;
	tex_format_ depth_fmt = render_preferred_depth_fmt();

	uwp_target = tex_create(tex_type_rendertarget, color_fmt);
	tex_set_id       (uwp_target, "sk/platform/swapchain");
	tex_set_color_arr(uwp_target, swapchain->width, swapchain->height, nullptr, 1, nullptr, 8);

	tex_t zbuffer = tex_add_zbuffer(uwp_target, depth_fmt);
	tex_set_id (zbuffer, "sk/platform/swapchain_zbuffer");
	tex_release(zbuffer);

	return true;
}

///////////////////////////////////////////

void uwp_step_begin_flat() {
	for (int32_t i = 0; i < uwp_windows.count; i++) {
		if (!uwp_windows[i].resize_pending) continue;
		uwp_windows[i].resize_pending = false;
		if (platform_win_resize((platform_win_t)i, uwp_windows[i].resize_x, uwp_windows[i].resize_y)) {
			window_event_t e = { platform_evt_resize };
			e.data.resize = { uwp_windows[i].resize_x, uwp_windows[i].resize_y };
			platform_win_add_event((platform_win_t)i, e);
		}
	}

	platform_evt_       evt  = {};
	platform_evt_data_t data = {};
	while (platform_win_next_event(uwp_platform_win, &evt, &data)) {
		switch (evt) {
		case platform_evt_app_focus:    sk_set_app_focus (data.app_focus); break;
		case platform_evt_key_press:    input_key_inject_press  (data.press_release); uwp_physical_key_interact(); break;
		case platform_evt_key_release:  input_key_inject_release(data.press_release); uwp_physical_key_interact(); break;
		case platform_evt_character:    input_text_inject_char  (data.character);                                  break;
		case platform_evt_mouse_press:  if (sk_app_focus() == app_focus_active) input_key_inject_press  (data.press_release); break;
		case platform_evt_mouse_release:if (sk_app_focus() == app_focus_active) input_key_inject_release(data.press_release); break;
		case platform_evt_scroll:       if (sk_app_focus() == app_focus_active) uwp_mouse_scroll += data.scroll;              break;
		case platform_evt_close:        sk_quit(); break;
		case platform_evt_resize:       uwp_target_resize(data.resize.width, data.resize.height); break;
		case platform_evt_none: break;
		default: break;
		}
	}

	uwp_mouse_frame_get = uwp_windows[uwp_platform_win].mouse_point;
}

///////////////////////////////////////////

void uwp_step_end_flat() {
	skg_event_begin("Setup");

	skg_draw_begin();

	// Wipe our swapchain color and depth target clean, and then set them up for rendering!
	color128 color = render_get_clear_color_ln();
	skg_tex_target_bind(&uwp_target->tex);
	skg_target_clear(true, &color.r);

	input_update_poses(true);

	skg_event_end();
	skg_event_begin("Draw");

	matrix view = render_get_cam_final        ();
	matrix proj = render_get_projection_matrix();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1, render_get_filter());
	render_clear();

	skg_event_end();
	skg_event_begin("Present");

	// This copies the color data over to the swapchain, and resolves any
	// multisampling on the primary target texture.
	skg_swapchain_t* swapchain = platform_win_get_swapchain(uwp_platform_win);
	skg_tex_copy_to_swapchain(&uwp_target->tex, swapchain);

	uwp_render_sys->profile_frame_duration = stm_since(uwp_render_sys->profile_frame_start);
	skg_swapchain_present(swapchain);

	skg_event_end();
}

///////////////////////////////////////////

void uwp_stop_flat() {
	winrt::Windows::ApplicationModel::Core::CoreApplication::Exit();
	tex_release(uwp_target);
}

///////////////////////////////////////////

void uwp_physical_key_interact() {
	// On desktop, we want to hide soft keyboards on physical presses
	input_set_last_physical_keypress_time(time_totalf_unscaled());
	platform_keyboard_show(false, text_context_text);
}

///////////////////////////////////////////

void uwp_on_corewindow_character(CoreWindow const &, CharacterReceivedEventArgs const &args) {
	input_text_inject_char((uint32_t)args.KeyCode());
}

///////////////////////////////////////////

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

	if (down) input_key_inject_press  ((key_)vk);
	else      input_key_inject_release((key_)vk);
}

///////////////////////////////////////////

inline float dips_to_pixels(float dips, float DPI) {
	return dips * DPI / 96.f + 0.5f;
}

///////////////////////////////////////////

inline float pixels_to_dips(float pixels, float DPI) {
	return (float(pixels) * 96.f / DPI);
}

///////////////////////////////////////////

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
	return uwp_mouse_scroll;
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

void uwp_target_resize(int32_t width, int32_t height) {
	if (uwp_target == nullptr || (width == tex_get_width(uwp_target) && height == tex_get_height(uwp_target)))
		return;

	device_data.display_width  = width;
	device_data.display_height = height;
	log_diagf("Resizing to: %d<~BLK>x<~clr>%d", width, height);

	tex_set_color_arr(uwp_target, width, height, nullptr, 1, nullptr, 8);
	render_update_projection();
}

///////////////////////////////////////////
// Window code                           //
///////////////////////////////////////////

class UWPWindow : public winrt::implements<UWPWindow, IFrameworkView> {
public:
	UWPWindow(platform_win_t win_id) :
		m_visible           (true),
		m_in_sizemove       (false),
		m_DPI               (96.f),
		m_logicalWidth      (800.f),
		m_logicalHeight     (600.f),
		m_nativeOrientation (DisplayOrientations::None),
		m_currentOrientation(DisplayOrientations::None),
		win_id              (win_id) { }
	~UWPWindow() { }

	// IFrameworkView methods
	void Initialize(const CoreApplicationView &applicationView) {
		applicationView.Activated  ({ this, &UWPWindow::OnActivated });
		CoreApplication::Suspending({ this, &UWPWindow::OnSuspending });
		CoreApplication::Resuming  ({ this, &UWPWindow::OnResuming   });
	}

	///////////////////////////////////////////

	void Uninitialize() { }

	///////////////////////////////////////////

	void Load(winrt::hstring const&) { }

	///////////////////////////////////////////

	void SetWindow(CoreWindow const & window) {
		auto dispatcher                = CoreWindow             ::GetForCurrentThread().Dispatcher();
		auto navigation                = SystemNavigationManager::GetForCurrentView();
		auto currentDisplayInformation = DisplayInformation     ::GetForCurrentView();

#if defined(NTDDI_WIN10_RS2) && (NTDDI_VERSION >= NTDDI_WIN10_RS2)
		try {
			window.ResizeStarted  ([this](auto&&, auto&&) { m_in_sizemove = true; });
			window.ResizeCompleted([this](auto&&, auto&&) { m_in_sizemove = false; HandleWindowSizeChanged(); });
		} catch (...) { } // Requires Windows 10 Creators Update (10.0.15063) or later
#endif
		window.SizeChanged                            ({ this, &UWPWindow::OnWindowSizeChanged  });
		window.PointerPressed                         ({ this, &UWPWindow::OnMouseButtonDown    });
		window.PointerReleased                        ({ this, &UWPWindow::OnMouseButtonUp      });
		window.PointerWheelChanged                    ({ this, &UWPWindow::OnWheelChanged       });
		window.VisibilityChanged                      ({ this, &UWPWindow::OnVisibilityChanged  });
		window.Activated                              ({ this, &UWPWindow::OnActivation         });
		window.CharacterReceived                      (uwp_on_corewindow_character);
		dispatcher.AcceleratorKeyActivated            (uwp_on_corewindow_keypress);
		currentDisplayInformation.DpiChanged          ({ this, &UWPWindow::OnDpiChanged         });
		currentDisplayInformation.OrientationChanged  ({ this, &UWPWindow::OnOrientationChanged });

		window.Closed([this](auto&&, auto&&) { window_event_t e = { platform_evt_close }; platform_win_add_event(win_id, e); uwp_windows[win_id].run_thread = false; });

		// UWP on Xbox One triggers a back request whenever the B button is pressed
		// which can result in the app being suspended if unhandled
		navigation.BackRequested([](const winrt::Windows::Foundation::IInspectable&, const BackRequestedEventArgs& args)
			{ args.Handled(true); });

		m_DPI                = currentDisplayInformation.LogicalDpi();
		m_logicalWidth       = window.Bounds().Width;
		m_logicalHeight      = window.Bounds().Height;
		m_nativeOrientation  = currentDisplayInformation.NativeOrientation ();
		m_currentOrientation = currentDisplayInformation.CurrentOrientation();
		/*device_data.display_width = (int32_t)dips_to_pixels(m_logicalWidth, m_DPI);
		device_data.display_height = (int32_t)dips_to_pixels(m_logicalHeight, m_DPI);

		DXGI_MODE_ROTATION rotation = ComputeDisplayRotation();
		if (rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270) {
			int32_t swap_tmp = device_data.display_width;
			device_data.display_width  = device_data.display_height;
			device_data.display_height = swap_tmp;
		}*/
		HandleWindowSizeChanged();

		// Get the HWND of the UWP window
		// https://kennykerr.ca/2012/11/09/windows-8-whered-you-put-my-hwnd/
		struct 
		__declspec(uuid("45D64A29-A63E-4CB6-B498-5781D298CB4F")) 
		__declspec(novtable)
		ICoreWindowInterop : ::IUnknown
		{
			virtual HRESULT __stdcall get_WindowHandle  (HWND* hwnd) = 0;
			virtual HRESULT __stdcall put_MessageHandled(unsigned char) = 0;
		};
		winrt::com_ptr<ICoreWindowInterop> interop {};
		winrt::check_hresult(winrt::get_unknown(window)->QueryInterface(interop.put()));
		winrt::check_hresult(interop->get_WindowHandle(&uwp_windows[win_id].handle));

		uwp_windows[win_id].initialized = true;
		uwp_windows[win_id].valid       = true;
	}

	///////////////////////////////////////////

	void Run() {
		uwp_windows[win_id].run_thread = true;
		while (uwp_windows[win_id].run_thread == true) {
			auto core_window = CoreWindow::GetForCurrentThread();
			core_window.Dispatcher().ProcessEvents(m_visible
				? CoreProcessEventsOption::ProcessAllIfPresent
				: CoreProcessEventsOption::ProcessOneAndAllPending);

			if (uwp_mouse_set) {
				Point pos = core_window.PointerPosition();
				Rect  win = core_window.Bounds();

				vec2 new_point = uwp_mouse_set_delta + vec2{ 
					dips_to_pixels(pos.X, m_DPI) - dips_to_pixels(win.X, m_DPI),
					dips_to_pixels(pos.Y, m_DPI) - dips_to_pixels(win.Y, m_DPI)};

				core_window.PointerPosition(Point(
					pixels_to_dips(new_point.x, m_DPI) + win.X,
					pixels_to_dips(new_point.y, m_DPI) + win.Y));

				uwp_windows[win_id].mouse_point = new_point;
				uwp_mouse_set = false;
			} else {
				Point pos = core_window.PointerPosition();
				Rect  win = core_window.Bounds();
				uwp_windows[win_id].mouse_point = {
					dips_to_pixels(pos.X, m_DPI) - dips_to_pixels(win.X, m_DPI),
					dips_to_pixels(pos.Y, m_DPI) - dips_to_pixels(win.Y, m_DPI)};
			}

			Sleep(1);
		}
	}

protected:

	///////////////////////////////////////////
	
	// Event handlers
	void OnActivated(CoreApplicationView const & /*applicationView*/, IActivatedEventArgs const & args) {
		if (args.Kind() == ActivationKind::Launch) {
			auto launchArgs = (const LaunchActivatedEventArgs*)(&args);
			if (launchArgs->PrelaunchActivated()) {
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

	///////////////////////////////////////////

	void OnSuspending(IInspectable const & /*sender*/, SuspendingEventArgs const &) { }

	///////////////////////////////////////////

	void OnResuming(IInspectable const & /*sender*/, IInspectable const & /*args*/) { }

	///////////////////////////////////////////

	void OnWindowSizeChanged(CoreWindow const & sender, WindowSizeChangedEventArgs const & /*args*/)
	{
		m_logicalWidth  = sender.Bounds().Width;
		m_logicalHeight = sender.Bounds().Height;

		if (m_in_sizemove)
			return;

		HandleWindowSizeChanged();
	}

	///////////////////////////////////////////

	void OnMouseButtonDown(CoreWindow const & /*sender*/, PointerEventArgs const &args) {
		window_event_t e = { platform_evt_mouse_press };
		auto           p = args.CurrentPoint().Properties();
		if (p.IsLeftButtonPressed  () && input_key(key_mouse_left)    == button_state_inactive) { e.data.press_release = key_mouse_left;    platform_win_add_event(win_id, e); }
		if (p.IsRightButtonPressed () && input_key(key_mouse_right)   == button_state_inactive) { e.data.press_release = key_mouse_right;   platform_win_add_event(win_id, e); }
		if (p.IsMiddleButtonPressed() && input_key(key_mouse_center)  == button_state_inactive) { e.data.press_release = key_mouse_center;  platform_win_add_event(win_id, e); }
		if (p.IsXButton1Pressed    () && input_key(key_mouse_back)    == button_state_inactive) { e.data.press_release = key_mouse_back;    platform_win_add_event(win_id, e); }
		if (p.IsXButton2Pressed    () && input_key(key_mouse_forward) == button_state_inactive) { e.data.press_release = key_mouse_forward; platform_win_add_event(win_id, e); }
	}

	///////////////////////////////////////////

	void OnMouseButtonUp(CoreWindow const & /*sender*/, PointerEventArgs const &args) {
		window_event_t e = { platform_evt_mouse_release };
		auto           p = args.CurrentPoint().Properties();
		if (!p.IsLeftButtonPressed  () && input_key(key_mouse_left)    == button_state_active) { e.data.press_release = key_mouse_left;    platform_win_add_event(win_id, e); }
		if (!p.IsRightButtonPressed () && input_key(key_mouse_right)   == button_state_active) { e.data.press_release = key_mouse_right;   platform_win_add_event(win_id, e); }
		if (!p.IsMiddleButtonPressed() && input_key(key_mouse_center)  == button_state_active) { e.data.press_release = key_mouse_center;  platform_win_add_event(win_id, e); }
		if (!p.IsXButton1Pressed    () && input_key(key_mouse_back)    == button_state_active) { e.data.press_release = key_mouse_back;    platform_win_add_event(win_id, e); }
		if (!p.IsXButton2Pressed    () && input_key(key_mouse_forward) == button_state_active) { e.data.press_release = key_mouse_forward; platform_win_add_event(win_id, e); }
	}

	///////////////////////////////////////////

	void OnWheelChanged(CoreWindow const & /*sender*/, PointerEventArgs const &args) {
		window_event_t e = { platform_evt_scroll };
		e.data.scroll = (float)args.CurrentPoint().Properties().MouseWheelDelta();
		platform_win_add_event(win_id, e);
	}

	///////////////////////////////////////////

	void OnVisibilityChanged(CoreWindow const & /*sender*/, VisibilityChangedEventArgs const & args) {
		m_visible = args.Visible();

		window_event_t e = { platform_evt_app_focus };
		e.data.app_focus = m_visible
			? app_focus_active
			: app_focus_background;
		platform_win_add_event(win_id, e);
	}

	///////////////////////////////////////////

	void OnActivation(CoreWindow const & /*sender*/, WindowActivatedEventArgs const & args) {
		window_event_t e = { platform_evt_app_focus };
		e.data.app_focus = args.WindowActivationState() != CoreWindowActivationState::Deactivated
			? app_focus_active
			: app_focus_background;
		platform_win_add_event(win_id, e);
	}

	///////////////////////////////////////////

	void OnAcceleratorKeyActivated(CoreDispatcher const &, AcceleratorKeyEventArgs const & args)
	{
		if (args.EventType() == CoreAcceleratorKeyEventType::SystemKeyDown
			&&  args.VirtualKey() == VirtualKey::Enter
			&&  args.KeyStatus().IsMenuKeyDown
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

	///////////////////////////////////////////

	void OnDpiChanged(DisplayInformation const & sender, IInspectable const & /*args*/) {
		m_DPI = sender.LogicalDpi();
		HandleWindowSizeChanged();
	}

	///////////////////////////////////////////

	void OnOrientationChanged(DisplayInformation const & sender, IInspectable const & /*args*/) {
		auto resizeManager = CoreWindowResizeManager::GetForCurrentView();
		resizeManager.ShouldWaitForLayoutCompletion(true);

		m_currentOrientation = sender.CurrentOrientation();

		HandleWindowSizeChanged();

		resizeManager.NotifyLayoutCompleted();
	}

	///////////////////////////////////////////

private:
	bool  m_visible;
	bool  m_in_sizemove;
	float m_logicalWidth;
	float m_logicalHeight;
	float m_DPI;
	platform_win_t win_id;

	winrt::Windows::Graphics::Display::DisplayOrientations	m_nativeOrientation;
	winrt::Windows::Graphics::Display::DisplayOrientations	m_currentOrientation;

	///////////////////////////////////////////

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

	///////////////////////////////////////////

	void HandleWindowSizeChanged() {
		int32_t output_width  = maxi(1,(int32_t)dips_to_pixels(m_logicalWidth,  m_DPI));
		int32_t output_height = maxi(1,(int32_t)dips_to_pixels(m_logicalHeight, m_DPI));

		DXGI_MODE_ROTATION rotation = ComputeDisplayRotation();

		if (rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270) {
			int32_t swap_tmp = output_width;
			output_width  = output_height;
			output_height = swap_tmp;
		}

		if (output_width  == uwp_windows[win_id].resize_x &&
			output_height == uwp_windows[win_id].resize_y)
			return;

		uwp_windows[win_id].resize_pending = true;
		uwp_windows[win_id].resize_x = output_width;
		uwp_windows[win_id].resize_y = output_height;
	}
};

///////////////////////////////////////////

class UWPWindowFactory : public winrt::implements<UWPWindowFactory, IFrameworkViewSource> {
public:
	platform_win_t win_id;
	UWPWindowFactory(platform_win_t win_id) : win_id(win_id) {}
	IFrameworkView CreateView() {
		return winrt::make<UWPWindow>(win_id);
	}
};

///////////////////////////////////////////

void uwp_window_thread(void *win_id_ptr) {
	platform_win_t       win_id        = *(platform_win_t*)win_id_ptr;
	IFrameworkViewSource windowFactory = winrt::make<UWPWindowFactory>(win_id);
	CoreApplication::Run(windowFactory);
}

///////////////////////////////////////////

platform_win_t platform_win_make(const char* title, recti_t win_rect, platform_surface_ surface_type) {
	uwp_windows.add({});
	platform_win_t win_id = uwp_windows.count - 1;
	window_t      *win    = &uwp_windows[win_id];
	win->title = platform_to_wchar(title);
	mtx_init(&win->event_mtx, mtx_plain);

	_beginthread(uwp_window_thread, 0, &win_id);

	// Wait until the window thread is ready
	while (!win->initialized) { Sleep(1); }

	// Not all windows need a swapchain, but here's where we make 'em for those
	// that do.
	if (surface_type == platform_surface_swapchain) {
		skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
		skg_tex_fmt_ depth_fmt = (skg_tex_fmt_)render_preferred_depth_fmt();
		win->swapchain     = skg_swapchain_create(win->handle, color_fmt, skg_tex_fmt_none, win->resize_x, win->resize_y);
		win->has_swapchain = true;

		log_diagf("Created swapchain: %dx%d color:%s depth:%s", win->swapchain.width, win->swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));
	}

	if (!win->valid) {
		uwp_windows.remove(win_id);
		return -1;
	} else {
		return win_id;
	}
}

///////////////////////////////////////////

void platform_win_destroy(platform_win_t window) {
	window_t* win = &uwp_windows[window];

	if (win->has_swapchain) {
		skg_swapchain_destroy(&win->swapchain);
	}

	mtx_destroy(&win->event_mtx);

	win->events.free();
	sk_free(win->title);
	*win = {};
}

///////////////////////////////////////////

bool platform_win_resize(platform_win_t window_id, int32_t width, int32_t height) {
	window_t* win = &uwp_windows[window_id];

	width  = maxi(1, width);
	height = maxi(1, height);

	if (win->has_swapchain == false || (width == win->swapchain.width && height == win->swapchain.height))
		return false;

	skg_swapchain_resize(&win->swapchain, width, height);
	return true;
}

///////////////////////////////////////////

bool platform_win_next_event(platform_win_t window_id, platform_evt_* out_event, platform_evt_data_t* out_event_data) {
	window_t *window = &uwp_windows[window_id];
	if (window->events.count == 0) return false;

	mtx_lock(&window->event_mtx);
	*out_event      = window->events[0].type;
	*out_event_data = window->events[0].data;
	window->events.remove(0);
	mtx_unlock(&window->event_mtx);
	return true;
}

///////////////////////////////////////////

platform_win_type_ platform_win_type() { return platform_win_type_creatable; }

///////////////////////////////////////////

platform_win_t platform_win_get_existing() { return -1; }

///////////////////////////////////////////

skg_swapchain_t* platform_win_get_swapchain(platform_win_t window) { return uwp_windows[window].has_swapchain ? &uwp_windows[window].swapchain : nullptr; }

///////////////////////////////////////////

recti_t platform_win_rect(platform_win_t window_id) {
	window_t* win = &uwp_windows[window_id];

	// See if we can update it real quick
	//RECT r = {};
	//if (GetWindowRect(win->handle, &r))
	//	win->save_rect = r;

	return recti_t{
		win->save_rect.left,
		win->save_rect.top,
		win->save_rect.right  - win->save_rect.left,
		win->save_rect.bottom - win->save_rect.top };
}

///////////////////////////////////////////

void platform_win_add_event(platform_win_t win_id, window_event_t evt) {
	mtx_lock(&uwp_windows[win_id].event_mtx);
	uwp_windows[win_id].events.add(evt);
	mtx_unlock(&uwp_windows[win_id].event_mtx);
}

}

#endif // defined(SK_OS_WINDOWS_UWP)

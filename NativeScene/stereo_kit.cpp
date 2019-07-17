#include "stereo_kit.h"

#include "d3d.h"
#include "render.h"

#include <thread> // sleep_for

using namespace std;

bool sk_focused = true;
bool sk_run     = true;
HWND sk_window  = nullptr;

float  sk_timef = 0;
double sk_time  = 0;
double sk_time_start    = 0;
double sk_time_elapsed  = 0;
float  sk_time_elapsedf = 0;

bool sk_init(const char *app_name) {
	MSG      msg     = {0};
	WNDCLASS wc      = {0}; 
	wc.lpfnWndProc = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch(message) {
		case WM_CLOSE: sk_run = false; PostQuitMessage(0); break;
		case WM_SETFOCUS:  sk_focused = true;  break;
		case WM_KILLFOCUS: sk_focused = false; break;
		case WM_SIZE:       if (wParam != SIZE_MINIMIZED) d3d_resize_screen((UINT)LOWORD(lParam), (UINT)HIWORD(lParam)); break;
		case WM_SYSCOMMAND: if ((wParam & 0xfff0) == SC_KEYMENU) return (LRESULT)0; // Disable alt menu
		default: return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return (LRESULT)0;
	};
	wc.hInstance     = GetModuleHandle(NULL);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = app_name;
	if( !RegisterClass(&wc) ) return false;
	sk_window = CreateWindow(wc.lpszClassName, app_name, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 20, 20, 640, 480, 0, 0, wc.hInstance, nullptr);
	if( !sk_window ) return false;

	d3d_init(sk_window);
	render_initialize();
}

void sk_shutdown() {
	render_shutdown();
	d3d_shutdown();
}

void sk_update_timer() {
	FILETIME time;
	GetSystemTimePreciseAsFileTime(&time);
	LONGLONG ll_now = (LONGLONG)time.dwLowDateTime + ((LONGLONG)(time.dwHighDateTime) << 32LL);
	double time_curr = ll_now / 10000000.0;

	if (sk_time_start == 0)
		sk_time_start = time_curr;
	double new_time = time_curr - sk_time_start;
	sk_time_elapsed  = new_time - sk_time;
	sk_time          = new_time;
	sk_time_elapsedf = (float)sk_time_elapsed;
	sk_timef         = (float)sk_time;
}

bool sk_step(void (*app_update)(void), void (*app_draw)(void)) {
	MSG msg = {0};
	if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage (&msg);
		return sk_run;
	}
	sk_update_timer();

	app_update();
	d3d_render_begin();
	app_draw();
	render_draw();
	d3d_render_end();
	render_clear();

	this_thread::sleep_for(chrono::milliseconds(sk_focused?1:250));

	return sk_run;
}
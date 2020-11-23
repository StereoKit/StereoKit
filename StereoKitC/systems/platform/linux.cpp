#include "linux.h"
#if defined(SK_OS_LINUX)

#include "../../log.h"
#include "../../stereokit.h"

#include "openxr.h"
#include "flatscreen_input.h"
#include "../render.h"
#include "../input.h"
#include "../../_stereokit.h"
#include "../../libraries/sk_gpu.h"
#include <unistd.h>

namespace sk {

skg_swapchain_t linux_swapchain;

///////////////////////////////////////////

Display                 *dpy;
Window                  root;

GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None };
XVisualInfo             *vi;
Colormap                cmap;
XSetWindowAttributes    swa;
Window                  win;

bool                    fix_glx_oxr = true;

uint32_t                key_mask;


///////////////////////////////////////////
// Start Moses's input thread bs
///////////////////////////////////////////


enum _linux_key_types {
		notakey = 0, // {0} initialized will always be this
		keysym = 1,
		mouse = 2,
	} how_to_deal_with;

	struct _linux_key_map {
		int what; // true if XQueryPointer, false if XQueryKeymap
		int xkey_or_key_mask[8]; // X keysyms are a lot more specific than VK keys. for example, capital A vs lowercase a go into xk_a, or left and right shift go into... just.. shift.
	};



	struct _linux_key_map map[256] = {
	{0},
	{/*0x01 - key_mouse_left */ mouse,{(1<<8),} },
	{/*0x02 - key_mouse_right */ mouse,{(1<<9),} },{0},
	{/*0x04 - key_mouse_center */ mouse,{(1<<10),} },
	{/*0x05 - key_mouse_forward */ mouse,{(1<<11),} },
	{/*0x06 - key_mouse_back */ mouse,{(1<<12),} },{0},
	{/*0x08 - key_backspace */ keysym,{XK_BackSpace,} },
	{/*0x09 - key_tab */ keysym,{XK_Tab,} },{0},{0},{0},
	{/*0x0D - key_return */ keysym,{XK_Return,} },{0},{0},
	{/*0x10 - key_shift */ keysym,{XK_Shift_L,XK_Shift_R} },
	{/*0x11 - key_ctrl */ keysym,{XK_Control_L,XK_Control_R} },
	{/*0x12 - key_alt */ keysym,{XK_Alt_L,XK_Alt_R} },{0},
	{/*0x14 - key_caps_lock */ keysym,{XK_Caps_Lock,} },{0},{0},{0},{0},{0},{0},
	{/*0x1B - key_esc */ keysym,{XK_Escape,} },{0},{0},{0},{0},
	{/*0x20 - key_space */ keysym,{XK_space,XK_KP_Space} },{0},{0},
	{/*0x23 - key_end */ keysym,{XK_End,XK_KP_End} },
	{/*0x24 - key_home */ keysym,{XK_Home,XK_KP_Home} },
	{/*0x25 - key_left */ keysym,{XK_Left,XK_KP_Left} },
	{/*0x26 - key_up */ keysym,{XK_Up,XK_KP_Up} },
	{/*0x27 - key_right */ keysym,{XK_Right,XK_KP_Right} },
	{/*0x28 - key_down */ keysym,{XK_Down,XK_KP_Down} },{0},
	{/*0x2A - key_printscreen */ keysym,{XK_Print,} },{0},{0},
	{/*0x2D - key_insert */ keysym,{XK_Insert,XK_KP_Insert} },
	{/*0x2E - key_del */ keysym,{XK_Delete,XK_KP_Delete} },{0},
	{/*0x30 - key_0 */ keysym,{XK_0,} },
	{/*0x31 - key_1 */ keysym,{XK_1,} },
	{/*0x32 - key_2 */ keysym,{XK_2,} },
	{/*0x33 - key_3 */ keysym,{XK_3,} },
	{/*0x34 - key_4 */ keysym,{XK_4,} },
	{/*0x35 - key_5 */ keysym,{XK_5,} },
	{/*0x36 - key_6 */ keysym,{XK_6,} },
	{/*0x37 - key_7 */ keysym,{XK_7,} },
	{/*0x38 - key_8 */ keysym,{XK_8,} },
	{/*0x39 - key_9 */ keysym,{XK_9,} },{0},{0},{0},{0},{0},{0},{0},
	{/*0x41 - key_a */ keysym,{XK_A,XK_a} },
	{/*0x42 - key_b */ keysym,{XK_B,XK_b} },
	{/*0x43 - key_c */ keysym,{XK_C,XK_c} },
	{/*0x44 - key_d */ keysym,{XK_D,XK_d} },
	{/*0x45 - key_e */ keysym,{XK_E,XK_e} },
	{/*0x46 - key_f */ keysym,{XK_F,XK_f} },
	{/*0x47 - key_g */ keysym,{XK_G,XK_g} },
	{/*0x48 - key_h */ keysym,{XK_H,XK_h} },
	{/*0x49 - key_i */ keysym,{XK_I,XK_i} },
	{/*0x4A - key_j */ keysym,{XK_J,XK_j} },
	{/*0x4B - key_k */ keysym,{XK_K,XK_k} },
	{/*0x4C - key_l */ keysym,{XK_L,XK_l} },
	{/*0x4D - key_m */ keysym,{XK_M,XK_m} },
	{/*0x4E - key_n */ keysym,{XK_N,XK_n} },
	{/*0x4F - key_o */ keysym,{XK_O,XK_o} },
	{/*0x50 - key_p */ keysym,{XK_P,XK_p} },
	{/*0x51 - key_q */ keysym,{XK_Q,XK_q} },
	{/*0x52 - key_r */ keysym,{XK_R,XK_r} },
	{/*0x53 - key_s */ keysym,{XK_S,XK_s} },
	{/*0x54 - key_t */ keysym,{XK_T,XK_t} },
	{/*0x55 - key_u */ keysym,{XK_U,XK_u} },
	{/*0x56 - key_v */ keysym,{XK_V,XK_v} },
	{/*0x57 - key_w */ keysym,{XK_W,XK_w} },
	{/*0x58 - key_x */ keysym,{XK_X,XK_x} },
	{/*0x59 - key_y */ keysym,{XK_Y,XK_y} },
	{/*0x5A - key_z */ keysym,{XK_Z,XK_z} },
	{/*0x5B - key_lcmd */ keysym,{XK_Super_L,} },
	{/*0x5C - key_rcmd */ keysym,{XK_Super_R,} },{0},{0},{0},
	{/*0x60 - key_num0 */ keysym,{XK_KP_0,} },
	{/*0x61 - key_num1 */ keysym,{XK_KP_1,} },
	{/*0x62 - key_num2 */ keysym,{XK_KP_2,} },
	{/*0x63 - key_num3 */ keysym,{XK_KP_3,} },
	{/*0x64 - key_num4 */ keysym,{XK_KP_4,} },
	{/*0x65 - key_num5 */ keysym,{XK_KP_5,} },
	{/*0x66 - key_num6 */ keysym,{XK_KP_6,} },
	{/*0x67 - key_num7 */ keysym,{XK_KP_7,} },
	{/*0x68 - key_num8 */ keysym,{XK_KP_8,} },
	{/*0x69 - key_num9 */ keysym,{XK_KP_9,} },
	{/*0x6A - key_multiply */ keysym,{XK_KP_Multiply,} },
	{/*0x6B - key_add */ keysym,{XK_KP_Add,} },{0},
	{/*0x6D - key_subtract */ keysym,{XK_KP_Subtract,} },
	{/*0x6E - key_decimal */ keysym,{XK_KP_Decimal,} },
	{/*0x6F - key_divide */ keysym,{XK_KP_Divide,} },
	{/*0x70 - key_f1 */ keysym,{XK_F1,} },
	{/*0x71 - key_f2 */ keysym,{XK_F2,} },
	{/*0x72 - key_f3 */ keysym,{XK_F3,} },
	{/*0x73 - key_f4 */ keysym,{XK_F4,} },
	{/*0x74 - key_f5 */ keysym,{XK_F5,} },
	{/*0x75 - key_f6 */ keysym,{XK_F6,} },
	{/*0x76 - key_f7 */ keysym,{XK_F7,} },
	{/*0x77 - key_f8 */ keysym,{XK_F8,} },
	{/*0x78 - key_f9 */ keysym,{XK_F9,} },
	{/*0x79 - key_f10 */ keysym,{XK_F10,} },
	{/*0x7A - key_f11 */ keysym,{XK_F11,} },
	{/*0x7B - key_f12 */ keysym,{XK_F12,} },{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
	{/*0xFF - key_MAX */ keysym,{0x69,} },
	}; 

	bool _linux_pressed_sk_keys[256] = {0};
	int scrollwheel;
	int mouseX;
	int mouseY;

bool _in_thread_get_keypress(){
											char keys_return[32];
    								XQueryKeymap(dpy, keys_return);

												for(int i=0; i<key_MAX; i++) {
													if (map[i].what != keysym) {continue;}


												bool isPressed = false;
													for (int j=0; j<7; j++){
														if (map[i].xkey_or_key_mask[j] == 0) { break;}
														KeyCode kc2 = XKeysymToKeycode(dpy, map[i].xkey_or_key_mask[j]);
														isPressed = !!(keys_return[kc2 >> 3] & (1 << (kc2 & 7)));
														if (isPressed == true) {_linux_pressed_sk_keys[i] = true; break;}

													}
													if (isPressed == false) {_linux_pressed_sk_keys[i] = false;}
													if (_linux_pressed_sk_keys[i]) { printf("yes 0x%x\n", i);}
												}



	return false;
													
}


void* linux_input_pthread(void* no)
{


    XEvent event;
 
    


				usleep(100000);
    XSelectInput(dpy, win, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask); // | ButtonPressMask | ButtonReleaseMask 
 
    /* map (show) the window */
    //XMapWindow(dpy, win);
				//int what = 1;
				//KeySym* aye = XGetKeyboardMapping(dpy,8,0xf8,&what);


    /* event loop */
    while (1)
    {
        XNextEvent(dpy, &event);
        /* keyboard events */
        if (event.type == KeyPress || event.type == KeyRelease)
        {
												//printf("got keypress\n");

            	_in_thread_get_keypress();
        }
								else if (event.type == ButtonPress)
								{
											switch(event.xbutton.button){
												case (1):
													_linux_pressed_sk_keys[key_mouse_left] = true;	break;
												case (2):
													_linux_pressed_sk_keys[key_mouse_center] = true; break;
												case (3):
													_linux_pressed_sk_keys[key_mouse_right] = true; break;
												case (4): /*scroll up*/ scrollwheel++; break;
												case (5): /*scroll down*/ scrollwheel--; break;
												case (9):
													_linux_pressed_sk_keys[key_mouse_forward] = true; break;
												case (8):
													_linux_pressed_sk_keys[key_mouse_back] = true; break;
											}
												//printf("%d asd;flk? %d\n",event.xbutton.state,event.xbutton.button);
        }else if (event.type == ButtonRelease){
												//printf("%d assdf3d;flk? %d\n",event.xbutton.state,event.xbutton.button);
											switch(event.xbutton.button){
												case (1):
													_linux_pressed_sk_keys[key_mouse_left] = false;	break;
												case (2):
													_linux_pressed_sk_keys[key_mouse_center] = false; break;
												case (3):
													_linux_pressed_sk_keys[key_mouse_right] = false; break;
												case (9):
													_linux_pressed_sk_keys[key_mouse_forward] = false; break;
												case (8):
													_linux_pressed_sk_keys[key_mouse_back] = false; break;
													/* We get a ButtonRelease event directly after ButtonPress for scroll wheel stuff.
													Since all it does is bump a variable up or down, we don't need to do anything for release.*/
											}

								}else if (event.type == MotionNotify){
									printf("motionnotify!\n");
									mouseX = event.xmotion.x;
									mouseY = event.xmotion.y;
								} else {
								log_diagf("uncaught X event StereoKitC/systems/platform/linux.cpp\n");
									
								}
    }

 
    return 0;
}


///////////////////////////////////////////
// End Moses's input thread bs
///////////////////////////////////////////


bool linux_init() {
	dpy = XOpenDisplay(0);
	if (dpy == nullptr) {
		log_fail_reason(90, "Cannot connect to X server");
		return false;
	}

	root = DefaultRootWindow(dpy);
	vi   = glXChooseVisual(dpy, 0, att);

	if (vi == nullptr) {
		log_fail_reason(90, "No appropriate GLX visual found");
		return false;
	}

	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask;

	win = XCreateWindow(dpy, root, 0, 0, 1280, 720, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

	XMapWindow(dpy, win);
	XStoreName(dpy, win, sk_app_name);

	skg_setup_xlib(dpy, vi, &win);

	return true;
}

///////////////////////////////////////////

bool linux_start() {
	fix_glx_oxr = false;

	sk_info.display_width  = sk_settings.flatscreen_width;
	sk_info.display_height = sk_settings.flatscreen_height;
	sk_info.display_type   = display_opaque;
	skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
	skg_tex_fmt_ depth_fmt = skg_tex_fmt_depth16;
	linux_swapchain = skg_swapchain_create(&win, color_fmt, depth_fmt, sk_info.display_width, sk_info.display_height);
	sk_info.display_width  = linux_swapchain.width;
	sk_info.display_height = linux_swapchain.height;

	pthread_t imnotimportant;
 

	flatscreen_input_init();
	pthread_create(&imnotimportant,nullptr,linux_input_pthread,(void*)1);

	return true;
}

///////////////////////////////////////////

bool linux_get_cursor(vec2 &out_pos) {
	// Window root_window, child_window;
	// int root_cursor_pos[2], win_cursor_pos[2];

	// bool result = XQueryPointer(
	// 	dpy, win,
	// 	&root_window, &child_window, //Root window, child window
	// 	&root_cursor_pos[0], &root_cursor_pos[1], //Root relative X and Y coordinates
	// 	&win_cursor_pos[0], &win_cursor_pos[1], //Window relative X and Y coordinates
	// 	&key_mask //Key mask
	// );

	// if(result) {
	// 	out_pos.x = (float) win_cursor_pos[0];
	// 	out_pos.y = (float) win_cursor_pos[1];
	// }

	// return result;
	out_pos.x = (float)mouseX;
	out_pos.y = (float)mouseY;
	return true;
}

///////////////////////////////////////////

bool linux_key_down(key_ key) {
	return _linux_pressed_sk_keys[key];
}

///////////////////////////////////////////

void linux_stop() {
	flatscreen_input_shutdown();
	skg_swapchain_destroy(&linux_swapchain);
}

///////////////////////////////////////////

void linux_shutdown() {
	glXMakeCurrent(dpy, None, nullptr);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

///////////////////////////////////////////

void linux_step_begin() {
	if(fix_glx_oxr) {
		fix_glx_oxr = false;
		// XDestroyWindow(dpy, win);
	}

	flatscreen_input_update();
}

///////////////////////////////////////////

void linux_step_end() {
	skg_draw_begin();

	color128 col = render_get_clear_color();
	skg_swapchain_bind(&linux_swapchain, true, &col.r);

	input_update_predicted();

	matrix view = render_get_cam_root  ();
	matrix proj = render_get_projection();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1);
	render_clear();
}

///////////////////////////////////////////

void linux_vsync() {
	skg_swapchain_present(&linux_swapchain);
}

} // namespace sk

#endif // SK_OS_LINUX

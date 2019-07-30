#include "win32_input.h"

#include <windows.h>

#include "win32.h"
#include "input.h"
#include "input_hand.h"
#include "render.h"
#include "d3d.h"

int win32_input_pointers[2];

void win32_input_init() {
	win32_input_pointers[0] = input_add_pointer(pointer_source_gaze | pointer_source_gaze_cursor | pointer_source_can_press);
	win32_input_pointers[1] = input_add_pointer(pointer_source_gaze | pointer_source_gaze_head);
}
void win32_input_shutdown() {
}

void win32_input_update() {
	pointer_t *pointer_cursor = input_get_pointer(win32_input_pointers[0]);
	pointer_t *pointer_head   = input_get_pointer(win32_input_pointers[1]);

	camera_t    *cam = nullptr;
	transform_t *cam_tr = nullptr;
	render_get_cam(&cam, &cam_tr);

	const hand_t &hand = input_hand(hand_right);
	vec3 hand_pos     = hand.root.position;
	quat hand_rot     = hand.root.orientation;
	bool l_pressed    = false;
	bool r_pressed    = false;
	bool hand_tracked = false;
	vec3 pointer_dir  = pointer_cursor->ray.dir;

	pointer_cursor->state = pointer_state_none;
	pointer_head  ->state = pointer_state_none;

	if (cam != nullptr) {
		pointer_head->state = pointer_state_available;
		pointer_head->ray.pos = cam_tr->_position;
		pointer_head->ray.dir = transform_forward(*cam_tr);

		POINT cursor_pos;
		if (GetCursorPos(&cursor_pos) && ScreenToClient(win32_window, &cursor_pos))
		{
			float x = (((cursor_pos.x / (float)d3d_screen_width ) - 0.5f) *  2.f);
			float y = (((cursor_pos.y / (float)d3d_screen_height) - 0.5f) * -2.f);
			if (x >= -1 && y >= -1 && x <= 1 && y <= 1) {
				hand_tracked = true;
				l_pressed    = GetKeyState(VK_LBUTTON) < 0;
				r_pressed    = GetKeyState(VK_RBUTTON) < 0;

				// convert screen pos to world ray
				DirectX::XMMATRIX mat, view, proj;
				camera_view(*cam_tr, view);
				camera_proj(*cam,    proj);
				mat = view * proj;
				DirectX::XMMATRIX inv        = DirectX::XMMatrixInverse(nullptr, mat);
				DirectX::XMVECTOR cursor_vec = DirectX::XMVectorSet(x, y, 1.0f, 0.0f);
				cursor_vec                   = DirectX::XMVector3Transform(cursor_vec, inv);
				DirectX::XMStoreFloat3((DirectX::XMFLOAT3 *) &pointer_dir, cursor_vec);
				pointer_dir = vec3_normalize(pointer_dir);

				// make a hand pose from the cursor ray
				hand_pos = cam_tr->_position + pointer_dir * (0.6f + win32_scroll * 0.001f);
				hand_rot = quat_lookat({ 0,0,0 }, pointer_dir);
			}
		}
	}

	pointer_cursor->state |= pointer_state_available;
	pointer_cursor->ray.dir     = pointer_dir;
	pointer_cursor->ray.pos     = hand_pos;
	pointer_cursor->orientation = hand_rot;

	input_hand_sim(hand_right, hand_pos, hand_rot, hand_tracked, l_pressed, r_pressed);
}
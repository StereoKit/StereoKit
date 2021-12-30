#include "VirtualKeyboard.h"
#include "stereokit_ui.h"
#include "systems/input_keyboard.h"
#include "libraries/array.h"
#include "libraries/unicode.h"

namespace sk {
	const keyboard_layout_t virtualkeyboard_get_uskeyboard_layout =
		{
			////////////       text input        //////////////
			{
				//Normal
				{
					{
						{
							{  u"`",  u"`",1,key_backtick},{u"1", u"1",1,key_1}, {u"2", u"2",1,key_2}, {u"3", u"3",1,key_3}, {u"4", u"4",1,key_4}, {u"5", u"5",1,key_5}, {u"6", u"6",1,key_6}, {u"7", u"7",1,key_7}, {u"8", u"8",1,key_8}, {u"9", u"9",1,key_9}, {u"0", u"0",1,key_0}, {u"-", u"-",1,key_minus}, {u"=", u"=",1,key_equals},{u"\b", u"<--",1.5f,key_backspace}
						},
						{
							{u"\t", u"Tab",1.5,key_tab},{u"q", u"q",1,key_q},{u"w", u"w",1,key_w},{u"e", u"e",1,key_e},{u"r", u"r",1,key_r},{u"t", u"t",1,key_t},{u"y", u"y",1,key_y},{u"u", u"u",1,key_u},{u"i", u"i",1,key_i},{u"o", u"o",1,key_o},{u"p", u"p",1,key_p},{u"[", u"[",1,key_bracket_open},{u"]", u"]",1,key_bracket_close},{u"\\", u"\\",1,key_slash_back}
						},
						{
							{u"", u"CapsLk",1.9,key_caps_lock,special_key_shift},{u"a", u"a",1,key_a},{u"s", u"s",1,key_s},{u"d", u"d",1,key_d},{u"f", u"f",1,key_f},{u"g", u"g",1,key_g},{u"h", u"h",1,key_h},{u"j", u"j",1,key_j},{u"k", u"k",1,key_k},{u"l", u"l",1,key_l},{u";", u";",1,key_semicolon},{u"'", u"'",1,key_apostrophe},{u"\r", u"Enter",1.9,key_return}
						},
						{
							{u"", u"Shift",2.5,key_shift,special_key_shift},{u"z", u"z",1,key_z},{u"x", u"x",1,key_x},{u"c", u"c",1,key_c},{u"v", u"v",1,key_v},{u"b", u"b",1,key_b},{u"n", u"n",1,key_n},{u"m", u"m",1,key_m},{u",", u",",1,key_comma},{u".", u".",1,key_period},{u"/", u"/",1,key_slash_fwd},{u"", u"Shift",2.5,key_shift,special_key_shift},{u"", u"",1,key_none,special_key_spacer},{u"", u"^",1,key_up}
						},
						{
							{u"", u"Ctrl",1.75,key_ctrl,special_key_ctrl},{u"", u"Cmd",1,key_rcmd},{u"", u"Alt",1.75,key_alt,special_key_alt},{u" ", u"     ",6.4,key_space},{u"", u"Alt",1.75,key_alt,special_key_alt},{u"", u"Ctrl",1.75,key_ctrl,special_key_ctrl},{u"", u"",2,key_none,special_key_spacer},{u"", u"<",1,key_left},{u"", u"v",1,key_down},{u"", u" >",1,key_right}
						}
					}
				},
				//shift
				{
					{
						{
							{ u"~", u"~",1,key_backtick},{u"!", u"!",1,key_1}, {u"@", u"@",1,key_2}, {u"#", u"#",1,key_3}, {u"$", u"$",1,key_4}, {u"%", u"%",1,key_5}, {u"^", u"^",1,key_6}, {u"&", u"&",1,key_7}, {u"*", u"*",1,key_8}, {u"(", u"(",1,key_9}, {u")", u")",1,key_0}, {u"_", u"_",1,key_minus}, {u"+", u"+",1,key_equals},{u"\b", u"<--",1.5f,key_backspace}
						},
						{
							{u"\t", u"Tab",1.5,key_tab},{u"Q", u"Q",1,key_q},{u"W", u"W",1,key_w},{u"E", u"E",1,key_e},{u"R", u"R",1,key_r},{u"T", u"T",1,key_t},{u"Y", u"Y",1,key_y},{u"U", u"U",1,key_u},{u"I", u"I",1,key_i},{u"O", u"O",1,key_o},{u"P", u"P",1,key_p},{u"{u", u"{u",1,key_bracket_open},{u"}", u"}",1,key_bracket_close},{u"|", u"|",1,key_slash_back}
						},
						{
							{u"", u"CapsLk",1.9,key_caps_lock,special_key_shift},{u"A", u"A",1,key_a},{u"S", u"S",1,key_s},{u"D", u"D",1,key_d},{u"F", u"F",1,key_f},{u"G", u"G",1,key_g},{u"H", u"H",1,key_h},{u"J", u"J",1,key_j},{u"K", u"K",1,key_k},{u"L", u"L",1,key_l},{u":", u":",1,key_semicolon},{u"\"", u"\"",1,key_apostrophe},{u"\n", u"NewLine",1.9,key_return}
						},
						{
							{u"", u"Shift",2.5,key_shift,special_key_shift},{u"Z", u"Z",1,key_z},{u"X", u"X",1,key_x},{u"C", u"C",1,key_c},{u"V", u"V",1,key_v},{u"B", u"B",1,key_b},{u"N", u"N",1,key_n},{u"M", u"M",1,key_m},{u"<", u"<",1,key_comma},{u">", u">",1,key_period},{u"?", u"?",1,key_slash_fwd},{u"", u"Shift",2.5,key_shift,special_key_shift},{u"", u"",1,key_none,special_key_spacer},{u"", u"^",1,key_up}
						},
						{
							{u"", u"Ctrl",1.75,key_ctrl,special_key_ctrl},{u"", u"Cmd",1,key_rcmd},{u"", u"Alt",1.75,key_alt,special_key_alt},{u" ", u"     ",6.4,key_space},{u"", u"Alt",1.75,key_alt,special_key_alt},{u"", u"Ctrl",1.75,key_ctrl,special_key_ctrl},{u"", u"",2,key_none,special_key_spacer},{u"", u"<",1,key_left},{u"", u"v",1,key_down},{u"", u" >",1,key_right}
						}
					}
				},
				{
					// altgr
				},
				{
					// altgr and shift
				},
				{
					// fn
				}
			},
			/////////////////////////////////////////////////////////////////////////////
			////////////       uri text input        //////////////
			{
				//Normal
				{
					{
						{
							{ u"`", u"`",1,key_backtick},{u"1", u"1",1,key_1}, {u"2", u"2",1,key_2}, {u"3", u"3",1,key_3}, {u"4", u"4",1,key_4}, {u"5", u"5",1,key_5}, {u"6", u"6",1,key_6}, {u"7", u"7",1,key_7}, {u"8", u"8",1,key_8}, {u"9", u"9",1,key_9}, {u"0", u"0",1,key_0}, {u"-", u"-",1,key_minus}, {u"=", u"=",1,key_equals},{u"\b", u"<--",1.5f,key_backspace}
						},
						{
							{u"\t", u"Tab",1.5,key_tab},{u"q", u"q",1,key_q},{u"w", u"w",1,key_w},{u"e", u"e",1,key_e},{u"r", u"r",1,key_r},{u"t", u"t",1,key_t},{u"y", u"y",1,key_y},{u"u", u"u",1,key_u},{u"i", u"i",1,key_i},{u"o", u"o",1,key_o},{u"p", u"p",1,key_p},{u"[", u"[",1,key_bracket_open},{u"]", u"]",1,key_bracket_close},{u"\\", u"\\",1,key_slash_back}
						},
						{
							{u"", u"CapsLk",1.9,key_caps_lock,special_key_shift},{u"a", u"a",1,key_a},{u"s", u"s",1,key_s},{u"d", u"d",1,key_d},{u"f", u"f",1,key_f},{u"g", u"g",1,key_g},{u"h", u"h",1,key_h},{u"j", u"j",1,key_j},{u"k", u"k",1,key_k},{u"l", u"l",1,key_l},{u";", u";",1,key_semicolon},{u"'", u"'",1,key_apostrophe},{u"\r", u"Enter",1.9,key_return}
						},
						{
							{u"", u"Shift",2.5,key_shift,special_key_shift},{u"z", u"z",1,key_z},{u"x", u"x",1,key_x},{u"c", u"c",1,key_c},{u"v", u"v",1,key_v},{u"b", u"b",1,key_b},{u"n", u"n",1,key_n},{u"m", u"m",1,key_m},{u",", u",",1,key_comma},{u".", u".",1,key_period},{u"/", u"/",1,key_slash_fwd},{u"", u"Shift",2.5,key_shift,special_key_shift},{u"", u"",1,key_none,special_key_spacer},{u"", u"^",1,key_up}
						},
						{
							{u"", u"Ctrl",1.75,key_ctrl,special_key_ctrl},{u"", u"Cmd",1,key_rcmd},{u"", u"Alt",1.75,key_alt,special_key_alt},{u" ", u"     ",6.4,key_space},{u"", u"Alt",1.75,key_alt,special_key_alt},{u"", u"Ctrl",1,key_ctrl,special_key_ctrl},{u".com", u".com",1},{u"https://", u"https://",1},{u"://", u"://",1},{u"", u"<",1,key_left},{u"", u"v",1,key_down},{u"", u" >",1,key_right}
						}
					}
				},
				//shift
				{
					{
						{
							{ u"~", u"~",1,key_backtick},{u"!", u"!",1,key_1}, {u"@", u"@",1,key_2}, {u"#", u"#",1,key_3}, {u"$", u"$",1,key_4}, {u"%", u"%",1,key_5}, {u"^", u"^",1,key_6}, {u"&", u"&",1,key_7}, {u"*", u"*",1,key_8}, {u"(", u"(",1,key_9}, {u")", u")",1,key_0}, {u"_", u"_",1,key_minus}, {u"+", u"+",1,key_equals},{u"\b", u"<--",1.5f,key_backspace}
						},
						{
							{u"\t", u"Tab",1.5,key_tab},{u"Q", u"Q",1,key_q},{u"W", u"W",1,key_w},{u"E", u"E",1,key_e},{u"R", u"R",1,key_r},{u"T", u"T",1,key_t},{u"Y", u"Y",1,key_y},{u"U", u"U",1,key_u},{u"I", u"I",1,key_i},{u"O", u"O",1,key_o},{u"P", u"P",1,key_p},{u"{u", u"{u",1,key_bracket_open},{u"}", u"}",1,key_bracket_close},{u"|", u"|",1,key_slash_back}
						},
						{
							{u"", u"CapsLk",1.9,key_caps_lock,special_key_shift},{u"A", u"A",1,key_a},{u"S", u"S",1,key_s},{u"D", u"D",1,key_d},{u"F", u"F",1,key_f},{u"G", u"G",1,key_g},{u"H", u"H",1,key_h},{u"J", u"J",1,key_j},{u"K", u"K",1,key_k},{u"L", u"L",1,key_l},{u":", u":",1,key_semicolon},{u"\"", u"\"",1,key_apostrophe},{u"\n", u"NewLine",1.9,key_return}
						},
						{
							{u"", u"Shift",2.5,key_shift,special_key_shift},{u"Z", u"Z",1,key_z},{u"X", u"X",1,key_x},{u"C", u"C",1,key_c},{u"V", u"V",1,key_v},{u"B", u"B",1,key_b},{u"N", u"N",1,key_n},{u"M", u"M",1,key_m},{u"<", u"<",1,key_comma},{u">", u">",1,key_period},{u"?", u"?",1,key_slash_fwd},{u"", u"Shift",2.5,key_shift,special_key_shift},{u"", u"",1,key_none,special_key_spacer},{u"", u"^",1,key_up}
						},
						{
							{u"", u"Ctrl",1.75,key_ctrl,special_key_ctrl},{u"", u"Cmd",1,key_rcmd},{u"", u"Alt",1.75,key_alt,special_key_alt},{u" ", u"     ",6.4,key_space},{u"", u"Alt",1.75,key_alt,special_key_alt},{u"", u"Ctrl",1,key_ctrl,special_key_ctrl},{u".com", u".com",1},{u"https://", u"https://",1},{u"://", u"://",1},{u"", u"<",1,key_left},{u"", u"v",1,key_down},{u"", u" >",1,key_right}
						}
					}
				},
				{
					// altgr
				},
				{
					// altgr and shift
				},
				{
					// fn
				}
			},
			/////////////////////////////////////////////////////////////////////////////
			////////////       number input        //////////////
			{
				//Normal
				{
					{
						{
							{u"\r", u"Enter",1.6,key_return},{u"\b", u"<--",1.6f,key_backspace}
						},
						{
							{u"7", u"7",1,key_7},{u"8", u"8",1,key_8},{u"9", u"9",1,key_9}
						},
						{
							{u"4", u"4",1,key_4},{u"5", u"5",1,key_5},{u"6", u"6",1,key_6}
						},
						{
							{u"1", u"1",1,key_1},{u"2", u"2",1,key_2},{u"3", u"3",1,key_3}
						},
						{
							{u"0", u"0",3.6,key_0}
						}
					}
				},
			{
				//shift
			},
			{
				// altgr
			},
			{
				// altgr and shift
			},
			{
				// fn
			}
		},
		/////////////////////////////////////////////////////////////////////////////
		////////////       decimal input        //////////////
			{
				//Normal
				{
					{
						{
							{u"\r", u"Enter",1.6,key_return},{u"\b", u"<--",1.6f,key_backspace}
						},
						{
							{u"7", u"7",1,key_7},{u"8", u"8",1,key_8},{u"9", u"9",1,key_9}
						},
						{
							{u"4", u"4",1,key_4},{u"5", u"5",1,key_5},{u"6", u"6",1,key_6}
						},
						{
							{u"1", u"1",1,key_1},{u"2", u"2",1,key_2},{u"3", u"3",1,key_3}
						},
						{
							{u"0", u"0",1.6,key_0},{u".", u".",1.6,key_period}
						}
					}
				},
			{
				//shift
			},
			{
				// altgr
			},
			{
				// altgr and shift
			},
			{
				// fn
			}
		},
		/////////////////////////////////////////////////////////////////////////////
		////////////       signed input        //////////////
			{
				//Normal
				{
					{
						{
							{u"\r", u"Enter",1.6,key_return},{u"\b", u"<--",1.6f,key_backspace}
						},
						{
							{u"7", u"7",1,key_7},{u"8", u"8",1,key_8},{u"9", u"9",1,key_9}
						},
						{
							{u"4", u"4",1,key_4},{u"5", u"5",1,key_5},{u"6", u"6",1,key_6}
						},
						{
							{u"1", u"1",1,key_1},{u"2", u"2",1,key_2},{u"3", u"3",1,key_3}
						},
						{
							{u"0", u"0",1.6,key_0},{u"-", u"-",1.6,key_minus}
						}
					}
				},
			{
				//shift
			},
			{
				// altgr
			},
			{
				// altgr and shift
			},
			{
				// fn
			}
			},
			/////////////////////////////////////////////////////////////////////////////
			////////////       signed decimal input        //////////////
			{
				//Normal
				{
					{
						{
							{u"\r", u"Enter",1.6,key_return},{u"\b", u"<--",1.6f,key_backspace}
						},
						{
							{u"7", u"7",1,key_7},{u"8", u"8",1,key_8},{u"9", u"9",1,key_9}
						},
						{
							{u"4", u"4",1,key_4},{u"5", u"5",1,key_5},{u"6", u"6",1,key_6}
						},
						{
							{u"1", u"1",1,key_1},{u"2", u"2",1,key_2},{u"3", u"3",1,key_3}
						},
						{
							{u"0", u"0",1,key_0},{u".", u".",1,key_period},{u"-", u"-",1,key_minus}
						}
					}
				},
			{
				//shift
			},
			{
				// altgr
			},
			{
				// altgr and shift
			},
			{
				// fn
			}
			},
			/////////////////////////////////////////////////////////////////////////////
		};

	bool32_t keyboard_fn = false;
	bool32_t keyboard_altgr = false;
	bool32_t keyboard_shift = false;
	bool32_t keyboard_open = false;

	bool32_t keyboard_ctrl = false;
	bool32_t keyboard_alt = false;

	pose_t keyboard_pose = { {-0.1f, 0.1f, -0.2f},quat_lookat({0,0,0},{1,1,1})};
	array_t<key_> pressed_keys;
	const keyboard_layout_t* current_keyboard_layout;
	input_text_context_type_ input_type;
	const keyboard_layout_t* virtualkeyboard_get_system_keyboard_layout()
	{
		return &virtualkeyboard_get_uskeyboard_layout;
	}

	void virtualkeyboard_open(bool open, input_text_context_type_ type)
	{
		input_type = type;
		if (open != keyboard_open) {
			if (open) {
				if (keyboard_alt) {
					input_keyboard_inject_release(key_alt);
					keyboard_alt = false;
				}
				if (keyboard_altgr) {
					input_keyboard_inject_release(key_alt);
					keyboard_altgr = false;
				}
				if (keyboard_ctrl) {
					input_keyboard_inject_release(key_ctrl);
					keyboard_ctrl = false;
				}
				if (keyboard_fn) {
					keyboard_fn = false;
				}
				if (keyboard_alt) {
					input_keyboard_inject_release(key_alt);
					keyboard_alt = false;
				}
				if (keyboard_shift) {
					input_keyboard_inject_release(key_shift);
					keyboard_shift = false;
				}
			}
			keyboard_open = open;
		}
	}

	bool virtualkeyboard_get_open() {
		return keyboard_open;
	}

	void virtualkeyboard_initialize()
	{
		pressed_keys.free();
		current_keyboard_layout = virtualkeyboard_get_system_keyboard_layout();
	}

	void send_key_data(const char16_t* charkey,key_ key) {
		pressed_keys.add(key);
		input_keyboard_inject_press(key);
		char32_t c = 0;
		while (utf16_decode_fast_b(charkey, &charkey, &c)) {
			input_text_inject_char(c);
		}
	}

	void remove_last_clicked_keys() {
		for (int i = 0; i < pressed_keys.count; i++)
		{
			input_keyboard_inject_release(pressed_keys[0]);
			pressed_keys.remove(0);
		}
	}

	void virtualkeyboard_keypress(keyboard_layout_Key_t key) {
		send_key_data(key.clicked_text, key.key_event_type);
		if (key.special_key == special_key_close_keyboard) {
			keyboard_open = false;
		}
	}

	void virtualkeyboard_update()
	{
		if (keyboard_open) {
			if (current_keyboard_layout == nullptr) {
				return;
			}
			remove_last_clicked_keys();
			hierarchy_push(render_get_cam_root());
			ui_window_begin("Keyboard", keyboard_pose, {0,0},ui_win_body);
			ui_push_no_keyboard_loss(true);
			int typeIndex = 0;
			if (keyboard_shift) {
				if (keyboard_altgr) {
					typeIndex = 3;
				}
				else {
					typeIndex = 1;
				}
			}
			else {
				if (keyboard_altgr) {
					typeIndex = 2;
				}
			}
			if (keyboard_fn) {
				typeIndex = 3;
			}
			const keyboard_layout_Layer_t* layer = &current_keyboard_layout->text_layer[typeIndex];
			switch (input_type)
			{
			case sk::input_text_context_type_number:
				layer = &(current_keyboard_layout->number_layer[typeIndex]);
				break;
			case sk::input_text_context_type_number_decimal:
				layer = &current_keyboard_layout->number_decimal_layer[typeIndex];
				break;
			case sk::input_text_context_type_number_signed:
				layer = &current_keyboard_layout->number_signed_layer[typeIndex];
				break;
			case sk::input_text_context_type_number_signed_decimal:
				layer = &current_keyboard_layout->number_signed_decimal_layer[typeIndex];
				break;
			case sk::input_text_context_type_text_uri:
				layer = &current_keyboard_layout->text_uri_layer[typeIndex];
				break;
			}
			float buttonSize = 0.035f;
			for (int row = 0; row < 6; row++)
			{
				for (int i = 0; i < 35; i++)
				{
					keyboard_layout_Key_t key = layer->normal_keys[row][i];
					if (key.width > 0) {
						ui_push_idi((i * row) + 1000);
						if (key.special_key == special_key_spacer) {
							ui_label_sz(" ", { buttonSize * key.width,buttonSize });
						}
						else if (key.special_key == special_key_alt) {
							if (ui_toggle_sz_16(key.display_text, keyboard_alt, {buttonSize * key.width,buttonSize})) {
								if (keyboard_alt) {
									input_keyboard_inject_press(key_alt);
								} else {
									input_keyboard_inject_release(key_alt);
								}
							}
						}
						else if (key.special_key == special_key_ctrl) {
							if (ui_toggle_sz_16(key.display_text, keyboard_ctrl, { buttonSize * key.width,buttonSize })) {
								if (keyboard_ctrl) {
									input_keyboard_inject_press(key_ctrl);
								} else {
									input_keyboard_inject_release(key_ctrl);
								}
							}
						}
						else if (key.special_key == special_key_shift) {
							if (ui_toggle_sz_16(key.display_text, keyboard_shift, { buttonSize * key.width,buttonSize })) {
								if (keyboard_shift) {
									input_keyboard_inject_press(key_shift);
								} else {
									input_keyboard_inject_release(key_shift);
								}
							}
						}
						else if (key.special_key == special_key_fn) {
							ui_toggle_sz_16(key.display_text, keyboard_fn, { buttonSize * key.width,buttonSize });
						} 
						else if (key.special_key == special_key_alt_gr) {
							if (ui_toggle_sz_16(key.display_text, keyboard_altgr, { buttonSize * key.width,buttonSize })) {
								if (keyboard_altgr) {
									input_keyboard_inject_press(key_alt);
								} else {
									input_keyboard_inject_release(key_alt);
								}
							}
						}
						else if (ui_button_sz_16(key.display_text, { buttonSize * key.width,buttonSize })) {
							virtualkeyboard_keypress(key);
						}
						ui_pop_id();
					}
					ui_sameline();
				}
				ui_nextline();
			}
			ui_pop_no_keyboard_loss();
			ui_window_end();
			hierarchy_pop();
		}
	}
}
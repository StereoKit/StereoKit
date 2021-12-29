#include "VirtualKeyboard.h"
#include "stereokit_ui.h"
#include "systems/input_keyboard.h"
#include "libraries/array.h"

namespace sk {
	const keyboard_layout_t virtualkeyboard_get_uskeyboard_layout =
		{
			////////////       text input        //////////////
			{
				//Normal
				{
					{
						{
							{ "`","`",1,key_backtick},{"1","1",1,key_1}, {"2","2",1,key_2}, {"3","3",1,key_3}, {"4","4",1,key_4}, {"5","5",1,key_5}, {"6","6",1,key_6}, {"7","7",1,key_7}, {"8","8",1,key_8}, {"9","9",1,key_9}, {"0","0",1,key_0}, {"-","-",1,key_minus}, {"=","=",1,key_equals},{"\b","<--",1.5f,key_backspace}
						},
						{
							{"\t","Tab",1.5,key_tab},{"q","q",1,key_q},{"w","w",1,key_w},{"e","e",1,key_e},{"r","r",1,key_r},{"t","t",1,key_t},{"y","y",1,key_y},{"u","u",1,key_u},{"i","i",1,key_i},{"o","o",1,key_o},{"p","p",1,key_p},{"[","[",1,key_bracket_open},{"]","]",1,key_bracket_close},{"\\","\\",1,key_slash_back}
						},
						{
							{"","CapsLk",1.9,key_caps_lock,special_key_shift},{"a","a",1,key_a},{"s","s",1,key_s},{"d","d",1,key_d},{"f","f",1,key_f},{"g","g",1,key_g},{"h","h",1,key_h},{"j","j",1,key_j},{"k","k",1,key_k},{"l","l",1,key_l},{";",";",1,key_semicolon},{"'","'",1,key_apostrophe},{"\r","Enter",1.9,key_return}
						},
						{
							{"","Shift",2.5,key_shift,special_key_shift},{"z","z",1,key_z},{"x","x",1,key_x},{"c","c",1,key_c},{"v","v",1,key_v},{"b","b",1,key_b},{"n","n",1,key_n},{"m","m",1,key_m},{",",",",1,key_comma},{".",".",1,key_period},{"/","/",1,key_slash_fwd},{"","Shift",2.5,key_shift,special_key_shift},{"","",1,key_none,special_key_spacer},{"","^",1,key_up}
						},
						{
							{"","Ctrl",1.75,key_ctrl},{"","Cmd",1,key_rcmd},{"","Alt",1.75,key_alt},{" ","     ",6.4,key_space},{"","Alt",1.75,key_alt},{"","Ctrl",1.75,key_ctrl},{"","",2,key_none,special_key_spacer},{"","<",1,key_left},{"","v",1,key_down},{""," >",1,key_right}
						}
					}
				},
				//shift
				{
					{
						{
							{ "~","~",1,key_backtick},{"!","!",1,key_1}, {"@","@",1,key_2}, {"#","#",1,key_3}, {"$","$",1,key_4}, {"%","%",1,key_5}, {"^","^",1,key_6}, {"&","&",1,key_7}, {"*","*",1,key_8}, {"(","(",1,key_9}, {")",")",1,key_0}, {"_","_",1,key_minus}, {"+","+",1,key_equals},{"\b","<--",1.5f,key_backspace}
						},
						{
							{"\t","Tab",1.5,key_tab},{"Q","Q",1,key_q},{"W","W",1,key_w},{"E","E",1,key_e},{"R","R",1,key_r},{"T","T",1,key_t},{"Y","Y",1,key_y},{"U","U",1,key_u},{"I","I",1,key_i},{"O","O",1,key_o},{"P","P",1,key_p},{"{","{",1,key_bracket_open},{"}","}",1,key_bracket_close},{"|","|",1,key_slash_back}
						},
						{
							{"","CapsLk",1.9,key_caps_lock,special_key_shift},{"A","A",1,key_a},{"S","S",1,key_s},{"D","D",1,key_d},{"F","F",1,key_f},{"G","G",1,key_g},{"H","H",1,key_h},{"J","J",1,key_j},{"K","K",1,key_k},{"L","L",1,key_l},{":",":",1,key_semicolon},{"\"","\"",1,key_apostrophe},{"\n","NewLine",1.9,key_return}
						},
						{
							{"","Shift",2.5,key_shift,special_key_shift},{"Z","Z",1,key_z},{"X","X",1,key_x},{"C","C",1,key_c},{"V","V",1,key_v},{"B","B",1,key_b},{"N","N",1,key_n},{"M","M",1,key_m},{"<","<",1,key_comma},{">",">",1,key_period},{"?","?",1,key_slash_fwd},{"","Shift",2.5,key_shift,special_key_shift},{"","",1,key_none,special_key_spacer},{"","^",1,key_up}
						},
						{
							{"","Ctrl",1.75,key_ctrl},{"","Cmd",1,key_rcmd},{"","Alt",1.75,key_alt},{" ","     ",6.4,key_space},{"","Alt",1.75,key_alt},{"","Ctrl",1.75,key_ctrl},{"","",2,key_none,special_key_spacer},{"","<",1,key_left},{"","v",1,key_down},{""," >",1,key_right}
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
							{"\r","Enter",1.6,key_return},{"\b","<--",1.6f,key_backspace}
						},
						{
							{"7","7",1,key_7},{"8","8",1,key_8},{"9","9",1,key_9}
						},
						{
							{"4","4",1,key_4},{"5","5",1,key_5},{"6","6",1,key_6}
						},
						{
							{"1","1",1,key_1},{"2","2",1,key_2},{"3","3",1,key_3}
						},
						{
							{"0","0",3.6,key_0}
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
							{"\r","Enter",1.6,key_return},{"\b","<--",1.6f,key_backspace}
						},
						{
							{"7","7",1,key_7},{"8","8",1,key_8},{"9","9",1,key_9}
						},
						{
							{"4","4",1,key_4},{"5","5",1,key_5},{"6","6",1,key_6}
						},
						{
							{"1","1",1,key_1},{"2","2",1,key_2},{"3","3",1,key_3}
						},
						{
							{"0","0",1.6,key_0},{".",".",1.6,key_period}
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
							{"\r","Enter",1.6,key_return},{"\b","<--",1.6f,key_backspace}
						},
						{
							{"7","7",1,key_7},{"8","8",1,key_8},{"9","9",1,key_9}
						},
						{
							{"4","4",1,key_4},{"5","5",1,key_5},{"6","6",1,key_6}
						},
						{
							{"1","1",1,key_1},{"2","2",1,key_2},{"3","3",1,key_3}
						},
						{
							{"0","0",1.6,key_0},{"-","-",1.6,key_minus}
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
							{"\r","Enter",1.6,key_return},{"\b","<--",1.6f,key_backspace}
						},
						{
							{"7","7",1,key_7},{"8","8",1,key_8},{"9","9",1,key_9}
						},
						{
							{"4","4",1,key_4},{"5","5",1,key_5},{"6","6",1,key_6}
						},
						{
							{"1","1",1,key_1},{"2","2",1,key_2},{"3","3",1,key_3}
						},
						{
							{"0","0",1,key_0},{".",".",1,key_period},{"-","-",1,key_minus}
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

	bool keyboardFN = false;
	bool keyboardAltgr = false;
	bool keyboardShift = false;
	bool keyboardOpen = false;
	pose_t keyboardPose = { {-0.1f, 0.1f, -0.2f},quat_lookat({0,0,0},{1,1,1})};
	array_t<key_> pressedKeys;
	const keyboard_layout_t* CurrentKeyboardLayout;
	keyboard_input_type_ Inputtype;
	const keyboard_layout_t* virtualkeyboard_get_system_keyboard_layout()
	{
		return &virtualkeyboard_get_uskeyboard_layout;
	}

	void virtualkeyboard_open(bool open, keyboard_input_type_ type)
	{
		Inputtype = type;
		if (open != keyboardOpen) {
			if (open) {
				keyboardShift = false;
			}
			keyboardOpen = open;
		}
	}

	void virtualkeyboard_initialize()
	{
		pressedKeys.free();
		CurrentKeyboardLayout = virtualkeyboard_get_system_keyboard_layout();
	}

	void SendKeyData(const char* charkey,key_ key) {
		pressedKeys.add(key);
		input_keyboard_inject_press(key);
		if (charkey != nullptr) {
			int index = 0;
			while (charkey[index] != '\0')
			{
				input_text_inject_char((char32_t)charkey[index]);
				index++;
			}
		}
	}

	void RemoveLastClickedKeys() {
		for (int i = 0; i < pressedKeys.count; i++)
		{
			input_keyboard_inject_release(pressedKeys[0]);
			pressedKeys.remove(0);
		}
	}

	void virtualkeyboard_keypress(keyboard_layout_Key_t key) {
		SendKeyData(key.clickedText, key.keyEventType);
		if (key.specialKey == special_key_shift) {
			keyboardShift = !keyboardShift;
		}
		if (key.specialKey == special_key_alt_gr) {
			keyboardAltgr = !keyboardAltgr;
		}
		if (key.specialKey == special_key_fn) {
			keyboardFN = !keyboardFN;
		}
		if (key.specialKey == special_key_close_keyboard) {
			keyboardOpen = false;
		}
	}

	void virtualkeyboard_update()
	{
		if (keyboardOpen) {
			if (CurrentKeyboardLayout == nullptr) {
				return;
			}
			RemoveLastClickedKeys();
			hierarchy_push(render_get_cam_root());
			ui_window_begin("Keyboard", keyboardPose, {0,0},ui_win_body);
			ui_push_no_keyboard_loss(true);
			int typeIndex = 0;
			if (keyboardShift) {
				if (keyboardAltgr) {
					typeIndex = 3;
				}
				else {
					typeIndex = 1;
				}
			}
			else {
				if (keyboardAltgr) {
					typeIndex = 2;
				}
			}
			if (keyboardFN) {
				typeIndex = 3;
			}
			const keyboard_layout_Layer_t* layer = &CurrentKeyboardLayout->text_layer[typeIndex];
			switch (Inputtype)
			{
			case sk::number:
				layer = &(CurrentKeyboardLayout->number_layer[typeIndex]);
				break;
			case sk::Decimal:
				layer = &(CurrentKeyboardLayout->number_decimal_layer[typeIndex]);
				break;
			case sk::Signed:
				layer = &CurrentKeyboardLayout->number_signed_layer[typeIndex];
				break;
			case sk::Number_Decimal:
				layer = &CurrentKeyboardLayout->number_decimal_layer[typeIndex];
				break;
			case sk::Number_Signed:
				layer = &CurrentKeyboardLayout->number_signed_layer[typeIndex];
				break;
			case sk::Number_Signed_Decimal:
				layer = &CurrentKeyboardLayout->number_signed_decimal_layer[typeIndex];
				break;
			}
			float buttonSize = 0.035f;
			for (int row = 0; row < 6; row++)
			{
				for (int i = 0; i < 35; i++)
				{
					keyboard_layout_Key_t key = layer->normalKeys[row][i];
					if (key.width > 0) {
						ui_push_idi((i * row) + 1000);
						if (key.specialKey == special_key_spacer) {
							ui_label_sz(" ", { buttonSize * key.width,buttonSize });
						} else if (ui_button_sz(key.displayText, { buttonSize * key.width,buttonSize })) {
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
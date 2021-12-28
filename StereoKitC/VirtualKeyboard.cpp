#include "VirtualKeyboard.h"
#include "stereokit_ui.h"
#include "systems/input_keyboard.h"
#include "libraries/array.h"

namespace sk {
	bool keyboardShift = false;
	bool keyboardOpen = false;
	pose_t keyboardPose = { {-0.1f, 0.1f, -0.2f},quat_lookat({0,0,0},{1,1,1})};
	array_t<key_> pressedKeys;
	const char* Spacer = "Spacer";

	keyboard_layout_t CurrentKeyboardLayout;

	keyboard_layout_t virtualkeyboard_get_usKeyboard_Layout()
	{
		return
		{
			{
				{
					{"`",1},{"1",1}, {"2",1}, {"3",1}, {"4",1}, {"5",1}, {"6",1}, {"7",1}, {"8",1}, {"9",1}, {"0",1}, {"-",1}, {"=",1},{"<--",1.5f},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},
					{"Tab",1.5},{"q",1},{"w",1},{"e",1},{"r",1},{"t",1},{"y",1},{"u",1},{"i",1},{"o",1},{"p",1},{"[",1},{"]",1},{"\\",1},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},
					{"CapsLk",1.9},{"a",1},{"s",1},{"d",1},{"f",1},{"g",1},{"h",1},{"j",1},{"k",1},{"l",1},{";",1},{"'",1},{"Enter",1.9},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},
					{"LShift",2.5},{"z",1},{"x",1},{"c",1},{"v",1},{"b",1},{"n",1},{"m",1},{",",1},{".",1},{"/",1},{"RShift",2.5},{"Spacer",1},{"/\\",1},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},
					{"LCtrl",1.75},{"Sys",1},{"LAlt",1.75},{"     ",6.4},{"RAlt",1.75},{"RCtrl",1.75},{"Spacer",2},{"< ",1},{"\\/",1},{" >",1}
				}
			},
			{
				{
					{"~",1},{"!",1}, {"@",1}, {"#",1}, {"$",1}, {"%",1}, {"^",1}, {"&",1}, {"*",1}, {"(",1}, {")",1}, {"_",1}, {"+",1},{"<--",1.5f},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},
					{"Tab",1.5},{"Q",1},{"W",1},{"E",1},{"R",1},{"T",1},{"Y",1},{"U",1},{"I",1},{"O",1},{"P",1},{"{",1},{"}",1},{"|",1},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},
					{"CapsLk",1.9},{"A",1},{"S",1},{"D",1},{"F",1},{"G",1},{"H",1},{"J",1},{"K",1},{"L",1},{":",1},{"\"",1},{"NewLine",1.9},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},
					{"LShift",2.5},{"Z",1},{"X",1},{"C",1},{"V",1},{"B",1},{"N",1},{"M",1},{"<",1},{">",1},{"?",1},{"RShift",2.5},{"Spacer",1},{"/\\",1},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},
					{"LCtrl",1.75},{"Sys",1},{"LAlt",1.75},{"     ",6.4},{"RAlt",1.75},{"RCtrl",1.75},{"Spacer",2},{"< ",1},{"\\/",1},{" >",1}
				}
			}
		};
	}

	keyboard_layout_t virtualkeyboard_getKeyboard_Layout()
	{
		return CurrentKeyboardLayout;
	}

	void virtualkeyboard_setKeyboard_Layout(keyboard_layout_t layout)
	{
		CurrentKeyboardLayout = layout;
	}

	bool virtualkeyboard_getopen()
	{
		return keyboardOpen;
	}

	void virtualkeyboard_setopen(bool open)
	{
		if (open != keyboardOpen) {
			if (open) {
				keyboardShift = false;
			}
			keyboardOpen = open;
		}
	}

	bool virtualkeyboard_getshift()
	{
		return keyboardShift;
	}

	void virtualkeyboard_setshift(bool shift)
	{
		keyboardShift = shift;
	}

	void virtualkeyboard_initialize()
	{
		pressedKeys.free();
		CurrentKeyboardLayout = virtualkeyboard_get_usKeyboard_Layout();
	}
	//Is a partial Match
	bool CheckKey(const char* key, const char* check) {
		int charindex = 0;
		bool returnFalse = false;
		bool returnTrue = false;
		if (key[1] == '\0') {
			return false;
		}
		while (!returnTrue && !returnFalse)
		{
			if (check[charindex] == '\0') {
				if (check[charindex] == key[charindex])
				{
					returnTrue = true;
				}
				else {
					returnFalse = true;
				}
			}
			if (key[charindex] == '\0') {
				if (key[charindex] == check[charindex])
				{
					returnTrue = true;
				}
				else {
					returnFalse = true;
				}
			}
			if (key[charindex] != check[charindex]) {
				returnFalse = true;
			}
			charindex++;
		}
		return returnTrue;
	}

	void SendKeyData(const char* charkey,key_ key) {
		pressedKeys.add(key);
		input_keyboard_inject_press(key);
		if (charkey != nullptr) {
			input_text_inject_char((char32_t)(*charkey));
		}
	}

	key_ GetKeyFromString(const char* key) {
		if (key == nullptr) {
			return key_::key_none;
		}
		char firstChar = *key;
		if (firstChar <= 'z' && firstChar > 'a') {
			return (key_)((firstChar - 'a') + key_::key_a);
		}
		if (firstChar <= 'Z' && firstChar > 'A') {
			return (key_)((firstChar - 'A') + key_::key_a);
		}
		if (firstChar <= '9' && firstChar > '0') {
			return (key_)((firstChar - '0') + key_::key_0);
		}
		if (firstChar == '`') {
			return key_::key_backtick;
		}
		if (firstChar == '!') {
			//Could not find exclamation mark
			return key_::key_none;
		}
		if (firstChar == '@') {
			//Could not find at key
			return key_::key_none;
		}
		if (firstChar == '#') {
			//Could not find hashTag key
			return key_::key_none;
		}
		if (firstChar == '$') {
			//Could not find doller key
			return key_::key_none;
		}
		if (firstChar == '%') {
			//Could not find persent key
			return key_::key_none;
		}
		if (firstChar == '^') {
			//Could not find caret key
			return key_::key_none;
		}
		if (firstChar == '&') {
			//Could not find and key
			return key_::key_none;
		}
		if (firstChar == '*') {
			return key_::key_multiply;
		}
		if (firstChar == '(') {
			//Could not find
			return key_::key_none;
		}
		if (firstChar == ')') {
			//Could not find
			return key_::key_none;
		}
		if (firstChar == '-') {
			return key_::key_subtract;
		}
		if (firstChar == '=') {
			return key_::key_equals;
		}
		if (firstChar == '_') {
			//Could not find
			return key_::key_none;
		}
		if (firstChar == '+') {
			return key_::key_add;
		}
		if (firstChar == '[') {
			return key_::key_bracket_open;
		}
		if (firstChar == '{') {
			//Could not find
			return key_::key_none;
		}
		if (firstChar == ']') {
			return key_::key_bracket_close;
		}
		if (firstChar == '}') {
			//Could not find
			return key_::key_none;
		}
		if (firstChar == '\\') {
			return key_::key_slash_back;
		}
		if (firstChar == '|') {
			//Could not find
			return key_::key_none;
		}
		if (firstChar == ';') {
			return key_::key_semicolon;
		}
		if (firstChar == ':') {
			//Could not find
			return key_::key_none;
		}
		if (firstChar == '"') {
			//Could not find
			return key_::key_none;
		}
		if (firstChar == '\'') {
			return key_::key_apostrophe;
		}
		if (firstChar == ',') {
			return key_::key_comma;
		}
		if (firstChar == '<') {
			//Could not find
			return key_::key_none;
		}
		if (firstChar == '>') {
			//Could not find
			return key_::key_none;
		}
		if (firstChar == '.') {
			return key_::key_period;
		}
		if (firstChar == '/') {
			return key_::key_slash_fwd;
		}
		if (firstChar == '?') {
			//Could not find
			return key_::key_none;
		}
		return key_::key_none;
	}

	void virtualkeyboard_keyPress(const char* key) {
		//Check If BackSpace Key
		if (CheckKey(key, "<--")) {
			SendKeyData("\b", key_::key_backspace);
		}
		//Check If Tab Key
		else if(CheckKey(key, "Tab")) {
			SendKeyData("\t", key_::key_tab);
		}
		//Check If Caps Lock Key
		else if (CheckKey(key, "CapsLk")) {
			SendKeyData(NULL, key_::key_caps_lock);
		}
		//Check If Enter Key
		else if (CheckKey(key, "Enter")) {
			SendKeyData("\r", key_::key_return);
		}
		//Check If NewLine Key
		else if (CheckKey(key, "NewLine")) {
			SendKeyData("\n", key_::key_return);
		}
		//Check If Left Shift Key
		else if (CheckKey(key, "LShift")) {
			keyboardShift = !keyboardShift;
			SendKeyData(NULL, key_::key_shift);
		}
		//Check If Right Shift Key
		else if (CheckKey(key, "RShift")) {
			keyboardShift = !keyboardShift;
			SendKeyData(NULL, key_::key_shift);
		}
		//Check If Up Key
		else if (CheckKey(key, "/\\")) {
			SendKeyData(NULL, key_::key_up);
		}
		//Check If Left Key
		else if (CheckKey(key, "< ")) {
			SendKeyData(NULL, key_::key_left);
		}
		//Check If Right Key
		else if (CheckKey(key, " >")) {
			SendKeyData(NULL, key_::key_right);
		}
		//Check If Left Ctrl Key
		else if (CheckKey(key, " LCtrl")) {
			SendKeyData(NULL, key_::key_ctrl);
		}
		//Check If Right Ctrl Key
		else if (CheckKey(key, " RCtrl")) {
			SendKeyData(NULL, key_::key_ctrl);
		}
		//Check If Left Alt Key
		else if (CheckKey(key, " LAlt")) {
			SendKeyData(NULL, key_::key_alt);
		}
		//Check If Right Alt Key
		else if (CheckKey(key, " RAlt")) {
			SendKeyData(NULL, key_::key_alt);
		}
		//Check If Space Bar Key
		else if (CheckKey(key, "     ")) {
			SendKeyData(" ", key_::key_space);
		}
		else {
			SendKeyData(key, GetKeyFromString(key));
		}
	}

	void RemoveLastClickedKeys() {
		for (int i = 0; i < pressedKeys.count; i++)
		{
			input_keyboard_inject_release(pressedKeys[0]);
			pressedKeys.remove(0);
		}
	}


	void virtualkeyboard_update()
	{
		if (keyboardOpen) {
			RemoveLastClickedKeys();
			ui_window_begin("Keyboard", keyboardPose, {0,0},ui_win_body);
			ui_push_no_keyboard_loss(true);
			keyboard_layout_Layer_t layer = (keyboardShift) ? CurrentKeyboardLayout.shiftLayer : CurrentKeyboardLayout.normalLayer;
			float buttonSize = 0.035f;
			for (int row = 0; row < 6; row++)
			{
				for (int i = 0; i < 35; i++)
				{
					const char* character = layer.key[(row*35)+i].key;
					float width = layer.key[(row * 35) + i].width;
					if (width > 0) {
						//Only care about the last 2 Characters and first
						if ((character[0] == Spacer[0])&&(character[6] == Spacer[6]) && (character[5] == Spacer[5])) {
							ui_label_sz(" ", { buttonSize * width,buttonSize });
						} else if (ui_button_sz(character, { buttonSize * width,buttonSize })) {
							virtualkeyboard_keyPress(character);
						}
					}
					ui_sameline();
				}
				ui_nextline();
			}
			ui_pop_no_keyboard_loss();
			ui_window_end();
		}
	}
}
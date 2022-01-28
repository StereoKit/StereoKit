#pragma once

#include "../stereokit.h"

namespace sk {

typedef enum special_key_ {
	special_key_none,
	special_key_shift,
	special_key_alt_gr,
	special_key_fn,
	special_key_close_keyboard,
	special_key_alt,
	special_key_ctrl,
	special_key_spacer,
	special_key_nextline,
	special_key_end,
} special_key_;

typedef struct keylayout_key_t {
	const char16_t* clicked_text;
	const char16_t* display_text;
	float           width;
	key_            key_event_type;
	special_key_    special_key;
} keylayout_key_t;

typedef struct keylayout_t {
	// Layers:
	// 0. Normal key layout
	// 1. Shift
	// 2. AltGr
	// 3. AltGr + Shift
	// 4. fnkey
	const keylayout_key_t *text  [2];
	const keylayout_key_t *uri   [2];
	const keylayout_key_t *number[2];
	//keylayout_t decimal       [5];
	//keylayout_t signed        [5];
	//keylayout_t signed_decimal[5];
} keylayout_t;

const keylayout_key_t en_text_normal[] = {
	{u"`",  u"`",1,key_backtick},{u"1", u"1",1,key_1}, {u"2", u"2",1,key_2}, {u"3", u"3",1,key_3}, {u"4", u"4",1,key_4}, {u"5", u"5",1,key_5}, {u"6", u"6",1,key_6}, {u"7", u"7",1,key_7}, {u"8", u"8",1,key_8}, {u"9", u"9",1,key_9}, {u"0", u"0",1,key_0}, {u"-", u"-",1,key_minus}, {u"=", u"=",1,key_equals},{u"\b", u"<--",1.5f,key_backspace},{u"", u"",2,key_none,special_key_spacer},{u"", u"X",1,key_none,special_key_close_keyboard}, {u"", u"",0,key_none,special_key_nextline},
	{u"\t", u"Tab",1.5f,key_tab},{u"q", u"q",1,key_q},{u"w", u"w",1,key_w},{u"e", u"e",1,key_e},{u"r", u"r",1,key_r},{u"t", u"t",1,key_t},{u"y", u"y",1,key_y},{u"u", u"u",1,key_u},{u"i", u"i",1,key_i},{u"o", u"o",1,key_o},{u"p", u"p",1,key_p},{u"[", u"[",1,key_bracket_open},{u"]", u"]",1,key_bracket_close},{u"\\", u"\\",1,key_slash_back},{u"", u"",0,key_none,special_key_nextline},
	{u"", u"CapsLk",1.9f,key_caps_lock,special_key_shift},{u"a", u"a",1,key_a},{u"s", u"s",1,key_s},{u"d", u"d",1,key_d},{u"f", u"f",1,key_f},{u"g", u"g",1,key_g},{u"h", u"h",1,key_h},{u"j", u"j",1,key_j},{u"k", u"k",1,key_k},{u"l", u"l",1,key_l},{u";", u";",1,key_semicolon},{u"'", u"'",1,key_apostrophe},{u"\r", u"Enter",1.9f,key_return},{u"", u"",0,key_none,special_key_nextline},
	{u"", u"Shift",2.5f,key_shift,special_key_shift},{u"z", u"z",1,key_z},{u"x", u"x",1,key_x},{u"c", u"c",1,key_c},{u"v", u"v",1,key_v},{u"b", u"b",1,key_b},{u"n", u"n",1,key_n},{u"m", u"m",1,key_m},{u",", u",",1,key_comma},{u".", u".",1,key_period},{u"/", u"/",1,key_slash_fwd},{u"", u"Shift",2.5f,key_shift,special_key_shift},{u"", u"",1,key_none,special_key_spacer},{u"", u"^",1,key_up},{u"", u"",0,key_none,special_key_nextline},
	{u"", u"Ctrl",1.75f,key_ctrl,special_key_ctrl},{u"", u"Cmd",1,key_rcmd},{u"", u"Alt",1.75f,key_alt,special_key_alt},{u" ", u"     ",6.4f,key_space},{u"", u"Alt",1.75f,key_alt,special_key_alt},{u"", u"Ctrl",1.75f,key_ctrl,special_key_ctrl},{u"", u"",2,key_none,special_key_spacer},{u"", u"<",1,key_left},{u"", u"v",1,key_down},{u"", u" >",1,key_right},{u"", u"",0,key_none,special_key_end}
};
const keylayout_key_t en_text_shift[] = {
	{u"~", u"~",1,key_backtick},{u"!", u"!",1,key_1}, {u"@", u"@",1,key_2}, {u"#", u"#",1,key_3}, {u"$", u"$",1,key_4}, {u"%", u"%",1,key_5}, {u"^", u"^",1,key_6}, {u"&", u"&",1,key_7}, {u"*", u"*",1,key_8}, {u"(", u"(",1,key_9}, {u")", u")",1,key_0}, {u"_", u"_",1,key_minus}, {u"+", u"+",1,key_equals},{u"\b", u"<--",1.5f,key_backspace},{u"", u"",2,key_none,special_key_spacer},{u"", u"X",1,key_none,special_key_close_keyboard},{u"", u"",0,key_none,special_key_nextline},
	{u"\t", u"Tab",1.5f,key_tab},{u"Q", u"Q",1,key_q},{u"W", u"W",1,key_w},{u"E", u"E",1,key_e},{u"R", u"R",1,key_r},{u"T", u"T",1,key_t},{u"Y", u"Y",1,key_y},{u"U", u"U",1,key_u},{u"I", u"I",1,key_i},{u"O", u"O",1,key_o},{u"P", u"P",1,key_p},{u"{", u"{",1,key_bracket_open},{u"}", u"}",1,key_bracket_close},{u"|", u"|",1,key_slash_back},{u"", u"",0,key_none,special_key_nextline},
	{u"", u"CapsLk",1.9f,key_caps_lock,special_key_shift},{u"A", u"A",1,key_a},{u"S", u"S",1,key_s},{u"D", u"D",1,key_d},{u"F", u"F",1,key_f},{u"G", u"G",1,key_g},{u"H", u"H",1,key_h},{u"J", u"J",1,key_j},{u"K", u"K",1,key_k},{u"L", u"L",1,key_l},{u":", u":",1,key_semicolon},{u"\"", u"\"",1,key_apostrophe},{u"\n", u"NewLine",1.9f,key_return},{u"", u"",0,key_none,special_key_nextline},
	{u"", u"Shift",2.5f,key_shift,special_key_shift},{u"Z", u"Z",1,key_z},{u"X", u"X",1,key_x},{u"C", u"C",1,key_c},{u"V", u"V",1,key_v},{u"B", u"B",1,key_b},{u"N", u"N",1,key_n},{u"M", u"M",1,key_m},{u"<", u"<",1,key_comma},{u">", u">",1,key_period},{u"?", u"?",1,key_slash_fwd},{u"", u"Shift",2.5f,key_shift,special_key_shift},{u"", u"",1,key_none,special_key_spacer},{u"", u"^",1,key_up},{u"",u"",0,key_none,special_key_nextline},
	{u"", u"Ctrl",1.75f,key_ctrl,special_key_ctrl},{u"", u"Cmd",1,key_rcmd},{u"", u"Alt",1.75f,key_alt,special_key_alt},{u" ", u"     ",6.4f,key_space},{u"", u"Alt",1.75f,key_alt,special_key_alt},{u"", u"Ctrl",1.75f,key_ctrl,special_key_ctrl},{u"", u"",2,key_none,special_key_spacer},{u"", u"<",1,key_left},{u"", u"v",1,key_down},{u"", u" >",1,key_right},{u"",u"",0,key_none,special_key_end}
};
const keylayout_key_t en_uri_normal[] = {
	{u"`", u"`",1,key_backtick},{u"1", u"1",1,key_1}, {u"2", u"2",1,key_2}, {u"3", u"3",1,key_3}, {u"4", u"4",1,key_4}, {u"5", u"5",1,key_5}, {u"6", u"6",1,key_6}, {u"7", u"7",1,key_7}, {u"8", u"8",1,key_8}, {u"9", u"9",1,key_9}, {u"0", u"0",1,key_0}, {u"-", u"-",1,key_minus}, {u"=", u"=",1,key_equals},{u"\b", u"<--",1.5f,key_backspace},{u"", u"",2,key_none,special_key_spacer},{u"", u"X",1,key_none,special_key_close_keyboard},{u"",u"",0,key_none,special_key_nextline},
	{u"\t", u"Tab",1.5f,key_tab},{u"q", u"q",1,key_q},{u"w", u"w",1,key_w},{u"e", u"e",1,key_e},{u"r", u"r",1,key_r},{u"t", u"t",1,key_t},{u"y", u"y",1,key_y},{u"u", u"u",1,key_u},{u"i", u"i",1,key_i},{u"o", u"o",1,key_o},{u"p", u"p",1,key_p},{u"[", u"[",1,key_bracket_open},{u"]", u"]",1,key_bracket_close},{u"\\", u"\\",1,key_slash_back},{u".com", u".com",3.25f},{u"",u"",0,key_none,special_key_nextline},
	{u"", u"CapsLk",1.9f,key_caps_lock,special_key_shift},{u"a", u"a",1,key_a},{u"s", u"s",1,key_s},{u"d", u"d",1,key_d},{u"f", u"f",1,key_f},{u"g", u"g",1,key_g},{u"h", u"h",1,key_h},{u"j", u"j",1,key_j},{u"k", u"k",1,key_k},{u"l", u"l",1,key_l},{u";", u";",1,key_semicolon},{u"'", u"'",1,key_apostrophe},{u"\r", u"Enter",1.9f,key_return},{u"https://", u"https://",3.25f},{u"",u"",0,key_none,special_key_nextline},
	{u"", u"Shift",2.5f,key_shift,special_key_shift},{u"z", u"z",1,key_z},{u"x", u"x",1,key_x},{u"c", u"c",1,key_c},{u"v", u"v",1,key_v},{u"b", u"b",1,key_b},{u"n", u"n",1,key_n},{u"m", u"m",1,key_m},{u",", u",",1,key_comma},{u".", u".",1,key_period},{u"/", u"/",1,key_slash_fwd},{u"", u"Shift",2.5f,key_shift,special_key_shift},{u"", u"",1,key_none,special_key_spacer},{u"", u"^",1,key_up},{u"",u"",0,key_none,special_key_nextline},
	{u"", u"Ctrl",1.75f,key_ctrl,special_key_ctrl},{u"", u"Cmd",1,key_rcmd},{u"", u"Alt",1.75f,key_alt,special_key_alt},{u" ", u"     ",6.4f,key_space},{u"", u"Alt",1.75f,key_alt,special_key_alt},{u"", u"Ctrl",1.75f,key_ctrl,special_key_ctrl},{u"", u"",2,key_none,special_key_spacer},{u"", u"<",1,key_left},{u"", u"v",1,key_down},{u"", u" >",1,key_right},{u"",u"",0,key_none,special_key_end}
};
const keylayout_key_t en_uri_shift[] = {
	{u"~", u"~",1,key_backtick},{u"!", u"!",1,key_1}, {u"@", u"@",1,key_2}, {u"#", u"#",1,key_3}, {u"$", u"$",1,key_4}, {u"%", u"%",1,key_5}, {u"^", u"^",1,key_6}, {u"&", u"&",1,key_7}, {u"*", u"*",1,key_8}, {u"(", u"(",1,key_9}, {u")", u")",1,key_0}, {u"_", u"_",1,key_minus}, {u"+", u"+",1,key_equals},{u"\b", u"<--",1.5f,key_backspace},{u"", u"",2,key_none,special_key_spacer},{u"", u"X",1,key_none,special_key_close_keyboard},{u"",u"",0,key_none,special_key_nextline},
	{u"\t", u"Tab",1.5f,key_tab},{u"Q", u"Q",1,key_q},{u"W", u"W",1,key_w},{u"E", u"E",1,key_e},{u"R", u"R",1,key_r},{u"T", u"T",1,key_t},{u"Y", u"Y",1,key_y},{u"U", u"U",1,key_u},{u"I", u"I",1,key_i},{u"O", u"O",1,key_o},{u"P", u"P",1,key_p},{u"{", u"{",1,key_bracket_open},{u"}", u"}",1,key_bracket_close},{u"|", u"|",1,key_slash_back},{u".com", u".com",3.25f},{u"",u"",0,key_none,special_key_nextline},
	{u"", u"CapsLk",1.9f,key_caps_lock,special_key_shift},{u"A", u"A",1,key_a},{u"S", u"S",1,key_s},{u"D", u"D",1,key_d},{u"F", u"F",1,key_f},{u"G", u"G",1,key_g},{u"H", u"H",1,key_h},{u"J", u"J",1,key_j},{u"K", u"K",1,key_k},{u"L", u"L",1,key_l},{u":", u":",1,key_semicolon},{u"\"", u"\"",1,key_apostrophe},{u"\n", u"NewLine",1.9f,key_return},{u"https://", u"https://",3.25f},{u"",u"",0,key_none,special_key_nextline},
	{u"", u"Shift",2.5f,key_shift,special_key_shift},{u"Z", u"Z",1,key_z},{u"X", u"X",1,key_x},{u"C", u"C",1,key_c},{u"V", u"V",1,key_v},{u"B", u"B",1,key_b},{u"N", u"N",1,key_n},{u"M", u"M",1,key_m},{u"<", u"<",1,key_comma},{u">", u">",1,key_period},{u"?", u"?",1,key_slash_fwd},{u"", u"Shift",2.5f,key_shift,special_key_shift},{u"", u"",1,key_none,special_key_spacer},{u"", u"^",1,key_up},{u"",u"",0,key_none,special_key_nextline},
	{u"", u"Ctrl",1.75f,key_ctrl,special_key_ctrl},{u"", u"Cmd",1,key_rcmd},{u"", u"Alt",1.75f,key_alt,special_key_alt},{u" ", u"     ",6.4f,key_space},{u"", u"Alt",1.75f,key_alt,special_key_alt},{u"", u"Ctrl",1.75f,key_ctrl,special_key_ctrl},{u"", u"",2,key_none,special_key_spacer},{u"", u"<",1,key_left},{u"", u"v",1,key_down},{u"", u" >",1,key_right},{u"",u"",0,key_none,special_key_end}
};
const keylayout_key_t en_num_normal[] = {
	{u"\r", u"Enter",1.6f,key_return},{u"\b", u"<--",1.6f,key_backspace}, {u"",u"",0,key_none,special_key_nextline},
	{u"7", u"7",1,key_7},{u"8", u"8",1,key_8     },{u"9", u"9",1,key_9}, {u"",u"",0,key_none,special_key_nextline},
	{u"4", u"4",1,key_4},{u"5", u"5",1,key_5     },{u"6", u"6",1,key_6}, {u"",u"",0,key_none,special_key_nextline},
	{u"1", u"1",1,key_1},{u"2", u"2",1,key_2     },{u"3", u"3",1,key_3}, {u"",u"",0,key_none,special_key_nextline},
	{u"0", u"0",1,key_0},{u".", u".",1,key_period},{u"-", u"-",1,key_minus}, {u"",u"",0,key_none,special_key_end}
};

const keylayout_t virtualkeyboard_layout_en_us = {
	{ en_text_normal, en_text_shift },
	{ en_uri_normal,  en_uri_shift },
	{ en_num_normal }
};

}
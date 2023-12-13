/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../stereokit.h"

namespace sk {

typedef enum skey_ {
	skey_none,
	skey_close,
	skey_spacer,
	skey_nextline,
	skey_go,
} skey_;

typedef struct keylayout_key_t {
	const char*  clicked_text;
	union {
		const char*  display_text;
		sprite_t     display_sprite;
	};
	uint8_t      width;
	uint8_t      key_event_type;
	uint8_t      special_key;
	uint8_t      is_sprite;
} keylayout_key_t;

typedef struct keylayout_info_t {
	keylayout_key_t* keys;
	int32_t          key_ct;
	int32_t          width_cells;
	int32_t          width_gutters;
} keylayout_info_t;

typedef struct keylayout_t {
	// Layers:
	// 0. Normal key layout
	// 1. Shift
	// 2. Symbols
	keylayout_info_t text   [3];
	keylayout_info_t uri    [3];
	keylayout_info_t number [3];
} keylayout_t;

/*const keylayout_key_t en_text_normal[] = {
	{"q","q",    2,key_q},                {"w","w",  2,key_w},             {"e", "e",2,key_e},    {"r", "r",2,key_r},    {"t", "t",2,key_t},     {"y", "y",     2,key_y},     {"u", "u",2,key_u},{"i", "i",2,key_i},{"o", "o",  2,key_o},        {"p","p",2,key_p},{"", "",0,key_none,skey_nextline},
	{"", "",     1,key_none, skey_spacer},{"a","a",  2,key_a},             {"s", "s",2,key_s},    {"d", "d",2,key_d},    {"f", "f",2,key_f},     {"g", "g",     2,key_g},     {"h", "h",2,key_h},{"j", "j",2,key_j},{"k", "k",  2,key_k},        {"l","l",2,key_l},{"", "",0,key_none,skey_nextline},
	{"", "Shift",3,key_shift,skey_shift}, {"z","z",  2,key_z},             {"x", "x",2,key_x},    {"c", "c",2,key_c},    {"v", "v",2,key_v},     {"b", "b",     2,key_b},     {"n", "n",2,key_n},{"m", "m",2,key_m},{"\b", "<-",3,key_backspace},                  {"", "",0,key_none,skey_nextline},
	{"", "X",    2,key_none, skey_close}, {"", "123",3,key_none,skey_syms},{",", ",",2,key_comma},{" ", " ",7,key_space},{".", ".",2,key_period},{"\n","return",4,key_return},{"", "",0,key_none,skey_end}
};
const keylayout_key_t en_text_shift[] = {
	{"Q","Q",    2,key_q},               {"W","W",  2,key_w},             {"E", "E",2,key_e},{"R", "R",2,key_r},    {"T", "T",2,key_t},        {"Y", "Y",     2,key_y},     {"U", "U",2,key_u},{"I", "I",2,key_i},{"O", "O", 2,key_o},        {"P", "P",2,key_p},{"", "",0,key_none,skey_nextline},
	{"", "",     1,key_none,skey_spacer},{"A","A",  2,key_a},             {"S", "S",2,key_s},{"D", "D",2,key_d},    {"F", "F",2,key_f},        {"G", "G",     2,key_g},     {"H", "H",2,key_h},{"J", "J",2,key_j},{"K", "K", 2,key_k},        {"L", "L",2,key_l},{"", "",0,key_none,skey_nextline},
	{"", "Shift",3,key_shift,skey_shift},{"Z","Z",  2,key_z},             {"X", "X",2,key_x},{"C", "C",2,key_c},    {"V", "V",2,key_v},        {"B", "B",     2,key_b},     {"N", "N",2,key_n},{"M", "M",2,key_m},{"\b","<-",3,key_backspace},                   {"", "",0,key_none,skey_nextline},
	{"", "X",    2,key_none, skey_close},{"", "123",3,key_none,skey_syms},{"!", "!",2,key_1},{" ", " ",7,key_space},{"?", "?",2,key_slash_fwd},{"\n","return",4,key_return},{"",  "", 0,key_none,skey_end}
};
const keylayout_key_t en_text_symbols[] = {
	{"1","1",    2,key_1},               {"2","2",  2,key_2},             {"3", "3",2,key_3},        {"4", "4",2,key_4},        {"5", "5",2,key_5},        {"6", "6",     2,key_6},     {"7", "7",2,key_7},         {"8", "8", 2,key_8},         {"9", "9", 2,key_9},         {"0", "0", 2,key_0}, {"", "",0,key_none,skey_nextline},
	{"", "",     1,key_none,skey_spacer},{"-","-",  2,key_minus},         {"/", "/",2,key_slash_fwd},{":", ":",2,key_semicolon},{";", ";",2,key_semicolon},{"(", "(",     2,key_9},     {")", ")",2,key_0},         {"$", "$", 2,key_4},         {"&", "&", 2,key_7},         {"@", "@", 2,key_2}, {"", "",0,key_none,skey_nextline},
	{"", "Shift",3,key_shift,skey_shift},{"*","*",  2,key_8},             {"=", "=",2,key_equals},   {"+", "+",2,key_equals},   {"#", "#",2,key_3},        {"%", "%",     2,key_5},     {"'", "'",2,key_apostrophe},{"\"","\"",2,key_apostrophe},{"\b","<-",3,key_backspace},                      {"", "",0,key_none,skey_nextline},
	{"", "X",    2,key_none, skey_close},{"", "123",3,key_none,skey_syms},{"!", "!",2,key_1},        {" ", " ",7,key_space},    {"?", "?",2,key_slash_fwd},{"\n","return",4,key_return},{"",  "", 0,key_none,skey_end}
};
const keylayout_key_t en_uri_normal[] = {
	{"`", "`",     2,key_backtick},         {"1","1",  2,key_1},   {"2","2",  2,key_2},           {"3","3",2,key_3},    {"4","4",  2,key_4},           {"5","5",   2,key_5},             {"6","6",2,key_6},   {"7","7",2,key_7},   {"8","8",2,key_8},    {"9","9", 2,key_9},     {"0","0",2,key_0},        {"-","-",    2,key_minus},           {"=", "=",    2,key_equals},          {"\b",      "<-",      3,key_backspace}, {"",    "",    2,key_none,skey_spacer},{"","X",2,key_none,skey_close},{"","",0,key_none,skey_nextline},
	{"\t","Tab",   3,key_tab},              {"q","q",  2,key_q},   {"w","w",  2,key_w},           {"e","e",2,key_e},    {"r","r",  2,key_r},           {"t","t",   2,key_t},             {"y","y",2,key_y},   {"u","u",2,key_u},   {"i","i",2,key_i},    {"o","o", 2,key_o},     {"p","p",2,key_p},        {"[","[",    2,key_bracket_open},    {"]", "]",    2,key_bracket_close},   {"\\",      "\\",      2,key_slash_back},{".com",".com",4},                                                    {"","",0,key_none,skey_nextline},
	{"\r","Enter", 4,key_return},           {"a","a",  2,key_a},   {"s","s",  2,key_s},           {"d","d",2,key_d},    {"f","f",  2,key_f},           {"g","g",   2,key_g},             {"h","h",2,key_h},   {"j","j",2,key_j},   {"k","k",2,key_k},    {"l","l", 2,key_l},     {";",";",2,key_semicolon},{"'","'",    2,key_apostrophe},      {"\r","Enter",3,key_return},          {"https://","https://",4},                                                                                     {"","",0,key_none,skey_nextline},
	{"",  "Shift", 5,key_shift, skey_shift},{"z","z",  2,key_z},   {"x","x",  2,key_x},           {"c","c",2,key_c},    {"v","v",  2,key_v},           {"b","b",   2,key_b},             {"n","n",2,key_n},   {"m","m",2,key_m},   {",",",",2,key_comma},{".",".", 2,key_period},{"/","/",2,key_slash_fwd},{"", "Shift",4,key_shift,skey_shift},{"",  "^",    2,key_up},                                                                                                                             {"","",0,key_none,skey_nextline},
	{"",  "Ctrl",  4,key_ctrl,  skey_ctrl}, {"", "Cmd",3,key_rcmd},{"", "Alt",3,key_alt,skey_alt},{" "," ",9,key_space},{"", "Alt",4,key_alt,skey_alt},{"", "Ctrl",4,key_ctrl,skey_ctrl},{"", "<",2,key_left},{"", "v",2,key_down},{"", ">",2,key_right}, {"", "", 0,key_none,skey_end}
};
const keylayout_key_t en_uri_shift[] = {
	{"~", "~",     2,key_backtick},        {"!", "!", 2,key_1},   {"@", "@", 2,key_2},           {"#", "#",2,key_3},    {"$","$",  2,key_4},           {"%","%",   2,key_5},             {"^","^",2,key_6},   {"&","&",2,key_7},   {"*","*",2,key_8},    {"(","(",2,key_9},     {")", ")",2,key_0},        {"_", "_",    2,key_minus},           {"+", "+",      2,key_equals},       {"\b","<-",            3,key_backspace}, {"", "",2,key_none,skey_spacer},{"", "X",2,key_none,skey_close},{"","",0,key_none,skey_nextline},
	{"\t","Tab",   3,key_tab},             {"Q", "Q", 2,key_q},   {"W", "W", 2,key_w},           {"E", "E",2,key_e},    {"R","R",  2,key_r},           {"T","T",   2,key_t},             {"Y","Y",2,key_y},   {"U","U",2,key_u},   {"I","I",2,key_i},    {"O","O",2,key_o},     {"P", "P",2,key_p},        {"{", "{",    2,key_bracket_open},    {"}", "}",      2,key_bracket_close},{"|", "|",             2,key_slash_back},{".com", ".com",4},                                             {"","",0,key_none,skey_nextline},
	{"",  "Enter", 4,key_return},          {"A", "A", 2,key_a},   {"S", "S", 2,key_s},           {"D", "D",2,key_d},    {"F","F",  2,key_f},           {"G","G",   2,key_g},             {"H","H",2,key_h},   {"J","J",2,key_j},   {"K","K",2,key_k},    {"L","L",2,key_l},     {":", ":",2,key_semicolon},{"\"","\"",   2,key_apostrophe},      {"\n","NewLine",3,key_return},       {"https://","https://",4},                                                                               {"","",0,key_none,skey_nextline},
	{"",  "Shift", 5,key_shift,skey_shift},{"Z", "Z", 2,key_z},   {"X", "X", 2,key_x},           {"C", "C",2,key_c},    {"V","V",  2,key_v},           {"B","B",   2,key_b},             {"N","N",2,key_n},   {"M","M",2,key_m},   {"<","<",2,key_comma},{">",">",2,key_period},{"?", "?",2,key_slash_fwd},{"",  "Shift",4,key_shift,skey_shift},{"",  "^",      2,key_up},                                                                                                                    {"","",0,key_none,skey_nextline},
	{"",  "Ctrl",  4,key_ctrl,skey_ctrl},  {"", "Cmd",3,key_rcmd},{"", "Alt",3,key_alt,skey_alt},{" ", " ",9,key_space},{"", "Alt",4,key_alt,skey_alt},{"", "Ctrl",4,key_ctrl,skey_ctrl},{"", "<",2,key_left},{"", "v",2,key_down},{"", ">",2,key_right},{"", "", 0,key_none,skey_end}
};
const keylayout_key_t en_num_normal[] = {
	{"7", "7",2,key_7}, {"8", "8",2,key_8     }, {"9", "9",    2,key_9},      {"\b","<-",2,key_backspace}, {"","",0,key_none,skey_nextline},
	{"4", "4",2,key_4}, {"5", "5",2,key_5     }, {"6", "6",    2,key_6},      {"-", "-", 2,key_minus},     {"","",0,key_none,skey_nextline},
	{"1", "1",2,key_1}, {"2", "2",2,key_2     }, {"3", "3",    2,key_3},                                   {"","",0,key_none,skey_nextline},
	{"0", "0",2,key_0}, {".", ".",2,key_period}, {"\r","Enter",4,key_return},                              {"","",0,key_none,skey_end}
};*/

keylayout_t virtualkeyboard_layout_en_us = {
	/*{ en_text_normal, en_text_shift, en_text_symbols },
	{ _countof(en_text_normal), _countof(en_text_shift), _countof(en_text_symbols) },
	{ en_uri_normal,  en_uri_shift },
	{ _countof(en_uri_normal), _countof(en_uri_shift) },
	{ en_num_normal },
	{ _countof(en_num_normal) }*/
};

}
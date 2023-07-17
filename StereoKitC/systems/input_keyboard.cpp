#include "input.h"
#include "input_keyboard.h"
#include "../platforms/platform_utils.h"
#include "../libraries/array.h"
#include "../libraries/tinycthread.h"

namespace sk {

///////////////////////////////////////////

struct keyboard_event_t {
	key_    key;
	uint8_t scan;
	uint8_t code;
	int16_t down;
};

struct keyboard_t {
	uint8_t                   keys[key_MAX];
	uint8_t                   scans[key_MAX];
	uint8_t                   codes[key_MAX];
	array_t<keyboard_event_t> events;
	array_t<char32_t>         characters;
	int32_t                   queue_counter;
};

///////////////////////////////////////////

keyboard_t                input_key_data        = {};
array_t<keyboard_event_t> input_key_pending     = {};
array_t<char32_t>         input_chars_pending   = {};
mtx_t                     input_key_pending_mtx = {};
bool                      input_key_suspended   = false;
float                     input_last_physical_keypress = -1000;

///////////////////////////////////////////

uint8_t scan_map_qwerty[256] = {};
uint8_t code_map_qwerty[256] = {};

void init_qwerty_lookups() {
	scan_map_qwerty[key_q] = 17;
	scan_map_qwerty[key_w] = 18;
	scan_map_qwerty[key_e] = 19;
	scan_map_qwerty[key_r] = 20;
	scan_map_qwerty[key_t] = 21;
	scan_map_qwerty[key_y] = 22;
	scan_map_qwerty[key_u] = 23;
	scan_map_qwerty[key_i] = 24;
	scan_map_qwerty[key_o] = 25;
	scan_map_qwerty[key_p] = 26;

	scan_map_qwerty[key_a] = 31;
	scan_map_qwerty[key_s] = 32;
	scan_map_qwerty[key_d] = 33;
	scan_map_qwerty[key_f] = 34;
	scan_map_qwerty[key_g] = 35;
	scan_map_qwerty[key_h] = 36;
	scan_map_qwerty[key_j] = 37;
	scan_map_qwerty[key_k] = 38;
	scan_map_qwerty[key_l] = 39;

	scan_map_qwerty[key_z] = 46;
	scan_map_qwerty[key_x] = 47;
	scan_map_qwerty[key_c] = 48;
	scan_map_qwerty[key_v] = 49;
	scan_map_qwerty[key_b] = 50;
	scan_map_qwerty[key_n] = 51;
	scan_map_qwerty[key_m] = 52;


	code_map_qwerty[key_q] = 24;
	code_map_qwerty[key_w] = 25;
	code_map_qwerty[key_e] = 26;
	code_map_qwerty[key_r] = 27;
	code_map_qwerty[key_t] = 28;
	code_map_qwerty[key_y] = 29;
	code_map_qwerty[key_u] = 30;
	code_map_qwerty[key_i] = 31;
	code_map_qwerty[key_o] = 32;
	code_map_qwerty[key_p] = 33;

	code_map_qwerty[key_a] = 38;
	code_map_qwerty[key_s] = 39;
	code_map_qwerty[key_d] = 40;
	code_map_qwerty[key_f] = 41;
	code_map_qwerty[key_g] = 42;
	code_map_qwerty[key_h] = 43;
	code_map_qwerty[key_j] = 44;
	code_map_qwerty[key_k] = 45;
	code_map_qwerty[key_l] = 46;

	code_map_qwerty[key_z] = 53;
	code_map_qwerty[key_x] = 54;
	code_map_qwerty[key_c] = 55;
	code_map_qwerty[key_v] = 56;
	code_map_qwerty[key_b] = 57;
	code_map_qwerty[key_n] = 58;
	code_map_qwerty[key_m] = 59;
}

///////////////////////////////////////////

void input_keyboard_initialize() {
	mtx_init(&input_key_pending_mtx, mtx_plain);
	init_qwerty_lookups();
}

///////////////////////////////////////////

void input_keyboard_shutdown() {
	mtx_destroy(&input_key_pending_mtx);
	input_key_pending    .free();
	input_key_data.events.free();
}

///////////////////////////////////////////

void input_keyboard_suspend(bool suspend) {
	input_key_suspended = suspend;
}

///////////////////////////////////////////

void input_keyboard_update() {
	input_text_reset();

	// Clear any just_in/active flags that were set on the last frame
	array_t<keyboard_event_t> &evts = input_key_data.events;
	for (int32_t i = 0; i < evts.count; i++) {
		if (evts[i].down) {
			input_key_data.keys [evts[i].key]  &= ~button_state_just_active;
			input_key_data.scans[evts[i].scan] &= ~button_state_just_active;
			input_key_data.codes[evts[i].code] &= ~button_state_just_active;
		}
		else {
			input_key_data.keys [evts[i].key]  &= ~button_state_just_inactive;
			input_key_data.scans[evts[i].scan] &= ~button_state_just_inactive;
			input_key_data.codes[evts[i].code] &= ~button_state_just_inactive;
		}
	}

	// Thread safe copy new key events into the main thread
	input_key_data.events.clear();
	input_key_data.characters.clear();
	mtx_lock(&input_key_pending_mtx);
	input_key_data.events    .add_range(input_key_pending  .data, input_key_pending  .count);
	input_key_data.characters.add_range(input_chars_pending.data, input_chars_pending.count);
	input_key_pending  .clear();
	input_chars_pending.clear();
	mtx_unlock(&input_key_pending_mtx);

	// Set key activity flags based on the new event queue
	evts = input_key_data.events;
	for (int32_t i = 0; i < evts.count; i++) {
		keyboard_event_t &evt = evts.get(i);
		if (evt.down) {
			input_key_data.keys [evt.key]  |= button_state_just_active | button_state_active;
			input_key_data.scans[evt.scan] |= button_state_just_active | button_state_active;
			input_key_data.codes[evt.code] |= button_state_just_active | button_state_active;
		} else {
			input_key_data.keys [evt.key]  &= ~button_state_active;
			input_key_data.keys [evt.key]  |=  button_state_just_inactive;

			input_key_data.scans[evt.scan] &= ~button_state_active;
			input_key_data.scans[evt.scan] |=  button_state_just_inactive;

			input_key_data.codes[evt.code] &= ~button_state_active;
			input_key_data.codes[evt.code] |=  button_state_just_inactive;
		}
	}
}

///////////////////////////////////////////

void input_key_inject_press(key_ key, uint8_t scan, uint8_t code) {
	// Don't inject keys if input is suspended
	if (input_key_suspended) return;

	mtx_lock(&input_key_pending_mtx);

	keyboard_event_t evt;
	evt.key  = key;
	evt.scan = scan != key_none ? scan : code - 7;
	evt.code = code != key_none ? code : scan + 7;
	evt.down = 1;
	input_key_pending.add(evt);

	mtx_unlock(&input_key_pending_mtx);
}

///////////////////////////////////////////

void input_key_inject_release(key_ key, uint8_t scan, uint8_t code) {
	// Don't inject keys if input is suspended, unless the key was pressed
	// before input was suspended.
	if (input_key_suspended && (input_keyboard_get(key) & button_state_inactive))
		return;

	mtx_lock(&input_key_pending_mtx);

	keyboard_event_t evt;
	evt.key  = key;
	evt.scan = scan != key_none ? scan : code - 7;
	evt.code = code != key_none ? code : scan + 7;
	evt.down = 0;
	input_key_pending.add(evt);

	mtx_unlock(&input_key_pending_mtx);
}

///////////////////////////////////////////

button_state_ input_keyboard_get(key_ key) {
	return (button_state_)input_key_data.keys[key];
}

///////////////////////////////////////////

button_state_ input_keyboard_get_code(key_ key) {
	uint8_t code = code_map_qwerty[key];
	return (button_state_)input_key_data.codes[code];
}

///////////////////////////////////////////

void input_text_inject_char(char32_t character) {
	// Don't inject characters if input is suspended
	if (input_key_suspended) return;

	mtx_lock(&input_key_pending_mtx);

	input_chars_pending.add(character);

	mtx_unlock(&input_key_pending_mtx);
}

///////////////////////////////////////////

char32_t input_text_consume() {
	// Return false if queue is empty
	if (input_key_data.queue_counter >= input_key_data.characters.count) return 0;

	// Next item in queue for the next consume call
	char32_t result = input_key_data.characters[input_key_data.queue_counter];
	input_key_data.queue_counter++;

	return result;
}

///////////////////////////////////////////

void input_text_reset() {
	input_key_data.queue_counter = 0;
}

///////////////////////////////////////////

}
#include "input.h"
#include "input_keyboard.h"
#include "../platforms/platform_utils.h"
#include "../libraries/array.h"
#include "../libraries/tinycthread.h"

namespace sk {

///////////////////////////////////////////

struct keyboard_event_t {
	key_    key;
	int16_t down;
};

struct keyboard_t {
	uint8_t                   keys[key_MAX];
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

void input_keyboard_initialize() {
	mtx_init(&input_key_pending_mtx, mtx_plain);
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
		if (evts[i].down)
			input_key_data.keys[evts[i].key] &= ~button_state_just_active;
		else
			input_key_data.keys[evts[i].key] &= ~button_state_just_inactive;
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
			input_key_data.keys[evt.key] |= button_state_just_active | button_state_active;
		} else {
			input_key_data.keys[evt.key] &= ~button_state_active;
			input_key_data.keys[evt.key] |=  button_state_just_inactive;
		}
	}
}

///////////////////////////////////////////

void input_keyboard_inject_press(key_ key) {
	// Don't inject keys if input is suspended
	if (input_key_suspended) return;

	mtx_lock(&input_key_pending_mtx);

	keyboard_event_t evt;
	evt.key  = key;
	evt.down = 1;
	input_key_pending.add(evt);

	mtx_unlock(&input_key_pending_mtx);
}

///////////////////////////////////////////

void input_keyboard_inject_release(key_ key) {
	// Don't inject keys if input is suspended, unless the key was pressed
	// before input was suspended.
	if (input_key_suspended && (input_keyboard_get(key) & button_state_inactive))
		return;

	mtx_lock(&input_key_pending_mtx);

	keyboard_event_t evt;
	evt.key  = key;
	evt.down = 0;
	input_key_pending.add(evt);

	mtx_unlock(&input_key_pending_mtx);
}

///////////////////////////////////////////

button_state_ input_keyboard_get(key_ key) {
	return (button_state_)input_key_data.keys[key];
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
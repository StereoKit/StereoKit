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

struct input_keyboard_state_t {
	keyboard_t                key_data;
	array_t<keyboard_event_t> key_pending;
	array_t<char32_t>         chars_pending;
	mtx_t                     key_pending_mtx;
	bool                      key_suspended;
	float                     last_physical_keypress;
};
static input_keyboard_state_t local = {};

///////////////////////////////////////////

void input_keyboard_initialize() {
	local = {};
	local.last_physical_keypress = -1000;

	mtx_init(&local.key_pending_mtx, mtx_plain);
}

///////////////////////////////////////////

void input_keyboard_shutdown() {
	mtx_destroy(&local.key_pending_mtx);
	local.key_pending        .free();
	local.chars_pending      .free();
	local.key_data.events    .free();
	local.key_data.characters.free();
	local = {};
}

///////////////////////////////////////////

void input_keyboard_suspend(bool suspend) {
	local.key_suspended = suspend;
}

///////////////////////////////////////////

void input_keyboard_update() {
	input_text_reset();

	// Clear any just_in/active flags that were set on the last frame
	array_t<keyboard_event_t> &evts = local.key_data.events;
	for (int32_t i = 0; i < evts.count; i++) {
		if (evts[i].down)
			local.key_data.keys[evts[i].key] &= ~button_state_just_active;
		else
			local.key_data.keys[evts[i].key] &= ~button_state_just_inactive;
	}

	// Thread safe copy new key events into the main thread
	local.key_data.events    .clear();
	local.key_data.characters.clear();
	mtx_lock(&local.key_pending_mtx);
	local.key_data.events    .add_range(local.key_pending  .data, local.key_pending  .count);
	local.key_data.characters.add_range(local.chars_pending.data, local.chars_pending.count);
	local.key_pending  .clear();
	local.chars_pending.clear();
	mtx_unlock(&local.key_pending_mtx);

	// Set key activity flags based on the new event queue
	evts = local.key_data.events;
	for (int32_t i = 0; i < evts.count; i++) {
		keyboard_event_t &evt = evts.get(i);
		if (evt.down) {
			local.key_data.keys[evt.key] |= button_state_just_active | button_state_active;
		} else {
			local.key_data.keys[evt.key] &= ~button_state_active;
			local.key_data.keys[evt.key] |=  button_state_just_inactive;
		}
	}
}

///////////////////////////////////////////

void input_key_inject_press(key_ key) {
	// Don't inject keys if input is suspended
	if (local.key_suspended) return;

	mtx_lock(&local.key_pending_mtx);

	keyboard_event_t evt;
	evt.key  = key;
	evt.down = 1;
	local.key_pending.add(evt);

	mtx_unlock(&local.key_pending_mtx);
}

///////////////////////////////////////////

void input_key_inject_release(key_ key) {
	// Don't inject keys if input is suspended, unless the key was pressed
	// before input was suspended.
	if (local.key_suspended && (input_keyboard_get(key) & button_state_inactive))
		return;

	mtx_lock(&local.key_pending_mtx);

	keyboard_event_t evt;
	evt.key  = key;
	evt.down = 0;
	local.key_pending.add(evt);

	mtx_unlock(&local.key_pending_mtx);
}

///////////////////////////////////////////

button_state_ input_keyboard_get(key_ key) {
	return (button_state_)local.key_data.keys[key];
}

///////////////////////////////////////////

void input_text_inject_char(char32_t character) {
	// Don't inject characters if input is suspended
	if (local.key_suspended) return;

	mtx_lock(&local.key_pending_mtx);

	local.chars_pending.add(character);

	mtx_unlock(&local.key_pending_mtx);
}

///////////////////////////////////////////

char32_t input_text_consume() {
	// Return false if queue is empty
	if (local.key_data.queue_counter >= local.key_data.characters.count) return 0;

	// Next item in queue for the next consume call
	char32_t result = local.key_data.characters[local.key_data.queue_counter];
	local.key_data.queue_counter++;

	return result;
}

///////////////////////////////////////////

void input_text_reset() {
	local.key_data.queue_counter = 0;
}

///////////////////////////////////////////

float input_get_last_physical_keypress_time() {
	return local.last_physical_keypress;
}

///////////////////////////////////////////

void input_set_last_physical_keypress_time(float time) {
	local.last_physical_keypress = time;

}

///////////////////////////////////////////

}
#include "input.h"
#include "input_keyboard.h"
#include "../platforms/platform.h"
#include "../libraries/array.h"
#include "../libraries/ferr_thread.h"
#include "../libraries/unicode.h"

namespace sk {

///////////////////////////////////////////

struct keyboard_t {
	uint8_t                   keys[key_MAX];
	array_t<keyboard_event_t> events;       // This frame's events, in order
	int32_t                   event_cursor; // input_keyboard_consume
	int32_t                   text_cursor;  // Deprecated input_text_consume
};

///////////////////////////////////////////

struct input_keyboard_state_t {
	keyboard_t                key_data;
	array_t<keyboard_event_t> pending;             // Guarded by pending_mtx
	key_mod_                  pending_mods;        // Guarded by pending_mtx
	ft_mutex_t                pending_mtx;
	bool                      key_suspended;
	uint64_t                  warned_char_mask;    // Guarded by pending_mtx
	float                     last_physical_keypress;
};
static input_keyboard_state_t local = {};

///////////////////////////////////////////

static key_mod_ input_key_to_mod(key_ key) {
	switch (key) {
	case key_shift: return key_mod_shift;
	case key_ctrl:  return key_mod_ctrl;
	case key_alt:   return key_mod_alt;
	case key_lcmd:
	case key_rcmd:  return key_mod_cmd;
	default:        return key_mod_none;
	}
}

///////////////////////////////////////////

// Stamps the event with the modifiers held right now, so a consumer walking
// the queue sees the state at each event. Callers must hold pending_mtx.
static void input_pending_add(keyboard_event_type_ type, key_ key, char32_t character) {
	keyboard_event_t evt = {};
	evt.type      = type;
	evt.key       = key;
	evt.modifiers = local.pending_mods;
	evt.character = character;
	local.pending.add(evt);
}

///////////////////////////////////////////

// The text channel carries insertable text only. Newline and tab qualify, the
// rest of the control codes are editing keys. Surrogates and codepoints past
// the Unicode range are not text at all.
bool input_text_insertable(char32_t character) {
	if (character >= 0xD800 && character <= 0xDFFF) return false;
	if (character >  0x10FFFF)                      return false;
	return (character >= 0x20 && character != 0x7f) ||
	        character == '\n' || character == '\t';
}

///////////////////////////////////////////

// One warning per distinct character, so a held key's repeats don't spam the
// log. Callers must hold pending_mtx.
static bool input_text_warn_once(char32_t character) {
	uint64_t bit = 1ULL << (character & 63);
	if (local.warned_char_mask & bit) return false;
	local.warned_char_mask |= bit;
	return true;
}

///////////////////////////////////////////

static void input_text_warn_skipped(char32_t character) {
	if (character < 0x20 || character == 0x7f)
		log_warnf("Ignored control character U+%04X injected as text. Editing "
		          "keys go through input_key_inject_press instead.", (uint32_t)character);
	else
		log_warnf("Ignored invalid codepoint U+%X injected as text.", (uint32_t)character);
}

///////////////////////////////////////////

void input_keyboard_initialize() {
	local = {};
	local.last_physical_keypress = -1000;
	local.pending_mtx            = ft_mutex_create();
}

///////////////////////////////////////////

void input_keyboard_shutdown() {
	ft_mutex_destroy(&local.pending_mtx);
	local.pending         .free();
	local.key_data.events .free();
	local = {};
}

///////////////////////////////////////////

void input_keyboard_suspend(bool suspend) {
	local.key_suspended = suspend;

	// Modifier releases can get lost around a suspend, and a stale modifier
	// would mis-stamp every event after. Fresh presses will set them again.
	if (suspend) {
		ft_mutex_lock(local.pending_mtx);
		local.pending_mods = key_mod_none;
		ft_mutex_unlock(local.pending_mtx);
	}
}

///////////////////////////////////////////

void input_keyboard_update() {
	local.key_data.event_cursor = 0;
	local.key_data.text_cursor  = 0;

	// Clear any just_in/active flags that were set on the last frame
	array_t<keyboard_event_t> &evts = local.key_data.events;
	for (int32_t i = 0; i < evts.count; i++) {
		if      (evts[i].type == keyboard_event_type_key_press  ) local.key_data.keys[evts[i].key] &= ~button_state_just_active;
		else if (evts[i].type == keyboard_event_type_key_release) local.key_data.keys[evts[i].key] &= ~button_state_just_inactive;
	}

	// Thread safe copy new events into the main thread
	evts.clear();
	ft_mutex_lock(local.pending_mtx);
	evts.add_range(local.pending.data, local.pending.count);
	local.pending.clear();
	ft_mutex_unlock(local.pending_mtx);

	// Set key activity flags based on the new event queue
	for (int32_t i = 0; i < evts.count; i++) {
		keyboard_event_t &evt = evts.get(i);
		if (evt.type == keyboard_event_type_key_press) {
			local.key_data.keys[evt.key] |= button_state_just_active | button_state_active;
		} else if (evt.type == keyboard_event_type_key_release) {
			local.key_data.keys[evt.key] &= ~button_state_active;
			local.key_data.keys[evt.key] |=  button_state_just_inactive;
		}
	}
}

///////////////////////////////////////////

void input_key_inject_press(key_ key) {
	// Don't inject keys if input is suspended
	if (local.key_suspended || (uint32_t)key >= key_MAX) return;

	ft_mutex_lock(local.pending_mtx);

	// A modifier's own press carries itself, its release does not
	local.pending_mods |= input_key_to_mod(key);
	input_pending_add(keyboard_event_type_key_press, key, 0);

	ft_mutex_unlock(local.pending_mtx);
}

///////////////////////////////////////////

void input_key_inject_release(key_ key) {
	if ((uint32_t)key >= key_MAX) return;

	// Don't inject keys if input is suspended, unless the key was pressed
	// before input was suspended.
	if (local.key_suspended && (input_keyboard_get(key) & button_state_inactive))
		return;

	ft_mutex_lock(local.pending_mtx);

	local.pending_mods &= ~input_key_to_mod(key);
	input_pending_add(keyboard_event_type_key_release, key, 0);

	ft_mutex_unlock(local.pending_mtx);
}

///////////////////////////////////////////

button_state_ input_keyboard_get(key_ key) {
	if ((uint32_t)key >= key_MAX) return button_state_inactive;
	return (button_state_)local.key_data.keys[key];
}

///////////////////////////////////////////

void input_text_inject(const char* text_utf8) {
	// Don't inject characters if input is suspended
	if (local.key_suspended || text_utf8 == nullptr) return;

	char32_t warn_char = 0;
	char32_t curr      = 0;

	// One lock for the whole string, so a paste arrives as a single block
	ft_mutex_lock(local.pending_mtx);

	while (utf8_decode_fast_b(text_utf8, &text_utf8, &curr)) {
		// Carriage returns are line breaks, and CRLF is a single one
		if (curr == '\r') {
			if (*text_utf8 == '\n') text_utf8++;
			curr = '\n';
		}
		if (input_text_insertable(curr)) {
			input_pending_add(keyboard_event_type_text, key_none, curr);
		} else if (warn_char == 0 && input_text_warn_once(curr)) {
			warn_char = curr;
		}
	}

	ft_mutex_unlock(local.pending_mtx);

	if (warn_char != 0)
		input_text_warn_skipped(warn_char);
}

///////////////////////////////////////////

void input_text_inject_char(char32_t character) {
	// Don't inject characters if input is suspended
	if (local.key_suspended) return;

	if (character == '\r') character = '\n'; // Carriage return is a line break

	bool insertable = input_text_insertable(character);
	bool warn       = false;

	ft_mutex_lock(local.pending_mtx);

	if (insertable) input_pending_add(keyboard_event_type_text, key_none, character);
	else            warn = input_text_warn_once(character);

	ft_mutex_unlock(local.pending_mtx);

	if (warn)
		input_text_warn_skipped(character);
}

///////////////////////////////////////////

// The event queue and its cursors are unguarded main thread state, same as
// the rest of the polled input API.
keyboard_event_t input_keyboard_consume() {
	keyboard_event_t result = {};
	if (local.key_data.event_cursor >= local.key_data.events.count) return result;

	// Next item in queue for the next consume call
	result = local.key_data.events[local.key_data.event_cursor];
	local.key_data.event_cursor++;

	return result;
}

///////////////////////////////////////////

int32_t input_keyboard_event_count() {
	return local.key_data.events.count;
}

///////////////////////////////////////////

keyboard_event_t input_keyboard_event_at(int32_t index) {
	keyboard_event_t result = {};
	if (index < 0 || index >= local.key_data.events.count) return result;
	return local.key_data.events[index];
}

///////////////////////////////////////////

char32_t input_text_consume() {
	// This cursor is independent of the event cursor, so a consumer of the
	// deprecated API sees every text event regardless of who else has read.
	array_t<keyboard_event_t> &evts = local.key_data.events;
	while (local.key_data.text_cursor < evts.count) {
		keyboard_event_t &evt = evts.get(local.key_data.text_cursor);
		local.key_data.text_cursor++;
		if (evt.type == keyboard_event_type_text) return evt.character;
	}
	return 0;
}

///////////////////////////////////////////

void input_text_reset() {
	local.key_data.text_cursor = 0;
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

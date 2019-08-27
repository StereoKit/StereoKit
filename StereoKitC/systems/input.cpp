#include "../stereokit.h"
#include "input.h"
#include "input_hand.h"

#include <vector>
using namespace std;

///////////////////////////////////////////

struct input_event_t {
	input_source_ source;
	input_state_  event;
	void (*event_callback)(input_source_ source, input_state_ evt, const pointer_t &pointer);
};

vector<input_event_t> input_listeners;
vector<pointer_t>     input_pointers;

///////////////////////////////////////////

int input_add_pointer(input_source_ source) {
	input_pointers.push_back({ source, pointer_state_none });
	return (int)input_pointers.size() - 1;
}

///////////////////////////////////////////

pointer_t *input_get_pointer(int id) {
	return &input_pointers[id];
}

///////////////////////////////////////////

int input_pointer_count(input_source_ filter) {
	int result = 0;
	for (size_t i = 0; i < input_pointers.size(); i++) {
		if (input_pointers[i].source & filter)
			result += 1;
	}
	return result;
}

///////////////////////////////////////////

pointer_t input_pointer(int32_t index, input_source_ filter) {
	int curr = 0;
	for (size_t i = 0; i < input_pointers.size(); i++) {
		if (input_pointers[i].source & filter) {
			if (curr == index)
				return input_pointers[i];
			curr += 1;
		}
	}
	return {};
}

///////////////////////////////////////////

void input_subscribe(input_source_ source, input_state_ event, void (*event_callback)(input_source_ source, input_state_ event, const pointer_t &pointer)) {
	input_listeners.push_back({ source, event, event_callback });
}

///////////////////////////////////////////

void input_unsubscribe(input_source_ source, input_state_ event, void (*event_callback)(input_source_ source, input_state_ event, const pointer_t &pointer)) {
	for (int i = (int)input_listeners.size()-1; i >= 0; i--) {
		if (input_listeners[i].source         == source && 
			input_listeners[i].event          == event  && 
			input_listeners[i].event_callback == event_callback) {
			input_listeners.erase(input_listeners.begin() + i);
		}
	}
}

///////////////////////////////////////////

void input_fire_event(input_source_ source, input_state_ event, const pointer_t &pointer) {
	for (size_t i = 0; i < input_listeners.size(); i++) {
		if (input_listeners[i].source & source && input_listeners[i].event & event) {
			input_listeners[i].event_callback(source, event, pointer);
		}
	}
}

///////////////////////////////////////////

bool input_init() {
	input_hand_init();
	return true;
}

///////////////////////////////////////////

void input_shutdown() {
	input_pointers .clear();
	input_listeners.clear();
	input_hand_shutdown();
}

///////////////////////////////////////////

void input_update() {
	input_hand_update();
}


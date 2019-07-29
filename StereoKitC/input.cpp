#include "stereokit.h"
#include "input.h"
#include "input_hand.h"

#include <vector>
using namespace std;

vector<pointer_t> input_pointers;

int input_add_pointer(pointer_source_ source) {
	input_pointers.push_back({ source, pointer_state_none });
	return input_pointers.size() - 1;
}
pointer_t *input_get_pointer(int id) {
	return &input_pointers[id];
}

int input_pointer_count(pointer_source_ filter) {
	int result = 0;
	for (size_t i = 0; i < input_pointers.size(); i++) {
		if (input_pointers[i].source & filter)
			result += 1;
	}
	return result;
}
pointer_t input_pointer(int index, pointer_source_ filter) {
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

void input_init() {
	input_hand_init();
}
void input_shutdown() {
	input_hand_shutdown();
}
void input_update() {
	input_hand_update();
}
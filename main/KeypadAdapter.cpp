#include "../headers/KeypadAdapter.h"

KeypadAdapter::KeypadAdapter() {}
KeypadAdapter::~KeypadAdapter() {}

void KeypadAdapter::setKey(uint8_t key, bool pressed) {
	if (key < NUM_KEYS) {
		keys[key] = pressed;
	}
}

bool KeypadAdapter::isKeyPressed(uint8_t key) {
	if (key < NUM_KEYS) {
		return keys[key];
	}
	return false;
}

const bool *KeypadAdapter::getKeys() {
	return keys;
}

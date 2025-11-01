#include "../headers/Keypad.h"

Keypad::Keypad() {}
Keypad::~Keypad() {}

void Keypad::setKey(uint8_t key, bool pressed) {
	if (key < NUM_KEYS) {
		keys[key] = pressed;
	}
}

bool Keypad::isKeyPressed(uint8_t key) {
	if (key < NUM_KEYS) {
		return keys[key];
	}
	return false;
}

const bool *Keypad::getKeys() {
	return keys;
}

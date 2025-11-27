#include "headers/Display.h"
#include <Arduino.h>
#include <cstring>

Display::Display() {}
Display::~Display() {}

uint32_t Display::getPixel(uint8_t x, uint8_t y) {
	return screenBuffer[(y * VIDEO_WIDTH) + x];
}

void Display::setPixel(uint8_t x, uint8_t y, uint32_t value) {
	screenBuffer[(y * VIDEO_WIDTH) + x] = value;
}

void Display::clearBuffer() {
	memset(screenBuffer, 0, sizeof(screenBuffer));
}

uint8_t Display::getHeight() {
	return VIDEO_HEIGHT;
}

uint8_t Display::getWidth() {
	return VIDEO_WIDTH;
}

uint32_t *Display::getBuffer() {
	return screenBuffer;
}
#include "../headers/CPU.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <Arduino.h>

CPU::CPU(Memory &memory, Display &display, Keypad &keypad)
    : RAM(memory), display(display), keypad(keypad) {
	init();
}

CPU::~CPU() {};

void CPU::init() {
	regI = 0;
	soundTimer = 0;
	delayTimer = 0;
	stackPointer = 0;
	instruction = 0;
	programCounter = 0x200;
}

void CPU::clockCycle() {
	fetchInstruction();
	programCounter += 2;
	decodeExecuteInstruction();
}

void CPU::fetchInstruction() {
	uint8_t msB = RAM.read(programCounter);
	uint8_t lsB = RAM.read(programCounter + 0x1);
	instruction = msB << 8 | lsB;
}

void CPU::decodeExecuteInstruction() {
	uint8_t opcode = (instruction & 0xF000) >> 12;
	uint8_t x = getNibble(2);
	uint8_t y = getNibble(3);
	uint8_t n = getNibble(4);
	uint8_t nn = (y << 4) | n;
	uint16_t nnn = (x << 8) | nn;

	switch (opcode) {
		case 0x0:
			switch (instruction) {
				case 0x00E0:
					display.clearBuffer();
					break;

				case 0x00EE:
					programCounter = stack[stackPointer];
					stackPointer -= 1;
					break;
			}
			break;

		case 0x1:
			programCounter = nnn;
			break;

		case 0x2:
			stack[++stackPointer] = programCounter;
			programCounter = nnn;
			break;

		case 0x3:
			if (regV[x] == nn)
				programCounter += 2;
			break;

		case 0x4:
			if (regV[x] != nn)
				programCounter += 2;
			break;

		case 0x5:
			if (regV[x] == regV[y])
				programCounter += 2;
			break;

		case 0x6:
			regV[x] = nn;
			break;

		case 0x7:
			regV[x] = regV[x] + nn;
			break;

		case 0x8:
			switch (n) {
				case 0x0:
					regV[x] = regV[y];
					break;

				case 0x1:
					regV[x] = regV[x] | regV[y];
					break;

				case 0x2:
					regV[x] = regV[x] & regV[y];
					break;

				case 0x3:
					regV[x] = regV[x] ^ regV[y];
					break;

				case 0x4:

					if (regV[x] + regV[y] > 255)
						regV[0xF] = 0x1; // Carry
					else
						regV[0xF] = 0x0;

					regV[x] = regV[x] + regV[y];
					break;

				case 0x5:

					if (regV[x] > regV[y])
						regV[0xF] = 0x1; // Not borrow
					else
						regV[0xF] = 0x0;

					regV[x] = regV[x] - regV[y];
					break;

				case 0x6:
					regV[0xF] = (regV[x] & 0x01);
					regV[x] = regV[x] >> 1;
					break;

				case 0x7:
					if (regV[y] > regV[x])
						regV[0xF] = 0x1; // Not borrow
					else
						regV[0xF] = 0x0;

					regV[x] = regV[y] - regV[x];
					break;

				case 0xE:
					regV[0xF] = (regV[x] & 0x80) >> 7;
					regV[x] = regV[x] << 1;
					break;

				default:
					// Invalid opcode
					break;
			}
			break;

		case 0x9:
			if (regV[x] != regV[y])
				programCounter += 2;
			break;

		case 0xA:
			regI = nnn;
			break;

		case 0xB:
			programCounter = regV[0x0] + nnn;
			break;

		case 0xC:
			regV[x] = (rand() % 256) && nn;
			break;

		case 0xD: {
			uint8_t Vx = regV[x];
			uint8_t Vy = regV[y];
			regV[0xF] = 0x0;

			// Leer sprite desde memoria
			uint8_t sprite[n];
			for (int i = 0; i < n; i++) {
				sprite[i] = RAM.read(regI + i);
			}

			// Dibujar sprite usando XOR y buffer 0/1
			for (int row = 0; row < n; row++) {
				uint8_t byte = sprite[row];
				for (int col = 0; col < 8; col++) {
					if (!(byte & (0x80 >> col)))
						continue;

					uint8_t px = (Vx + col) % display.getWidth();  // wrap horizontal
					uint8_t py = (Vy + row) % display.getHeight(); // wrap vertical

					if (display.getPixel(px, py) == 1){
						regV[0xF] = 1; // colisión
					}

					if(display.getPixel(px,py) == 1){
						display.setPixel(px, py, 0);
					}else{
						display.setPixel(px, py, 1);
					}
				}
			}

			break;
		}

		case 0xE:
			switch (nn) {
				case 0x9E:
					if (keypad.isKeyPressed(regV[x]))
						programCounter += 2;
					break;

				case 0xA1:
					if (!keypad.isKeyPressed(regV[x]))
						programCounter += 2;
					break;

				default:
					// Invalid opcode
					break;
			}

			break;

		case 0xF:
			switch (nn) {
				case 0x07:
					regV[x] = delayTimer;
					break;
				case 0x0A: {
					bool pressed = false;
					for (uint8_t i = 0; i < NUM_KEYS; i++) {
						if (keypad.isKeyPressed(i)) {
							pressed = true;
							regV[x] = i;
							break;
						}
					}
					if (!pressed) {
						programCounter -= 2;
					}

					break;
				}

				case 0x15:
					delayTimer = regV[x];
					break;

				case 0x18:
					soundTimer = regV[x];
					break;

				case 0x1E:
					regI += regV[x];
					break;

				case 0x29:
					regI = regV[x] * 5;
					break;

				case 0x33: {
					uint8_t value = regV[x];

					RAM.write(regI + 0x2, value % 10);
					value /= 10;
					RAM.write(regI + 0x1, value % 10);
					value /= 10;
					RAM.write(regI, value % 10);
					value /= 10;
					break;
				}

				case 0x55:
					for (uint8_t i = 0; i <= x; i++) {
						RAM.write(regI + i, regV[i]);
					}
					break;

				case 0x65:
					for (uint8_t i = 0; i <= x; i++) {
						regV[i] = RAM.read(regI + i);
					}
					break;
			}

			break;
	}
}

void CPU::updateTimers() {
	if (delayTimer > 0) {
		--delayTimer;
	}

	if (soundTimer > 0) {
		--soundTimer;
	}
}

uint8_t CPU::getNibble(int i) {
	uint8_t nibble = 0;
	switch (i) {
		case 1:
			nibble = (instruction & 0xF000) >> 12;
			break;
		case 2:
			nibble = (instruction & 0x0F00) >> 8;
			break;
		case 3:
			nibble = (instruction & 0x00F0) >> 4;
			break;
		case 4:
			nibble = instruction & 0x000F;
			break;
		default:
			throw std::out_of_range("Nibble i must be 1-4");
	}

	return nibble;
}

#include "headers/includes.h"
#include <Arduino.h>
#include <Keypad.h>
#include <cstring>

////////////////////////////////////////////////////////////////////////////

// Pines para el display TFT, usar los pines de SPI por defecto de la placa

#define TFT_CS 46
#define TFT_DC 9
#define TFT_RST 10

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

////////////////////////////////////////////////////////////////////////////

// Definicion del KEYPAD y sus pines

#define ROWS 4
#define COLS 4

char keys[ROWS][COLS] = {
    {0xA, 0x7, 0x4, 0x1},
    {0x0, 0x8, 0x5, 0x2},
    {0xB, 0x9, 0x6, 0x3},
    {0xF, 0xE, 0xD, 0xC}};

uint8_t rowPins[ROWS] = {15, 16, 17, 18};
uint8_t colPins[COLS] = {4, 5, 6, 7};

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

////////////////////////////////////////////////////////////////////////////

Memory memory;
Display display;
KeypadAdapter keypad;
Platform platform(tft);
CPU chip8(memory, display, keypad);
ROMLoader romLoader;

hw_timer_t *chip8ClockTimer = NULL;
hw_timer_t *chip8DelaySoundTimer = NULL;

volatile bool updateTFTScreenFlag = false;
volatile bool chip8ClockCycleFlag = false;

volatile bool romSelected = false;
volatile int highlightIndex = 0;

volatile bool chip8running = false;

void inicializarCHIP8Timers();

void IRAM_ATTR CHIP8ClockCycle();
void IRAM_ATTR CHIP8UpdateTimers();

void keypadEvent();
bool scanKeypad();
void drawROMMenuTFT(int);
void selectROM();
void stopAndExit();

////////////////////////////////////////////////////////////////////////////

void setup() {

	Serial.begin(115200);
	platform.init();
	drawROMMenuTFT(highlightIndex);
	kpd.setDebounceTime(20); // 20ms

	inicializarCHIP8Timers();
}

void loop() {

	bool scanned = scanKeypad();

	if (!romSelected) {
		if (scanned) {
			selectROM();
		}
		return;
	}

	if (!chip8running) {
		String romPath = "/" + romLoader.roms[highlightIndex];
		bool loaded = romLoader.loadROM(romPath, memory.getFirstPosition());
		chip8running = true;
		chip8.init();

		timerRestart(chip8ClockTimer);
		timerRestart(chip8DelaySoundTimer);
		timerStart(chip8ClockTimer);
		timerStart(chip8DelaySoundTimer);
	}


	if (keypad.isKeyPressed(0xF)) {
		stopAndExit();
		return;
	}


	if (chip8ClockCycleFlag) {
		chip8ClockCycleFlag = false;
		chip8.clockCycle();
	}
	
	if (updateTFTScreenFlag) {
		updateTFTScreenFlag = false;
		chip8.updateTimers();
		platform.UpdateScreen(display.getBuffer());
	}


}

////////////////////////////////////////////////////////////////////////////

/*	ISRs  */

void IRAM_ATTR CHIP8ClockCycle() {
	chip8ClockCycleFlag = true;
}

void IRAM_ATTR CHIP8UpdateTimers() {
	updateTFTScreenFlag = true;
}

////////////////////////////////////////////////////////////////////////////

/* Mis funciones */

void inicializarCHIP8Timers() {
	chip8ClockTimer = timerBegin(1000000);
	timerAttachInterrupt(chip8ClockTimer, &CHIP8ClockCycle);
	timerAlarm(chip8ClockTimer, 1666, true, 0);

	chip8DelaySoundTimer = timerBegin(1000000);
	timerAttachInterrupt(chip8DelaySoundTimer, &CHIP8UpdateTimers);
	timerAlarm(chip8DelaySoundTimer, 16666, true, 0);
}

bool scanKeypad() {
	if (kpd.getKeys()) {
		for (int i = 0; i < LIST_MAX; i++) {
			if (kpd.key[i].stateChanged) {
				switch (kpd.key[i].kstate) {
					case PRESSED:
					case HOLD:
						keypad.setKey(kpd.key[i].kchar, true);
						break;
					case RELEASED:
					case IDLE:
						keypad.setKey(kpd.key[i].kchar, false);
						break;
				}
			}
		}


		return true;
	} else {
		return false;
	}
}

void drawROMMenuTFT(int highlightIndex) {
	tft.fillScreen(ST77XX_BLACK);
	tft.setTextSize(1);

	for (int i = 0; i < romLoader.romCount; i++) {
		if (i == highlightIndex) {
			tft.setTextColor(ST77XX_RED);
		} else {
			tft.setTextColor(ST77XX_YELLOW);
		}
		tft.setCursor(0, i * 16); // ajusta la altura según textSize
		tft.printf("%d-%s\n", i, romLoader.roms[i].c_str());
	}
}

void selectROM() {
	if (keypad.isKeyPressed(0x8)) {
		highlightIndex = (highlightIndex + 1) % romLoader.romCount;
		drawROMMenuTFT(highlightIndex);
	}
	if (keypad.isKeyPressed(0x2)) {
		highlightIndex--;
		if (highlightIndex < 0)
			highlightIndex = romLoader.romCount - 1;
		drawROMMenuTFT(highlightIndex);
	}
	if (keypad.isKeyPressed(0x5)) {
		romSelected = true;
		tft.fillScreen(ST77XX_BLACK);
	}
}

void stopAndExit() {
	romSelected = false;
	chip8running = false;
	drawROMMenuTFT(highlightIndex);
}

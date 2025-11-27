#include "headers/CPU.h"
#include "headers/Display.h"
#include "headers/KeypadAdapter.h"
#include "headers/Memory.h"
#include "headers/Platform.h"
#include "headers/ROMLoader.h"

#include <Arduino.h>
#include <Keypad.h>
#include <cstring>

// Pines para el display TFT, usar los pines de SPI por defecto de la placa
#define TFT_CS 46
#define TFT_DC 9
#define TFT_RST 10

////////////////////////////////////////////////////////////////////////////

// Definiciones de teclas y pines keypad
#define ROWS 4
#define COLS 4

char keys[ROWS][COLS] = {
    {0xA, 0x7, 0x4, 0x1},
    {0x0, 0x8, 0x5, 0x2},
    {0xB, 0x9, 0x6, 0x3},
    {0xF, 0xE, 0xD, 0xC}};

uint8_t rowPins[ROWS] = {15, 16, 17, 18};
uint8_t colPins[COLS] = {4, 5, 6, 7};

////////////////////////////////////////////////////////////////////////////

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

Memory memory;
Display display;
KeypadAdapter keypad;
Platform platform(tft);
CPU chip8(memory, display, keypad);
ROMLoader romLoader;

hw_timer_t *chip8ClockTimer = NULL;
hw_timer_t *chip8DelaySoundTimer = NULL;
hw_timer_t *keypadScanTimer = NULL;
hw_timer_t *keypadDebounceTimer = NULL;

volatile bool updateTFTScreenFlag = false;
volatile bool chip8ClockCycleFlag = false;
volatile bool scanKeypadFlag = false;
volatile bool debouncerFlag = true;

volatile bool romSelected = false; 
volatile int highlightIndex = 0;   // Me chirrian ambas cosas
volatile bool chip8running = false;

void inicializarCHIP8Timers();
void inicializarKeypadTimers();

void IRAM_ATTR CHIP8ClockCycle();
void IRAM_ATTR CHIP8UpdateTimers();
void IRAM_ATTR keypadScanInterrupt();

void scanKeypad();
void drawROMMenuTFT(int);
void selectROM();

////////////////////////////////////////////////////////////////////////////

void setup() {

	Serial.begin(115200);
	platform.init();
	inicializarKeypadTimers();

	drawROMMenuTFT(highlightIndex);
}

void loop() {

    if(scanKeypadFlag && debouncerFlag){
        scanKeypadFlag = false;
        debouncerFlag = false; // reiniciamos debounce
        scanKeypad();
    }

	if(!romSelected){
		selectROM();
		return;
	}

	if(!chip8running){
		String romPath = "/" + romLoader.roms[highlightIndex];
		romLoader.loadROM(romPath, memory.getFirstPosition());
		chip8running = true;
		inicializarCHIP8Timers();
	}


	if (updateTFTScreenFlag) {
		updateTFTScreenFlag = false;
		chip8.updateTimers();
		platform.UpdateScreen(display.getBuffer());
	}

	if(chip8ClockCycleFlag){
		chip8ClockCycleFlag = false;
		chip8.clockCycle();
	}


}

////////////////////////////////////////////////////////////////////////////

// Definiciones de funciones

void IRAM_ATTR CHIP8ClockCycle() {
	chip8ClockCycleFlag = true;
}

void IRAM_ATTR CHIP8UpdateTimers() {
	updateTFTScreenFlag = true;
}

void IRAM_ATTR keypadScanInterrupt() {

		debouncerFlag = false;
		scanKeypadFlag = true;
		timerStop(keypadScanTimer);
		timerStart(keypadDebounceTimer);
	

}

void IRAM_ATTR debounceTimerISR() {
	debouncerFlag = true;
	timerStop(keypadDebounceTimer);
	timerStart(keypadScanTimer);
}

void inicializarCHIP8Timers() {
	chip8ClockTimer = timerBegin(1000000);
	timerAttachInterrupt(chip8ClockTimer, &CHIP8ClockCycle);
	timerAlarm(chip8ClockTimer, 1666, true, 0);

	chip8DelaySoundTimer = timerBegin(1000000);
	timerAttachInterrupt(chip8DelaySoundTimer, &CHIP8UpdateTimers);
	timerAlarm(chip8DelaySoundTimer, 16666, true, 0);

	timerStart(chip8ClockTimer);
	timerStart(chip8DelaySoundTimer);
}

void inicializarKeypadTimers() {
	keypadScanTimer = timerBegin(1000000);
	timerAttachInterrupt(keypadScanTimer, &keypadScanInterrupt);
	timerAlarm(keypadScanTimer, 1000, true, 0);
	timerStart(keypadScanTimer);

	keypadDebounceTimer = timerBegin(1000000);
	timerAttachInterrupt(keypadDebounceTimer, &debounceTimerISR);
	timerAlarm(keypadDebounceTimer, 1000, true, 0);
	timerStop(keypadDebounceTimer);
}

void scanKeypad(){
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
		// delay(150); // debounce
	}
	if (keypad.isKeyPressed(0x2)) {
		highlightIndex--;
		if (highlightIndex < 0)
			highlightIndex = romLoader.romCount - 1;
		drawROMMenuTFT(highlightIndex);
		// delay(150);
	}
	if (keypad.isKeyPressed(0x5)) {
		romSelected = true;
		tft.fillScreen(ST77XX_BLACK);
	}
}

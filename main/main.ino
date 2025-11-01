#include "../headers/CPU.h"
#include "../headers/Display.h"
#include "../headers/Keypad.h"
#include "../headers/Memory.h"
#include "../headers/Platform.h"
#include "../headers/ROMLoader.h"
#include <Arduino.h>
#include <cstring>

// Pines para el display TFT, usar los pines de SPI por defecto de la placa
#define TFT_CS 10
#define TFT_DC 6
#define TFT_RST 7

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

Memory memory;
Display display;
Keypad keypad;
Platform platform(tft);
CPU cpu(memory, display, keypad);
ROMLoader romLoader;
int address = 0x200;

hw_timer_t *CPUFreq = NULL;
hw_timer_t *CH8TimersFreq = NULL;

bool cpuTick = false;
bool ch8timersTick = false;

void IRAM_ATTR CPUCycle() {
	cpuTick = true;
}

void IRAM_ATTR CPUTimers() {
	ch8timersTick = true;
}

void drawROMMenuTFT(int highlightIndex) {
	tft.fillScreen(ST77XX_BLACK); // limpiar pantalla

	tft.setTextSize(2); // ajusta según quieras que se vea grande/pequeño
	tft.setTextColor(ST77XX_YELLOW);
	tft.setCursor(0, 0);

	for (int i = 0; i < romLoader.romCount; i++) {
		if (i == highlightIndex) {
			tft.setTextColor(ST77XX_RED); // ROM seleccionada
		} else {
			tft.setTextColor(ST77XX_YELLOW);
		}
		tft.printf("%d-%s\n", i, romLoader.roms[i].c_str());
	}
}

int selectROMSerial() {
	int selected = 0;
	drawROMMenuTFT(selected);

	Serial.println("Selecciona el número de ROM:");

	for (int i = 0; i < romLoader.romCount; i++)
		Serial.printf("%d: %s\n", i, romLoader.roms[i].c_str());

	while (true) {
		if (Serial.available()) {
			int choice = Serial.parseInt();
			if (choice >= 0 && choice < romLoader.romCount) {
				selected = choice;
				drawROMMenuTFT(selected); // actualizar TFT
				Serial.printf("Seleccionaste: %s\n", romLoader.roms[selected].c_str());
				tft.fillScreen(ST77XX_BLACK);
				return selected;
			} else {
				Serial.println("Número inválido, intenta otra vez.");
			}
		}
	}
}

void setup() {

	pinMode(38, OUTPUT);
	digitalWrite(38, LOW);

	Serial.begin(115200);

	platform.init();

	// Carga del ROM
	int romIndex = selectROMSerial();
	String romPath = "/" + romLoader.roms[romIndex];

	try {
		romLoader.loadROM(romPath, memory.getFirstPosition());
	} catch (String error) {
		Serial.println(error);
	}

	// Inicializacion de timers

	CPUFreq = timerBegin(1000000); // 1000 Hz
	timerAttachInterrupt(CPUFreq, &CPUCycle);
	timerAlarm(CPUFreq, 1666, true, 0);

	CH8TimersFreq = timerBegin(1000000); // 1000 Hz
	timerAttachInterrupt(CH8TimersFreq, &CPUTimers);
	timerAlarm(CH8TimersFreq, 16666, true, 0);

	timerStart(CPUFreq);
	timerStart(CH8TimersFreq);
}

void loop() {

	if (cpuTick) {
		cpu.clockCycle();
		cpuTick = false;
	}

	if (ch8timersTick) {
		cpu.updateTimers();
		platform.UpdateScreen(display.getBuffer());
		ch8timersTick = false;
	}
}
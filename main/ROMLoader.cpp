#include "headers/ROMLoader.h"
#include <FS.h>
#include <LittleFS.h>
#include <string>

#define FORMAT_LITTLEFS_IF_FAILED true

ROMLoader::ROMLoader() {
	loadROMList();
}

bool ROMLoader::loadROM(String &romPath, uint8_t *mem) {

	if (!LittleFS.begin(true)) {
		Serial.println("Error montando LittleFS");
		return false;
	}

	File archivo = LittleFS.open(romPath.c_str(), "r");
	if (!archivo) {
		throw String("No se pudo abrir el archivo rom correctamente");
		return false;
	}

	uint16_t addr = 0x200;

	while (archivo.available()) {

		uint8_t byteLeido = archivo.read();
		mem[addr++] = byteLeido;
	}

	archivo.close();

	return true;
}

void ROMLoader::loadROMList() {
	if (!LittleFS.begin(true)) {
		Serial.println("Error montando LittleFS");
		return;
	}

	File root = LittleFS.open("/");
	File file = root.openNextFile();
	while (file) {
		String name = file.name();
		if (name.endsWith(".ch8")) {
			roms[romCount++] = name;
		}
		file = root.openNextFile();
	}

	if (romCount == 0) {
		Serial.println("No se encontraron ROMs!");
	}
}

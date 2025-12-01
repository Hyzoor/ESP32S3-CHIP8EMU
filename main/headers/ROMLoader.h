#ifndef ROMLOADER_H
#define ROMLOADER_H
#include <Arduino.h>

class ROMLoader {

  public:
	ROMLoader();
	bool loadROM(String &romPath, uint8_t *mem);
	void loadROMList();
	String roms[20]; // máximo 20 ROMs
	int romCount = 0;

  private:
};

#endif
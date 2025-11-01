#ifndef MEMORY_H
#define MEMORY_H
#include <cstdint>

class Memory {

  public:
	Memory();
	~Memory();

	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	uint8_t *getFirstPosition();

  private:
	uint8_t memoryArray[4096]{};
	void initSprites();
};

#endif

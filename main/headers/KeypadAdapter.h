#ifndef KEYPADADAPTER_H
#define KEYPADADAPTER_H
#include <cstdint>

#define NUM_KEYS 16

class KeypadAdapter {
  public:
	KeypadAdapter();
	~KeypadAdapter();

	void setKey(uint8_t key, bool pressed);
	bool isKeyPressed(uint8_t key);
	const bool *getKeys();

  private:
	bool keys[NUM_KEYS]{};
};

#endif
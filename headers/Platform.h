#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define VIDEO_WIDTH 64
#define VIDEO_HEIGHT 32
#define PIXEL_SCALE 2

class Platform {
  public:
	Platform(Adafruit_ST7735 &tft);
	~Platform();

	void init();
	void UpdateScreen(uint32_t *buffer);

  private:
	int scaledWidth = VIDEO_WIDTH * PIXEL_SCALE;
	int scaledHeight = VIDEO_HEIGHT * PIXEL_SCALE;

	int offsetX = 0;
	int offsetY = 0;
	uint8_t lastBuffer[VIDEO_WIDTH * VIDEO_HEIGHT]; // buffer anterior
	Adafruit_ST7735 &tft;
};

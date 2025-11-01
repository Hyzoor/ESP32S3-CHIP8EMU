#include "../headers/Platform.h"

Platform::Platform(Adafruit_ST7735 &screen) : tft(screen) {}

Platform::~Platform() {}

void Platform::init() {
	tft.initR(INITR_BLACKTAB);
	tft.setRotation(1);
	tft.fillScreen(ST77XX_BLACK);
	offsetX = (tft.width() - scaledWidth) / 2;
	offsetY = (tft.height() - scaledHeight) / 2;
}

void Platform::UpdateScreen(uint32_t *buffer) {

	for (int y = 0; y < VIDEO_HEIGHT; y++) {
		for (int x = 0; x < VIDEO_WIDTH; x++) {
			int idx = y * VIDEO_WIDTH + x;
			if (buffer[idx] != lastBuffer[idx]) {
				lastBuffer[idx] = buffer[idx]; // LOCURA DE OPTIMIZACION!!, SOLO DIBUJO SI HA CAMBIADO EL PIXEL
				uint16_t color = buffer[idx] ? ST77XX_YELLOW : ST77XX_BLACK;
				tft.fillRect(offsetX + x * PIXEL_SCALE, offsetY + y * PIXEL_SCALE, PIXEL_SCALE, PIXEL_SCALE, color);
			}
		}
	}
}

/* GxEPD2-style display facade for the Xteink X3 (BOARD_XTEINK_X3 builds).
 *
 * Adapts the vendored FreeInk SDK panel stack (EpdBus + Uc8253X3Driver /
 * Uc8279Driver, runtime-detected -- newer X3 units ship UC8279d silicon on
 * the same glass) to the small subset of the GxEPD2_BW API this project's
 * renderer uses: Adafruit_GFX drawing plus init / firstPage / nextPage /
 * fillScreen / drawInvertedBitmap / hibernate.
 *
 * Full-frame single-page mode only: the 792x528 1bpp framebuffer is 52KB,
 * which the ESP32-C3's RAM holds comfortably, so firstPage()/nextPage()
 * run the paged-drawing do-loop exactly once.
 *
 * This file is part of esp32-weather-epd (GPL-3.0); the vendored FreeInk
 * sources it wraps are MIT (see LICENSE in this library's root).
 */

#ifndef __X3_DISPLAY_H__
#define __X3_DISPLAY_H__

#include <Adafruit_GFX.h>
#include <Arduino.h>

// GxEPD2 color constants for code that is shared with the GxEPD2 panels.
// On this mono panel every non-white color renders black.
#ifndef GxEPD_BLACK
#define GxEPD_BLACK 0x0000
#define GxEPD_WHITE 0xFFFF
#define GxEPD_RED 0xF800
#define GxEPD_YELLOW 0xFFE0
#define GxEPD_GREEN 0x07E0
#define GxEPD_BLUE 0x001F
#define GxEPD_ORANGE 0xFD20
#endif

namespace freeink
{
class EpdBus;
class PanelDriver;
}

class X3Display : public Adafruit_GFX
{
public:
  static const uint16_t WIDTH = 792;
  static const uint16_t HEIGHT = 528;

  X3Display();

  // GxEPD2_BW-compatible entry points used by the project
  void init(uint32_t serial_diag_bitrate, bool initial = true,
            uint16_t reset_duration = 10, bool pulldown_rst_mode = false);
  void setFullWindow() {}
  void firstPage();
  bool nextPage(); // pushes the frame + refreshes; always returns false
  void fillScreen(uint16_t color) override;
  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  void drawInvertedBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                          int16_t w, int16_t h, uint16_t color);
  void hibernate();
  void powerOff();

private:
  uint8_t *_fb;
  freeink::EpdBus *_bus;
  freeink::PanelDriver *_drv;
  bool _begun;
};

#endif

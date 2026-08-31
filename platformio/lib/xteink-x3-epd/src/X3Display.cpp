/* GxEPD2-style display facade for the Xteink X3. See X3Display.h. */

#include "X3Display.h"

#include <BoardConfig.h>
#include <XteinkDetect.h>
#include "bus/EpdBus.h"
#include "driver/PanelDriver.h"
#include "driver/Uc8253X3Driver.h"
#include "driver/Uc8279Driver.h"

using namespace freeink;

X3Display::X3Display()
    : Adafruit_GFX(WIDTH, HEIGHT), _fb(nullptr), _bus(nullptr),
      _drv(nullptr), _begun(false)
{
}

void X3Display::init(uint32_t serial_diag_bitrate, bool initial,
                     uint16_t reset_duration, bool pulldown_rst_mode)
{
  (void)serial_diag_bitrate;
  (void)initial;
  (void)reset_duration;
  (void)pulldown_rst_mode;
  if (_begun)
  {
    return;
  }
  if (!_fb)
  {
    _fb = static_cast<uint8_t *>(malloc(WIDTH / 8 * HEIGHT));
  }
  memset(_fb, 0xFF, WIDTH / 8 * HEIGHT); // white

  // Newer X3 production units carry a UC8279d instead of the UC8253 on the
  // same board/glass; fingerprint the controller before bring-up (the probe
  // releases the pins afterwards).
  if (detectX3DisplayController() == X3DisplayVerdict::Uc8279Confirmed)
  {
    BoardConfig::selectDevice(BoardConfig::Board::XteinkX3Uc8279);
    Serial.println("[x3] UC8279d display controller detected");
    _drv = new Uc8279Driver();
  }
  else
  {
    BoardConfig::selectDevice(BoardConfig::Board::XteinkX3);
    Serial.println("[x3] UC8253 display controller assumed");
    _drv = new Uc8253X3Driver();
  }

  const auto &dp = BoardConfig::ACTIVE.display;
  EpdPins pins;
  pins.sclk = dp.sclk;
  pins.mosi = dp.mosi;
  pins.cs = dp.cs;
  pins.dc = dp.dc;
  pins.rst = dp.rst;
  pins.busy = dp.busy;
  pins.powerEnable = dp.powerEnable;
  _bus = new EpdBus();
  _bus->begin(pins, _drv->spiHz(), _drv->busyPolarity());
  _drv->begin(*_bus);
  _begun = true;
} // end init

void X3Display::firstPage()
{
  fillScreen(GxEPD_WHITE);
} // end firstPage

bool X3Display::nextPage()
{
  // Single full-frame page: push the buffer and run a full (GC) refresh.
  _drv->display(*_bus, _fb, nullptr, RefreshMode::Full, false);
  return false;
} // end nextPage

void X3Display::fillScreen(uint16_t color)
{
  memset(_fb, (color == GxEPD_WHITE) ? 0xFF : 0x00, WIDTH / 8 * HEIGHT);
} // end fillScreen

void X3Display::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT || !_fb)
  {
    return;
  }
  uint32_t i = static_cast<uint32_t>(y) * (WIDTH / 8) + (x >> 3);
  if (color == GxEPD_WHITE)
  {
    _fb[i] |= (0x80 >> (x & 7));
  }
  else
  {
    _fb[i] &= ~(0x80 >> (x & 7));
  }
} // end drawPixel

void X3Display::drawInvertedBitmap(int16_t x, int16_t y,
                                   const uint8_t *bitmap, int16_t w,
                                   int16_t h, uint16_t color)
{
  // GxEPD2 semantics: draw `color` where the bitmap bit is 0.
  int16_t wb = (w + 7) / 8;
  for (int16_t j = 0; j < h; ++j)
  {
    for (int16_t i = 0; i < w; ++i)
    {
      uint8_t b = pgm_read_byte(&bitmap[j * wb + (i >> 3)]);
      if (!(b & (0x80 >> (i & 7))))
      {
        drawPixel(x + i, y + j, color);
      }
    }
  }
} // end drawInvertedBitmap

void X3Display::hibernate()
{
  if (_begun && _drv)
  {
    _drv->deepSleep(*_bus);
  }
} // end hibernate

void X3Display::powerOff()
{
  hibernate();
} // end powerOff

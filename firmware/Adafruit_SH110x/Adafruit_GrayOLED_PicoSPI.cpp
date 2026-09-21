/*!
 * @file Adafruit_GrayOLED.cpp
 *
 * This is documentation for Adafruit's generic library for grayscale
 * OLED displays: http://www.adafruit.com/category/63_98
 *
 * These displays use I2C or SPI to communicate. I2C requires 2 pins
 * (SCL+SDA) and optionally a RESET pin. SPI requires 4 pins (MOSI, SCK,
 * select, data/command) and optionally a reset pin. Hardware SPI or
 * 'bitbang' software SPI are both supported.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 */

// Not for ATtiny, at all
#if !defined(__AVR_ATtiny85__) && !defined(__AVR_ATtiny84__)

#include "Adafruit_GrayOLED_PicoSPI.h"
#include <Adafruit_GFX.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include <algorithm>
#include <cstring>


// SOME DEFINES AND STATIC VARIABLES USED INTERNALLY -----------------------

#define grayoled_swap(a, b)                                                    \
  (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) ///< No-temp-var swap operation

// CONSTRUCTORS, DESTRUCTOR ------------------------------------------------

/*!
    @brief  Constructor for SPI GrayOLED displays, using software (bitbang)
            SPI.
    @param  bpp Bits per pixel, 1 for monochrome, 4 for 16-gray
    @param  w
            Display width in pixels
    @param  h
            Display height in pixels
    @param  mosi_pin
            MOSI (master out, slave in) pin (using Arduino pin numbering).
            This transfers serial data from microcontroller to display.
    @param  sclk_pin
            SCLK (serial clock) pin (using Arduino pin numbering).
            This clocks each bit from MOSI.
    @param  dc_pin
            Data/command pin (using Arduino pin numbering), selects whether
            display is receiving commands (low) or data (high).
    @param  rst_pin
            Reset pin (using Arduino pin numbering), or -1 if not used
            (some displays might be wired to share the microcontroller's
            reset pin).
    @param  cs_pin
            Chip-select pin (using Arduino pin numbering) for sharing the
            bus with other devices. Active low.
    @note   Call the object's begin() function before use -- buffer
            allocation is performed there!
*/
Adafruit_GrayOLED::Adafruit_GrayOLED(uint8_t bpp, uint16_t w, uint16_t h,
                                     int8_t mosi_pin, int8_t sclk_pin,
                                     int8_t dc_pin, int8_t rst_pin,
                                     int8_t cs_pin, spi_inst_t *spi , uint baud_rate)
    : Adafruit_GFX(w, h), dcPin(dc_pin), csPin(cs_pin), rstPin(rst_pin),
      _bpp(bpp) {

      spi_inst = spi;
      spi_init(spi_inst, baud_rate);
      gpio_set_function(mosi_pin, GPIO_FUNC_SPI);
      gpio_set_function(sclk_pin, GPIO_FUNC_SPI);
      gpio_set_function(cs_pin, GPIO_FUNC_SPI);
}

/*!
    @brief  Destructor for Adafruit_GrayOLED object.
*/
Adafruit_GrayOLED::~Adafruit_GrayOLED(void) {
  if (buffer) {
    free(buffer);
    buffer = NULL;
  }
}

// LOW-LEVEL UTILS ---------------------------------------------------------

/*!
    @brief Issue single command byte to OLED, using I2C or hard/soft SPI as
   needed.
    @param c The single byte command
*/
void Adafruit_GrayOLED::oled_command(uint8_t c) {

    gpio_put(dcPin, 0);
    cs_select();
    spi_write_blocking(spi_inst, &c, 1);
    cs_deselect();
}

// Issue list of commands to GrayOLED
/*!
    @brief Issue multiple bytes of commands OLED, using I2C or hard/soft SPI as
   needed.
    @param c Pointer to the command array
    @param n The number of bytes in the command array
    @returns True for success on ability to write the data in I2C.
*/

bool Adafruit_GrayOLED::oled_commandList(const uint8_t *c, uint8_t n)
{
  gpio_put(dcPin, 0);
  cs_select();
  int rc = spi_write_blocking(spi_inst, c, n);
  cs_deselect();

  return rc == n;
}

// ALLOCATE & INIT DISPLAY -------------------------------------------------

/*!
    @brief  Allocate RAM for image buffer, initialize peripherals and pins.
            Note that subclasses must call this before other begin() init
    @param  addr
            I2C address of corresponding oled display.
            SPI displays (hardware or software) do not use addresses, but
            this argument is still required. Default if unspecified is 0x3C.
    @param  reset
            If true, and if the reset pin passed to the constructor is
            valid, a hard reset will be performed before initializing the
            display. If using multiple oled displays on the same bus, and
            if they all share the same reset pin, you should only pass true
            on the first display being initialized, false on all others,
            else the already-initialized displays would be reset. Default if
            unspecified is true.
    @return true on successful allocation/init, false otherwise.
            Well-behaved code should check the return value before
            proceeding.
    @note   MUST call this function before any drawing or updates!
*/
bool Adafruit_GrayOLED::_init(uint8_t addr, bool reset) {

  // attempt to malloc the bitmap framebuffer
  if ((!buffer) &&
      !(buffer = (uint8_t *)malloc(_bpp * WIDTH * ((HEIGHT + 7) / 8)))) {
    return false;
  }

  // Reset OLED if requested and reset pin specified in constructor
  if (reset && (rstPin >= 0)) {

    gpio_init(rstPin);
    gpio_set_dir(rstPin, GPIO_OUT);
    gpio_put(rstPin, 1);
    sleep_ms(10);
    gpio_put(rstPin, 0); // reset
    sleep_ms(10);
    gpio_put(rstPin, 1);
    sleep_ms(10);
  }

  gpio_init(dcPin);
  gpio_set_dir(dcPin, GPIO_OUT);

  clearDisplay();

  // set max dirty window
  window_x1 = 0;
  window_y1 = 0;
  window_x2 = WIDTH - 1;
  window_y2 = HEIGHT - 1;

  return true; // Success
}

// DRAWING FUNCTIONS -------------------------------------------------------

/*!
    @brief  Set/clear/invert a single pixel. This is also invoked by the
            Adafruit_GFX library in generating many higher-level graphics
            primitives.
    @param  x
            Column of display -- 0 at left to (screen width - 1) at right.
    @param  y
            Row of display -- 0 at top to (screen height -1) at bottom.
    @param  color
            Pixel color, one of: MONOOLED_BLACK, MONOOLED_WHITE or
   MONOOLED_INVERT.
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_GrayOLED::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
    // Pixel is in-bounds. Rotate coordinates if needed.
    switch (getRotation()) {
    case 1:
      grayoled_swap(x, y);
      x = WIDTH - x - 1;
      break;
    case 2:
      x = WIDTH - x - 1;
      y = HEIGHT - y - 1;
      break;
    case 3:
      grayoled_swap(x, y);
      y = HEIGHT - y - 1;
      break;
    }

    // adjust dirty window
    window_x1 = std::min(window_x1, x);
    window_y1 = std::min(window_y1, y);
    window_x2 = std::max(window_x2, x);
    window_y2 = std::max(window_y2, y);

    if (_bpp == 1) {
      switch (color) {
      case MONOOLED_WHITE:
        buffer[x + (y / 8) * WIDTH] |= (1 << (y & 7));
        break;
      case MONOOLED_BLACK:
        buffer[x + (y / 8) * WIDTH] &= ~(1 << (y & 7));
        break;
      case MONOOLED_INVERSE:
        buffer[x + (y / 8) * WIDTH] ^= (1 << (y & 7));
        break;
      }
    }
    if (_bpp == 4) {
      uint8_t *pixelptr = &buffer[x / 2 + (y * WIDTH / 2)];
      // Serial.printf("(%d, %d) -> offset %d\n", x, y, x/2 + (y * WIDTH / 2));
      if (x % 2 == 0) { // even, left nibble
        uint8_t t = pixelptr[0] & 0x0F;
        t |= (color & 0xF) << 4;
        pixelptr[0] = t;
      } else { // odd, right lower nibble
        uint8_t t = pixelptr[0] & 0xF0;
        t |= color & 0xF;
        pixelptr[0] = t;
      }
    }
  }
}

/*!
    @brief  Clear contents of display buffer (set all pixels to off).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Adafruit_GrayOLED::clearDisplay(void) {
  memset(buffer, 0, _bpp * WIDTH * ((HEIGHT + 7) / 8));
  // set max dirty window
  window_x1 = 0;
  window_y1 = 0;
  window_x2 = WIDTH - 1;
  window_y2 = HEIGHT - 1;
}

/*!
    @brief  Return color of a single pixel in display buffer.
    @param  x
            Column of display -- 0 at left to (screen width - 1) at right.
    @param  y
            Row of display -- 0 at top to (screen height -1) at bottom.
    @return true if pixel is set (usually MONOOLED_WHITE, unless display invert
   mode is enabled), false if clear (MONOOLED_BLACK).
    @note   Reads from buffer contents; may not reflect current contents of
            screen if display() has not been called.
*/
bool Adafruit_GrayOLED::getPixel(int16_t x, int16_t y) {
  if ((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
    // Pixel is in-bounds. Rotate coordinates if needed.
    switch (getRotation()) {
    case 1:
      grayoled_swap(x, y);
      x = WIDTH - x - 1;
      break;
    case 2:
      x = WIDTH - x - 1;
      y = HEIGHT - y - 1;
      break;
    case 3:
      grayoled_swap(x, y);
      y = HEIGHT - y - 1;
      break;
    }
    return (buffer[x + (y / 8) * WIDTH] & (1 << (y & 7)));
  }
  return false; // Pixel out of bounds
}

/*!
    @brief  Get base address of display buffer for direct reading or writing.
    @return Pointer to an unsigned 8-bit array, column-major, columns padded
            to full byte boundary if needed.
*/
uint8_t *Adafruit_GrayOLED::getBuffer(void) { return buffer; }

// OTHER HARDWARE SETTINGS -------------------------------------------------

/*!
    @brief  Enable or disable display invert mode (white-on-black vs
            black-on-white). Handy for testing!
    @param  i
            If true, switch to invert mode (black-on-white), else normal
            mode (white-on-black).
    @note   This has an immediate effect on the display, no need to call the
            display() function -- buffer contents are not changed, rather a
            different pixel mode of the display hardware is used. When
            enabled, drawing MONOOLED_BLACK (value 0) pixels will actually draw
   white, MONOOLED_WHITE (value 1) will draw black.
*/
void Adafruit_GrayOLED::invertDisplay(bool i) {
  oled_command(i ? GRAYOLED_INVERTDISPLAY : GRAYOLED_NORMALDISPLAY);
}

/*!
    @brief  Adjust the display contrast.
    @param  level The contrast level from 0 to 0x7F
    @note   This has an immediate effect on the display, no need to call the
            display() function -- buffer contents are not changed.
*/
void Adafruit_GrayOLED::setContrast(uint8_t level) {
  uint8_t cmd[] = {GRAYOLED_SETCONTRAST, level};
  oled_commandList(cmd, 2);
}

#endif /* ATTIN85 not supported */

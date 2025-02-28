/*!
 * @file Adafruit_SH110X.cpp
 *
 * @mainpage Arduino library for monochrome OLEDs based on SH110X drivers.
 *
 * @section intro_sec Introduction
 *
 * This is documentation for Adafruit's SH110X library for monochrome
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
 * @section dependencies Dependencies
 *
 * This library depends on <a
 * href="https://github.com/adafruit/Adafruit-GFX-Library"> Adafruit_GFX</a>
 * being present on your system. Please make sure you have installed the latest
 * version before using this library.
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries, with
 * contributions from the open source community.
 *
 * @section license License
 *
 * BSD license, all text above, and the splash screen included below,
 * must be included in any redistribution.
 *
 */

#include "Adafruit_SH110X.h"
#include "splash.h"
#include "algorithm"
#include "pico/stdlib.h"

// CONSTRUCTORS, DESTRUCTOR ------------------------------------------------

/*!
    @brief  Constructor for SPI SH110X displays, using software (bitbang)
            SPI.
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
Adafruit_SH110X::Adafruit_SH110X(uint16_t w, uint16_t h, int16_t mosi_pin,
                                 int16_t sclk_pin, int16_t dc_pin,
                                 int16_t rst_pin, int16_t cs_pin, spi_inst_t *spi , uint baud_rate)
    : Adafruit_GrayOLED(1, w, h, mosi_pin, sclk_pin, dc_pin, rst_pin, cs_pin, spi, baud_rate) {}

/*!
    @brief  Destructor for Adafruit_SH110X object.
*/
Adafruit_SH110X::~Adafruit_SH110X(void) {}

// REFRESH DISPLAY ---------------------------------------------------------

/*!
    @brief  Push data currently in RAM to SH110X display.
    @note   Drawing operations are not visible until this function is
            called. Call after each graphics command, or after a whole set
            of graphics commands, as best needed by one's own application.
*/
void Adafruit_SH110X::display(void) {
  // ESP8266 needs a periodic yield() call to avoid watchdog reset.
  // With the limited size of SH110X displays, and the fast bitrate
  // being used (1 MHz or more), I think one yield() immediately before
  // a screen write and one immediately after should cover it.  But if
  // not, if this becomes a problem, yields() might be added in the
  // 32-byte transfer condition below.
  //yield();

  // uint16_t count = WIDTH * ((HEIGHT + 7) / 8);
  uint8_t *ptr = buffer;
  uint8_t dc_byte = 0x40;
  uint8_t pages = ((HEIGHT + 7) / 8);

  uint8_t bytes_per_page = WIDTH;

  /*
  Serial.print("Window: (");
  Serial.print(window_x1);
  Serial.print(", ");
  Serial.print(window_y1);
  Serial.print(" -> (");
  Serial.print(window_x2);
  Serial.print(", ");
  Serial.print(window_y2);
  Serial.println(")");
  */

  uint8_t first_page = window_y1 / 8;
  //  uint8_t last_page = (window_y2 + 7) / 8;
  uint8_t page_start = std::min(bytes_per_page, (uint8_t)window_x1);
  uint8_t page_end = (uint8_t)std::max((int)0, (int)window_x2);
  /*
  Serial.print("Pages: ");
  Serial.print(first_page);
  Serial.print(" -> ");
  Serial.println(last_page);
  pages = min(pages, last_page);

  Serial.print("Page addr: ");
  Serial.print(page_start);
  Serial.print(" -> ");
  Serial.println(page_end);
  */

  for (uint8_t p = first_page; p < pages; p++) {
    uint8_t bytes_remaining = bytes_per_page;
    ptr = buffer + (uint16_t)p * (uint16_t)bytes_per_page;
    // fast forward to dirty rectangle beginning
    ptr += page_start;
    bytes_remaining -= page_start;
    // cut off end of dirty rectangle
    bytes_remaining -= (WIDTH - 1) - page_end;

    uint8_t cmd[] = {
        (uint8_t)(SH110X_SETPAGEADDR + p),
        (uint8_t)(0x10 + ((page_start + _page_start_offset) >> 4)),
        (uint8_t)((page_start + _page_start_offset) & 0xF)};

      gpio_put(dcPin, 0);

      cs_select();
      spi_write_blocking(spi_inst, cmd, 3);
      cs_deselect();

      gpio_put(dcPin, 1);

      cs_select();
      spi_write_blocking(spi_inst, ptr, bytes_remaining);
      cs_deselect();
  }

  // reset dirty window
  window_x1 = 1024;
  window_y1 = 1024;
  window_x2 = -1;
  window_y2 = -1;
}

/*!
 * @file Adafruit_SH1106G.cpp
 *
 */

#include "Adafruit_SH110X.h"
#include "oe-splash.h"

// CONSTRUCTORS, DESTRUCTOR ------------------------------------------------

/*!
    @brief  Constructor for SPI SH1106G displays, using software (bitbang)
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
Adafruit_SH1106G::Adafruit_SH1106G(uint16_t w, uint16_t h, int16_t mosi_pin,
                                   int16_t sclk_pin, int16_t dc_pin,
                                   int16_t rst_pin, int16_t cs_pin, spi_inst_t *spi , uint baud_rate)
    : Adafruit_SH110X(w, h, mosi_pin, sclk_pin, dc_pin, rst_pin, cs_pin, spi, baud_rate) {}

/*!
    @brief  Destructor for Adafruit_SH1106G object.
*/
Adafruit_SH1106G::~Adafruit_SH1106G(void) {}

/*!
    @brief  Allocate RAM for image buffer, initialize peripherals and pins.
    @param  addr
            I2C address of corresponding SH110X display (or pass 0 to use
            default of 0x3C for 128x32 display, 0x3D for all others).
            SPI displays (hardware or software) do not use addresses, but
            this argument is still required (pass 0 or any value really,
            it will simply be ignored). Default if unspecified is 0.
    @param  reset
            If true, and if the reset pin passed to the constructor is
            valid, a hard reset will be performed before initializing the
            display. If using multiple SH110X displays on the same bus, and
            if they all share the same reset pin, you should only pass true
            on the first display being initialized, false on all others,
            else the already-initialized displays would be reset. Default if
            unspecified is true.
    @return true on successful allocation/init, false otherwise.
            Well-behaved code should check the return value before
            proceeding.
    @note   MUST call this function before any drawing or updates!
*/
bool Adafruit_SH1106G::begin(uint8_t addr, bool reset) {

  Adafruit_GrayOLED::_init(addr, reset);

  _page_start_offset =
      2; // the SH1106 display we have found requires a small offset into memory

#ifndef SH110X_NO_SPLASH
  drawBitmap((WIDTH - OE_SPLASH_WIDTH) / 2, (HEIGHT - OE_SPLASH_HEIGHT) / 2,
             oe_splash_data, OE_SPLASH_WIDTH, OE_SPLASH_HEIGHT, 1);
#endif

  // Init sequence, make sure its under 32 bytes, or split into multiples!
  // clang-format off
  static const uint8_t init[] = {
      SH110X_DISPLAYOFF,               // 0xAE
      SH110X_SETDISPLAYCLOCKDIV, 0x80, // 0xD5, 0x80,
      SH110X_SETMULTIPLEX, 0x3F,       // 0xA8, 0x3F,
      SH110X_SETDISPLAYOFFSET, 0x00,   // 0xD3, 0x00,
      SH110X_SETSTARTLINE,             // 0x40
      SH110X_DCDC, 0x8B,               // DC/DC on
      SH110X_SEGREMAP + 1,             // 0xA1
      SH110X_COMSCANDEC,               // 0xC8
      SH110X_SETCOMPINS, 0x12,         // 0xDA, 0x12,
      SH110X_SETCONTRAST, 0xFF,        // 0x81, 0xFF
      SH110X_SETPRECHARGE, 0x1F,       // 0xD9, 0x1F,
      SH110X_SETVCOMDETECT, 0x40,      // 0xDB, 0x40,
      0x33,                            // Set VPP to 9V
      SH110X_NORMALDISPLAY,
      SH110X_MEMORYMODE, 0x10,         // 0x20, 0x00
      SH110X_DISPLAYALLON_RESUME,
  };
  // clang-format on

  if (!oled_commandList(init, sizeof(init))) {
    return false;
  }

  sleep_ms(100);                     // 100ms delay recommended
  oled_command(SH110X_DISPLAYON); // 0xaf

  return true; // Success
}

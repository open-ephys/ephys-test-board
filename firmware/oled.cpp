#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "oled.h"
#include "ephys-tester.h"

#include "Adafruit_SH110X.h"

static Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT,
    OLED_MOSI, OLED_SCLK, OLED_nDC, OLED_nRES, OLED_nCS, OLED_SPI);

static void print_mode(const mode_context_t *const ctx)
{
    display.print("Mode:");
    if (mode_selection(ctx) == SELECTION_MODE) display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print(" ");
    display.print(mode_str(ctx, SELECTION_MODE));
    display.println(" ");
    if (mode_selection(ctx) == SELECTION_MODE)  display.setTextColor(SH110X_WHITE, SH110X_BLACK); 
}

static void print_channel(const mode_context_t *const ctx)
{
    display.print(" Channel:");
    if (mode_selection(ctx) == SELECTION_CHANNEL) display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print(" ");
    display.print(mode_str(ctx, SELECTION_CHANNEL));
    display.println(" ");
    if (mode_selection(ctx) == SELECTION_CHANNEL) display.setTextColor(SH110X_WHITE, SH110X_BLACK);
}

static void print_signal(const mode_context_t *const ctx)
{
    display.print("Signal:");
    if (mode_selection(ctx) == SELECTION_WAVEFORM) display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print(" ");
    display.print(mode_str(ctx, SELECTION_WAVEFORM));
    display.println(" ");
    if (mode_selection(ctx) == SELECTION_WAVEFORM) display.setTextColor(SH110X_WHITE, SH110X_BLACK);
}

static void print_amplitude_uV(const mode_context_t *const ctx)
{
    display.print(" Amp. (uV):");
    if (mode_selection(ctx) == SELECTION_AMPLITUDE) display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print(" ");
    display.print(mode_str(ctx, SELECTION_AMPLITUDE));
    display.println(" ");
    if (mode_selection(ctx) == SELECTION_AMPLITUDE) display.setTextColor(SH110X_WHITE, SH110X_BLACK);
}

static void print_freq_hz(const mode_context_t *const ctx)
{
    display.print(" Freq. (Hz):");
    if (mode_selection(ctx) == SELECTION_FREQHZ) display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print(" ");
    display.print(mode_str(ctx, SELECTION_FREQHZ));
    display.println(" ");
    if (mode_selection(ctx) == SELECTION_FREQHZ) display.setTextColor(SH110X_WHITE, SH110X_BLACK);
}

static void draw_battery(int16_t x, int16_t y, float frac)
{
    uint8_t bm[] = {
        0b01100000,
        0b11110000,
        (frac > 0.75 ? (uint8_t)0b11110000 : (uint8_t)0b10010000), 
        (frac > 0.50 ? (uint8_t)0b11110000 : (uint8_t)0b10010000), 
        (frac > 0.25 ? (uint8_t)0b11110000 : (uint8_t)0b10010000), 
        (frac > 0.10 ? (uint8_t)0b11110000 : (uint8_t)0b10010000), 
        0b11110000,
    };
    display.drawBitmap(x, y, bm, 4, 7, SH110X_WHITE);
}

static void draw_bolt(int16_t x, int16_t y)
{
    static const uint8_t bm[] = {
        0b00010000,
        0b00110000,
        0b01100000,
        0b11111000,
        0b00110000,
        0b01100000,
        0b01000000,
    };
    display.drawBitmap(x, y, bm, 5, 7, SH110X_WHITE);
}

int oled_init()
{
    gpio_init(OLED_PWR); gpio_set_dir(OLED_PWR, GPIO_OUT);
    gpio_put(OLED_PWR, 1);

    // Display initialization
    display.setRotation(2);
    display.begin(0, 1);
    display.display(); // splash
    display.setTextSize(1);
    display.setTextWrap(false);
    display.setTextColor(SH110X_WHITE, SH110X_BLACK); // white text, black background
    
    return 0;
}

int oled_update(const mode_context_t *const ctx)
{
    display.clearDisplay();

    display.setCursor(0, 0);
    display.print(ctx->module.name);

    if (ctx->usb_detected)
        draw_bolt(SCREEN_WIDTH - 5, 0);
    else
        draw_battery(SCREEN_WIDTH - 5, 0, ctx->battery_frac);

    display.drawLine(0, 9, SCREEN_WIDTH, 9, SH110X_WHITE);
    //display.drawLine(SCREEN_WIDTH - 12, 0, SCREEN_WIDTH - 12, 9, SH110X_WHITE);

    display.setCursor(0, 12);
    print_mode(ctx);

    display.setCursor(0, 22);
    print_channel(ctx);

    display.setCursor(0, 32);
    print_signal(ctx);

    display.setCursor(0, 42);
    print_amplitude_uV(ctx);

    display.setCursor(0, 52);
    print_freq_hz(ctx);

    display.display();
    return 0;
}
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "oled.h"
#include "ephys-tester.h"

#include "Adafruit_SH110X.h"

static Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT,
    OLED_MOSI, OLED_SCLK, OLED_nDC, OLED_nRES, OLED_nCS, OLED_SPI);

static volatile bool blink = false;

static void print_mode(const mode_context_t *const ctx)
{
    display.print(title_str(ctx, SELECTION_DEST));
    if (mode_selection(ctx) == SELECTION_DEST) display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print(" ");
    display.print(mode_str(ctx, SELECTION_DEST));
    display.println(" ");
    if (mode_selection(ctx) == SELECTION_DEST)  display.setTextColor(SH110X_WHITE, SH110X_BLACK);
}

static void print_channel(const mode_context_t *const ctx)
{
    display.print(" ");
    display.print(title_str(ctx, SELECTION_CHANNEL));
    if (mode_selection(ctx) == SELECTION_CHANNEL) display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print(" ");
    display.print(mode_str(ctx, SELECTION_CHANNEL));
    display.println(" ");
    if (mode_selection(ctx) == SELECTION_CHANNEL) display.setTextColor(SH110X_WHITE, SH110X_BLACK);
}

static void print_signal(const mode_context_t *const ctx)
{
    display.print(title_str(ctx, SELECTION_WAVEFORM));
    if (mode_selection(ctx) == SELECTION_WAVEFORM) display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print(" ");
    display.print(mode_str(ctx, SELECTION_WAVEFORM));
    display.println(" ");
    if (mode_selection(ctx) == SELECTION_WAVEFORM) display.setTextColor(SH110X_WHITE, SH110X_BLACK);
}

static void print_amplitude(const mode_context_t *const ctx)
{
    display.print(" ");
    display.print(title_str(ctx, SELECTION_AMPLITUDE));
    if (mode_selection(ctx) == SELECTION_AMPLITUDE) display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print(" ");
    display.print(mode_str(ctx, SELECTION_AMPLITUDE));
    display.println(" ");
    if (mode_selection(ctx) == SELECTION_AMPLITUDE) display.setTextColor(SH110X_WHITE, SH110X_BLACK);
}

static void print_offset(const mode_context_t *const ctx)
{
    display.print(" ");
    display.print(title_str(ctx, SELECTION_OFFSET));
    if (mode_selection(ctx) == SELECTION_OFFSET) display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print(" ");
    display.print(mode_str(ctx, SELECTION_OFFSET));
    display.println(" ");
    if (mode_selection(ctx) == SELECTION_OFFSET) display.setTextColor(SH110X_WHITE, SH110X_BLACK);
}

static void print_freq_hz(const mode_context_t *const ctx)
{
    display.print(" ");
    display.print(title_str(ctx, SELECTION_FREQHZ));
    if (mode_selection(ctx) == SELECTION_FREQHZ) display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print(" ");
    display.print(mode_str(ctx, SELECTION_FREQHZ));
    display.println(" ");
    if (mode_selection(ctx) == SELECTION_FREQHZ) display.setTextColor(SH110X_WHITE, SH110X_BLACK);
}

static void draw_menu_item(const mode_context_t *const ctx, mode_selection_t mode, void (*print)(const mode_context_t *const ctx))
{
    const int SUBMENUSTART = 12;
    const int SUBMENUEND = 52;
    const int SUBMENULINEH = 10;

    int line = (int)mode;
    int selection_line =(int)(mode_selection(ctx));
    int position = SUBMENUSTART + line * SUBMENULINEH;
    int selection_position = SUBMENUSTART + selection_line * SUBMENULINEH;

    int offset = selection_position > SUBMENUEND ? selection_position - SUBMENUEND : 0;

    if (position - offset >= SUBMENUSTART && position - offset <= SUBMENUEND) // submenu boundaries
    {
        display.setCursor(0, position - offset);
        print(ctx);
    }
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

static void draw_high_clip(int16_t x, int16_t y, bool invert)
{
    static const uint8_t bm[] = {
        0b11111111, 0b10000000,
        0b00001000, 0b00000000,
        0b00011100, 0b00000000,
        0b00111110, 0b00000000,
        0b00001000, 0b00000000,
        0b00001000, 0b00000000,
        0b00001000, 0b00000000
    };
    display.drawBitmap(x, y, bm, 9, 7, invert ? SH110X_BLACK: SH110X_WHITE);
}

static void draw_low_clip(int16_t x, int16_t y, bool invert)
{
    static const uint8_t bm[] = {
        0b00001000, 0b00000000,
        0b00001000, 0b00000000,
        0b00001000, 0b00000000,
        0b00111110, 0b00000000,
        0b00011100, 0b00000000,
        0b00001000, 0b00000000,
        0b11111111, 0b10000000
    };
    display.drawBitmap(x, y, bm, 9, 7, invert ? SH110X_BLACK: SH110X_WHITE);
}

int oled_init()
{
    gpio_init(OLED_PWR); gpio_set_dir(OLED_PWR, GPIO_OUT);
    gpio_put(OLED_PWR, 1);

    // Display initialization
    display.setRotation(2);
    display.setTextSize(1);
    display.setTextWrap(false);
    display.setTextColor(SH110X_WHITE, SH110X_BLACK); // white text, black background
    display.begin(0, 1);

    display.setCursor(0, 0);
    display.print("v");
    display.print(VERSION_MAJOR);
    display.print(".");
    display.print(VERSION_MINOR);

    display.display(); // splash

    return 0;
}

int oled_update_map_menu(const mode_context_t *const ctx)
{
    display.clearDisplay();

    display.setCursor(0, 0);
    display.print(ctx->module.name);

    if (ctx->usb_detected)
        draw_bolt(SCREEN_WIDTH - 5, 0);
    else
        draw_battery(SCREEN_WIDTH - 5, 0, ctx->battery_frac);

    display.drawLine(0, 9, SCREEN_WIDTH, 9, SH110X_WHITE);

    display.setCursor(0, 16);
    display.print("Select channel map: ");

    display.setCursor(0, 28);
    display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.print(ctx->channel_map.name);
    display.setTextColor(SH110X_WHITE, SH110X_BLACK);

    display.display();

    return 0;
}

int oled_update_main_menu(const mode_context_t *const ctx, bool blink)
{
    display.clearDisplay();

    // Header
    display.setCursor(0, 0);
    display.print(ctx->module.name);

    switch (ctx->clipping)
    {
        case CLIP_LOW:
            draw_low_clip(SCREEN_WIDTH - 9, 0, blink);
            break;
        case CLIP_HIGH:
            draw_high_clip(SCREEN_WIDTH - 9, 0, blink);
            break;
        case CLIP_NONE:
            if (ctx->usb_detected)
                draw_bolt(SCREEN_WIDTH - 5, 0);
            else
                draw_battery(SCREEN_WIDTH - 5, 0, ctx->battery_frac);
            break;
    }
    display.drawLine(0, 9, SCREEN_WIDTH, 9, SH110X_WHITE);

    // Menu
    draw_menu_item(ctx, SELECTION_DEST, print_mode);
    draw_menu_item(ctx, SELECTION_CHANNEL, print_channel);
    draw_menu_item(ctx, SELECTION_WAVEFORM, print_signal);
    draw_menu_item(ctx, SELECTION_OFFSET, print_offset);
    draw_menu_item(ctx, SELECTION_AMPLITUDE, print_amplitude);
    draw_menu_item(ctx, SELECTION_FREQHZ, print_freq_hz);

    display.display();
    return 0;
}
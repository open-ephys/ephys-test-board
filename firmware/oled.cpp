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

// TODO: provide some type of context struct to hold all of the mode state
int oled_update(const mode_context_t *const ctx)
{
    display.clearDisplay();

    display.setCursor(0, 0);
    display.print(ctx->module.name);
    display.println(ctx->module.pcb_rev);
    display.drawLine(0, 9, SCREEN_WIDTH, 9, SH110X_WHITE);
    display.drawLine(SCREEN_WIDTH - 12, 0, SCREEN_WIDTH - 12, 9, SH110X_WHITE);

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
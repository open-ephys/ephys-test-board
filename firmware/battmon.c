#include "battmon.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "ephys-tester.h"
#include <math.h>

#define BATT_VOLTAGE_LOW 3.6f // Low voltage of linear discharge range
#define BATT_VOLTAGE_HIGH 4.0f // High voltage of linear discharge range

static float batt_mon_read_voltage()
{
    const float conversion_factor = 3.3f / (1 << 12);
    return (adc_read() << 1) * conversion_factor; // Read ADC value, account for the 0.5x voltage divider, and scale to volts
}

int batt_mon_init()
{
    adc_init();
    adc_gpio_init(BATT_MON_PIN);
    adc_select_input(BATT_MON_ADC);
    adc_set_temp_sensor_enabled(false); // Disable temperature sensor to save power

    gpio_init(VBUS_DETECT);
    gpio_set_dir(VBUS_DETECT, GPIO_IN);

    return 0;
}

bool batt_mon_monitor(mode_context_t *ctx)
{
    float batt_frac = (batt_mon_read_voltage() - BATT_VOLTAGE_LOW) / (BATT_VOLTAGE_HIGH - BATT_VOLTAGE_LOW);

    bool ctx_changed = false;
    if (fabsf(ctx->battery_frac - batt_frac) > 0.01f) {
        ctx->battery_frac = batt_frac;
        ctx_changed = true;
    } 

    bool usb_detected = gpio_get(VBUS_DETECT);
    if (ctx->usb_detected != usb_detected) {
        ctx->usb_detected = usb_detected;
        ctx_changed = true;
    }

    return ctx_changed;
}

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#include "oled.h"

extern "C"
{
#include "ephys-tester.h"
#include "channels.h"
#include "eeprom.h"
#include "ad5683.h"
#include "quadrature.h"
#include "mode.h"
#include "tiny-json.h"
#include "lut.h"
#include "spikes.h"
#include "sr.h"
}

queue_t signal_generator_cmd_queue;
volatile bool channel_increment_request = false;
volatile bool knob_press_detected = false;

typedef struct timer_callback_data_t {
    pio_spi_inst_t *dac_spi;
    uint16_t rshift;
    int step;
    const uint16_t *lut;
    size_t lut_len;
} timer_callback_data_t;

// WARNING: If this call takes longer than the timer period, it will completely take
// over a core and lock out all other activity (e.g. command dequeueing that
// would cause the timer to stop, etc.).
bool dac_update_callback(struct repeating_timer *t)
{
    static size_t i = 0;
    timer_callback_data_t *timer_data = (timer_callback_data_t *)t->user_data;

    ad5683_write_dac(timer_data->dac_spi, *(timer_data->lut + (i % timer_data->lut_len)), timer_data->rshift);
    i += timer_data->step;

    return true;
}

bool channel_increment_callback(struct repeating_timer *t)
{
    channel_increment_request = true;
    return true;
}

void knob_press_callback(uint gpio, uint32_t events)
{
    knob_press_detected = true;
}

// TODO: Calling cancel_repeating_timer on a timer that has not had alarm added
// to it seems to cause segfault. Is this expected or SDK bug?
// cancel_repeating_timer docs say it checks for existence before cancelling.
// Can we inspect timer object to see if its appropriate to cancel rather than
// holding this first variable?
bool cancel_repeating_timer_safe(repeating_timer_t *timer, bool timer_cancelled)
{
    return timer_cancelled ? true : cancel_repeating_timer(timer);
}

void core1_entry()
{
    // DAC initialization
    pio_spi_inst_t dac_spi = {
        .pio = DAC_PIO,
        .sm = 0,
    };

    ad5683_init(&dac_spi);

    struct repeating_timer timer;
    timer_callback_data_t timer_data;
    mode_signal_t signal;
    bool timer_cancelled = true;
    static int last_timer_usec = 0;
    alarm_pool_t *alarm_pool = alarm_pool_create_with_unused_hardware_alarm(10);

    while (true)
    {
        queue_remove_blocking(&signal_generator_cmd_queue, &signal);

        timer_data.dac_spi = &dac_spi;
        timer_data.rshift = signal.amp_rshift;
        timer_cancelled = cancel_repeating_timer_safe(&timer, timer_cancelled);
        int timer_usec = last_timer_usec;

        switch (signal.waveform)
        {
            case WAVEFORM_GND:
                ad5683_write_dac(&dac_spi, 0, 0);
                sr_source(SIGNAL_INTERNAL);
                continue;
            case WAVEFORM_EXTERNAL:
                ad5683_write_dac(&dac_spi, 0, 0);
                sr_source(SIGNAL_EXTERNAL);
                continue;
            case WAVEFORM_SINE:
                timer_data.lut = SINE_LUT;
                timer_data.lut_len = SINE_LUT_LENGTH;
                timer_data.step = FREQ_LUT[signal.freq_lut_idx][3];
                timer_usec = FREQ_LUT[signal.freq_lut_idx][2];
                sr_source(SIGNAL_INTERNAL);
                break;
            case WAVEFORM_SAW:
                timer_data.lut = SAW_LUT;
                timer_data.lut_len = SAW_LUT_LENGTH;
                timer_data.step = FREQ_LUT[signal.freq_lut_idx][3];
                timer_usec = FREQ_LUT[signal.freq_lut_idx][2];
                sr_source(SIGNAL_INTERNAL);
                break;
            case WAVEFORM_SPIKES:
                timer_data.lut = SPIKES;
                timer_data.lut_len = SPIKES_LENGTH;
                timer_data.step = 1;
                timer_usec = SPIKES_SAMP_PERIOD_USEC;
                sr_source(SIGNAL_INTERNAL);
                break;
            default:
                continue;
        }

        if (timer_cancelled || last_timer_usec != timer_usec)
        {
            timer_cancelled = !alarm_pool_add_repeating_timer_us(alarm_pool, -timer_usec, dac_update_callback, &timer_data, &timer);
            last_timer_usec = timer_usec;
        }
    }
}

int main()
{
    // OLED
    oled_init();

    // Quadrature encoder initialization
    quad_init();

    // Setup user IO context
    mode_context_t ctx;
    mode_init(&ctx);

    // Examine module EEPROM
    eeprom_init();
    eeprom_read_module(&ctx);

    // Channels
    channels_init(&ctx);

    // Queue for conveying settings to waveform generator and send default state
    queue_init(&signal_generator_cmd_queue, sizeof(mode_signal_t), 10);

    // Launch the second core to handle the waveform generator
    multicore_launch_core1(core1_entry);

    // Force first mode update
    mode_update_from_knob(&ctx, quad_get_delta());

    // Setup knob button callback
    gpio_init(ENC_BUT);
    gpio_set_dir(ENC_BUT, GPIO_IN);
    gpio_set_irq_enabled_with_callback(ENC_BUT, GPIO_IRQ_EDGE_RISE, true, &knob_press_callback);

    // Let splash screen hang around for a while
    sleep_ms(1000);
    oled_update(&ctx);

    // Send default state to waveform generator
    queue_add_blocking(&signal_generator_cmd_queue, &(ctx.signal));

    // Auto channel increment timer
    struct repeating_timer channel_timer;
    bool channel_timer_cancelled = true;

    while(true)
    {

        bool update_oled_required = false;

        if (knob_press_detected)
        {
            mode_cycle_selection(&ctx);
            knob_press_detected = false;
            update_oled_required = true;
        }

        if (quad_pending_turn())
        {
            quad_acknowledge_turn();
            int what_changed = mode_update_from_knob(&ctx, quad_get_delta());

            if (what_changed == 1)
                if (ctx.test_dest == TEST_CYCLE_CHANNEL){
                    channel_timer_cancelled = cancel_repeating_timer_safe(&channel_timer, channel_timer_cancelled);
                    channel_timer_cancelled = !add_repeating_timer_ms(-AUTO_CHAN_DWELL_MS, channel_increment_callback, NULL, &channel_timer);
                } else {
                    // NB: Prevent multiple, overlapping timers
                    channel_timer_cancelled = cancel_repeating_timer_safe(&channel_timer, channel_timer_cancelled);
                    channels_update(&ctx);
                }
            else if (what_changed == 2)
                queue_try_add(&signal_generator_cmd_queue, &(ctx.signal));
            else if (what_changed == 3)
                channels_update(&ctx);

            update_oled_required = true;
        }

        // Handle auto channel increment
        if (channel_increment_request)
        {
            channel_increment_request = false;
            mode_increment_channel(&ctx);
            channels_update(&ctx);
            update_oled_required = true;
        }

        if (update_oled_required) {
            oled_update(&ctx);
        }
    }
}

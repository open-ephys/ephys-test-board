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
#include "sr.h"
}

queue_t signal_generator_cmd_queue;
volatile bool channel_increment_request = false;

typedef struct timer_callback_data_t {
    pio_spi_inst_t *dac_spi;
    uint16_t rshift;
    int step;
    int offset;
} timer_callback_data_t;

// WARNING: If this call takes longer than the timer period, it will completely take
// over a core and lock out all other activity (e.g. command dequeueing that
// would cause the timer to stop, etc.).
bool dac_update_callback(struct repeating_timer *t)
{
    static size_t i = 0;
    timer_callback_data_t *timer_data = (timer_callback_data_t *)t->user_data;

    ad5683_write_dac(timer_data->dac_spi, WAVEFORM_LUT[timer_data->offset + (i % LUT_WAVEFORM_LENGTH)], timer_data->rshift);
    i += timer_data->step;
    
    return true;
}

bool channel_increment_callback(struct repeating_timer *t)
{
    channel_increment_request = true;
    return true;
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
        timer_data.step = FREQ_LUT[signal.freq_lut_idx][3];

        timer_cancelled = cancel_repeating_timer_safe(&timer, timer_cancelled);

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
                timer_data.offset = LUT_SINE_OFFSET;
                sr_source(SIGNAL_INTERNAL);
                break;
            case WAVEFORM_SAW:
                timer_data.offset = LUT_SAWTOOTH_OFFSET;
                sr_source(SIGNAL_INTERNAL);
                break;
            case WAVEFORM_SPIKES:
                timer_data.offset = LUT_SPIKES_OFFSET;
                sr_source(SIGNAL_INTERNAL);
                break;
            default:
                continue;
        }

        if (timer_cancelled || last_timer_usec != FREQ_LUT[signal.freq_lut_idx][2])
        {
            timer_cancelled = !alarm_pool_add_repeating_timer_us(alarm_pool, -FREQ_LUT[signal.freq_lut_idx][2], dac_update_callback, &timer_data, &timer);
            last_timer_usec = FREQ_LUT[signal.freq_lut_idx][2];
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
    mode_update_from_knob(&ctx, quad_get_delta(), quad_get_button());

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
        // Decode user input
        if (quad_pending_turn())
        {
            quad_acknowledge_turn();
            int what_changed = mode_update_from_knob(&ctx, quad_get_delta(), quad_get_button());

            if (what_changed == 1)  
                if (ctx.test_dest == TEST_CYCLE_CHANNEL){
                    channel_timer_cancelled = cancel_repeating_timer_safe(&channel_timer, channel_timer_cancelled);
                    channel_timer_cancelled = !add_repeating_timer_ms(-AUTO_CHAN_DWELL_MS, channel_increment_callback, NULL, &channel_timer);
                } else {
                    // NB: Prevent multiple, overlapping timers
                    channel_timer_cancelled = cancel_repeating_timer_safe(&channel_timer, channel_timer_cancelled);
                }
            else if (what_changed == 2) 
                queue_try_add(&signal_generator_cmd_queue, &(ctx.signal));
            else if (what_changed == 3)
                channels_update(&ctx);

            oled_update(&ctx);
        }

        // Handle auto channel increment
        if (channel_increment_request)
        {   
            channel_increment_request = false;
            mode_increment_channel(&ctx);
            channels_update(&ctx);
            oled_update(&ctx);
        }
    }
}

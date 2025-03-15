#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#include "oled.h"

extern "C"
{
#include "ephys-tester.h"
#include "channels.h"
#include "ad5683.h"
#include "quadrature.h"
#include "mode.h"

//#include "parse.h"
#include "tiny-json.h"
#include "lut.h"
}

// TODO: Create command specification
// #define STR_BUFFER_BYTES    256
// #define MAX_COMMANDS        256

// int64_t alarm_callback(alarm_id_t id, __unused void *user_data) {
//     printf("Timer %d fired!\n", (int) id);
//     timer_fired = true;
//     // Can return a value here in us to fire in the future
//     return 0;
// }


queue_t signal_generator_cmd_queue;

typedef struct timer_callback_data_t {
    pio_spi_inst_t *dac_spi;
    float scale;
    int step;
    int offset;
} timer_callback_data_t;

bool repeating_timer_callback(struct repeating_timer *t) {

    static size_t i = 0;
    timer_callback_data_t *timer_data = (timer_callback_data_t *)t->user_data;
    ad5683_write_dac(timer_data->dac_spi, SINE[i % LUT_LENGTH]);
    i += timer_data->step;
    return true;
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
    bool first = true;

    while (true)
    {
        queue_remove_blocking(&signal_generator_cmd_queue, &signal);
        timer_data.dac_spi = &dac_spi;
        timer_data.scale = 1.0; // TODO
        timer_data.step = FREQ_LUT[signal.freq_lut_idx][3];

    //     switch (signal.waveform)
    //     {
    //         WAVEFORM_GND,
    //         WAVEFORM_EXTERNAL,
    //             ad5683_write_dac(&dac_spi, 0);


    //         WAVEFORM_SINE,
    // WAVEFORM_SAW,
    // WAVEFORM_SPIKES,



    //         case SIGNAL
    //     }

        timer_data.offset = (int)signal.waveform; // TODO

        if (!first)
        {
            cancel_repeating_timer(&timer);
        }

        add_repeating_timer_us(-FREQ_LUT[signal.freq_lut_idx][2], repeating_timer_callback, &timer_data, &timer);
        first = false;
    }
}

int main()
{
    // OLED
    oled_init();

    // Quadrature encoder initialization
    quad_init();

    // Setup user IO context
    mode_context_t mode_ctx;
    mode_init(&mode_ctx);

    // Channels
    channels_init(&mode_ctx);

    // Queue for conveying settings to waveform generator
    queue_init(&signal_generator_cmd_queue, sizeof(mode_signal_t), 10);

    // Launch the second core to handle the waveform generator
    multicore_launch_core1(core1_entry);

    // Force first mode update
    mode_update(&mode_ctx, quad_get_delta(), quad_get_button());

    // Let splash screen hang around for a while
    sleep_ms(1000);
    oled_update(&mode_ctx);

    // Send default state to waveform generator
    queue_add_blocking(&signal_generator_cmd_queue, &(mode_ctx.signal));

    while(1)
    {
        if (quad_pending_turn())
        {
            mode_update(&mode_ctx, quad_get_delta(), quad_get_button());
            oled_update(&mode_ctx);

            // TODO: Do this for every turn? Probably not because it will introduce noise when e.g. they are just changing the amplitude or waveform.
            channels_update(&mode_ctx);
            quad_acknowledge_turn();

            queue_try_add(&signal_generator_cmd_queue, &(mode_ctx.signal));
        }
    }
}

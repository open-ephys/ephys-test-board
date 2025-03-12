#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

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
#include "sine.h"
}

// TODO: Create command specification
// #define STR_BUFFER_BYTES    256
// #define MAX_COMMANDS        256

void core1_entry()
{

    // DAC initialization
    pio_spi_inst_t dac_spi = {
        .pio = DAC_PIO,
        .sm = 0,
    };

    ad5683_init(&dac_spi);

    // TODO: Should we use a timer here? It seems like the best performance would actually be as below. 
    // i guess the downside would be that the updates would not be regular or at least their period will not be well known so its hard to make a lookup table work?
    //add_repeating_timer_us()

    uint i = 0;
    uint step = 2;
    while (1) {
        ad5683_write_dac(&dac_spi, SINE1024[i % 1024]);
        i += step;
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

    // Command interface
    //stdio_init_all();

    // Channels
    channels_init(&mode_ctx);


    // // JSON parser buffer initialization
    // char str[STR_BUFFER_BYTES];
    // json_t buff[MAX_COMMANDS];

    // while (1) {

    //     char *rv = fgets(str, sizeof str, stdin);

    //     if (rv == NULL) {
    //         puts("Invalid JSON command string.");
    //         continue;
    //     }

    //     // NB: this call modifies str, so dont try to use it again!
    //     json_t const *root = json_create(str, buff, sizeof buff / sizeof *buff);

    //     if (root == NULL) {
    //         puts("Invalid JSON formatting.");
    //         continue;
    //     }

    //     // Bit array (128 bits)
    //     bit_arr_t chs[BIT_ARR_LEN];

    //     int rc = parse_channels(root, chs, BIT_ARR_LEN);
    //     if (rc) {
    //         puts(parse_error(rc));
    //         continue;
    //     }

    //     sr_update(chs, BIT_ARR_LEN);
    // }

    // Launch the second core to handle the DAC
    multicore_launch_core1(core1_entry); 
  
    // Force first mode update
    mode_update(&mode_ctx, quad_get_delta(), quad_get_button());

    // Let splash screen hang around for a while
    sleep_ms(1000);
    oled_update(&mode_ctx);

    while(1)
    {
        if (quad_pending_turn())
        {
            mode_update(&mode_ctx, quad_get_delta(), quad_get_button());
            oled_update(&mode_ctx);

            // TODO: Do this for every turn? Probably not because it will introduce noise when e.g. they are just changing the amplitude or waveform. 
            channels_update(&mode_ctx);
            quad_acknowledge_turn();
        } 
    }
}

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "pico/sync.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "hardware/sync.h"
#include "hardware/gpio.h"
#include "tusb.h"

#include "device.h"
#include "usb_device.h"
#include "applets.h"
#include "apdu.h"
#include "include/local.h"
#include "include/lfs_init.h"

bool led_state = false;

void led_toggle()
{
    if(led_state)
    {
        led_off();
        led_state = false;
    }
    else
    {
        led_on();
        led_state = true;
    }
}

int main()
{
    // Initialize system
    stdio_init_all();
    init_pinout();
    
    // Initialize filesystem
    littlefs_init();

    // Initialize USB device
    usb_device_init();

    // Install applets after all required systems are initialized
    applets_install();

    // Initialize APDU buffer before applets
    init_apdu_buffer();

    // Start periodic task for button checks and LED updates
    start_periodic_task(10);

    // Start LED blinking with 500ms interval
    start_blinking_interval(0, 500);

    // Main loop
    while (true) {
        device_loop(0);
        sleep_ms(1);
    }
}

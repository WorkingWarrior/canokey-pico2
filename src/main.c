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

int main()
{
    stdio_init_all();
    init_pinout();

    littlefs_init();
    usb_device_init();
    
    applets_install();
    
    init_apdu_buffer();
    
    start_periodic_task(10);
    
    start_blinking_interval(0, 500);

    while (true) {
        device_loop(0);
        sleep_ms(1);
    }
}

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
#include "bsp/board_api.h"
#include "tusb.h"

#include "device.h"
#include "usb_device.h"
#include "applets.h"
#include "apdu.h"
#include "rand.h"
#include "include/local.h"
#include "include/lfs_init.h"

int main()
{
    board_init();
    init_pinout();
    
    littlefs_init();
    usb_device_init();

    if (board_init_after_tusb) {
        board_init_after_tusb();
    }

    applets_install();
    init_apdu_buffer();

    start_periodic_task(100);

    while (true) {
        device_loop();
    }
}

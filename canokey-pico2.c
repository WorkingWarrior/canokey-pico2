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

#include "device.h"
#include "rand.h"
#include "include/local.h"

int main()
{
    stdio_init_all();
    init_pinout();
    
    start_periodic_task(100);

    while (true) {
        device_loop();
    }
}

#include <stdint.h>
#include "device.h"

void fm_nss_low(void) {}

void fm_nss_high(void) {}

void fm_transmit(uint8_t *buf, uint8_t len) { 
    (void)buf;
    (void)len;
}

void fm_receive(uint8_t *buf, uint8_t len) { 
    (void)buf;
    (void)len;
}

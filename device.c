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
#include "include/local.h"

static void (*timeout_callback)(void) = NULL;
static struct repeating_timer timer;
static bool is_timer_running = false;

void init_pinout(void) {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void device_delay(int ms) {
    sleep_ms(ms);
}

uint32_t device_get_tick(void) {
    return to_ms_since_boot(get_absolute_time());
}

int device_atomic_compare_and_swap(volatile uint32_t *var, uint32_t expect, uint32_t update) {
    return __sync_bool_compare_and_swap(var, expect, update) ? 1 : 0;
}

int device_spinlock_lock(volatile uint32_t *lock, uint32_t blocking) {
    do {
        if (device_atomic_compare_and_swap(lock, 0, 1)) {
            return 0;
        }
        
        if (!blocking) {
            return -1;
        }
    } while (blocking);
    
    return -1;
}

void device_spinlock_unlock(volatile uint32_t *lock) {
    __sync_lock_release(lock);
}

void led_on(void) {
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
}

void led_off(void) {
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}

static void alarm_callback(uint alarm_num) {
    // Clear/cancel the alarm
    hardware_alarm_set_target(alarm_num, 0);
    
    if (timeout_callback) {
        timeout_callback();
    }
}

void device_set_timeout(void (*callback)(void), uint16_t timeout) {
    // Zapisanie callbacka
    timeout_callback = callback;
    
    // Konfiguracja alarmu
    hardware_alarm_claim(0);
    hardware_alarm_set_callback(0, alarm_callback);
    
    // Ustawienie alarmu (timeout w ms)
    absolute_time_t target = make_timeout_time_ms(timeout);
    hardware_alarm_set_target(0, target);
}

uint32_t random32(void) {
    return get_rand_32();
}

void random_buffer(uint8_t *buf, size_t len) {
    if (!buf || !len) {
        return;
    }
    
    while (len) {
        uint32_t random = get_rand_32();
        size_t chunk = (len < 4) ? len : 4;
        
        for (size_t i = 0; i < chunk; i++) {
            *buf++ = (uint8_t)(random >> (i * 8));
        }
        
        len -= chunk;
    }
}

static bool periodic_task_callback(struct repeating_timer *t) {
    device_update_led();

    return true;
}

void start_periodic_task(uint32_t period_ms) {
    if (!is_timer_running) {
        add_repeating_timer_ms(period_ms, periodic_task_callback, NULL, &timer);
        is_timer_running = true;
    }
}

void stop_periodic_task(void) {
    if (is_timer_running) {
        cancel_repeating_timer(&timer);
        is_timer_running = false;
    }
}
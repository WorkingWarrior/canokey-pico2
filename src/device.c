#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "pico/sync.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "hardware/sync.h"
#include "hardware/gpio.h"

#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
#include <tusb.h>

#include "device.h"
#include "include/local.h"

#define LONG_PRESS_TIME_MS (1000)

static absolute_time_t button_press_start;
static bool button_was_pressed = false;

static void (*timeout_callback)(void) = NULL;
static struct repeating_timer timer;
static bool is_timer_running = false;

void init_pinout(void)
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

bool __no_inline_not_in_flash_func(get_bootsel_button)(void)
{
    const uint CS_PIN_INDEX = 1;
    uint32_t flags = save_and_disable_interrupts();

    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    for (volatile int i = 0; i < 1000; ++i)
        ;

    bool button_state = (sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));

    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    restore_interrupts(flags);

    return button_state;
}

uint32_t board_button_read(void)
{
    return 0 == get_bootsel_button();
}

static void check_button_state(void)
{
    bool button_pressed = board_button_read();

    if (button_pressed && !button_was_pressed)
    {
        button_press_start = get_absolute_time();
        button_was_pressed = true;
    }
    else if (!button_pressed && button_was_pressed)
    {
        int64_t press_duration = absolute_time_diff_us(button_press_start, get_absolute_time()) / 1000;

        if (press_duration >= LONG_PRESS_TIME_MS)
        {
            set_touch_result(TOUCH_LONG);
        }
        else
        {
            set_touch_result(TOUCH_SHORT);
        }
        button_was_pressed = false;
    }
    else if (!button_pressed)
    {
        set_touch_result(TOUCH_NO);
    }
}

void device_delay(int ms) {
    uint32_t start_ms = device_get_tick();
    while(device_get_tick() - start_ms < ms) {
        tud_task();
    }
}

uint32_t device_get_tick(void)
{
    return to_ms_since_boot(get_absolute_time());
}

int device_atomic_compare_and_swap(volatile uint32_t *var, uint32_t expect, uint32_t update)
{
    return __sync_bool_compare_and_swap(var, expect, update) ? 1 : 0;
}

int device_spinlock_lock(volatile uint32_t *lock, uint32_t blocking)
{
    do
    {
        if (device_atomic_compare_and_swap(lock, 0, 1))
        {
            return 0;
        }

        if (!blocking)
        {
            return -1;
        }
    } while (blocking);

    return -1;
}

void device_spinlock_unlock(volatile uint32_t *lock)
{
    __sync_lock_release(lock);
}

void led_on(void)
{
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
}

void led_off(void)
{
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}

static void alarm_callback(uint alarm_num)
{
    hardware_alarm_set_target(alarm_num, 0);

    if (timeout_callback)
    {
        timeout_callback();
    }
}

void device_set_timeout(void (*callback)(void), uint16_t timeout)
{
    timeout_callback = callback;

    hardware_alarm_claim(0);
    hardware_alarm_set_callback(0, alarm_callback);

    absolute_time_t target = make_timeout_time_ms(timeout);
    hardware_alarm_set_target(0, target);
}

uint32_t random32(void)
{
    return get_rand_32();
}

void random_buffer(uint8_t *buf, size_t len)
{
    if (!buf || !len)
    {
        return;
    }

    while (len)
    {
        uint32_t random = get_rand_32();
        size_t chunk = (len < 4) ? len : 4;

        for (size_t i = 0; i < chunk; i++)
        {
            *buf++ = (uint8_t)(random >> (i * 8));
        }

        len -= chunk;
    }
}

static bool periodic_task_callback(struct repeating_timer *t)
{
    device_update_led();
    check_button_state();

    return true;
}

void start_periodic_task(uint32_t period_ms)
{
    if (!is_timer_running)
    {
        add_repeating_timer_ms(period_ms, periodic_task_callback, NULL, &timer);
        is_timer_running = true;
    }
}

void stop_periodic_task(void)
{
    if (is_timer_running)
    {
        cancel_repeating_timer(&timer);
        is_timer_running = false;
    }
}

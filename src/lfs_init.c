#include <fs.h>
#include "lfs.h"
#include "include/lfs_init.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "device.h"

// #define FS_SIZE (2 * 1024 * 1024)
#define FS_SIZE (0x40000)
#define FS_OFFSET (PICO_FLASH_SIZE_BYTES - FS_SIZE)
#define CACHE_SIZE (256)
#define LOOKAHEAD_SIZE (0x20)

static uint8_t read_buffer[CACHE_SIZE];
static uint8_t prog_buffer[CACHE_SIZE];
static alignas(4) uint8_t lookahead_buffer[LOOKAHEAD_SIZE];

static_assert(FS_OFFSET >= 0, "Filesystem size exceeds available flash memory");
static_assert(FS_OFFSET % FLASH_SECTOR_SIZE == 0, "FS_OFFSET must be aligned to flash sector size");

static uint32_t fs_base(const struct lfs_config *c) {
    (void)c;
    return FS_OFFSET;
}

static int pico_read(const struct lfs_config *c,
                    lfs_block_t block,
                    lfs_off_t off,
                    void *buffer,
                    lfs_size_t size) {
    if (buffer == NULL) return LFS_ERR_INVAL;
    
    uint32_t addr = fs_base(c) + (block * FLASH_SECTOR_SIZE) + off;
    if (addr + size > PICO_FLASH_SIZE_BYTES) {
        return LFS_ERR_IO;
    }
    
    uint8_t *p = (uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + addr);
    memcpy(buffer, p, size);
    return 0;
}

static int pico_prog(const struct lfs_config *c,
                    lfs_block_t block,
                    lfs_off_t off,
                    const void *buffer,
                    lfs_size_t size) {
    if (buffer == NULL) return LFS_ERR_INVAL;
    
    uint32_t addr = fs_base(c) + (block * FLASH_SECTOR_SIZE) + off;
    if (addr + size > PICO_FLASH_SIZE_BYTES) {
        return LFS_ERR_IO;
    }
    
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(addr, buffer, size);
    restore_interrupts(ints);
    return 0;
}

static int pico_erase(const struct lfs_config *c, lfs_block_t block) {
    uint32_t off = block * FLASH_SECTOR_SIZE;
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(fs_base(c) + off, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
    return 0;
}

static int pico_sync(const struct lfs_config *c) {
    (void)c;
    return 0;
}

const struct lfs_config lfs_pico_flash_config = {
    .read = pico_read,
    .prog = pico_prog,
    .erase = pico_erase,
    .sync = pico_sync,
    .read_size = 1,
    .prog_size = FLASH_PAGE_SIZE,
    .block_size = FLASH_SECTOR_SIZE,
    .block_count = FS_SIZE / FLASH_SECTOR_SIZE,
    .cache_size = CACHE_SIZE,
    .lookahead_size = LOOKAHEAD_SIZE,
    .block_cycles = 500,
    .name_max = LFS_NAME_MAX,
    .file_max = FS_SIZE,
    .attr_max = LFS_ATTR_MAX,
    .read_buffer = read_buffer,
    .prog_buffer = prog_buffer,
    .lookahead_buffer = lookahead_buffer
};

static bool try_mount(void) {
    return (fs_mount(&lfs_pico_flash_config) == MOUNT_SUCCESS);
}

static void handle_mount_failure(void) {
    for(int i = 0; i < 10; i++) {
        led_on();
        sleep_ms(100);
        led_off();
        sleep_ms(100);
    }
    watchdog_enable(1, 1);
    while(1);
}

void littlefs_init(void) {
    for(int retry = 0; retry < MAX_MOUNT_RETRIES; retry++) {
        if(try_mount()) {
            return;
        }
        sleep_ms(100);
    }

    fs_format(&lfs_pico_flash_config);
    if(!try_mount()) {
        handle_mount_failure();
    }
}

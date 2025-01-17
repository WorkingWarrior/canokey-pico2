#include <fs.h>
#include "lfs.h"
#include "include/lfs_init.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#define FS_SIZE (1.8 * 1024 * 1024)

static uint32_t fs_base(const struct lfs_config *c) {
    uint32_t storage_size = c->block_count * c->block_size;
    return PICO_FLASH_SIZE_BYTES - storage_size;
}

static int pico_read(const struct lfs_config *c,
                    lfs_block_t block,
                    lfs_off_t off,
                    void *buffer,
                    lfs_size_t size) {
    (void)c;
    uint8_t *p = (uint8_t *)(XIP_NOCACHE_NOALLOC_BASE + fs_base(c) + 
                            (block * FLASH_SECTOR_SIZE) + off);
    memcpy(buffer, p, size);
    return 0;
}

static int pico_prog(const struct lfs_config *c,
                    lfs_block_t block,
                    lfs_off_t off,
                    const void *buffer,
                    lfs_size_t size) {
    (void)c;
    uint32_t p = (block * FLASH_SECTOR_SIZE) + off;
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(fs_base(c) + p, buffer, size);
    restore_interrupts(ints);
    return 0;
}

static int pico_erase(const struct lfs_config *c, lfs_block_t block) {
    (void)c;
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
    .read           = pico_read,
    .prog           = pico_prog,
    .erase          = pico_erase,
    .sync           = pico_sync,
    .read_size      = 1,
    .prog_size      = FLASH_PAGE_SIZE,
    .block_size     = FLASH_SECTOR_SIZE,
    .block_count    = FS_SIZE / FLASH_SECTOR_SIZE,
    .cache_size     = FLASH_SECTOR_SIZE,
    .lookahead_size = 16,
    .block_cycles   = 500,
};

static bool try_mount(void) {
    return (fs_mount(&lfs_pico_flash_config) == MOUNT_SUCCESS);
}

static void handle_mount_failure(void) {
    for(;;) {
        ;
    }
}

void littlefs_init(void) {
    for(int retry = 0; retry < MAX_MOUNT_RETRIES; retry++) {
        if(try_mount()) {
            return;
        }
    }

    fs_format(&lfs_pico_flash_config);
    
    if(!try_mount()) {
        handle_mount_failure();
    }
}

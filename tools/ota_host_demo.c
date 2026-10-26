#include "ota_update.h"
#include "ota_flash_sim.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint8_t storage[4096];

int main(int argc, char **argv)
{
    uint8_t image[257];
    OtaFlashSim sim;
    OtaFlashPort port;
    OtaImageManifest manifest;
    OtaUpdate update;
    OtaUpdate resumed;
    const OtaFlashRegion active = {0U, 1024U};
    const OtaFlashRegion staging = {1024U, 1024U};
    const OtaFlashRegion journal = {2048U, 128U};
    uint32_t image_crc;
    uint16_t block_crc;
    size_t i;
    int corrupt = argc == 2 && strcmp(argv[1], "corrupt") == 0;
    int recovery = argc == 2 && strcmp(argv[1], "recovery") == 0;
    if (argc > 2 || (argc == 2 && !corrupt && !recovery &&
                     strcmp(argv[1], "normal") != 0)) {
        fprintf(stderr, "usage: ota_host_demo [normal|corrupt|recovery]\n");
        return 2;
    }
    for (i = 0U; i < sizeof image; ++i) {
        image[i] = (uint8_t)(i * 7U + 3U);
    }
    if (ota_image_crc32(image, sizeof image, &image_crc) != OTA_IMAGE_OK ||
        ota_flash_sim_init(&sim, storage, sizeof storage,
            (OtaFlashGeometry){4096U, 64U, 4U}, &port) != OTA_FLASH_OK) {
        return 3;
    }
    manifest = (OtaImageManifest){OTA_IMAGE_MAGIC, OTA_IMAGE_FORMAT_VERSION,
                                  2U, sizeof image, image_crc};
    if (ota_update_init(&update, &port, active, staging, journal,
                        &manifest, 1U, 256U, 0xFFFFU, 0U) != OTA_UPDATE_OK ||
        ota_update_begin(&update) != OTA_UPDATE_OK ||
        ota_block_crc16(image, 256U, 0xFFFFU, 0U, &block_crc) != OTA_BLOCK_OK ||
        ota_update_receive(&update, 0U, image, 256U, block_crc) != OTA_UPDATE_OK ||
        ota_block_crc16(image + 256U, 1U, 0xFFFFU, 0U, &block_crc) != OTA_BLOCK_OK ||
        ota_update_receive(&update, 256U, image + 256U, 1U,
                           block_crc) != OTA_UPDATE_COMPLETE) {
        fprintf(stderr, "receive failed\n");
        return 4;
    }
    if (corrupt) {
        storage[staging.start + 8U] ^= 1U;
        if (ota_update_verify_staging(&update) != OTA_UPDATE_IMAGE_ERROR ||
            ota_update_boot_eligible(&update)) {
            return 5;
        }
        puts("CORRUPT: staging CRC rejected; boot denied");
        return 0;
    }
    if (ota_update_verify_staging(&update) != OTA_UPDATE_OK) {
        return 6;
    }
    if (recovery) {
        sim.programmed_bytes = 0U;
        sim.fail_program_after = 40U;
        if (ota_update_apply(&update) != OTA_UPDATE_STORAGE_ERROR ||
            ota_update_boot_eligible(&update)) {
            return 7;
        }
        sim.fail_program_after = SIZE_MAX;
        if (ota_update_init(&resumed, &port, active, staging, journal,
                            &manifest, 1U, 256U, 0xFFFFU, 0U) != OTA_UPDATE_OK ||
            ota_update_resume(&resumed) != OTA_UPDATE_RECOVERY_NEEDED ||
            ota_update_boot_eligible(&resumed) ||
            ota_update_retry_apply(&resumed) != OTA_UPDATE_OK) {
            return 8;
        }
        update = resumed;
    } else if (ota_update_apply(&update) != OTA_UPDATE_OK) {
        return 9;
    }
    if (!ota_update_boot_eligible(&update) ||
        memcmp(storage + active.start, image, sizeof image) != 0) {
        return 10;
    }
    puts(recovery ? "RECOVERY: interrupted copy rejected, explicit retry applied; boot eligible (Host model)"
                  : "NORMAL: image applied and CRC verified; boot eligible (Host model)");
    return 0;
}

#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include "ota_block_validator.h"
#include "ota_flash.h"
#include "ota_image.h"

typedef enum {
    OTA_UPDATE_IDLE = 0,
    OTA_UPDATE_RECEIVING,
    OTA_UPDATE_RECEIVED,
    OTA_UPDATE_READY,
    OTA_UPDATE_APPLYING,
    OTA_UPDATE_APPLIED,
    OTA_UPDATE_INVALID_IMAGE,
    OTA_UPDATE_RECOVERY_REQUIRED
} OtaUpdateState;

typedef enum {
    OTA_UPDATE_OK = 0,
    OTA_UPDATE_COMPLETE,
    OTA_UPDATE_INVALID_ARGUMENT,
    OTA_UPDATE_INVALID_STATE,
    OTA_UPDATE_BLOCK_REJECTED,
    OTA_UPDATE_STORAGE_ERROR,
    OTA_UPDATE_IMAGE_ERROR,
    OTA_UPDATE_JOURNAL_ERROR,
    OTA_UPDATE_RECOVERY_NEEDED
} OtaUpdateResult;

typedef struct {
    OtaFlashPort *flash;
    OtaFlashRegion active;
    OtaFlashRegion staging;
    OtaFlashRegion journal;
    OtaImageManifest manifest;
    OtaBlockSession blocks;
    OtaUpdateState state;
    OtaUpdateResult last_error;
    OtaBlockResult last_block_result;
    OtaFlashResult last_flash_result;
    uint32_t journal_sequence;
    uint8_t recovery_can_apply; /* Valid READY/APPLYING lineage only. */
    uint32_t minimum_version;
    uint32_t max_block_size;
    uint16_t block_crc_initial;
    uint16_t block_crc_xorout;
} OtaUpdate;

/* Regions must be disjoint and erase-aligned; journal needs two erase units. */
OtaUpdateResult ota_update_init(OtaUpdate *update, OtaFlashPort *flash,
                                OtaFlashRegion active, OtaFlashRegion staging,
                                OtaFlashRegion journal,
                                const OtaImageManifest *manifest,
                                uint32_t minimum_version,
                                uint32_t max_block_size,
                                uint16_t block_crc_initial,
                                uint16_t block_crc_xorout);

OtaUpdateResult ota_update_begin(OtaUpdate *update);
OtaUpdateResult ota_update_receive(OtaUpdate *update, uint32_t offset,
                                    const uint8_t *data, size_t length,
                                    uint16_t expected_crc);
OtaUpdateResult ota_update_verify_staging(OtaUpdate *update);
OtaUpdateResult ota_update_apply(OtaUpdate *update);
/* Reconstruct a conservative state from the two journal slots and storage. */
OtaUpdateResult ota_update_resume(OtaUpdate *update);
/* Explicit recovery after an interrupted/failed apply; rechecks staging. */
OtaUpdateResult ota_update_retry_apply(OtaUpdate *update);
/* Host model only: CRC readback is not a target boot-vector validity check. */
int ota_update_boot_eligible(OtaUpdate *update);

#endif

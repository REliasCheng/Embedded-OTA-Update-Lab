#ifndef OTA_IMAGE_H
#define OTA_IMAGE_H

#include <stddef.h>
#include <stdint.h>

#define OTA_IMAGE_MAGIC UINT32_C(0x4F544131) /* "OTA1" as an integer tag */
#define OTA_IMAGE_FORMAT_VERSION UINT32_C(1)

typedef struct {
    uint32_t magic;
    uint32_t format_version;
    uint32_t firmware_version;
    uint32_t image_size;
    uint32_t image_crc32;
} OtaImageManifest;

typedef enum {
    OTA_IMAGE_OK = 0,
    OTA_IMAGE_INVALID_ARGUMENT,
    OTA_IMAGE_UNSUPPORTED_FORMAT,
    OTA_IMAGE_INVALID_SIZE,
    OTA_IMAGE_VERSION_REJECTED,
    OTA_IMAGE_LENGTH_MISMATCH,
    OTA_IMAGE_CRC_MISMATCH
} OtaImageResult;

/* Numeric minimum-version policy is software-only, not hardware anti-rollback. */
OtaImageResult ota_image_manifest_validate(const OtaImageManifest *manifest,
                                            uint32_t capacity,
                                            uint32_t minimum_version);

/* CRC-32/ISO-HDLC: reflected 0xEDB88320, init and xorout 0xFFFFFFFF. */
uint32_t ota_crc32_begin(void);
OtaImageResult ota_crc32_update(uint32_t *state, const uint8_t *data,
                                size_t length);
uint32_t ota_crc32_finish(uint32_t state);
OtaImageResult ota_image_crc32(const uint8_t *data, size_t length,
                               uint32_t *crc_out);

/* Exact byte count and whole-image CRC are required; no source authentication. */
OtaImageResult ota_image_verify_buffer(const OtaImageManifest *manifest,
                                        uint32_t capacity,
                                        uint32_t minimum_version,
                                        const uint8_t *data, size_t length);

#endif

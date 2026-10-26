#ifndef OTA_BLOCK_VALIDATOR_H
#define OTA_BLOCK_VALIDATOR_H

#include <stddef.h>
#include <stdint.h>

/* A policy bound for one in-memory block, independent of any wire protocol. */
#define OTA_BLOCK_MAX_SIZE UINT32_C(4096)

typedef enum {
    OTA_BLOCK_OK = 0,
    OTA_BLOCK_COMPLETE,
    OTA_BLOCK_INVALID_PARAMETER,
    OTA_BLOCK_UNINITIALIZED,
    OTA_BLOCK_INVALID_STATE,
    OTA_BLOCK_INVALID_IMAGE_SIZE,
    OTA_BLOCK_INVALID_BLOCK_LENGTH,
    OTA_BLOCK_OUT_OF_BOUNDS,
    OTA_BLOCK_OUT_OF_ORDER,
    OTA_BLOCK_DUPLICATE_OR_OVERLAP,
    OTA_BLOCK_CRC_MISMATCH,
    OTA_BLOCK_ALREADY_COMPLETE
} OtaBlockResult;

/* Caller owns this object and must not change its fields between API calls. */
typedef struct {
    uint32_t region_capacity;
    uint32_t image_size;
    uint32_t accepted_bytes;
    uint32_t max_block_size;
    uint16_t crc_initial;
    uint16_t crc_xorout;
    uint8_t initialized;
} OtaBlockSession;

/* On failure an existing session is left unchanged. Successful init resets it. */
OtaBlockResult ota_block_session_init(OtaBlockSession *session,
                                      uint32_t region_capacity,
                                      uint32_t image_size,
                                      uint32_t max_block_size,
                                      uint16_t crc_initial,
                                      uint16_t crc_xorout);

/* CRC-16 reflected polynomial 0xA001; nonempty data, at most 4096 bytes. */
OtaBlockResult ota_block_crc16(const uint8_t *data, size_t length,
                               uint16_t initial, uint16_t xorout,
                               uint16_t *crc_out);

/* Strictly sequential blocks. Only a valid block advances accepted_bytes. */
OtaBlockResult ota_block_accept(OtaBlockSession *session, uint32_t offset,
                                const uint8_t *data, size_t length,
                                uint16_t expected_crc);

/* Returns 0 for NULL, uninitialized, or invalid session state. */
int ota_block_session_is_complete(const OtaBlockSession *session);

#endif

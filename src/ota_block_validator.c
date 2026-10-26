#include "ota_block_validator.h"

static int session_is_valid(const OtaBlockSession *session)
{
    return session->initialized == 1U &&
           session->region_capacity > 0U &&
           session->image_size > 0U &&
           session->image_size <= session->region_capacity &&
           session->max_block_size > 0U &&
           session->max_block_size <= OTA_BLOCK_MAX_SIZE &&
           session->accepted_bytes <= session->image_size;
}

OtaBlockResult ota_block_session_init(OtaBlockSession *session,
                                      uint32_t region_capacity,
                                      uint32_t image_size,
                                      uint32_t max_block_size,
                                      uint16_t crc_initial,
                                      uint16_t crc_xorout)
{
    if (session == NULL) {
        return OTA_BLOCK_INVALID_PARAMETER;
    }
    if (region_capacity == 0U || image_size == 0U ||
        image_size > region_capacity) {
        return OTA_BLOCK_INVALID_IMAGE_SIZE;
    }
    if (max_block_size == 0U || max_block_size > OTA_BLOCK_MAX_SIZE) {
        return OTA_BLOCK_INVALID_BLOCK_LENGTH;
    }

    *session = (OtaBlockSession){region_capacity, image_size, 0U,
                                 max_block_size, crc_initial, crc_xorout, 1U};
    return OTA_BLOCK_OK;
}

OtaBlockResult ota_block_crc16(const uint8_t *data, size_t length,
                               uint16_t initial, uint16_t xorout,
                               uint16_t *crc_out)
{
    uint16_t crc = initial;
    size_t index;
    unsigned int bit;

    if (data == NULL || crc_out == NULL) {
        return OTA_BLOCK_INVALID_PARAMETER;
    }
    if (length == 0U || length > (size_t)OTA_BLOCK_MAX_SIZE) {
        return OTA_BLOCK_INVALID_BLOCK_LENGTH;
    }

    for (index = 0U; index < length; ++index) {
        crc = (uint16_t)(crc ^ data[index]);
        for (bit = 0U; bit < 8U; ++bit) {
            crc = (crc & 1U) != 0U
                      ? (uint16_t)((crc >> 1U) ^ UINT16_C(0xA001))
                      : (uint16_t)(crc >> 1U);
        }
    }
    *crc_out = (uint16_t)(crc ^ xorout);
    return OTA_BLOCK_OK;
}

OtaBlockResult ota_block_accept(OtaBlockSession *session, uint32_t offset,
                                const uint8_t *data, size_t length,
                                uint16_t expected_crc)
{
    uint16_t actual_crc;
    OtaBlockResult crc_result;

    if (session == NULL) {
        return OTA_BLOCK_INVALID_PARAMETER;
    }
    if (session->initialized == 0U) {
        return OTA_BLOCK_UNINITIALIZED;
    }
    if (!session_is_valid(session)) {
        return OTA_BLOCK_INVALID_STATE;
    }
    if (session->accepted_bytes == session->image_size) {
        return OTA_BLOCK_ALREADY_COMPLETE;
    }
    if (data == NULL) {
        return OTA_BLOCK_INVALID_PARAMETER;
    }
    if (length == 0U || length > (size_t)session->max_block_size) {
        return OTA_BLOCK_INVALID_BLOCK_LENGTH;
    }
    if (offset >= session->image_size) {
        return OTA_BLOCK_OUT_OF_BOUNDS;
    }
    if (offset < session->accepted_bytes) {
        return OTA_BLOCK_DUPLICATE_OR_OVERLAP;
    }
    if (offset > session->accepted_bytes) {
        return OTA_BLOCK_OUT_OF_ORDER;
    }
    /* Subtract before comparing: offset + length may overflow uint32_t. */
    /* length is now at most 4096, so converting it to uint32_t is safe. */
    if ((uint32_t)length > session->image_size - offset ||
        (uint32_t)length > session->region_capacity - offset) {
        return OTA_BLOCK_OUT_OF_BOUNDS;
    }

    crc_result = ota_block_crc16(data, length, session->crc_initial,
                                 session->crc_xorout, &actual_crc);
    if (crc_result != OTA_BLOCK_OK) {
        return crc_result;
    }
    if (actual_crc != expected_crc) {
        return OTA_BLOCK_CRC_MISMATCH;
    }

    session->accepted_bytes += (uint32_t)length;
    return session->accepted_bytes == session->image_size
               ? OTA_BLOCK_COMPLETE : OTA_BLOCK_OK;
}

int ota_block_session_is_complete(const OtaBlockSession *session)
{
    return session != NULL && session_is_valid(session) &&
           session->accepted_bytes == session->image_size;
}
